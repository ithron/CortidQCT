/**
 * @file      Mesh.cpp
 *
 * @brief     Implementation for Mesh type
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 * the AFL 3.0 license; see LICENSE for full license details.
 */

#include "Mesh.h"

#include <gsl/gsl>
#include <igl/read_triangle_mesh.h>
#include <igl/write_triangle_mesh.h>

namespace CortidQCT {

template <class T> Mesh<T> &Mesh<T>::loadFromFile(std::string const &filename) {
  using Eigen::Dynamic;
  using Eigen::Map;
  using Eigen::Matrix;
  using Eigen::MatrixXd;
  using Eigen::MatrixXi;
  using gsl::narrow;

  MatrixXd vertices;
  MatrixXi indices;

  if (!igl::read_triangle_mesh(filename, vertices, indices)) {
    throw std::invalid_argument("Failed to read mesh from file '" + filename +
                                "'");
  }

  auto const lVertexCount = narrow<Size>(vertices.rows());
  auto const lTriangleCount = narrow<Size>(indices.rows());

  // Reserve storage for vertex and index data
  auto vertexData = VertexData(3 * lVertexCount);
  auto indexData = IndexData(3 * lTriangleCount);

  // Copy data from eigen matrix into vertexData_
  Map<Matrix<Scalar, 3, Dynamic>>{vertexData.data(), 3, vertices.rows()} =
      vertices.cast<Scalar>().transpose();
  Map<Matrix<Index, 3, Dynamic>>{indexData.data(), 3, indices.rows()} =
      vertices.cast<Index>().transpose();

  Ensures(vertexData.size() == 3 * lVertexCount);
  Ensures(indexData.size() == 3 * lTriangleCount);

  vertexData_ = std::move(vertexData);
  indexData_ = std::move(indexData);

  return *this;
}

template <class T>
void Mesh<T>::writeToFile(std::string const &/*unused*/) const {}

/*************************************
 * Explicit template instanciations
 */

template class Mesh<float>;
template class Mesh<double>;

} // namespace CortidQCT
