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

#include "EigenAdaptors.h"
#include "MeshHelpers.h"
#include "Sampler.h"

#include <Eigen/Core>
#include <gsl/gsl>

namespace CortidQCT {

namespace {

template <class T> inline auto square(T &&x) noexcept(noexcept(x *x)) {
  return x * x;
}

/**
 * @brief Returns all element of the given discrete range as a vector
 * @param range DiscreteRange object
 * @return An Eigen vector containing all element of `range`
 */
template <class T>
Eigen::Matrix<T, Eigen::Dynamic, 1>
discreteRanteElementVector(DiscreteRange<T> const &range) {
  using Vector = Eigen::Matrix<T, Eigen::Dynamic, 1>;

  Vector t(gsl::narrow<Eigen::Index>(range.numElements()));

  for (auto i = 0; i < t.rows(); ++i) {
    t(i) = range.min + static_cast<T>(i) * range.stride;
  }

  return t;
}

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

  auto const t = discreteRanteElementVector(model.samplingRange);
  Matrix<Scalar, Dynamic, 3> samples(V.rows() * t.rows(), 3);

  for (auto i = 0; i < V.rows(); ++i) {
    auto const iStart = i * t.rows();
    samples.block(iStart, 0, t.rows(), 3).colwise() = t;
    samples.block(iStart, 0, t.rows(), 3).array().rowwise() *= N.row(i).array();
    samples.block(iStart, 0, t.rows(), 3).rowwise() += V.row(i);
  }

  return samples;
}

/// @brief Convert per vertex normals to angles with z-axis in degrees
template <class DerivedN>
Eigen::VectorXf normalsToAngles(Eigen::MatrixBase<DerivedN> const &N) {
  Expects(N.rows() > 0);
  Expects(N.cols() == 3);
  Eigen::VectorXf const angles =
      (1.f - N.col(2).array()).abs() * static_cast<float>(M_PI) / 2.f;

  return angles;
}

/**
 * @brief Updates the given model sampling positions matrix with new values
 *
 * Updates sampling position based on offset, angles based on normals and
 * densities.
 *
 * @param model MeasurementModel instance
 * @param N per-vertex normal matrix
 * @param densities densities sampled from input volume along lines through
 * vertices along normals from `N`
 * @param offset offset to add to the sampling positions
 * @param positionsOut position matrix to update
 */
template <class DerivedN, class DerivedD, class DerivedOut>
void updateModelSamplingPositions(MeasurementModel const &model,
                                  Eigen::MatrixBase<DerivedN> const &N,
                                  Eigen::MatrixBase<DerivedD> const &densities,
                                  float offset,
                                  Eigen::MatrixBase<DerivedOut> &positionsOut) {
  using Eigen::VectorXf;

  Expects(N.cols() == 3);
  Expects(N.rows() > 0);
  Expects(densities.rows() % N.rows() == 0);
  Expects(positionsOut.rows() == densities.rows());
  Expects(positionsOut.cols() == 3);

  auto const numVertices = N.rows();
  auto const numSamples = densities.rows() / N.rows();

  VectorXf const t =
      (discreteRanteElementVector(model.samplingRange).array() + offset)
          .matrix();

  auto const angles = normalsToAngles(N);

  positionsOut.col(0) = t.replicate(numVertices, 1);
  positionsOut.col(1) = densities;
  for (auto i = 0; i < numVertices; ++i) {
    positionsOut.col(2).segment(numSamples * i, numSamples).array() =
        angles.array();
  }
}

} // anonymous namespace

MeshFitter::Result MeshFitter::Impl::fit(VoxelVolume const &volume) {
  using Eigen::Dynamic;
  using Eigen::Matrix;
  using Eigen::MatrixXd;
  using Eigen::VectorXf;
  // Step:
  // 1. Move reference mesh to center
  // 2. Set deformedMesh = refMesh
  // 3. Compute normals of deformedMesh
  // 4. Sample volume along lines through the vertices of refMesh along surface
  //    normals
  // 5. For every displacement s in [-s0, +s0] compute the log likelihood using
  //    trilinear sampling of the measurement PDF
  // 6. Find optimal displacements
  // 7. Copmute target vertex positions using optimal displacements
  // 8. Fit mesh to target vertices
  // 9. Check for convergence, if not converged go to 3.

  auto const &conf = fitter_.configuration;
  auto const &model = fitter_.configuration.model;
  auto const modelSampler = ModelSampler{model};
  auto const volumeSampler = VolumeSampler{volume};
  auto const displacementRange = DiscreteRange<float>{
      model.samplingRange.min * 2.f, model.samplingRange.max * 2.f,
      model.samplingRange.stride};
  auto const displacementVector = discreteRanteElementVector(displacementRange);

  // Preparation step: Convert meshes into IGL compatible formats
  // Also tranlate the reference mesh according to the configured origin
  auto const translation = Adaptor::vec(conf.meshTranslation(volume));
  VertexMatrix<> const V0 =
      vertexMatrix(conf.referenceMesh).rowwise() + translation.transpose();
  FacetMatrix const F = facetMatrix(conf.referenceMesh);
  LabelVector const labels = labelVector(conf.referenceMesh);

  LabelVector const repLabels = labels.replicate(
      gsl::narrow<Eigen::Index>(model.samplingRange.numElements()), 1);

  LaplacianMatrix<> const L = laplacianMatrix(V0, F);

  // 2. set deformed mesh = reference mesh
  VertexMatrix<> V = V0; // NOLINT

  // Pre-allocate vector for volume samples
  VectorXf volumeSamples(
      V.rows() * gsl::narrow<Eigen::Index>(model.samplingRange.numElements()));

  // Pre-allocate matrix of measurement model sampling positions
  Matrix<float, Dynamic, 3> modelSamplingPositions(
      V.rows() * gsl::narrow<Eigen::Index>(model.samplingRange.numElements()),
      3);

  // Pre-allocate matrix for observation likelihood given displacement
  MatrixXd Lzs(V.rows(), displacementVector.rows());

  auto converged = false;

  while (!converged) {
    NormalMatrix<> const N = perVertexNormalMatrix(V, F);

    // sample volume
    auto const samplingPositions = samplingPoints(V, N, model);
    volumeSampler(samplingPositions, volumeSamples);

    // update model sampling positions
    updateModelSamplingPositions(model, N, volumeSamples, .0f,
                                 modelSamplingPositions);

    // Compute observation log likelihood for each displacement
#pragma omp parallel for
    for (auto i = 0; i < displacementVector.rows(); ++i) {
      modelSampler(modelSamplingPositions, displacementVector(i), repLabels,
                   Lzs.col(i));
    }
  }

  return MeshFitter::Result{};
}

} // namespace CortidQCT
