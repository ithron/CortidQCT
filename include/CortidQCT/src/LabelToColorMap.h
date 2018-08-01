/**
 * @file      LabelToColorMap.h
 *
 * @brief     This finle contains definitions for label to color map types.
 *
 * @author    Stefan Reinhold
 * @copyright
 *            Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <functional>

namespace CortidQCT {

/**
 * @brief Label to color map
 *
 * A function mapping an integer label to a 3-component floating point RGB
 * color. Takes the label as an argument and returns a std::array<Scalar, 3>
 * RGB color.
 *
 * @tparam Scalar Scalar type of color components
 * @tparam Label Label type
 */
template <class Scalar, class Label>
using LabelToColorMap = std::function<std::array<Scalar, 3>(Label)>;

/// Namespace containing label to color maps
namespace LabelToColorMaps {

/**
 * @brief Default label to color map
 *
 * Inverse of `ColorToLabelMaps::defaultMap()`.
 *
 * @tparam Scalar Scalar type of color components
 * @tparam Label Label type, must be construtible from 64 bit unsigned
 * integer
 * @param label the label
 * @return RGB color
 */
template <class Scalar, class Label>
inline std::array<Scalar, 3> defaultMap(Label label) {
  auto const convertedLabel = static_cast<std::uint64_t>(label);

  // Ensure the upper 5 bytes are all zero
  if ((convertedLabel & 0xffffffffff000000) != 0) {
    throw std::invalid_argument("Given label cannot losslessly be converted to "
                                "24-bit unsigned integer");
  }

  auto const redScaled = convertedLabel & 0xff;
  auto const greenScaled = (convertedLabel & 0xff00) >> 8;
  auto const blueScaled = (convertedLabel & 0xff0000) >> 16;

  auto const red = static_cast<Scalar>(redScaled) / Scalar{255};
  auto const green = static_cast<Scalar>(greenScaled) / Scalar{255};
  auto const blue = static_cast<Scalar>(blueScaled) / Scalar{255};

  return std::array<Scalar, 3>{{red, green, blue}};
}

} // namespace LabelToColorMaps

} // namespace CortidQCT
