/**
 * @file      MeshHelpers.h
 *
 * @brief     This header contains mesh related helper functions
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "Mesh.h"

#include <Eigen/Core>
#include <gsl/gsl>
#include <igl/cotmatrix.h>
#include <igl/per_vertex_normals.h>

namespace CortidQCT {

template <class T = float>
using VertexMatrix = Eigen::Matrix<T, Eigen::Dynamic, 3>;
template <class T = float> using NormalMatrix = VertexMatrix<T>;
using FacetMatrix =
    Eigen::Matrix<typename Mesh<float>::Index, Eigen::Dynamic, 3>;
using LabelVector =
    Eigen::Matrix<typename Mesh<float>::Label, Eigen::Dynamic, 1>;
template <class T = float> using LaplacianMatrix = Eigen::SparseMatrix<T>;

/// Returns the Nx3 vertex matrix of the mesh
template <class T> inline VertexMatrix<T> vertexMatrix(Mesh<T> const &mesh) {
  return mesh
      .withUnsafeVertexPointer([&mesh](auto const *ptr) {
        return Eigen::Map<Eigen::Matrix<T, 3, Eigen::Dynamic> const>{
            ptr, 3, gsl::narrow<Eigen::Index>(mesh.vertexCount())};
      })
      .transpose();
}

/// Returns the Mx3 index matrix of the mesh
template <class T> inline FacetMatrix facetMatrix(Mesh<T> const &mesh) {
  using Index = typename Mesh<T>::Index;
  return mesh
      .withUnsafeIndexPointer([&mesh](auto const *ptr) {
        return Eigen::Map<Eigen::Matrix<Index, 3, Eigen::Dynamic> const>{
            ptr, 3, gsl::narrow<Eigen::Index>(mesh.triangleCount())};
      })
      .transpose();
}

/// Returns a N-vector containing the per vertex labels
template <class T> inline LabelVector labelVector(Mesh<T> const &mesh) {
  using Label = typename Mesh<T>::Label;
  return mesh.withUnsafeLabelPointer([&mesh](auto const *ptr) {
    return Eigen::Map<Eigen::Matrix<Label, Eigen::Dynamic, 1> const>{
        ptr, gsl::narrow<Eigen::Index>(mesh.vertexCount())};
  });
}

/// Returns a Nx3 matrix with per-vertex normals
template <class DerivedV, class DerivedF>
inline NormalMatrix<typename DerivedV::Scalar>
perVertexNormalMatrix(Eigen::MatrixBase<DerivedV> const &V,
                      Eigen::MatrixBase<DerivedF> const &F) {
  // output matrix
  NormalMatrix<typename DerivedV::Scalar> normals;

  igl::per_vertex_normals(V, F, igl::PER_VERTEX_NORMALS_WEIGHTING_TYPE_ANGLE,
                          normals);

  return normals;
}

/// Returns a Nx3 matrix with per-vertex normals
template <class T>
inline NormalMatrix<T> perVertexNormalMatrix(Mesh<T> const &mesh) {
  return perVertexNormalMatrix(vertexMatrix(mesh), facetMatrix(mesh));
}

/// Returns the NxN sparse laplacian matrix (using cotangent weights)
template <class DerivedV, class DerivedF>
inline LaplacianMatrix<typename DerivedV::Scalar>
laplacianMatrix(Eigen::MatrixBase<DerivedV> const &V,
                Eigen::MatrixBase<DerivedF> const &F) {
  // output matrix
  Eigen::SparseMatrix<typename DerivedV::Scalar> laplacian;
  Expects(V.rows() > 0 && V.cols() == 3);
  Expects(F.rows() > 0 && F.cols() == 3);
  igl::cotmatrix(V, F, laplacian);

  return laplacian;
}

/// Returns the NxN sparse laplacian matrix (using cotangent weights)
template <class T>
inline LaplacianMatrix<T> laplacianMatrix(Mesh<T> const &mesh) {
  return laplacianMatrix(vertexMatrix(mesh), facetMatrix(mesh));
}

} // namespace CortidQCT
