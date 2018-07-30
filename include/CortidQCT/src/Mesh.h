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

#include <stdexcept>
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

  /// @name Accessors
  /// @{

  /// Number of vertices
  inline Size vertexCount() noexcept {
    assert(vertexData_.size() % 3 == 0);
    return vertexData_.size() / 3;
  }

  /// Number of triangles
  inline std::size_t triangleCount() noexcept {
    assert(indexData_.size() % 3 == 0);
    return indexData_.size() / 3;
  }

  /// @brief true iff the mesh is empty
  ///
  /// The mesh is empty iff the triangle or the vertex count is zero
  inline bool isEmpty() noexcept {
    return triangleCount() == 0 || vertexCount() == 0;
  }
  /// @}

  /// @name IO
  /// @{

  /// @brief Load mesh from ASCII file using format auto detecttion
  ///
  /// Supported file formats are: obj, off, stl, wrl, ply, mesh.
  ///
  /// @param filename Path to the file to load the mesh from
  /// @return Reference to the loaded mesh
  /// @throws std::invalid_argument if the mesh could not be loaded from the
  /// given filename
  Mesh &loadFromFile(std::string const &filename);

  /// @brief Writes mesh to ASCII file using format auto detection
  ///
  /// Supported file formats are: obj, off, stl, wrl, ply, mesh
  ///
  /// @param filename to the file to write the mesh to
  /// @throws std::invalid_argument if the mesh could nto be writted to the
  /// given file
  void writeToFile(std::string const &filename) const;

  /// @}

private:
  /// Stores vertex coordinates in column major order
  std::vector<Scalar> vertexData_;
  /// Stores per triangle vertex indices in column major oder
  std::vector<Index> indexData_;
};

} // namespace CortidQCT
