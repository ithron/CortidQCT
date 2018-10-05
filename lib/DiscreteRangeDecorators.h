/**
 * @file      DiscreteRangeDecorators.h
 *
 * @brief     This file contains some decorator functions for DiscreteRange
 * types
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 * You may use, distribute and modify this code under the terms of the
 * AFL 3.0 license; see LICENSE for full license details.
 */

#include "DiscreteRange.h"

#include <Eigen/Core>
#include <gsl/gsl>

namespace CortidQCT {
namespace Internal {

/**
 * @brief Returns all element of the given discrete range as a vector
 * @param range DiscreteRange object
 * @return An Eigen vector containing all element of `range`
 */
template <class T>
Eigen::Matrix<T, Eigen::Dynamic, 1>
discreteRangeElementVector(DiscreteRange<T> const &range) {
  using Vector = Eigen::Matrix<T, Eigen::Dynamic, 1>;

  Vector t(gsl::narrow<Eigen::Index>(range.numElements()));

  for (auto i = 0; i < t.rows(); ++i) {
    t(i) = range.min + static_cast<T>(i) * range.stride;
  }

  return t;
}

} // namespace Internal
} // namespace CortidQCT
