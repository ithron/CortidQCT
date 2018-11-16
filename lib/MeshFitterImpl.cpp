/**
 * @file      MeshFitterImpl.cpp
 *
 * @brief     Implementation file for MeshFitter::Impl
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "MeshFitterImpl.h"

#include "CommonMath.h"
#include "DiscreteRangeDecorators.h"
#include "DisplacementOptimizer.h"
#include "EigenAdaptors.h"
#include "MeshAdaptors.h"
#include "MeshFitterHiddenState.h"
#include "MeshHelpers.h"
#include "Sampler.h"
#include "WeightedARAPFitter.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <gsl/gsl>

namespace CortidQCT {

using namespace Internal;

/**********************
 * Helper Functions
 */

// MARK: -
// MARK: Helper Function

/**
 * @brief Returns a matrix containing positions to sample a voxel volume at
 *
 * Let \f$N := |V|\f$ and let \f$M := |R|\f$, where \f$R\f$ represent
 * `model.samplingRange`, then the returned NMx3 matrix contains N*M positions,
 * where each M consecutive postitions represent a line through a vertex in `V`
 * sampled along the surface normal in `N`.
 *
 * @param V Nx3 matrix with vertex positions
 * @param N Nx3 matrix of per-vertex surface normals
 * @param model MeasurementModel instance
 * @return NMx3 matrix containing sampling positions
 */
template <class DerivedV, class DerivedN>
Eigen::Matrix<typename DerivedV::Scalar, Eigen::Dynamic, 3>
samplingPoints(Eigen::MatrixBase<DerivedV> const &V,
               Eigen::MatrixBase<DerivedN> const &N,
               MeasurementModel const &model) {
  using Scalar = typename DerivedV::Scalar;
  using Eigen::Dynamic;
  using Eigen::Index;
  using Eigen::Matrix;
  using gsl::narrow_cast;

  auto const t = Internal::discreteRangeElementVector(model.samplingRange);
  Matrix<Scalar, Dynamic, 3> samples(V.rows() * t.rows(), 3);

  for (auto i = 0; i < V.rows(); ++i) {
    auto const iStart = i * t.rows();
    samples.block(iStart, 0, t.rows(), 3).colwise() = -t;
    samples.block(iStart, 0, t.rows(), 3).array().rowwise() *= N.row(i).array();
    samples.block(iStart, 0, t.rows(), 3).rowwise() += V.row(i);
  }

  return samples;
}

/***********************************
 * MeshFitter::Impl Implementation
 */

// MARK: -
// MARK: MeshFitter::Impl Implementation

MeshFitter::Result MeshFitter::Impl::fit(VoxelVolume const &volume) const {

  auto state = init(volume);

  VertexMatrix<float> Vlast =
      Adaptor::vertexMap(state.deformedMesh).transpose();

  while (!state.converged) {

    fitOneIteration(state);

    auto V = Adaptor::vertexMap(state.deformedMesh);

    auto const diff = (V.transpose() - Vlast).norm() / V.norm();
    Vlast = V.transpose();

    auto disNorm = Adaptor::map(state.displacementVector).norm() /
                   static_cast<float>(state.displacementVector.size());

    std::cout << "Converged after iteration " << state.iteration << ": "
              << std::boolalpha << state.converged << " (" << diff << " | "
              << disNorm << " | " << state.nonDecreasing << ")" << std::endl;
  }

  state.success = true;

  return state;
}

MeshFitter::State MeshFitter::Impl::init(VoxelVolume const &volume) const {
  using Eigen::Dynamic;
  using Eigen::Index;
  using Eigen::Map;
  using Eigen::Matrix;
  using Eigen::MatrixXf;
  using Eigen::Vector3f;
  using Eigen::VectorXf;
  using gsl::narrow_cast;

  MeshFitter::State state;

  auto const &conf = fitter_.configuration;
  // auto const &model = fitter_.configuration.model;

  // Init result state
  state.referenceMesh = conf.referenceMesh;

  auto const nVertices = narrow_cast<Index>(conf.referenceMesh.vertexCount());

  // Apply initial transofrmation on the vertices of the reference mesh
  auto const translation = Adaptor::vec(conf.meshTranslation(volume));
  Vector3f const rotInRad = Adaptor::vec(conf.referenceMeshRotation) *
                            static_cast<float>(M_PI) / 180.f;
  auto V0 = Adaptor::vertexMap(state.referenceMesh);

  // Apply transformation to vertices of reference mesh
  V0 = ((Eigen::Translation<float, 3>{translation} *
         Eigen::AngleAxisf(rotInRad[0], Vector3f::UnitX()) *
         Eigen::AngleAxisf(rotInRad[1], Vector3f::UnitY()) *
         Eigen::AngleAxisf(rotInRad[2], Vector3f::UnitZ())) *
        Adaptor::vec(conf.referenceMeshScale).asDiagonal() * V0);

  // Init deformed mesh with reference mesh
  state.deformedMesh = state.referenceMesh;

  // Init displacement vector
  state.displacementVector =
      std::vector<float>(narrow_cast<std::size_t>(nVertices), .0f);

  // Init weight vector
  state.weights = std::vector<float>(narrow_cast<std::size_t>(nVertices),
                                     1 / static_cast<float>(nVertices));

  // Init vertex normals
  state.vertexNormals.resize(narrow_cast<std::size_t>(nVertices));

  // Init hidden state
  state.hiddenState_ = std::make_unique<State::HiddenState>(
      volume, DisplacementOptimizer{conf},
      WeightedARAPFitter<float>{V0.transpose(), facetMatrix(conf.referenceMesh),
                                narrow_cast<float>(conf.sigmaE)},
      facetMatrix(conf.referenceMesh));

  // Init volume sampling positions
  auto const nSamples = conf.model.samplingRange.numElements() *
                        narrow_cast<std::size_t>(nVertices);
  state.volumeSamplingPositions.resize(nSamples);

  // Init volume samples
  state.volumeSamples.resize(nSamples);

  sampleVolume(state);

  return state;
} // namespace CortidQCT

