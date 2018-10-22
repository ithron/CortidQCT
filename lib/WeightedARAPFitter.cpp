/**
 * @file      WeightedARAPFitter.cpp
 *
 * @brief     Implementation file fot WeightedARAPFitter class
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 * You may use, distribute and modify this code under the terms of the
 * AFL 3.0 license; see LICENSE for full license details.
 */

#include "WeightedARAPFitter.h"

#include <fstream>
#include <gsl/gsl>
#include <limits>
#include <vector>

#include <unsupported/Eigen/KroneckerProduct>

namespace CortidQCT {
namespace Internal {

namespace {

template <class DerivedN, class DerivedGamma>
Eigen::Matrix<typename DerivedN::Scalar, 3, Eigen::Dynamic>
constructNNMatrix(Eigen::MatrixBase<DerivedN> const &N,
                  Eigen::MatrixBase<DerivedGamma> const &gamma) {
  using Scalar = typename DerivedN::Scalar;

  Eigen::Matrix<Scalar, 3, Eigen::Dynamic> NN(3, 3 * N.rows());

  for (auto i = 0; i < N.rows(); ++i) {
    NN.template block<3, 3>(0, 3 * i) = gamma(i) * N.row(i).transpose() * N.row(i);
  }

  return NN;
}

template <class DerivedNN>
Eigen::SparseMatrix<typename DerivedNN::Scalar>
constructBMatrix(Eigen::MatrixBase<DerivedNN> const &NN) {
  using Scalar = typename DerivedNN::Scalar;
  using Triplet = Eigen::Triplet<Scalar>;

  std::vector<Triplet> triplets;

  for (auto i = 0; i < NN.cols() / 3; ++i) {
    for (auto k = 0; k < 3; ++k) {
      for (auto l = 0; l < 3; ++l) {
        triplets.emplace_back(3 * i + k, 3 * i + l, NN(k, 3 * i + l));
      }
    }
  }

  Eigen::SparseMatrix<Scalar> B(NN.cols(), NN.cols());
  B.setFromTriplets(triplets.cbegin(), triplets.cend());

  return B;
}

template <class DerivedNN, class DerivedY>
Eigen::Matrix<typename DerivedNN::Scalar, Eigen::Dynamic, 1>
constructDVector(Eigen::MatrixBase<DerivedNN> const &NN,
                 Eigen::MatrixBase<DerivedY> const &Y) {
  using Scalar = typename DerivedNN::Scalar;

  Eigen::Matrix<Scalar, Eigen::Dynamic, 1> d(NN.cols());

  for (auto i = 0; i < Y.rows(); ++i) {
    d.template segment<3>(3 * i) =
        NN.template block<3, 3>(0, 3 * i) * Y.row(i).transpose();
  }

  return d;
}

} // anonymous namespace

template <class T>
WeightedARAPFitter<T>::WeightedARAPFitter(Mesh<T> const &mesh, T sigma) {
  V0_ = vertexMatrix(mesh);
  F_ = facetMatrix(mesh);
  sigmaSqInv_ = static_cast<Scalar>(1) / (sigma * sigma);
  computeLaplacian();
}

template <class T> void WeightedARAPFitter<T>::computeLaplacian() {
  L_ = laplacianMatrix(V0_, F_);
}

template <class T> void WeightedARAPFitter<T>::initRotationMatrix() {
  using Mat = Eigen::Matrix<Scalar, 3, 3>;
  R_ = Mat::Identity().replicate(1, V0_.rows());
}

template <class T>
void WeightedARAPFitter<T>::optimizePositions(
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> const &d,
    Eigen::ConjugateGradient<Eigen::SparseMatrix<Scalar>,
                             Eigen::Lower | Eigen::Upper> &solver,
    Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &cOut,
    VertexMatrix<Scalar> &Vout) const {

  using InnerIterator = typename LaplacianMatrix<Scalar>::InnerIterator;
  using Eigen::Dynamic;
  using Eigen::Map;
  using Eigen::Matrix;
  using Vector = Eigen::Matrix<Scalar, Dynamic, 1>;

  // copmute c vector
  for (auto i = 0; i < V0_.rows(); ++i) {

    Matrix<Scalar, 3, 3> const Ri = R_.template block<3, 3>(0, 3 * i);
    cOut.template segment<3>(3 * i).array() = 0;

    for (InnerIterator it{L_, i}; it; ++it) {
      auto const j = it.row();
      if (i == j) continue;

      cOut.template segment<3>(3 * i) +=
          it.value() * (Ri + R_.template block<3, 3>(0, 3 * j)) *
          (V0_.row(i) - V0_.row(j)).transpose() / static_cast<Scalar>(2);
    }
  }

  Vector const rhs = 2 * sigmaSqInv_ * cOut + d;
  Matrix<Scalar, 3, Dynamic> const Vtransposed = Vout.transpose();

  Vector const x = solver.solveWithGuess(
      rhs, Map<Vector const>{Vtransposed.data(), 3 * Vout.rows(), 1});

  Vout = Map<Matrix<Scalar, 3, Dynamic> const>{x.data(), 3, Vout.rows()}
             .transpose();
}

template <class T>
void WeightedARAPFitter<T>::optimizeRotations(VertexMatrix<T> const &V) {
  using Matrix3 = Eigen::Matrix<Scalar, 3, 3>;
  using Vector3 = Eigen::Matrix<Scalar, 3, 1>;
  using InnerIterator = typename LaplacianMatrix<Scalar>::InnerIterator;

  auto const n = V0_.rows();
  // Find rotations that minimize ARAP energy
  for (auto i = 0; i < n; ++i) {

    Matrix3 Si = Matrix3::Zero();
    for (InnerIterator it{L_, i}; it; ++it) {
      auto const j = it.row();
      if (i == j) continue;

      Vector3 const eij = (V0_.row(i) - V0_.row(j)).transpose();
      Vector3 const eijHat = (V.row(i) - V.row(j)).transpose();

      Si += it.value() * eijHat * eij.transpose();
    }

    auto const SVD = Si.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);

    Matrix3 U = SVD.matrixU();

    // Compute optimal rotation matrix
    Matrix3 Ri = SVD.matrixV() * U.transpose();
    // Ensure det(R) > 0
    if (Ri.determinant() < 0) {
      U.col(2) *= -1;
      Ri = SVD.matrixV() * U.transpose();
    }

    R_.template block<3, 3>(0, 3 * i) = Ri;
  }
}

template <class T>
T WeightedARAPFitter<T>::rigidityEnergy(VertexMatrix<Scalar> const &V) const {
  using InnerIterator = typename LaplacianMatrix<Scalar>::InnerIterator;

  auto energy = Scalar{0};

  for (auto i = 0; i < V.rows(); ++i) {

    for (InnerIterator it{L_, i}; it; ++it) {
      auto const j = it.row();
      if (i == j) continue;

      energy += it.value() * ((V.row(i) - V.row(j)).transpose() -
                              R_.template block<3, 3>(0, 3 * i) *
                                  (V0_.row(i) - V0_.row(j)).transpose())
                                 .squaredNorm();
    }
  }

  return energy;
}

template <class T>
VertexMatrix<T>
WeightedARAPFitter<T>::fit(VertexMatrix<T> const &Y, VertexMatrix<T> const &N,
                           Eigen::Matrix<Scalar, Eigen::Dynamic, 1> const &gamma) {
  using Eigen::Dynamic;
  using Eigen::Matrix;
  using Matrix3 = Eigen::Matrix<Scalar, 3, 3>;
  using SparseMatrix = Eigen::SparseMatrix<Scalar>;

  auto const n = V0_.rows();
  Expects(Y.rows() == n && N.rows() == n && Y.cols() == 3 && N.cols() == 3);

  initRotationMatrix();

  VertexMatrix<Scalar> V = V0_;
  VertexMatrix<Scalar> Vbest = V;

  auto const NN = constructNNMatrix(N, gamma);
  auto const B = constructBMatrix(NN);
  auto const d = constructDVector(NN, Y);
  SparseMatrix const A =
      2 * sigmaSqInv_ * Eigen::kroneckerProduct(-L_, Matrix3::Identity()) + B;

  Matrix<Scalar, Dynamic, 1> c(3 * n);

  Eigen::ConjugateGradient<SparseMatrix, Eigen::Lower | Eigen::Upper> cg;
  cg.compute(A);

  auto converged = false;
  auto eMin = std::numeric_limits<Scalar>::max();
  int nonDecrease{0};

  while (!converged) {
    optimizePositions(d, cg, c, V);
    optimizeRotations(V);

    auto const energy = rigidityEnergy(V);

    if (energy < eMin) {
      nonDecrease = 0;
      eMin = energy;
      Vbest = V;
    } else {
      ++nonDecrease;
    }
    converged = nonDecrease > 5;
  }

  return Vbest;
}

template class WeightedARAPFitter<float>;
template class WeightedARAPFitter<double>;

} // namespace Internal
} // namespace CortidQCT
