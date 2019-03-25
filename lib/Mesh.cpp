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

#include <Eigen/Sparse>
#include <gsl/gsl>
#include <igl/orient_outward.h>
#include <igl/orientable_patches.h>
#include <igl/ray_mesh_intersect.h>
#include <igl/readOFF.h>
#include <igl/read_triangle_mesh.h>
#include <igl/upsample.h>
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

struct SequencialTransform {
  template <class I, class O, class F>
  void operator()(I b, I e, O o, F &&f) const {
    using std::transform;
    transform(b, e, o, std::forward<F>(f));
  }
};

struct ParallelTransform {
  template <class I, class O, class F>
  void operator()(I b, I e, O o, F &&f) const {
    using gsl::narrow_cast;
    using std::distance;
    auto const N = distance(b, e);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-compare"

#pragma omp parallel for
    for (std::size_t i = 0; i < narrow_cast<std::size_t>(N); ++i) {
      o[i] = f(b[i]);
    }
  }

#pragma clang diagnostic pop
};

} // anonymous namespace

template <class T> void Mesh<T>::ensurePostconditions() const {
  using Eigen::Dynamic;
  using Eigen::Map;
  using Eigen::Matrix;

  Ensures(vertexData_.size() % 3 == 0);
  Ensures(indexData_.size() % 3 == 0);
  Ensures(labelData_.size() == vertexData_.size() / 3);
  Ensures(normalData_.size() % 3 == 0);
  Ensures(normalData_.size() == vertexData_.size());

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

  updatePerVertexNormals();

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

  updatePerVertexNormals();

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

template <bool IsForward> struct BarycentricPointValidatorHelper_;

template <> struct BarycentricPointValidatorHelper_<true> {

  template <class T>
  static void validate(BarycentricPoint<T, typename Mesh<T>::Index> const &,
                       Mesh<T> const &) {}

  template <class T, class Iter>
  static void validate(Iter begin, Iter end, Mesh<T> const &mesh) {
    auto const idxUB = mesh.triangleCount();
    if (std::any_of(begin, end, [idxUB](auto const &p) {
          return (p.triangleIndex < 0) ||
                 (gsl::narrow_cast<std::size_t>(p.triangleIndex) > idxUB);
        })) {
      throw std::out_of_range("At least one triangle index is out of range");
    }
  }
};

template <> struct BarycentricPointValidatorHelper_<false> {

  template <class T>
  static void validate(BarycentricPoint<T, typename Mesh<T>::Index> const &pt,
                       Mesh<T> const &mesh) {
    if ((pt.triangleIndex < 0) ||
        (gsl::narrow_cast<std::size_t>(pt.triangleIndex) >
         mesh.triangleCount())) {
      throw std::out_of_range("triangle index is out of range");
    }
  }

  template <class T, class Iter>
  static void validate(Iter, Iter, Mesh<T> const &) {}
};

template <class Iter>
using BarycentricPointValidator_ =
    BarycentricPointValidatorHelper_<std::is_base_of<
        std::forward_iterator_tag,
        typename std::iterator_traits<Iter>::iterator_category>::value>;

} // namespace Internal

template <class T>
template <class InputIterator, class OutputIterator>
void Mesh<T>::cartesianRepresentation(InputIterator begin, InputIterator end,
                                      OutputIterator out) const {

  withUnsafeVertexPointer([begin, end, out, this](auto const *vPtr) {
    this->barycentricInterpolation(begin, end, vPtr, out, 3);
  });
}

