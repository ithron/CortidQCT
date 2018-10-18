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

#include "DiscreteRangeDecorators.h"
#include "DisplacementOptimizer.h"
#include "EigenAdaptors.h"
#include "MeshHelpers.h"
#include "Sampler.h"
#include "WeightedARAPFitter.h"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/KroneckerProduct>
#include <Eigen/Sparse>
#include <gsl/gsl>

#include <fstream>

namespace CortidQCT {

using namespace Internal;

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

MeshFitter::Result MeshFitter::Impl::fit(VoxelVolume const &volume) {
  using Eigen::Dynamic;
  using Eigen::Index;
  using Eigen::Map;
  using Eigen::Matrix;
  using Eigen::MatrixXd;
  using Eigen::Vector3f;
  using Eigen::VectorXd;
  using Eigen::VectorXf;
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

  VectorXf γ;
  VectorXf optimalDisplacements;

  auto meshFitter =
      WeightedARAPFitter<float>(V, F, static_cast<float>(conf.sigmaE));

  auto converged = false;
  auto iterations = std::size_t{0};

  while (!converged) {
    ++iterations;
    NormalMatrix<> const N = perVertexNormalMatrix(V, F);

    // sample volume
    auto const samplingPositions = samplingPoints(V, N, model);
    volumeSampler(samplingPositions, volumeSamples);

    // find optimal displacements
    std::tie(optimalDisplacements, γ) =
        displacementOptimizer(N, labels, volumeSamples);

    // Copmute dispaced vertices
    VertexMatrix<> const Y =
        V - (N.array().colwise() * optimalDisplacements.array()).matrix();

    V = meshFitter.fit(Y, N, γ);

    // std::ofstream{"V" + std::to_string(iterations) + ".mat"} << V;

    converged = iterations >= conf.maxIterations;
    std::cout << "Converged after iteration " << iterations << ": "
              << std::boolalpha << converged << std::endl;
  }

  auto resultMesh = conf.referenceMesh;

  resultMesh.withUnsafeVertexPointer([&V](auto *vPtr) {
    Map<Matrix<float, 3, Dynamic>>{vPtr, 3, V.rows()} = V.transpose();
  });

  return MeshFitter::Result{resultMesh};
}

} // namespace CortidQCT
