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

#include <Eigen/Core>
#include <gsl/gsl>

namespace CortidQCT {

namespace {

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

  Matrix<Scalar, Dynamic, 1> t(
      narrow_cast<Index>(model.samplingRange.numElements()));
  Matrix<Scalar, Dynamic, 3> samples(V.rows() * t.rows(), 3);

  for (auto i = 0; i < t.rows(); ++i) {
    t(i) = model.samplingRange.min +
           static_cast<Scalar>(i) * model.samplingRange.stride;
  }

  for (auto i = 0; i < V.rows(); ++i) {
    auto const iStart = i * t.rows();
    samples.block(iStart, 0, t.rows(), 3).colwise() = t;
    samples.block(iStart, 0, t.rows(), 3).array().rowwise() *= N.row(i).array();
    samples.block(iStart, 0, t.rows(), 3).rowwise() += V.row(i);
  }

  return samples;
}

} // anonymous namespace

MeshFitter::Result MeshFitter::Impl::fit(VoxelVolume const &volume) {
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

  // Preparation step: Convert meshes into IGL compatible formats
  // Also tranlate the reference mesh according to the configured origin
  auto const translation = Adaptor::vec(conf.meshTranslation(volume));
  VertexMatrix<> const V0 =
      vertexMatrix(conf.referenceMesh).rowwise() + translation.transpose();
  FacetMatrix const F = facetMatrix(conf.referenceMesh);

  LaplacianMatrix<> const L = laplacianMatrix(V0, F);

  // 2. set deformed mesh = reference mesh
  VertexMatrix<> V = V0; // NOLINT

  auto converged = false;

  while (!converged) {
    NormalMatrix<> const N = perVertexNormalMatrix(V, F);

    auto const samples = samplingPoints(V, N, model);
  }

  return MeshFitter::Result{};
}

} // namespace CortidQCT
