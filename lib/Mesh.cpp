/**
 * @file      Mesh.cpp
 *
 * @brief     Implementation for Mesh type
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "Mesh.h"
#include "MatrixIO.h"

#include <gsl/gsl>
#include <igl/readOFF.h>
#include <igl/read_triangle_mesh.h>
#include <igl/write_triangle_mesh.h>

#include <fstream>
#include <limits>

namespace CortidQCT {

template <class T>
Mesh<T> &Mesh<T>::loadFromFile(std::string const &meshFilename,
                               std::string const &labelFilename) {
  using Eigen::Dynamic;
  using Eigen::Map;
  using Eigen::Matrix;
  using Eigen::MatrixXd;
  using Eigen::MatrixXi;
  using gsl::narrow;

  MatrixXd vertices;
  MatrixXi indices;

  constexpr auto magicLabel = std::numeric_limits<Label>::max();

  if (!igl::read_triangle_mesh(meshFilename, vertices, indices)) {
    throw std::invalid_argument("Failed to read mesh from file '" +
                                meshFilename + "'");
  }

  // load labels. Fill labels vector with prefined value to be able to check if
  // all labels are correctly initialized.
  Matrix<Label, Dynamic, 1> labels =
      magicLabel * Matrix<Label, Dynamic, 1>::Ones(vertices.rows());

  std::ifstream labelStream{labelFilename};
  if (!labelStream) {
    throw std::invalid_argument("Failed to read labels form file '" +
                                labelFilename + "'");
  }
  labelStream >> io(labels);

  // Check if labels were correctly initialized
  if ((labels.array() == magicLabel).any()) {
    throw std::invalid_argument(
        "Failed to read labels from file '" + labelFilename +
        "', maybe the files does not contain the right number of labels (" +
        std::to_string(vertices.rows()) + ")");
  }

  auto const lVertexCount = narrow<Size>(vertices.rows());
  auto const lTriangleCount = narrow<Size>(indices.rows());

  // Reserve storage for vertex and index data
  auto vertexData = VertexData(3 * lVertexCount);
  auto indexData = IndexData(3 * lTriangleCount);
  auto labelData = LabelData(lVertexCount);

  // Copy data from eigen matrix into vectors
  Map<Matrix<Scalar, 3, Dynamic>>{vertexData.data(), 3, vertices.rows()} =
      vertices.cast<Scalar>().transpose();
  Map<Matrix<Index, 3, Dynamic>>{indexData.data(), 3, indices.rows()} =
      vertices.cast<Index>().transpose();
  Map<Matrix<Label, Dynamic, 1>>{labelData.data(), 1, vertices.rows()} = labels;

  Ensures(vertexData.size() == 3 * lVertexCount);
  Ensures(indexData.size() == 3 * lTriangleCount);
  Ensures(labelData.size() == lVertexCount);

  vertexData_ = std::move(vertexData);
  indexData_ = std::move(indexData);
  labelData_ = std::move(labelData);

  return *this;
}

template <class T>
Mesh<T> &Mesh<T>::loadFromFile(std::string const &meshFilename,
                               ColorToLabelMap<Label, double> const &colorMap) {
  using Eigen::Dynamic;
  using Eigen::Map;
  using Eigen::Matrix;
  using Eigen::MatrixXd;
  using Eigen::MatrixXi;
  using gsl::narrow;

  MatrixXd vertices, colors;
  MatrixXi indices;

  if (!igl::readOFF(meshFilename, vertices, indices, colors)) {
    throw std::invalid_argument("Failed to read mesh from file '" +
                                meshFilename + "'");
  }

  auto const lVertexCount = narrow<Size>(vertices.rows());
  auto const lTriangleCount = narrow<Size>(indices.rows());

  // Reserve storage for vertex and index data
  auto vertexData = VertexData(3 * lVertexCount);
  auto indexData = IndexData(3 * lTriangleCount);
  auto labelData = LabelData(lVertexCount);

  // Copy data from eigen matrix into vectors
  Map<Matrix<Scalar, 3, Dynamic>>{vertexData.data(), 3, vertices.rows()} =
      vertices.cast<Scalar>().transpose();
  Map<Matrix<Index, 3, Dynamic>>{indexData.data(), 3, indices.rows()} =
      vertices.cast<Index>().transpose();

  // Convert colors to labels
  for (auto i = 0; i < colors.rows(); ++i) {
    labelData_[gsl::narrow_cast<std::size_t>(i)] =
        colorMap(colors(i, 0), colors(i, 1), colors(i, 2));
  }

  Ensures(vertexData.size() == 3 * lVertexCount);
  Ensures(indexData.size() == 3 * lTriangleCount);
  Ensures(labelData.size() == lVertexCount);

  vertexData_ = std::move(vertexData);
  indexData_ = std::move(indexData);
  labelData_ = std::move(labelData);

  return *this;
}

template <class T>
void Mesh<T>::writeToFile(std::string const & /*unused*/) const {}

/*************************************
 * Explicit template instanciations
 */

template class Mesh<float>;
template class Mesh<double>;

} // namespace CortidQCT
