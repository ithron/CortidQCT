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

#include <array>
#include <cassert>
#include <functional>
#include <unordered_map>

namespace CortidQCT {

/// 24-bit RGB color type
using ColorRGB = std::array<std::uint8_t, 3>;

} // namespace CortidQCT

namespace std {

template <> struct hash<CortidQCT::ColorRGB> {

  using argument_type = CortidQCT::ColorRGB;
  using result_type = std::size_t;

  result_type operator()(argument_type const &comp) const noexcept {
    auto const r = static_cast<std::uint32_t>(comp[0]);
    auto const g = static_cast<std::uint32_t>(comp[1]);
    auto const b = static_cast<std::uint32_t>(comp[2]);
    auto const val = r << 16 | g << 8 | b;

    return hash<std::uint32_t>{}(val);
  }
};

} // namespace std

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

/**
 * @brief A customaizable color to label map
 */
struct CustomMap {

  using LabelType = std::uint64_t;

  using Table = std::unordered_map<ColorRGB, LabelType>;

  /**
   * @brief mapping table
   *
   * Maps RGB colors to labels. Defautl value maps the colors
   * red, green, yellow, green, magenta, cyan to 0, 1, 2, 3, 4, 5,
   * respectively.
   */
  Table table = {{{{255, 0, 0}}, 0},   {{{0, 255, 0}}, 1},
                 {{{255, 255, 0}}, 2}, {{{0, 0, 255}}, 3},
                 {{{255, 0, 255}}, 4}, {{{0, 255, 255}}, 5}};

  /// The label that is returned when no matching entry in the table can be
  /// found
  LabelType undefinedLabel{0};

  /// Return a ColorToLabelMap representing the current map
  template <class Scalar, class Label>
  inline operator ColorToLabelMap<Label, Scalar>() const {
    return [map = *this](Scalar r, Scalar g, Scalar b) -> Label {
      return map.operator()<Label>(r, g, b);
    };
  }

  /**
   * @brief Converts a RGB color to a label using the pre-defined
   * lookup table
   *
   * @tparam Scalar scalar type of the color components (floating point)
   * @tparam Label label type, must be constructible from std::unit64_t
   * @param red red compnent in range [0, 1]
   * @param green green component in range [0, 1]
   * @param blue blue component in range [0, 1]
   * @return Label value from the lookup table. If no matching entry can be
   * found in the table, `undefinedLabel` is returned.
   */
  template <class Label, class Scalar>
  inline Label operator()(Scalar red, Scalar green, Scalar blue) const {
    assert(red >= Scalar{0} && red <= Scalar{1});
    assert(green >= Scalar{0} && green <= Scalar{1});
    assert(blue >= Scalar{0} && blue <= Scalar{1});

    auto const redScaled = static_cast<std::uint8_t>(red * Scalar{255});
    auto const greenScaled = static_cast<std::uint8_t>(green * Scalar{255});
    auto const blueScaled = static_cast<std::uint8_t>(blue * Scalar{255});

    auto const components = ColorRGB{{redScaled, greenScaled, blueScaled}};

    if (auto const labelIt = table.find(components); labelIt != table.end()) {
      return static_cast<Label>(labelIt->second);
    }

    return Label{static_cast<Label>(undefinedLabel)};
  }

  /**
   * @brief Loads the custom map from a YAML file
   * @param filename Path to the yaml file
   * @return Reference to the loaded map
   * @throws std::invalid_argument if loading fails
   */
  CustomMap &loadFromFile(std::string const &filename);

  /**
   * @brief Convenience initializer to load custom map from file
   * @see loadFromFile
   */
  inline static CustomMap fromFile(std::string const &filename) {
    return CustomMap{}.loadFromFile(filename);
  }
};

} // namespace ColorToLabelMaps

} // namespace CortidQCT