template <class T>
template <class PtIter, class AttrIter, class OutputIterator>
void Mesh<T>::barycentricInterpolation(PtIter pointsBegin, PtIter pointsEnd,
                                       AttrIter attrBegin, OutputIterator out,
                                       std::size_t attributeDimensions) const {

  // Type assertions
  using std::is_base_of;
  using std::iterator_traits;
  using PtTraits = iterator_traits<PtIter>;
  using AttrTraits = iterator_traits<AttrIter>;
  // using OutTraits = iterator_traits<OutputIterator>;
  static_assert(is_base_of<std::input_iterator_tag,
                           typename PtTraits::iterator_category>::value,
                "PtIter must be an input iterator");
  static_assert(is_base_of<std::random_access_iterator_tag,
                           typename AttrTraits::iterator_category>::value,
                "AttrIterator must be a random access iterator");

  using Validator = Internal::BarycentricPointValidator_<PtIter>;

  using namespace Internal::Adaptor;

  // Validate input. Will only result in code if PtIter conforms to
  // ForwardIterator
  Validator::validate(pointsBegin, pointsEnd, *this);

  auto const iMap = indexMap(*this);

  auto const N = gsl::narrow<Index>(attributeDimensions);

  for (auto ptI = pointsBegin; ptI != pointsEnd; ++ptI) {
    for (auto i = 0; i < N; ++i) {
      // Validate input. Will only result in code if PtIter does not conform to
      // ForwardIterator
      Validator::validate(*ptI, *this);

      std::array<Index, 3> const attrIdx{{N * iMap(0, ptI->triangleIndex) + i,
                                          N * iMap(1, ptI->triangleIndex) + i,
                                          N * iMap(2, ptI->triangleIndex) + i}};

      // Interpolate values
      *out++ = ptI->uv[0] * attrBegin[attrIdx[0]] +
               ptI->uv[1] * attrBegin[attrIdx[1]] +
               (1 - ptI->uv[0] - ptI->uv[1]) * attrBegin[attrIdx[2]];
    }
  }
}

template <class T>
RayMeshIntersection<T> Mesh<T>::rayIntersection(Ray<T> const &ray) const {
  using std::addressof;
  RayMeshIntersection<T> intersection;

  rayIntersections(addressof(ray), addressof(ray) + 1, addressof(intersection));

  return intersection;
}

template <class T>
template <class InputIterator, class OutputIterator>
void Mesh<T>::rayIntersections(InputIterator raysBegin, InputIterator raysEnd,
                               OutputIterator intersectionsOut) const {
  using namespace Internal::Adaptor;
  using Eigen::Map;
  using Eigen::Matrix;
  using gsl::narrow_cast;
  using std::abs;

  // Type validation
  using InputTraits = std::iterator_traits<InputIterator>;

  static_assert(
      std::is_convertible<typename InputTraits::value_type, Ray<T>>::value,
      "value_type of InputIterator must be convertible to Ray");

  // Get iterator categories for input and output iterator and check if both
  // are random access.
  using ITag = typename std::iterator_traits<InputIterator>::iterator_category;
  using OTag = typename std::iterator_traits<OutputIterator>::iterator_category;
  constexpr bool isRandomAccessI =
      std::is_base_of<std::random_access_iterator_tag, ITag>::value;
  constexpr bool isRandomAccessO =
      std::is_base_of<std::random_access_iterator_tag, OTag>::value;
  // If both iterator types are random access, parallel execution can be used.
  // If not use sequential code.
  using Transform = std::conditional_t<isRandomAccessI && isRandomAccessO,
                                       ParallelTransform, SequencialTransform>;

  auto const vMap = vertexMap(*this);
  auto const iMap = indexMap(*this);

  // Either transverse the data sequencially or parallel, depending on the
  // iterator types
  Transform{}(
      raysBegin, raysEnd, intersectionsOut, [&vMap, &iMap](auto const &ray) {
        // map ray elements to eigen vectors
        Map<Matrix<T, 3, 1> const> const sourceMap{ray.origin.data(), 3, 1};
        Map<Matrix<T, 3, 1> const> const dirMap{ray.direction.data(), 3, 1};
        igl::Hit hit;
        // Note: intersection.signedDistance is initialized to infinity
        RayMeshIntersection<T> intersection;

        if (igl::ray_mesh_intersect(sourceMap, dirMap, vMap.transpose(),
                                    iMap.transpose(), hit)) {
          intersection.position.triangleIndex = hit.id;
          intersection.position.uv[0] = narrow_cast<T>(hit.u);
          intersection.position.uv[1] = narrow_cast<T>(hit.v);
          intersection.signedDistance = narrow_cast<T>(hit.t);
        }

        // Search in reversed direction
        if (igl::ray_mesh_intersect(sourceMap, -dirMap, vMap.transpose(),
                                    iMap.transpose(), hit)) {
          // If we found a closer hit, update intersection
          if (abs(intersection.signedDistance) > abs(narrow_cast<T>(hit.t))) {
            intersection.position.triangleIndex = hit.id;
            intersection.position.uv[0] = narrow_cast<T>(hit.u);
            intersection.position.uv[1] = narrow_cast<T>(hit.v);
            intersection.signedDistance = narrow_cast<T>(-hit.t);
          }
        }

        return intersection;
      });
}