void MeshFitter::Impl::fitOneIteration(MeshFitter::State &state) const {

  if (state.hiddenState_ == nullptr) {
    throw std::invalid_argument("Invalid state argument, call init() first!");
  }

  findOptimalDisplacements(state);
  findOptimalDeformation(state);
  sampleVolume(state);
  computeLogLikelihood(state);
  checkConvergence(state);

  ++state.iteration;
}

void MeshFitter::Impl::findOptimalDisplacements(
    MeshFitter::State &state) const {
  if (state.hiddenState_ == nullptr) {
    throw std::invalid_argument("Invalid state argument, call init() first!");
  }

  // Get maps to state variables
  auto const labels = Adaptor::labelMap(state.referenceMesh);
  auto const N = Adaptor::map(state.vertexNormals);
  auto const volumeSamples = Adaptor::map(state.volumeSamples);
  auto optimalDisplacements = Adaptor::map(state.displacementVector);
  auto gamma = Adaptor::map(state.weights);

  // Find optimal displacements
  std::tie(optimalDisplacements, gamma) =
      state.hiddenState_->displacementOptimizer(
          N.transpose(), labels, volumeSamples, state.nonDecreasing,
          state.effectiveSigmaS);
}

void MeshFitter::Impl::findOptimalDeformation(MeshFitter::State &state) const {
  if (state.hiddenState_ == nullptr) {
    throw std::invalid_argument("Invalid state argument, call init() first!");
  }

  auto const N = Adaptor::map(state.vertexNormals);
  auto const optimalDisplacements = Adaptor::map(state.displacementVector);
  auto const gamma = Adaptor::map(state.weights);
  auto V = Adaptor::vertexMap(state.deformedMesh);

  VertexMatrix<> const Y =
      (V - (N.array().rowwise() * optimalDisplacements.array().transpose())
               .matrix())
          .transpose();

  // Fit mesh
  V = state.hiddenState_->meshFitter.fit(Y, N.transpose(), gamma).transpose();
}

void MeshFitter::Impl::sampleVolume(MeshFitter::State &state) const {

  using Eigen::Map;
  using Eigen::MatrixXf;
  using Eigen::VectorXf;

  if (state.hiddenState_ == nullptr) {
    throw std::invalid_argument("Invalid state argument, call init() first!");
  }

  auto const &conf = fitter_.configuration;
  auto const V = Adaptor::vertexMap(state.deformedMesh);
  auto const nVertices = V.cols();
  auto N = Adaptor::map(state.vertexNormals);
  auto volumeSamplingPositions = Adaptor::map(state.volumeSamplingPositions);
  auto volumeSamples = Adaptor::map(state.volumeSamples);

  // Compute normals
  N = perVertexNormalMatrix(V.transpose(), state.hiddenState_->F).transpose();

  // Copmute new sampling positions
  volumeSamplingPositions =
      samplingPoints(V.transpose(), N.transpose(), conf.model).transpose();

  // Sample the volume
  auto const volumeSampler = VolumeSampler{state.hiddenState_->volume};
  volumeSampler(volumeSamplingPositions.transpose(), volumeSamples);

  // Reorder samples
  state.hiddenState_->volumeSamplesMatrix =
      Map<MatrixXf const>{volumeSamples.data(),
                          volumeSamples.rows() / nVertices, nVertices}
          .transpose();

  volumeSamples = Map<VectorXf const>{
      state.hiddenState_->volumeSamplesMatrix.data(), volumeSamples.rows()};
}

void MeshFitter::Impl::computeLogLikelihood(MeshFitter::State &state) const {
  using Eigen::Map;
  using Eigen::VectorXf;
  using gsl::narrow_cast;

  if (state.hiddenState_ == nullptr) {
    throw std::invalid_argument("Invalid state argument, call init() first!");
  }

  auto const labels = Adaptor::labelMap(state.referenceMesh);
  auto const N = Adaptor::map(state.vertexNormals);
  auto const volumeSamples = Adaptor::map(state.volumeSamples);

  auto const llVec =
      state.hiddenState_->displacementOptimizer.logLikelihoodVector(
          N.transpose(), labels, volumeSamples);
  auto const LL = llVec.sum();

  state.logLikelihood = LL;
  state.perVertexLogLikelihood.resize(narrow_cast<std::size_t>(llVec.size()));

  Map<VectorXf>{state.perVertexLogLikelihood.data(), llVec.rows(), 1} = llVec;
}

void MeshFitter::Impl::checkConvergence(MeshFitter::State &state) const {
  if (state.hiddenState_ == nullptr) {
    throw std::invalid_argument("Invalid state argument, call init() first!");
  }

  auto const optimalDisplacements = Adaptor::map(state.displacementVector);
  auto const &conf = fitter_.configuration;

  state.converged = state.iteration >= conf.maxIterations ||
                    (optimalDisplacements.norm() < 1e-3f);

  auto disNorm = optimalDisplacements.norm() / optimalDisplacements.rows();
  if (disNorm < state.minDisNorm) {
    state.minDisNorm = disNorm;
    state.nonDecreasing = 0;
  } else {
    ++state.nonDecreasing;
  }
}

} // namespace CortidQCT
