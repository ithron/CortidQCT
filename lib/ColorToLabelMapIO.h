/**
 * @file      ColorToLabelMapIO.h
 *
 * @brief     This header contains private IO functions for ColorToLabelMap
 * types
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "ColorToLabelMap.h"

namespace YAML {

class Node;

} // namespace YAML

namespace CortidQCT {

namespace ColorToLabelMaps {
namespace IO {

/**
 * @brief Reads a custom color to label map from the given yaml node
 *
 * The YAML format is as follows:
 * `[[R1, G1, B1], L1], [R2, G2, B2, L2]]`, where Rx, Gx, Bx, L, are the
 * x-th red, green, blue color components and label values, respectively. The
 * color components must be given as unsigned 8-bit integers in the range [0,
 * 255]. Optional the color map can be specified as a map type. Then the data
 * specification above will go under the 'map' key and optionally, a
 * undefinedLabel can be set.
 *
 * @param node The YAML node to read the map from
 * @throws YAML::Exception on failure.
 * @throws std::invalid_argument if the YAML node is malformed
 * @return A ColorToLabelMaps::CustomMap object.
 */
CustomMap loadCustomMapFromYAMLNode(YAML::Node const &node);

/**
 * @brief Reads a ColorRGB from a YAML node.
 *
 * The  YAML node must be a sequence of 3 unsigned integers
 *
 * @param node The YAML node to read the color from.
 * @throws YAML::Exception on parsing failure
 * @throws std::invalid_argument if the YAML node is malformed.
 * @return A ColorRGB value
 */
ColorRGB loadColorRGBFromYAMLNode(YAML::Node const &node);

} // namespace IO

} // namespace ColorToLabelMaps
} // namespace CortidQCT
