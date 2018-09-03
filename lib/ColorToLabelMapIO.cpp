/**
 * @file      ColorToLabelMapIO.cpp
 *
 * @brief     Color to label maps IO functions implementations
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "ColorToLabelMapIO.h"

#include <gsl/gsl>
#include <yaml-cpp/yaml.h>

#include <exception>

namespace CortidQCT {
namespace ColorToLabelMaps {

CustomMap &CustomMap::loadFromFile(std::string const &filename) {
  try {
    auto const node = YAML::LoadFile(filename);

    *this = IO::loadCustomMapFromYAMLNode(node);

  } catch (YAML::Exception const &e) {
    throw std::invalid_argument(
        "Failed to load color to label map from file '" + filename +
        "': " + e.what());
  }

  return *this;
}

namespace IO {

CustomMap loadCustomMapFromYAMLNode(YAML::Node const &node) {
  auto table = CustomMap::Table{};

  if (!node.IsSequence() && !node.IsMap()) {
    throw std::invalid_argument(
        "Custom color map node must be a sequence or a map");
  }

  auto const &dataNode = node.IsSequence() ? node : node["map"];

  if (!dataNode) { throw std::invalid_argument("Missing 'map' entry"); }

  for (auto &&entryNode : dataNode) {
    if (!entryNode.IsSequence() || entryNode.size() != 2) {
      throw std::invalid_argument(
          "Color table entry must be a sequence of length 2");
    }

    auto const &colorNode = *entryNode.begin();
    auto const &labelNode = *(++entryNode.begin());

    if (!labelNode.IsScalar()) {
      throw std::invalid_argument("Label entry must be scalar");
    }

    auto const color = loadColorRGBFromYAMLNode(colorNode);
    auto const label = gsl::narrow<CustomMap::LabelType>(
        labelNode.as<unsigned long long>()); // NOLINT

    table.insert({color, label});
  }

  auto customMap = CustomMap{table};

  if (auto const &undefNode = node["undefinedLabel"]) {
    customMap.undefinedLabel = gsl::narrow<CustomMap::LabelType>(
        undefNode.as<unsigned long long>()); // NOLINT
  }

  return customMap;
}

ColorRGB loadColorRGBFromYAMLNode(YAML::Node const &node) {
  if (!node.IsSequence() || node.size() != 3) {
    throw std::invalid_argument("A color entry must be a sequence of length 3");
  }

  if (!node.begin()->IsScalar()) {
    throw std::invalid_argument("Color components must be scalar");
  }

  auto const r = node.begin()->as<unsigned int>();
  auto const g = (++node.begin())->as<unsigned int>();
  auto const b = (++++node.begin())->as<unsigned int>();

  auto const color =
      ColorRGB{{gsl::narrow<unsigned char>(r), gsl::narrow<unsigned char>(g),
                gsl::narrow<unsigned char>(b)}};

  return color;
}

} // namespace IO
} // namespace ColorToLabelMaps
} // namespace CortidQCT
