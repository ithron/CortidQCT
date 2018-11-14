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
#include "CheckExtension.h"
#include "MatrixIO.h"
#include "MeshHelpers.h"
#include "SIMesh.h"

#include <gsl/gsl>
#include <igl/orient_outward.h>
#include <igl/orientable_patches.h>
#include <igl/readOFF.h>
#include <igl/read_triangle_mesh.h>
#include <igl/writeOFF.h>
#include <igl/write_triangle_mesh.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>

namespace CortidQCT {

using namespace Internal;

namespace {

template <class DerivedV, class DerivedF>
void orientOutwards(Eigen::MatrixBase<DerivedV> const &V,
                    Eigen::MatrixBase<DerivedF> &F) {
  DerivedF C;
  Eigen::Matrix<bool, Eigen::Dynamic, 1> I;

  Expects(V.rows() > 0 && V.cols() == 3);
  Expects(F.rows() > 0 && F.cols() == 3);
  igl::orientable_patches(F.derived(), C.derived());

  igl::orient_outward(V.derived(), F.derived(), C, F.derived(), I);
}

template <class DerivedV, class DerivedF>
bool isOutwardOriented(Eigen::MatrixBase<DerivedV> const &V,
                       Eigen::MatrixBase<DerivedF> const &F) {
  using Scalar = typename DerivedV::Scalar;
  using Eigen::Dynamic;
  using Eigen::Matrix;

  NormalMatrix<Scalar> const N = perVertexNormalMatrix(V, F);
  Matrix<Scalar, 3, 1> const centroid = V.colwise().mean().transpose();

  Matrix<Scalar, Dynamic, 3> const Vcentered =
      V.rowwise() - centroid.transpose();

  auto const outwardsCount =
      ((Vcentered.array() * N.array()).rowwise().sum() > 0)
          .template cast<Eigen::Index>()
          .sum();

  return outwardsCount > V.rows() / 2;
}

} // anonymous namespace

template <class T> void Mesh<T>::ensurePostconditions() const {
  using Eigen::Dynamic;
  using Eigen::Map;
  using Eigen::Matrix;

  Ensures(vertexData_.size() % 3 == 0);
  Ensures(indexData_.size() % 3 == 0);
  Ensures(labelData_.size() == vertexData_.size() / 3);

  VertexMatrix<T> const V = vertexMatrix(*this);
  FacetMatrix const F = facetMatrix(*this);

  Ensures(isOutwardOriented(V, F));
}

template <class T>
Mesh<T> &Mesh<T>::loadFromFile(std::string const &meshFilename,
                               std::string const &labelFilename) {
  using Eigen::Dynamic;
  using Eigen::Map;
  using Eigen::Matrix;
  using Eigen::MatrixXd;
  using Eigen::MatrixXi;
  using gsl::narrow;

  // Check file format since igl does only print an error
  std::string const supportedFormats[] = {"obj", "off", "stl",
                                          "wrl", "ply", "mesh"};

  if (!IO::checkExtensions(meshFilename, supportedFormats)) {
    throw std::invalid_argument("Unsupported file format '" +
                                IO::extension(meshFilename) + "'");
  }

  MatrixXd vertices;
  MatrixXi indices;

  constexpr auto magicLabel = std::numeric_limits<Label>::max();

  // Ensure the file exists and is readable, otherwise
  // igl::read_triangle_mesh() might SEGFAULT.
  if (!std::ifstream{meshFilename} ||
      !igl::read_triangle_mesh(meshFilename, vertices, indices)) {
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

  orientOutwards(vertices, indices);

  // Reserve storage for vertex and index data
  auto vertexData = VertexData(3 * lVertexCount);
  auto indexData = IndexData(3 * lTriangleCount);
  auto labelData = LabelData(lVertexCount);

  // Copy data from eigen matrix into vectors
  Map<Matrix<Scalar, 3, Dynamic>>{vertexData.data(), 3, vertices.rows()} =
      vertices.cast<Scalar>().transpose();
  Map<Matrix<Index, 3, Dynamic>>{indexData.data(), 3, indices.rows()} =
      indices.cast<Index>().transpose();
  Map<Matrix<Label, Dynamic, 1>>{labelData.data(), vertices.rows(), 1} = labels;

  Ensures(vertexData.size() == 3 * lVertexCount);
  Ensures(indexData.size() == 3 * lTriangleCount);
  Ensures(labelData.size() == lVertexCount);

  vertexData_ = std::move(vertexData);
  indexData_ = std::move(indexData);
  labelData_ = std::move(labelData);

  ensurePostconditions();

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

  // Check file format since igl does only print an error
  std::string const supportedFormats[] = {"off", "coff"};

  if (!IO::checkExtensions(meshFilename, supportedFormats)) {
    throw std::invalid_argument("Unsupported file format '" +
                                IO::extension(meshFilename) + "'");
  }

  MatrixXd vertices, colors;
  MatrixXi indices;

  // Ensure the file exists and is readable, otherwise
  // igl::readOFF() might SEGFAULT.
  if (!std::ifstream{meshFilename} ||
      !igl::readOFF(meshFilename, vertices, indices, colors)) {
    throw std::invalid_argument("Failed to read mesh from file '" +
                                meshFilename + "'");
  }

  auto const lVertexCount = narrow<Size>(vertices.rows());
  auto const lTriangleCount = narrow<Size>(indices.rows());

  orientOutwards(vertices, indices);

  // Reserve storage for vertex and index data
  auto vertexData = VertexData(3 * lVertexCount);
  auto indexData = IndexData(3 * lTriangleCount);
  auto labelData = LabelData(lVertexCount);

  // Copy data from eigen matrix into vectors
  Map<Matrix<Scalar, 3, Dynamic>>{vertexData.data(), 3, vertices.rows()} =
      vertices.cast<Scalar>().transpose();
  Map<Matrix<Index, 3, Dynamic>>{indexData.data(), 3, indices.rows()} =
      indices.cast<Index>().transpose();

  // Convert colors to labels
  for (auto i = 0; i < colors.rows(); ++i) {
    labelData[gsl::narrow_cast<std::size_t>(i)] =
        colorMap(colors(i, 0), colors(i, 1), colors(i, 2));
  }

  Ensures(vertexData.size() == 3 * lVertexCount);
  Ensures(indexData.size() == 3 * lTriangleCount);
  Ensures(labelData.size() == lVertexCount);

  vertexData_ = std::move(vertexData);
  indexData_ = std::move(indexData);
  labelData_ = std::move(labelData);

  ensurePostconditions();

  return *this;
}

template <class T>
void Mesh<T>::writeToFile(std::string const &meshFilename,
                          std::string const &labelFilename) const {

  using Eigen::Dynamic;
  using Eigen::Map;
  using Eigen::Matrix;
  using gsl::narrow;

  if (isEmpty()) { return; }

  // Check file format since igl does only print an error
  std::string const supportedFormats[] = {"obj", "off",  "stl",   "wrl",
                                          "ply", "mesh", "simesh"};

  if (!IO::checkExtensions(meshFilename, supportedFormats)) {
    throw std::invalid_argument("Unsupported file format '" +
                                IO::extension(meshFilename) + "'");
  }

  // check for SIMEsh format
  if (IO::extension(meshFilename, true) == "simesh") {
    writeToSIMesh(*this, meshFilename);
  } else {

    // Convert vertex and index data to a format igl understads
    Map<Matrix<Scalar, 3, Dynamic> const> const Vmap{
        vertexData_.data(), 3, narrow<Eigen::Index>(vertexCount())};
    Map<Matrix<Index, 3, Dynamic> const> const Fmap{
        indexData_.data(), 3, narrow<Eigen::Index>(triangleCount())};

    Eigen::Matrix<double, Dynamic, 3> const V =
        Vmap.template cast<double>().transpose();
    Eigen::Matrix<int, Dynamic, 3> const F =
        Fmap.template cast<int>().transpose();

    if (!igl::write_triangle_mesh(meshFilename, V, F)) {
      throw std::invalid_argument("Failed to write mesh to file '" +
                                  meshFilename + "'");
    }
  }

  // Write lables
  std::ofstream labelStream{labelFilename};

  if (!labelStream) {
    throw std::invalid_argument("Failed to write per-vertex labels to file '" +
                                labelFilename + "'");
  }

  Map<Matrix<Label, Dynamic, 1> const> const labelMap{
      labelData_.data(), narrow<Eigen::Index>(vertexCount())};
  labelStream << labelMap;
}

template <class T>
void Mesh<T>::writeToFile(
    std::string const &meshFilename,
    LabelToColorMap<double, Label> const &labelMap) const {

  using Eigen::Dynamic;
  using Eigen::Map;
  using Eigen::Matrix;
  using gsl::narrow;

  if (isEmpty()) { return; };

  // Check file format since igl does only print an error
  std::string const supportedFormats[] = {"off", "coff", "simesh"};

  if (!IO::checkExtensions(meshFilename, supportedFormats)) {
    throw std::invalid_argument("Unsupported file format '" +
                                IO::extension(meshFilename) + "'");
  }

  // check for SIMEsh format
  if (IO::extension(meshFilename, true) == "simesh") {
    writeToSIMesh(*this, meshFilename, true);
  } else {
    // Convert vertex and index data to a format igl understads
    Map<Matrix<Scalar, 3, Dynamic> const> const Vmap{
        vertexData_.data(), 3, narrow<Eigen::Index>(vertexCount())};
    Map<Matrix<Index, 3, Dynamic> const> const Fmap{
        indexData_.data(), 3, narrow<Eigen::Index>(triangleCount())};

    Matrix<double, Dynamic, 3> const V =
        Vmap.template cast<double>().transpose();
    Matrix<int, Dynamic, 3> const F = Fmap.template cast<int>().transpose();
    Matrix<double, Dynamic, 3> C(Vmap.cols(), 3);

    Ensures(V.rows() > 0 && F.rows() > 0);
    Ensures(C.rows() == V.rows());

    // convert labels to colors
    for (auto i = 0; i < C.rows(); ++i) {
      auto const label = labelData_[narrow<Size>(i)];
      auto const color = labelMap(label);

      C(i, 0) = color[0];
      C(i, 1) = color[1];
      C(i, 2) = color[2];
    }

    // clang-format off
  // This assertion is to silence clang-tidy linter, somehow it does not
  // recognize that C was allocated before and complains about null pointer
  // dereferenceing when igl::writeOFF() calls C.maxCoeff().
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay, hicpp-no-array-decay)
  Ensures(C.data() != nullptr);
    // clang-format on

    if (!igl::writeOFF(meshFilename, V, F, C)) {
      throw std::invalid_argument("Failed to write mesh to file '" +
                                  meshFilename + "'");
    }
  }
}

/*************************************
 * Explicit template instanciations
 */

template class Mesh<float>;
template class Mesh<double>;

} // namespace CortidQCT
