/**
 * @file      Mesh.h
 *
 * @brief     Include file defining the Mesh data type
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "BarycentricPoint.h"
#include "ColorToLabelMap.h"
#include "LabelToColorMap.h"
#include "Ray.h"
#include "RayMeshIntersection.h"

#include <array>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace CortidQCT {

/**
 * @brief A triangle mesh class
 *
 * @tparam T Scalar type of the vector space of the embedding, must be a
 * floating point type
 * @nosubgrouping .
 */
template <class T> class Mesh {

  static_assert(std::is_floating_point<T>::value,
                "Scalar must be a floating point type");

public:
  /// Scalar type of the vector space the embedding
  using Scalar = T;

  /// Index type
  using Index = std::ptrdiff_t;

  /// Size type
  using Size = std::size_t;

  /// Label type
  using Label = unsigned int;

private:
  /// Type of vertex data storage
  using VertexData = std::vector<Scalar>;
  /// Type of index data storage
  using IndexData = std::vector<Index>;
  /// Tyep of per vertex label storage
  using LabelData = std::vector<Label>;

public:
  /// @name Construction
  /// @{

  /// Constructs an empty mesh
  inline Mesh() noexcept(noexcept(VertexData()) && noexcept(IndexData()) &&
                         noexcept(LabelData())) {}

  /// Constructs an uninitialized mesh with the given vertex and triangle count
  inline Mesh(std::size_t nVertices, std::size_t nTriangles) noexcept(
      noexcept(VertexData(3 * nVertices, T{0})) &&
      noexcept(IndexData(3 * nTriangles, Index{0})) &&
      noexcept(LabelData(nVertices, Label{0})))
      : vertexData_(3 * nVertices, T{0}), indexData_(3 * nTriangles, Index{0}),
        labelData_(nVertices, Label{0}) {}

  // @}

  /// @name Accessors
  /// @{

  /// Number of vertices
  inline Size vertexCount() const noexcept { return vertexData_.size() / 3; }

  /// Number of triangles
  inline Size triangleCount() const noexcept { return indexData_.size() / 3; }

  /// @brief true iff the mesh is empty
  ///
  /// The mesh is empty iff the triangle or the vertex count is zero
  inline bool isEmpty() const noexcept {
    return triangleCount() == 0 || vertexCount() == 0;
  }
  /// @}

  /// @name IO
  /// @{

  /// @brief Load mesh and labels from ASCII file using format auto detection
  ///
  /// Supported file formats are: obj, off, stl, wrl, ply, mesh, SIMesh.
  /// If a .off file with color data (COFF) is given and the labels should be
  /// extracted from the color data, use the overload
  /// `loadFromFile(std::string const &, ColorToLabelMap<Label, double> const
  /// &)`.
  ///
  /// @param meshFilename Path to the file to load the mesh from
  /// @param labelFilename Optional path to the file to load the vertex labels
  /// from.
  /// @return Reference to the loaded mesh
  /// @throws std::invalid_argument if the mesh or the labels could not be
  /// loaded from the given filename
  /// @throws std::invalid_argument if the file file format could not be
  /// recognized
  Mesh &loadFromFile(std::string const &meshFilename,
                     std::string const &labelFilename);

  /// @brief Load mesh from ASCII file using format auto detection and extract
  /// labels from per-vertex colors
  ///
  /// Supported file formats are: off (COFF), SIMesh.
  /// Per-vertex colors are converted to labels using the given colormap.
  /// For SIMesh format, the labels are directly read from the file.
  /// For other formats use the overload
  /// `loadFromFile(std::string const &, std::string const &)`
  ///
  /// @param meshFilename Path to the file to load the mesh from
  /// @param colorMap Mapping from RGB colorspace to labels.
  /// Defaults to `ColorToLabelMaps::defaultMap()`.
  /// @return Reference to the loaded mesh
  /// @throws std::invalid_argument if the mesh could not be loaded from the
  /// given filename
  /// @throws std::invalid_argument if the file file format could not be
  /// recognized
  Mesh &loadFromFile(std::string const &meshFilename,
                     ColorToLabelMap<Label, double> const &colorMap =
                         ColorToLabelMaps::defaultMap<Label, double>);

  /// @brief Writes mesh to ASCII file using format auto detection
  ///
  /// Supported file formats are: obj, off, stl, wrl, ply, mesh, SIMesh.
  /// Labels are written rowwise to `labelFilename`.
  /// For encoding the labels in the color attribute use the overload
  /// `writeToFile(std::string const &, LabelToColorMap<double, Label> const
  /// &)`.
  ///
  /// @param meshFilename path to the file to write the mesh to
  /// @param labelFilename path to the file to write the labels to
  /// @throws std::invalid_argument if the mesh or the labels could not be
  /// writted to the given file
  /// @throws std::invalid_argument if the file file format could not be
  /// recognized
  /// @note Does nothing on empty meshes
  void writeToFile(std::string const &meshFilename,
                   std::string const &labelFilename) const;

  /// @brief Writes mesh to ASCII file using format auto detection and encode
  /// labels as colors.
  ///
  /// Supported file formats are: off (coff), SIMesh.
  /// The labels are encoded in per-vertex colors using the given label to
  /// color map.
  /// For the 'SIMesh' format the labels are written directly into the mesh
  /// file, ignoring the color encoding.
  /// For storing the labels in a separate file use the overload
  /// `writeToFile(std::string const &, std::string const &)`.
  ///
  /// @param meshFilename path to the file to write the mesh to
  /// @param labelMap function that maps labels to RGB color values. Defaults
  /// to `LabelToColorMaps::defaultMap()`.
  /// @throws std::invalid_argument if the mesh could not be
  /// writted to the given file
  /// @throws std::invalid_argument if the file file format could not be
  /// recognized
  /// @note Does nothing on empty meshes
  void writeToFile(std::string const &meshFilename,
                   LabelToColorMap<double, Label> const &labelMap =
                       LabelToColorMaps::defaultMap<double, Label>) const;

  /// @}

  /**
   * @name Queries
   * @{
   */

  /**
   * @brief Returns the cartesian coordinate of a point in barycentric
   * coordinates
   * @param[in] point point in barycentric coodinate representation
   * @return vartesian coordinates of the point
   * @throws std::out_of_range if the triangle index of the point is invalid.
   */
  std::array<T, 3>
  cartesianRepresentation(BarycentricPoint<T, Index> const &point) const;

  /**
   * @brief Converts a sequence of barycentric coordinates into caresian
   * coordinates.
   *
   * The implementation uses barycetnricInterpolation. OutputIterator's
   * value_type must be T.
   *
   * @param[in] begin iterator that points to the first cartesian point
   * @param[in] end interator that points right after the last point
   * @param[out] out output iterator
   * @throws std::out_of_range if a triangle index is out of range
   * @see barycentricInterpolation
   */
  template <class InputIterator, class OutputIterator>
  void cartesianRepresentation(InputIterator begin, InputIterator end,
                               OutputIterator out) const;

  /**
   * @brief Converts a sequence of barycentric coordinates into caresian
   * coordinates.
   * @param[in] points std::vector of barycentric points
   * @return a std::vector of cartesian points
   * @throws std::out_of_range if a triangle index is out of range
   */
  inline std::vector<std::array<T, 3>> cartesianRepresentation(
      std::vector<BarycentricPoint<T, Index>> const &points) const {

    using Cartesian = std::array<T, 3>;
    std::vector<Cartesian> cartPoints(points.size());

    auto const out = reinterpret_cast<T *>(cartPoints.data());
    cartesianRepresentation(points.cbegin(), points.cend(), out);

    return cartPoints;
  }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
  /**
   * @brief Interpolates per-vertex values for points inside a triangle.
   *
   * This input of this function are some points on the surfac of the mesh in
   * barycentric representation and a vector of per-vertex attributes/values
   * that are to be interpolated.
   * The per-vertex attributes/values do not have to be scalar, vector values
   * attributes can be used by setting the `attributeDimension` paremter.
   * The attributes must be in row major order, i.e. a full attribute vector is
   * stored in consecutively in the underlying container.
   *
   * @tparam PtIter InputIterator of BarycentricPoint type
   * @tparam AttrIter RandomAccessIterator of scalar type
   * @tparam OutputIterator output iterator acceptiong scalar of the same type
   * as AttrIter
   * @param[in] pointsBegin input iterator pointing to the first barycentric
   * point
   * @param[in] pointsEnd input iterator pointing one element past the last
   * barycentric point
   * @param[in] attributesBegin random access iterator pointing to the first
   * attribute vector
   * @param[out] out output iterator
   * @param[in] attributeDimension number of attribute dimensions, defaults to
   * 1
   * @throws std::out_of_range if a trinagle index is out of range
   *  In case of an exception `out` is only untouched if and only if
   * `PtIter` conforms to `ForwardIterator`. If `PtIter` only conforms to
   * `IputIterator`, all values up to the one where the exception happened will
   * be written to `out`.
   */
  template <class PtIter, class AttrIter, class OutputIterator>
  void barycentricInterpolation(PtIter pointsBegin, PtIter pointsEnd,
                                AttrIter attributesBegin, OutputIterator out,
                                std::size_t attributeDimension = 1) const;
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
  /**
   * @brief Computes the intersection of a set of rays with the mesh.
   *
   * If for any ray no intersection can be found, the corresponding
   * RayMeshIntersection object is left with its default values (its signed
   * distance is infinity).
   *
   * @tparam InputIterator Input iterator with value_type of Ray
   * @tparam OutputIterator Output iterator with value_type of
   * RayMeshIntersection
   * @param raysBegin Iterator poiting to the first ray
   * @param raysEnd Iterator pointing one element past the last ray
   * @param intersectionsOut Output iterator for intersections
   */
  template <class InputIterator, class OutputIterator>
  void rayIntersections(InputIterator raysBegin, InputIterator raysEnd,
                        OutputIterator intersectionsOut) const;
#pragma clang diagnostic pop

  /**
   * @brief Computes intersection with the given ray and the mesh
   *
   * @see rayIntersections
   * @param ray Query ray
   * @return RayMeshIntersection object describing the intersection.
   */
  RayMeshIntersection<T> rayIntersection(Ray<T> const &ray) const;

  /// @}

  /**
   * @name Modifyers
   * @{
   */

  /**
   * @brief Upsample the mesh without touching the original vertices
   *
   * @param nTimes number of subsample iterations
   * @return Reference to `*this`
   */
  Mesh<T> &upsample(std::size_t nTimes = 1);

  /// @}

  /**
   * @name Raw Data Access
   * The methods in this section all call a functional with a pointer to raw
   * data as its argument. The pointer only guaranteed to be valid within the
   * call to the given functional.
   *
   * @attention Do not return the raw data pointer or save it in any other way!
   * @{
   */

  /**
   * @brief Calls the given functional with an unsafe pointer to the raw
   * vertex storage.
   *
   * Vertices are stored contiguously in memory: [x_0, y_0, z_0, x_1, y_1, z_1,
   * ...].
   *
   * @tparam F Function that accepts a `Scalar const *` pointer as the only
   * argument.
   * @throws noexcept(conditional) iff `f(Scalar const *)` is noexcept.
   * @return The return value of the functional
   */
  template <class F>
  inline auto withUnsafeVertexPointer(F &&f) const
      noexcept(noexcept(f(std::declval<const VertexData>().data()))) {
    return f(vertexData_.data());
  }

  template <class F>
  inline auto withUnsafeVertexPointer(F &&f) noexcept(
      noexcept(f(std::declval<VertexData>().data()))) {
    return f(vertexData_.data());
  }

  /**
   * @brief Calls the given functional with an unsafe pointer to the raw
   * index storage.
   *
   * Indices are stored contiguously in memory: [i_0, i_1, ...].
   *
   * @tparam F Function that accepts a `Index const *` pointer as the only
   * argument.
   * @throws noexcept(conditional) iff `f(Index const *)` is noexcept.
   * @return The return value of the functional
   */
  template <class F>
  inline auto withUnsafeIndexPointer(F &&f) const
      noexcept(noexcept(f(std::declval<const IndexData>().data()))) {
    return f(indexData_.data());
  }

  template <class F>
  inline auto withUnsafeIndexPointer(F &&f) noexcept(
      noexcept(f(std::declval<IndexData>().data()))) {
    return f(indexData_.data());
  }

  /**
   * @brief Calls the given functional with an unsafe pointer to the raw
   * label storage.
   *
   * Labels are stored contiguously in memory: [l_0, l_1, ...].
   *
   * @tparam F Function that accepts a `Label const *` pointer as the only
   * argument.
   * @throws noexcept(conditional) iff `f(Label const *)` is noexcept.
   * @return The return value of the functional
   */
  template <class F>
  inline auto withUnsafeLabelPointer(F &&f) const
      noexcept(noexcept(f(LabelData().data()))) {
    return f(labelData_.data());
  }

  template <class F>
  inline auto
  withUnsafeLabelPointer(F &&f) noexcept(noexcept(f(LabelData().data()))) {
    return f(labelData_.data());
  }

  /// @}
private:
  /// Ensures validility of the mesh
  void ensurePostconditions() const;

  /// Stores vertex coordinates in column major order
  VertexData vertexData_;
  /// Stores per triangle vertex indices in column major oder
  IndexData indexData_;
  /// Stores per vertex labels
  LabelData labelData_;
};

