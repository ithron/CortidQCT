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
#include "MeshAdaptors.h"
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
#include <iterator>
#include <limits>

namespace CortidQCT {

using namespace Internal;

namespace {

template <class DerivedV, class DerivedF>
void orientOutwards(Eigen::MatrixBase<DerivedV> const &V,
                    Eigen::MatrixBase<DerivedF> &F) {
  Eigen::Matrix<typename DerivedF::Scalar, Eigen::Dynamic, Eigen::Dynamic> C;
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
  std::string const supportedFormats[] = {"obj", "off",  "stl",   "wrl",
                                          "ply", "mesh", "simesh"};

  if (!IO::checkExtensions(meshFilename, supportedFormats)) {
    throw std::invalid_argument("Unsupported file format '" +
                                IO::extension(meshFilename) + "'");
  }

  Size lVertexCount{0};

  bool meshLoaded = false;
  if (IO::extension(meshFilename, true) == "simesh") {
    *this = readFromSIMesh<T>(meshFilename, false);
    meshLoaded = true;

    Map<Matrix<T, 3, Dynamic>> vertices{vertexData_.data(), 3,
                                        narrow<Eigen::Index>(vertexCount())};
    Map<Matrix<Index, 3, Dynamic>> indices{
        indexData_.data(), 3, narrow<Eigen::Index>(triangleCount())};

    VertexMatrix<T> V = vertices.transpose();
    FacetMatrix F = indices.transpose();
    orientOutwards(V, F);

    vertices = V.transpose();
    indices = F.transpose();

    lVertexCount = vertexCount();
  }

  MatrixXd vertices;
  MatrixXi indices;

  constexpr auto magicLabel = std::numeric_limits<Label>::max();

  if (!meshLoaded) {
    // Ensure the file exists and is readable, otherwise
    // igl::read_triangle_mesh() might SEGFAULT.
    if (!std::ifstream{meshFilename} ||
        !igl::read_triangle_mesh(meshFilename, vertices, indices)) {
      throw std::invalid_argument("Failed to read mesh from file '" +
                                  meshFilename + "'");
    }
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

  VertexData vertexData;
  IndexData indexData;
  if (!meshLoaded) {
    lVertexCount = narrow<Size>(vertices.rows());
    auto const lTriangleCount = narrow<Size>(indices.rows());

    orientOutwards(vertices, indices);

    // Reserve storage for vertex and index data
    vertexData = VertexData(3 * lVertexCount);
    indexData = IndexData(3 * lTriangleCount);

    // Copy data from eigen matrix into vectors
    Map<Matrix<Scalar, 3, Dynamic>>{vertexData.data(), 3, vertices.rows()} =
        vertices.cast<Scalar>().transpose();
    Map<Matrix<Index, 3, Dynamic>>{indexData.data(), 3, indices.rows()} =
        indices.cast<Index>().transpose();

    Ensures(vertexData.size() == 3 * lVertexCount);
    Ensures(indexData.size() == 3 * lTriangleCount);
  }

  auto labelData = LabelData(lVertexCount);
  Map<Matrix<Label, Dynamic, 1>>{labelData.data(), vertices.rows(), 1} = labels;
  Ensures(labelData.size() == lVertexCount);
  labelData_ = std::move(labelData);

  if (!meshLoaded) {
    vertexData_ = std::move(vertexData);
    indexData_ = std::move(indexData);
  }

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
  std::string const supportedFormats[] = {"off", "coff", "simesh"};

  if (!IO::checkExtensions(meshFilename, supportedFormats)) {
    throw std::invalid_argument("Unsupported file format '" +
                                IO::extension(meshFilename) + "'");
  }

  if (IO::extension(meshFilename, true) == "simesh") {
    *this = readFromSIMesh<T>(meshFilename, true);
    Map<Matrix<T, 3, Dynamic>> vertices{vertexData_.data(), 3,
                                        narrow<Eigen::Index>(vertexCount())};
    Map<Matrix<Index, 3, Dynamic>> indices{
        indexData_.data(), 3, narrow<Eigen::Index>(triangleCount())};

    VertexMatrix<T> V = vertices.transpose();
    FacetMatrix F = indices.transpose();
    orientOutwards(V, F);

    vertices = V.transpose();
    indices = F.transpose();

    ensurePostconditions();
    return *this;
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

template <class T>
std::array<T, 3> Mesh<T>::cartesianRepresentation(
    BarycentricPoint<T, Mesh<T>::Index> const &point) const {
  using gsl::narrow_cast;
  using Internal::Adaptor::indexMap;
  using Internal::Adaptor::vertexMap;

  if ((point.triangleIndex < 0) ||
      (narrow_cast<std::size_t>(point.triangleIndex) > triangleCount())) {
    throw std::range_error("Triangle index out of range");
  }

  auto const idx = point.triangleIndex;
  auto const idxMap = indexMap(*this);
  auto const vMap = vertexMap(*this);

  std::array<T, 3> cartPoint;
  Eigen::Map<Eigen::Matrix<T, 3, 1>> cartMap{cartPoint.data(), 3, 1};

  cartMap = point.uv[0] * vMap.col(idxMap(0, idx)) +
            point.uv[1] * vMap.col(idxMap(1, idx)) +
            (1 - point.uv[0] - point.uv[1]) * vMap.col(idxMap(2, idx));

  return cartPoint;
}

namespace Internal {

template <class T, class InputIterator>
inline void cartesianRepresentationValidateInput_(InputIterator begin,
                                                  InputIterator end,
                                                  Mesh<T> const &mesh) {
  auto const idxUB = mesh.triangleCount();
  if (std::any_of(begin, end, [idxUB](auto const &p) {
        return (p.triangleIndex < 0) ||
               (gsl::narrow_cast<std::size_t>(p.triangleIndex) > idxUB);
      })) {
    throw std::out_of_range("At least one triangle index is out of range");
  }
}

template <class T, class I, class D1, class D2>
inline auto cartesianRepresentationConv_(BarycentricPoint<T, I> const &pt,
                                         Eigen::MatrixBase<D1> const &iMap,
                                         Eigen::MatrixBase<D2> const &vMap) {
  auto const ti = pt.triangleIndex;

  return pt.uv[0] * vMap.col(iMap(0, ti)) + pt.uv[1] * vMap.col(iMap(1, ti)) +
         (1 - pt.uv[0] - pt.uv[1]) * vMap.col(iMap(2, ti));
}

template <class T, class InputIterator, class OutputIterator>
std::size_t cartesianRepresentationImpl_(InputIterator begin, InputIterator end,
                                         OutputIterator out,
                                         Mesh<T> const &mesh, std::true_type) {
  using Eigen::Map;
  using Eigen::Matrix;
  using gsl::narrow_cast;
  using Internal::Adaptor::indexMap;
  using Internal::Adaptor::vertexMap;
  using DTI = typename std::iterator_traits<InputIterator>::difference_type;
  using DTO = typename std::iterator_traits<OutputIterator>::difference_type;

  cartesianRepresentationValidateInput_(begin, end, mesh);

  auto const idxMap = indexMap(mesh);
  auto const vMap = vertexMap(mesh);

  auto const n = gsl::narrow<std::size_t>(std::distance(begin, end));

  if (std::is_base_of<std::input_iterator_tag,
                      typename std::iterator_traits<
                          OutputIterator>::iterator_category>::value) {
#pragma omp parallel for
    for (std::size_t i = 0; i < n; ++i) {
      Eigen::Map<Matrix<T, 3, 1>>{out[narrow_cast<DTO>(i)].data(), 3, 1} =
          cartesianRepresentationConv_(begin[narrow_cast<DTI>(i)], idxMap,
                                       vMap);
    }
  } else {
#pragma omp parallel for
    for (std::size_t i = 0; i < n; ++i) {

      thread_local std::array<T, 3> cart;
      thread_local Eigen::Map<Matrix<T, 3, 1>> cartMap{cart.data(), 3, 1};

      cartMap = cartesianRepresentationConv_(begin[narrow_cast<DTI>(i)], idxMap,
                                             vMap);
      out[narrow_cast<DTO>(i)] = cart;
    }
  }

  return n;
}

template <class T, class InputIterator, class OutputIterator>
std::size_t cartesianRepresentationImpl_(InputIterator begin, InputIterator end,
                                         OutputIterator out,
                                         Mesh<T> const &mesh, std::false_type) {
  using Eigen::Map;
  using Eigen::Matrix;
  using gsl::narrow_cast;
  using Internal::Adaptor::indexMap;
  using Internal::Adaptor::vertexMap;

  cartesianRepresentationValidateInput_(begin, end, mesh);

  auto const idxMap = indexMap(mesh);
  auto const vMap = vertexMap(mesh);

  auto const n = gsl::narrow<std::size_t>(std::distance(begin, end));

  std::transform(begin, end, out, [&idxMap, &vMap](auto const &pt) {
    std::array<T, 3> cart;
    Eigen::Map<Matrix<T, 3, 1>> cartMap{cart.data(), 3, 1};

    cartMap = cartesianRepresentationConv_(pt, idxMap, vMap);
    return cart;
  });

  return n;
}

} // namespace Internal

template <class T>
template <class InputIterator, class OutputIterator>
std::size_t Mesh<T>::cartesianRepresentation(InputIterator begin,
                                             InputIterator end,
                                             OutputIterator out) const {
  using Parallel = std::integral_constant<
      bool, std::is_base_of<std::random_access_iterator_tag,
                            typename std::iterator_traits<
                                OutputIterator>::iterator_category>::value &&
                std::is_base_of<std::random_access_iterator_tag,
                                typename std::iterator_traits<
                                    InputIterator>::iterator_category>::value>;

  return Internal::cartesianRepresentationImpl_(begin, end, out, *this,
                                                Parallel{});
}

/*************************************
 * Explicit template instanciations
 */

template class Mesh<float>;
template class Mesh<double>;

template std::size_t Mesh<float>::cartesianRepresentation(
    BarycentricPoint<float, std::ptrdiff_t> const *begin,
    BarycentricPoint<float, std::ptrdiff_t> const *end,
    std::array<float, 3> *out) const;
template std::size_t Mesh<double>::cartesianRepresentation(
    BarycentricPoint<double, std::ptrdiff_t> const *begin,
    BarycentricPoint<double, std::ptrdiff_t> const *end,
    std::array<double, 3> *out) const;
template std::size_t Mesh<float>::cartesianRepresentation(
    std::vector<BarycentricPoint<float, std::ptrdiff_t>>::const_iterator begin,
    std::vector<BarycentricPoint<float, std::ptrdiff_t>>::const_iterator end,
    std::vector<std::array<float, 3>>::iterator out) const;
template std::size_t Mesh<double>::cartesianRepresentation(
    std::vector<BarycentricPoint<double, std::ptrdiff_t>>::const_iterator begin,
    std::vector<BarycentricPoint<double, std::ptrdiff_t>>::const_iterator end,
    std::vector<std::array<double, 3>>::iterator out) const;

// namespace CortidQCT
} // namespace CortidQCT
