/**
 * @file      VoxelVolume.cpp
 *
 * @brief     Implementation file for the VoxelVolume type.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "VoxelVolume.h"

#include <exception>

namespace CortidQCT {

VoxelVolume &VoxelVolume::loadFromFile(std::string const & /*unused*/) {
  throw std::invalid_argument("Unsupported file type");
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code-return"
  return *this;
#pragma clang diagnostic pop
}

} // namespace CortidQCT
