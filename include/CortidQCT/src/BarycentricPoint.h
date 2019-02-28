/**
 * @file      BarycentricPoint.h
 *
 * @brief     This header contains the definition of the BarycentricPoint data
 * type.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2019 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <array>
#include <type_traits>

namespace CortidQCT {

template <class T> class Mesh;

/**
 * @brief BarycentricPoint data type. Represents a points on triangulation in
 * barycentric coordinates
 */
template <class T, class Index> struct alignas(8) BarycentricPoint {

  static_assert(std::is_floating_point<T>::value,
                "Scalar must be a floating point type");

  static_assert(std::is_integral<Index>::value,
                "Index must be a integral type");

  /// Barycentric coordinates
  std::array<T, 2> uv;
  /// Triangle index
  Index triangleIndex;
};

} // namespace CortidQCT
