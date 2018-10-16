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

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/KroneckerProduct>
#include <Eigen/Sparse>
#include <gsl/gsl>

#include <fstream>

namespace CortidQCT {

namespace {

template <class T> constexpr inline auto square(T &&x) noexcept {
  return x * x;
}

} // anonymous namespace

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

template <class DerivedV, class DerivedN, class DerivedY, class DerivedG>
VertexMatrix<typename DerivedV::Scalar>
weightedARAPDeformation(Eigen::MatrixBase<DerivedV> const &V0,
                        Eigen::MatrixBase<DerivedN> const &N,
                        LaplacianMatrix<typename DerivedV::Scalar> const &L,
                        Eigen::MatrixBase<DerivedY> const &Y,
                        Eigen::MatrixBase<DerivedG> const &γ, float σ) {
  using Scalar = typename DerivedV::Scalar;
  using Eigen::Dynamic;
  using Eigen::Index;
  using Eigen::Matrix;
  using Eigen::SparseMatrix;
  using InnerIterator = typename LaplacianMatrix<Scalar>::InnerIterator;
  using Triplet = Eigen::Triplet<Scalar>;

  auto const n = V0.rows();

  VertexMatrix<Scalar> V = V0;
  VertexMatrix<Scalar> Vlast = VertexMatrix<Scalar>::Zero(n, 3);

  Matrix<Scalar, 3, Dynamic> const V0T = V0.transpose();
  Matrix<Scalar, Dynamic, 1> x0 =
      Eigen::Map<Matrix<Scalar, Dynamic, 1> const>{V0T.data(), 3 * n, 1};

  Matrix<Scalar, 3, Dynamic> R =
      Matrix<Scalar, 3, 3>::Identity().replicate(1, n);

  Matrix<Scalar, 3, Dynamic> Rlast = Matrix<Scalar, 3, Dynamic>::Zero(3, 3 * n);

  bool converged = false;
  auto iteration = 0;
  while (!converged) {
    ++iteration;

    // Optimize position

    // Construct B matrix and c and d vector
    Matrix<Scalar, Dynamic, 1> c = Matrix<Scalar, Dynamic, 1>::Zero(3 * n);
    Matrix<Scalar, Dynamic, 1> d(3 * n);
    std::vector<Triplet> triplets;
    for (Index i = 0; i < n; ++i) {
      Matrix<Scalar, 3, 3> const Ni = γ(i) * N.row(i).transpose() * N.row(i);

      Matrix<Scalar, 3, 3> const Ri = R.template block<3, 3>(0, 3 * i);

      d.template segment<3>(3 * i) = Ni * Y.row(i).transpose();

      for (InnerIterator it(L.derived(), i); it; ++it) {
        auto const j = it.row();
        if (i == j) continue;
        Matrix<Scalar, 3, 3> const Rj = R.template block<3, 3>(0, 3 * j);

        c.template segment<3>(3 * i) += it.value() * (Ri + Rj) /
                                        static_cast<Scalar>(2) *
                                        (V0.row(i) - V0.row(j)).transpose();
      }

      for (Index k = 0; k < 3; ++k) {
        for (Index l = 0; l < 3; ++l) {
          triplets.emplace_back(3 * i + k, 3 * i + l, Ni(k, l));
        }
      }
    }
    SparseMatrix<Scalar> B(3 * n, 3 * n);
    B.setFromTriplets(triplets.cbegin(), triplets.cend());

    SparseMatrix<Scalar> const A =
        static_cast<Scalar>(2) / square(static_cast<Scalar>(σ)) *
            Eigen::kroneckerProduct(-L, Matrix<Scalar, 3, 3>::Identity()) +
        B;

    Matrix<Scalar, Dynamic, 1> const rhs =
        static_cast<Scalar>(2) / square(static_cast<Scalar>(σ)) * c + d;

    Eigen::ConjugateGradient<SparseMatrix<Scalar>, Eigen::Lower | Eigen::Upper>
        cg;
    cg.compute(A);
    // Matrix<Scalar, Dynamic, 1> const xHat = cg.solve(rhs);
    Matrix<Scalar, Dynamic, 1> const xHat = cg.solveWithGuess(rhs, x0);
    x0 = xHat;

    // Update V
    V = Eigen::Map<Matrix<Scalar, 3, Dynamic> const>{xHat.data(), 3, n}
            .transpose();

    // Find rotations that minimize ARAP energy
    for (Index i = 0; i < n; ++i) {

      Matrix<Scalar, 3, 3> Si = Matrix<Scalar, 3, 3>::Zero();
      for (InnerIterator it(L, i); it; ++it) {
        auto const j = it.row();
        if (i == j) continue;

        Matrix<Scalar, 3, 1> const eij = (V0.row(i) - V0.row(j)).transpose();
        Matrix<Scalar, 3, 1> const eijHat = (V.row(i) - V.row(j)).transpose();

        Si += it.value() * eijHat * eij.transpose();
      }

      auto const SVD = Si.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);

      Matrix<Scalar, 3, 3> U = SVD.matrixU();

      // Compute optimal rotation matrix
      Matrix<Scalar, 3, 3> Ri = SVD.matrixV() * U.transpose();
      // Ensure det(R) > 0
      if (Ri.determinant() < 0) {
        U.col(2) *= -1;
        Ri = SVD.matrixV() * U.transpose();
      }

      R.template block<3, 3>(0, 3 * i) = Ri;
    }

    auto const Vdiff = (V - Vlast).norm() / V.norm();
    auto const Rdiff = (R - Rlast).norm() / R.norm();
    converged = iteration > 100 || (Vdiff < 1e-3 && Rdiff < 1e-3);
    Rlast = R;
    Vlast = V;
  } // namespace CortidQCT

  return V;
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

  VectorXd γ;
  VectorXf optimalDisplacements;

  auto converged = false;
  auto iterations = 0;

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

    V = weightedARAPDeformation(
            V0.template cast<double>(), N.template cast<double>(), L,
            Y.template cast<double>(), γ.template cast<double>(),
            gsl::narrow_cast<float>(conf.sigmaE))
            .template cast<float>();

    converged = iterations >= conf.maxIterations;
    std::cout << "Converged after iteration " << iterations << ": "
              << std::boolalpha << converged << std::endl;
  }

  return MeshFitter::Result{};
}

} // namespace CortidQCT
