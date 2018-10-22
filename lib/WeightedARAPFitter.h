/**
 * @file      WeightedARAPFitter.h
 *
 * @brief     This file contains the definition of the WeightedARAPFitter
 * class.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 * You may use, distribute and modify this code under the terms of the
 * AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "MeshHelpers.h"

#include <Eigen/Core>
#include <Eigen/Sparse>

namespace CortidQCT {
namespace Internal {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
/**
 * @brief This class implements a weighted as-rigid-as-possible mesh fitting
 * algorithm that uses a point-to-plane metric.
 *
 * @tparam T Scalar type of the vertices
 */
template <class T> class WeightedARAPFitter {

public:
  using Scalar = T;

  /**
   * @brief Constructs an object that can be used to fit the given reference
   * mesh
   * @param referenceMesh The reference mesh
   */
  WeightedARAPFitter(Mesh<T> const &referenceMesh, T sigma);

  /**
   * @brief Constructs an object that can be used to fit the given reference
   * mesh
   * @param V The vertex matrix of the reference mesh (Nx3)
   * @param F The facet matrix of the reference mesh (Mx3)
   */
  template <class DerivedV, class DerivedF>
  inline WeightedARAPFitter(Eigen::MatrixBase<DerivedV> const &V,
                            Eigen::MatrixBase<DerivedF> const &F, T sigma)
      : V0_(V), F_(F), sigmaSqInv_(static_cast<T>(1) / (sigma * sigma)) {
    computeLaplacian();
  }

  /**
   * @brief Fits the reference mesh to the given target vertices by minimizing
   * the wiehgted point-to-plane distances under ARAP constraints.
   *
   * @param Y Target vertex matrix (Nx3)
   * @param N Target per-vertex normal matrix (Nx3)
   * @param gamma Weight vector (Nx1)
   * @return VertexMatrix (Nx3) representing the linear embedding of the
   * deformed mesh
   */
  VertexMatrix<Scalar> fit(VertexMatrix<Scalar> const &Y,
                           VertexMatrix<Scalar> const &N,
                           Eigen::Matrix<Scalar, Eigen::Dynamic, 1> const &gamma);

private:
  using RotationMatrix = Eigen::Matrix<Scalar, 3, Eigen::Dynamic>;

  /// Initializes the laplacian matrix L_
  void computeLaplacian();

  /// Initializes the per-vertex rotation matrix R_ with replicated identity
  /// matrices.
  void initRotationMatrix();

  /// Find the vertex positions that minimizes the weighted ARAP point-to-plane
  /// enegery
  void optimizePositions(
      Eigen::Matrix<Scalar, Eigen::Dynamic, 1> const &d,
      Eigen::ConjugateGradient<Eigen::SparseMatrix<Scalar>,
                               Eigen::Lower | Eigen::Upper> &solver,
      Eigen::Matrix<Scalar, Eigen::Dynamic, 1> &cOut,
      VertexMatrix<Scalar> &Vout) const;

  /// Find the optimal per-vertex rotations that minimize the ARAP energy
  void optimizeRotations(VertexMatrix<Scalar> const &V);

  /// Compute the rigidity energy of the deformed mesh
  Scalar rigidityEnergy(VertexMatrix<Scalar> const &V) const;

  /// Vertex matrix of the undeformed mesh
  VertexMatrix<Scalar> V0_;
  /// Facet matrix of the undeformed mesh
  FacetMatrix F_;
  /// Scale parameter
  Scalar sigmaSqInv_;
  /// Laplacian matrix of the undeformed mesh
  LaplacianMatrix<Scalar> L_;
  /// Matrix containing all per-vertex rotations
  RotationMatrix R_;
};
#pragma clang diagnostic pop

extern template class WeightedARAPFitter<float>;
extern template class WeightedARAPFitter<double>;

} // namespace Internal
} // namespace CortidQCT
