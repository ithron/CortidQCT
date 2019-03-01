/**
 * @file      Ray.h
 *
 * @brief     Definition of Ray data structure
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

/**
 * @brief A basic ray datatype
 */
template <class T> struct alignas(8) Ray {

  static_assert(std::is_floating_point<T>::value,
                "Scalar must be a floating point type");

  /// The origin of the ray in cartesian coordinates (x, y, z)
  std::array<T, 3> origin;
  /// The (normalized) direction of the ray (dx, dy,d z)
  std::array<T, 3> direction;
};

/// Alias for double precision rays
using Rayd = Ray<double>;

/// Lias for single precision rays
using Rayf = Ray<float>;

} // namespace CortidQCT

