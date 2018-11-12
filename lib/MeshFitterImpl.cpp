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
#include "MeshHelpers.h"
#include "Sampler.h"
#include "WeightedARAPFitter.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <gsl/gsl>

namespace CortidQCT {

using namespace Internal;

struct MeshFitter::HiddenState {
  VoxelVolume volume;
  DisplacementOptimizer displacementOptimizer;
  WeightedARAPFitter<float> meshFitter;
  FacetMatrix F;
  Eigen::MatrixXf volumeSamplesMatrix;

  HiddenState(VoxelVolume const &v, DisplacementOptimizer const &opt,
              WeightedARAPFitter<float> const &fitter, FacetMatrix const &f)
      : volume{v}, displacementOptimizer{opt}, meshFitter{fitter}, F{f} {}
};

/**************
 * MeshFitter::State implementations
 */
// MARK: -
// MARK: MeshFitter::State

MeshFitter::State::State(State const &rhs)
    : result{rhs.result}, hiddenState{std::make_unique<HiddenState>(
                              *rhs.hiddenState)} {}

MeshFitter::State::~State() = default;

MeshFitter::State &MeshFitter::State::operator=(State const &rhs) {
  result = rhs.result;
  hiddenState = std::make_unique<HiddenState>(*rhs.hiddenState);

  return *this;
}

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

MeshFitter::Result MeshFitter::Impl::fit(VoxelVolume const &volume) {
  using Eigen::Dynamic;
  using Eigen::Index;
  using Eigen::Map;
  using Eigen::Matrix;
  using Eigen::MatrixXd;
  using Eigen::MatrixXf;
  using Eigen::Vector3f;
  using Eigen::VectorXd;
  using Eigen::VectorXf;
  using gsl::narrow_cast;

  // Step:
  // 1. Move reference mesh to center
  // 2. Set deformedMesh = refMesh
  // 3. Compute normals of deformedMesh
  // 4. Sample volume along lines through the vertices of refMesh along
  // surface
  //    normals
  // 5. For every displacement s in [-s0, +s0] compute the log likelihood
  // using
  //    trilinear sampling of the measurement PDF
  // 6. Find optimal displacements
  // 7. Copmute target vertex positions using optimal displacements
  // 8. Fit mesh to target vertices
  // 9. Check for convergence, if not converged go to 3.

  auto const &conf = fitter_.configuration;
  auto const &model = fitter_.configuration.model;
  auto const volumeSampler = VolumeSampler{volume};

  auto displacementOptimizer = Internal::DisplacementOptimizer{conf};

  // Preparation step: Convert meshes into IGL compatible formats
  // Also tranlate the reference mesh according to the configured origin
  auto const translation = Adaptor::vec(conf.meshTranslation(volume));
  Vector3f const rotInRad = Adaptor::vec(conf.referenceMeshRotation) *
                            static_cast<float>(M_PI) / 180.f;
  VertexMatrix<> const V0 =
      ((Eigen::Translation<float, 3>{translation} *
        Eigen::AngleAxisf(rotInRad[0], Vector3f::UnitX()) *
        Eigen::AngleAxisf(rotInRad[1], Vector3f::UnitY()) *
        Eigen::AngleAxisf(rotInRad[2], Vector3f::UnitZ())) *
       Adaptor::vec(conf.referenceMeshScale).asDiagonal() *
       vertexMatrix(conf.referenceMesh).transpose())
          .transpose();
  FacetMatrix const F = facetMatrix(conf.referenceMesh);
  LabelVector const labels = labelVector(conf.referenceMesh);
  LaplacianMatrix<double> const L =
      laplacianMatrix(V0.template cast<double>(), F);

  // 2. set deformed mesh = reference mesh
  VertexMatrix<> V = V0;

  // Pre-allocate vector for volume samples
  VectorXf volumeSamples(V.rows() *
                         gsl::narrow<Index>(model.samplingRange.numElements()));
  MatrixXf volumeSamplesMatrix(
      gsl::narrow<Index>(model.samplingRange.numElements()), V.rows());

  VectorXf gamma;
  VectorXf optimalDisplacements;

  auto meshFitter =
      WeightedARAPFitter<float>(V, F, static_cast<float>(conf.sigmaE));

  auto converged = false;
  auto iterations = std::size_t{0};
  auto nonDecreasing = std::size_t{0};
  auto minDisNorm = std::numeric_limits<float>::max();

  auto Vlast = V;

  MeshFitter::Result result;

  while (!converged) {
    ++iterations;
    NormalMatrix<> const N = perVertexNormalMatrix(V, F);

    // sample volume
    auto const samplingPositions = samplingPoints(V, N, model);
    volumeSampler(samplingPositions, volumeSamples);

    // Reoder volume samples
    volumeSamplesMatrix =
        Map<MatrixXf const>{volumeSamples.data(),
                            volumeSamples.rows() / V.rows(), V.rows()}
            .transpose();
    volumeSamples = Map<VectorXf const>{volumeSamplesMatrix.data(),
                                        volumeSamples.rows(), 1};

    // find optimal displacements
    std::tie(optimalDisplacements, gamma) =
        displacementOptimizer(N, labels, volumeSamples, nonDecreasing);

    auto disNorm = optimalDisplacements.norm() / V.rows();
    if (disNorm < minDisNorm) {
      minDisNorm = disNorm;
      nonDecreasing = 0;
    } else {
      ++nonDecreasing;
    }

    // Copmute dispaced vertices
    VertexMatrix<> const Y =
        V - (N.array().colwise() * optimalDisplacements.array()).matrix();

    V = meshFitter.fit(Y, N, gamma);

    // std::ofstream{"V" + std::to_string(iterations) + ".mat"} << V;

    auto const diff = (V - Vlast).norm() / V.norm();
    Vlast = V;

    converged = iterations >= conf.maxIterations ||
                (optimalDisplacements.norm() < 1e-3f);
    std::cout << "Converged after iteration " << iterations << ": "
              << std::boolalpha << converged << " (" << diff << " | " << disNorm
              << " | " << nonDecreasing << ")" << std::endl;

    if (converged) {
      auto resultMesh = conf.referenceMesh;

      resultMesh.withUnsafeVertexPointer([&V](auto *vPtr) {
        Map<Matrix<float, 3, Dynamic>>{vPtr, 3, V.rows()} = V.transpose();
      });
      result.deformedMesh = resultMesh;

      // Copy displacements
      result.displacementVector = std::vector<float>(
          narrow_cast<std::size_t>(optimalDisplacements.size()));
      Map<VectorXf>{result.displacementVector.data(),
                    optimalDisplacements.size()} = optimalDisplacements;

      result.weights =
          std::vector<float>(narrow_cast<std::size_t>(gamma.size()));
      Map<VectorXf>{result.weights.data(), gamma.size()} = gamma;

      result.vertexNormals =
          std::vector<std::array<float, 3>>(narrow_cast<std::size_t>(N.rows()));
      Map<Matrix<float, 3, Dynamic>>{
          reinterpret_cast<float *>(result.vertexNormals.data()), 3, N.rows()} =
          N.transpose();

      result.iteration = iterations;
      result.converged = converged;
    }
  }

  result.success = true;

  return result;
}

MeshFitter::State MeshFitter::Impl::init(VoxelVolume const &volume) {
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
  state.result.referenceMesh = conf.referenceMesh;

  auto const nVertices = narrow_cast<Index>(conf.referenceMesh.vertexCount());

  // Apply initial transofrmation on the vertices of the reference mesh
  auto const translation = Adaptor::vec(conf.meshTranslation(volume));
  Vector3f const rotInRad = Adaptor::vec(conf.referenceMeshRotation) *
                            static_cast<float>(M_PI) / 180.f;
  auto V0 = Adaptor::vertexMap(state.result.referenceMesh);

  // Apply transformation to vertices of reference mesh
  V0 = ((Eigen::Translation<float, 3>{translation} *
         Eigen::AngleAxisf(rotInRad[0], Vector3f::UnitX()) *
         Eigen::AngleAxisf(rotInRad[1], Vector3f::UnitY()) *
         Eigen::AngleAxisf(rotInRad[2], Vector3f::UnitZ())) *
        Adaptor::vec(conf.referenceMeshScale).asDiagonal() * V0);

  // Init deformed mesh with reference mesh
  state.result.deformedMesh = state.result.referenceMesh;

  // Init displacement vector
  state.result.displacementVector =
      std::vector<float>(narrow_cast<std::size_t>(nVertices), .0f);

  // Init weight vector
  state.result.weights = std::vector<float>(narrow_cast<std::size_t>(nVertices),
                                            1 / static_cast<float>(nVertices));

  // Init vertex normals
  state.result.vertexNormals.resize(narrow_cast<std::size_t>(nVertices));
  auto N = Adaptor::map(state.result.vertexNormals);
  N = perVertexNormalMatrix(conf.referenceMesh).transpose();

  // Init hidden state
  state.hiddenState = std::make_unique<HiddenState>(
      volume, DisplacementOptimizer{conf},
      WeightedARAPFitter<float>{V0.transpose(), facetMatrix(conf.referenceMesh),
                                narrow_cast<float>(conf.sigmaE)},
      facetMatrix(conf.referenceMesh));

  // Init volume sampling positions
  auto const positions =
      samplingPoints(V0.transpose(), N.transpose(), conf.model);
  state.result.volumeSamplingPositions.resize(
      narrow_cast<std::size_t>(positions.rows()));
  auto samplingPositions = Adaptor::map(state.result.volumeSamplingPositions);
  samplingPositions = positions.transpose();

  // Init volume samples
  auto const volumeSampler = VolumeSampler(state.hiddenState->volume);
  state.result.volumeSamples.resize(
      narrow_cast<std::size_t>(samplingPositions.cols()));
  auto volumeSamples = Adaptor::map(state.result.volumeSamples);
  volumeSampler(samplingPositions.transpose(), volumeSamples);

  // Reserve memory for volume sample matrix
  state.hiddenState->volumeSamplesMatrix =
      Map<MatrixXf const>{volumeSamples.data(),
                          volumeSamples.rows() / nVertices, nVertices}
          .transpose();
  // reorder volume samples
  volumeSamples = Map<VectorXf const>{
      state.hiddenState->volumeSamplesMatrix.data(), volumeSamples.rows()};

  return state;
} // namespace CortidQCT

void MeshFitter::Impl::fitOneIteration(MeshFitter::State &state) {
  using Eigen::Dynamic;
  using Eigen::Index;
  using Eigen::Map;
  using Eigen::Matrix;
  using Eigen::MatrixXf;
  using Eigen::VectorXf;
  using Eigen::VectorXi;
  using gsl::narrow_cast;

  if (state.hiddenState == nullptr) {
    throw std::invalid_argument("Invalid state argument, call init() first!");
  }

  auto const &conf = fitter_.configuration;

  auto const nVertices =
      narrow_cast<Index>(state.result.referenceMesh.vertexCount());

  // Get maps to state variables
  auto V = Adaptor::vertexMap(state.result.deformedMesh);

  auto const labels = Adaptor::labelMap(state.result.referenceMesh);

  auto N = Adaptor::map(state.result.vertexNormals);
  auto optimalDisplacements = Adaptor::map(state.result.displacementVector);
  auto gamma = Adaptor::map(state.result.weights);
  auto volumeSamplingPositions =
      Adaptor::map(state.result.volumeSamplingPositions);
  auto volumeSamples = Adaptor::map(state.result.volumeSamples);

  // Find optimal displacements
  std::tie(optimalDisplacements, gamma) =
      state.hiddenState->displacementOptimizer(
          N.transpose(), labels, volumeSamples, state.result.nonDecreasing);

  auto disNorm = optimalDisplacements.norm() / V.rows();
  if (disNorm < state.result.minDisNorm) {
    state.result.minDisNorm = disNorm;
    state.result.nonDecreasing = 0;
  } else {
    ++state.result.nonDecreasing;
  }

  // Copmute dispaced vertices
  VertexMatrix<> const Y =
      (V - (N.array().rowwise() * optimalDisplacements.array().transpose())
               .matrix())
          .transpose();

  // Fit mesh
  VertexMatrix<float> const Vnew =
      state.hiddenState->meshFitter.fit(Y, N.transpose(), gamma);

  V = Vnew.transpose();

  state.result.converged = state.result.iteration >= conf.maxIterations ||
                           (optimalDisplacements.norm() < 1e-3f);

  // Compute normals
  N = perVertexNormalMatrix(V.transpose(), state.hiddenState->F).transpose();

  // Copmute new sampling positions
  volumeSamplingPositions =
      samplingPoints(V.transpose(), N.transpose(), conf.model).transpose();

  // Sample the volume
  auto const volumeSampler = VolumeSampler{state.hiddenState->volume};
  volumeSampler(volumeSamplingPositions.transpose(), volumeSamples);

  // Reorder samples
  state.hiddenState->volumeSamplesMatrix =
      Map<MatrixXf const>{volumeSamples.data(),
                          volumeSamples.rows() / nVertices, nVertices}
          .transpose();

  volumeSamples = Map<VectorXf const>{
      state.hiddenState->volumeSamplesMatrix.data(), volumeSamples.rows()};
}

} // namespace CortidQCT
