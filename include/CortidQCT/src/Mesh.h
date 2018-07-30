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
