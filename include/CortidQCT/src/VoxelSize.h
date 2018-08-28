/**
 * @file      VoxelSize.h
 *
 * @brief     This header contains the definition of the VoxelSize type
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <cassert>
#include <cstddef>

namespace CortidQCT {

/**
 * @brief Size type for a 3D voxel
 */
struct VoxelSize {
  /// Size along the x-axis
  float width = 0;
  /// Size along the y-axis
  float height = 0;
  /// Size along the z-axis
  float depth = 0;

  /// @brief subscript operator for dimension access
  ///
  /// Order is:
  ///   - 0: width
  ///   - 1: height
  ///   - 2: depth
  ///
  /// @param idx Index
  /// @pre `idx >= 0 && idx < 3`
  /// @return value of the indexed dimension
  inline constexpr float operator[](std::size_t idx) const noexcept {
    assert(idx < 3 && "Index out of bounds");
    switch (idx) {
      case 0:
        return width;
      case 1:
        return height;
      case 2:
        return depth;
      default:
        return width; // Just to return anything
    }
  }

  /// @brief subscript operator for dimension access
  ///
  /// Order is:
  ///   - 0: width
  ///   - 1: height
  ///   - 2: depth
  ///
  /// @param idx Index
  /// @pre `idx >= 0 && idx < 3`
  /// @return reference to indexed dimension
  inline float &operator[](std::size_t idx) noexcept {
    assert(idx < 3 && "Index out of bounds");
    switch (idx) {
      case 0:
        return width;
      case 1:
        return height;
      case 2:
        return depth;
      default:
        return width; // Just to return anything
    }
  }
};

} // namespace CortidQCT
