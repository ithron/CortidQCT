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

#include "ColorToLabelMap.h"
#include "LabelToColorMap.h"

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
 * @nosubgrouping
 */
template <class T> class Mesh {

  static_assert(std::is_floating_point_v<T>,
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
  inline Mesh() noexcept(noexcept(VertexData()) &&
                         noexcept(IndexData())) = default;

  // @}

  /// @name Accessors
  /// @{

  /// Number of vertices
  inline Size vertexCount() const noexcept {
    return vertexData_.size() / 3;
  }

  /// Number of triangles
  inline Size triangleCount() const noexcept {
    return indexData_.size() / 3;
  }

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
  /// Supported file formats are: obj, off, stl, wrl, ply, mesh.
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
  /// Supported file formats are: off (COFF).
  /// Per-vertex colors are converted to labels using the given colormap.
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
  /// Supported file formats are: obj, off, stl, wrl, ply, mesh.
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
  /// Supported file formats are: off (coff).
  /// The labels are encoded in per-vertex colors using the given label to
  /// color map.
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
  inline std::invoke_result_t<F, Scalar const *> withUnsafeVertexPointer(F &&f) const
      noexcept(noexcept(f(vertexData_.data()))) {
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
  inline std::invoke_result_t<F, Index const *> withUnsafeIndexPointer(F &&f) const
      noexcept(noexcept(f(indexData_.data()))) {
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
  inline std::invoke_result_t<F, Label const *> withUnsafeLabelPointer(F &&f) const
      noexcept(noexcept(f(labelData_.data()))) {
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

} // namespace CortidQCT
