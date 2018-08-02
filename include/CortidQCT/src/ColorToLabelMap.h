/**
 * @file      ColorToLabelMap.h
 *
 * @brief     This finle contains definitions for color to index map types.
 *
 * @author    Stefan Reinhold
 * @copyright
 *            Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <functional>
#include <cassert>

namespace CortidQCT {

/**
 * @brief Color to label map
 *
 * A function mapping 3-component floating point RGB colors to integer
 * labels. Takes three arguments of type `Scalar`: red, green, blue, Must return
 * a label of type `Label`.
 *
 * @tparam Label Label type
 * @tparam Scalar Scalar type of color components
 */
template <class Label, class Scalar>
using ColorToLabelMap = std::function<Label(Scalar, Scalar, Scalar)>;

/// Namespace containing color to label maps
namespace ColorToLabelMaps {

/**
 * @brief Default color to label map
 *
 * Converts RGB color to labels by treating each color value as 8 bit
 * components of a multi byte integerm where the red components represents
 * the least significant byte and the blue component the most significant
 * byte.
 *
 * @tparam Label Label type, must be construtible from 64 bit unsigned
 * integer
 * @tparam Scalar Scalar type of color components
 * @param red red component
 * @param green green component
 * @param blue blue component
 * @return label as blue * 256^2 + green * 256 + red
 */
template <class Label, class Scalar>
inline Label defaultMap(Scalar red, Scalar green, Scalar blue) {
  assert(red >= Scalar{0} && red <= Scalar{1});
  assert(green >= Scalar{0} && green <= Scalar{1});
  assert(blue >= Scalar{0} && blue <= Scalar{1});

  auto const redScaled = static_cast<std::uint64_t>(red * Scalar{255});
  auto const greenScaled = static_cast<std::uint64_t>(green * Scalar{255});
  auto const blueScaled = static_cast<std::uint64_t>(blue * Scalar{255});

  auto const combined = blueScaled << 16 | greenScaled << 8 | redScaled;

  return static_cast<Label>(combined);
}

} // namespace ColorToLabelMaps

} // namespace CortidQCT
