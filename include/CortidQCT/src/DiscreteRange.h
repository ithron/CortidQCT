/**
 * @file      DiscreteRange.h
 *
 * @brief     This header contains the definition of the DiscreteRange
 * template.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <limits>
#include <type_traits>

namespace CortidQCT {

/**
 * @brief Type representing a discrete closed, stridable range
 *
 * @tparam T Underlying value type of the range
 */
template <class T> struct DiscreteRange {
  /// type of the elements contained in the range
  using value_type = T;

  /// minimum element
  value_type min;
  /// maximum element
  value_type max;
  /// stride, i.e. distance between two consecutive elements
  value_type stride;

  /// Constructs a discrete range
  inline constexpr DiscreteRange(value_type min_, value_type max_,
                                 value_type stride_)
      : min{min_}, max{max_}, stride{stride_} {}

  /// Constructs a discreate range with an default stride of 1
  ///
  /// @note Only implemented for integral types
  inline constexpr DiscreteRange(value_type min_, value_type max_)
      : min{min_}, max{max_}, stride{1} {
    static_assert(std::is_integral<value_type>::value,
                  "Default stride is only defined for integral types.");
  }
};

/// Alias for float valued ranges
using DiscreteRangef = DiscreteRange<float>;
/// Alias for double valued ranges
using DiscreteRanged = DiscreteRange<double>;

/// Alias for integer value ranges
using DiescreteRangei = DiscreteRange<int>;

} // namespace CortidQCT