/*************************************
 * Explicit template instanciations
 */

extern template class Mesh<float>;
extern template class Mesh<double>;

extern template void Mesh<float>::barycentricInterpolation(
    BarycentricPoint<float, std::ptrdiff_t> const *,
    BarycentricPoint<float, std::ptrdiff_t> const *, float const *, float *,
    std::size_t) const;
extern template void Mesh<double>::barycentricInterpolation(
    BarycentricPoint<double, std::ptrdiff_t> const *,
    BarycentricPoint<double, std::ptrdiff_t> const *, double const *, double *,
    std::size_t) const;

extern template void Mesh<float>::cartesianRepresentation(
    BarycentricPoint<float, std::ptrdiff_t> const *,
    BarycentricPoint<float, std::ptrdiff_t> const *, float *) const;
extern template void Mesh<double>::cartesianRepresentation(
    BarycentricPoint<float, std::ptrdiff_t> const *,
    BarycentricPoint<float, std::ptrdiff_t> const *, double *) const;
extern template void Mesh<float>::cartesianRepresentation(
    std::vector<BarycentricPoint<float, std::ptrdiff_t>>::const_iterator,
    std::vector<BarycentricPoint<float, std::ptrdiff_t>>::const_iterator,
    float *) const;
extern template void Mesh<double>::cartesianRepresentation(
    std::vector<BarycentricPoint<double, std::ptrdiff_t>>::const_iterator,
    std::vector<BarycentricPoint<double, std::ptrdiff_t>>::const_iterator,
    double *) const;

extern template void
Mesh<float>::rayIntersections(Ray<float> const *, Ray<float> const *,
                              RayMeshIntersection<float> *) const;
extern template void Mesh<float>::rayIntersections(
    Ray<float> const *, Ray<float> const *,
    std::back_insert_iterator<std::vector<RayMeshIntersection<float>>>) const;
extern template void
Mesh<double>::rayIntersections(Ray<double> const *, Ray<double> const *,
                               RayMeshIntersection<double> *) const;
extern template void Mesh<double>::rayIntersections(
    Ray<double> const *, Ray<double> const *,
    std::back_insert_iterator<std::vector<RayMeshIntersection<double>>>) const;

} // namespace CortidQCT