template <class T> Mesh<T> &Mesh<T>::upsample(std::size_t nTimes) {

  using gsl::narrow_cast;
  using VMat = Eigen::Matrix<T, Eigen::Dynamic, 3>;
  using FMat = Eigen::Matrix<int, Eigen::Dynamic, 3>;

  if (nTimes == 0) return *this;

  // Copy vertces and indices into matrices so that igl can handle them
  VMat const V = Adaptor::vertexMap(*this).transpose();
  FMat const F = Adaptor::indexMap(*this).transpose().template cast<int>();

  VMat Vnew;
  FMat Fnew;

  igl::upsample(V, F, Vnew, Fnew, gsl::narrow<int>(nTimes));

  vertexData_.resize(3 * narrow_cast<std::size_t>(Vnew.rows()));
  indexData_.resize(3 * narrow_cast<std::size_t>(Fnew.rows()));
  labelData_.resize(narrow_cast<std::size_t>(Vnew.rows()), Label{0});

  // Assign copy new vertices and indices
  Adaptor::vertexMap(*this) = Vnew.transpose();
  Adaptor::indexMap(*this) = Fnew.transpose().template cast<Index>();

  // Labels are not touched
  return *this;
}

template <class T> void Mesh<T>::updatePerVertexNormals() {
  if (normalData_.size() != vertexData_.size()) {
    normalData_.resize(vertexData_.size());
  }

  Adaptor::vertexNormalMap(*this) =
      Internal::perVertexNormalMatrix(*this).transpose();
}

/*************************************
 * Explicit template instanciations
 */

template class Mesh<float>;
template class Mesh<double>;

template void Mesh<float>::barycentricInterpolation(
    BarycentricPoint<float, std::ptrdiff_t> const *,
    BarycentricPoint<float, std::ptrdiff_t> const *, float const *, float *,
    std::size_t) const;
template void Mesh<double>::barycentricInterpolation(
    BarycentricPoint<double, std::ptrdiff_t> const *,
    BarycentricPoint<double, std::ptrdiff_t> const *, double const *, double *,
    std::size_t) const;

template void Mesh<float>::cartesianRepresentation(
    BarycentricPoint<float, std::ptrdiff_t> const *,
    BarycentricPoint<float, std::ptrdiff_t> const *, float *) const;
template void Mesh<double>::cartesianRepresentation(
    BarycentricPoint<double, std::ptrdiff_t> const *,
    BarycentricPoint<double, std::ptrdiff_t> const *, double *) const;
template void Mesh<float>::cartesianRepresentation(
    std::vector<BarycentricPoint<float, std::ptrdiff_t>>::const_iterator,
    std::vector<BarycentricPoint<float, std::ptrdiff_t>>::const_iterator,
    float *) const;
template void Mesh<double>::cartesianRepresentation(
    std::vector<BarycentricPoint<double, std::ptrdiff_t>>::const_iterator,
    std::vector<BarycentricPoint<double, std::ptrdiff_t>>::const_iterator,
    double *) const;

template void Mesh<float>::rayIntersections(Ray<float> const *,
                                            Ray<float> const *,
                                            RayMeshIntersection<float> *) const;
template void Mesh<float>::rayIntersections(
    Ray<float> const *, Ray<float> const *,
    std::back_insert_iterator<std::vector<RayMeshIntersection<float>>>) const;
template void
Mesh<double>::rayIntersections(Ray<double> const *, Ray<double> const *,
                               RayMeshIntersection<double> *) const;
template void Mesh<double>::rayIntersections(
    Ray<double> const *, Ray<double> const *,
    std::back_insert_iterator<std::vector<RayMeshIntersection<double>>>) const;

// namespace CortidQCT
} // namespace CortidQCT
