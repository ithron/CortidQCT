/**
 * @file      VolumeSize.h
 *
 * @brief     This header contains the definition of the VolumeSize type
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
struct VolumeSize {
  /// Size along the x-axis
  std::size_t width = 0;
  /// Size along the y-axis
  std::size_t height = 0;
  /// Size along the z-axis
  std::size_t depth = 0;

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
  inline std::size_t &operator[](std::size_t idx) noexcept {
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

  /// @brief Returns the linear size (i.e. width * height * depth)
  inline constexpr std::size_t linear() const noexcept {
    return width * height * depth;
  }

  /// @name Comparison operators
  /// @{

  inline constexpr bool operator==(VolumeSize const &rhs) const {
    return width == rhs.width && height == rhs.height && depth == rhs.depth;
  }

  inline constexpr bool operator!=(VolumeSize const &rhs) const {
    return !(*this == rhs);
  }

  /// @}
};

} // namespace CortidQCT
