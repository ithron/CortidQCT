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
#include "CheckExtension.h"
#include "lib_config.h"

#include <exception>
#include <gsl/gsl>

#ifdef CORTIDQCT_WITH_IMAGESTACK
#  include "optional/LoadFromBST.h"
#endif

namespace CortidQCT {

VoxelVolume &VoxelVolume::loadFromFile(std::string const &filename) {
  using namespace std::string_literals;

  std::string const supportedExtensions[] = {
#ifdef CORTIDQCT_WITH_IMAGESTACK
      "bst",
#endif
      ""};

  constexpr auto numSupportedTypes =
      sizeof(supportedExtensions) / sizeof(std::string) - 1;

  static_assert(numSupportedTypes > 0, "No voxel volume filetype configured.");

  if (!IO::checkExtensions(filename, supportedExtensions)) {
    throw std::invalid_argument("Unsupported file type");
  }

  auto const extension = IO::extension(filename);

  auto closure = [this](float const *data, VolumeSize const &volumeSize,
                        VoxelSize const &voxelSize) mutable {
    using std::copy;
    auto const linearSize = volumeSize.linear();

    auto const dataSpan =
        gsl::make_span(data, gsl::narrow<std::ptrdiff_t>(linearSize));

    auto destData = std::vector<float>(dataSpan.begin(), dataSpan.end());

    voxelData_ = std::move(destData);
    volumeSize_ = volumeSize;
    voxelSize_ = voxelSize;
  };

  // Try to load data form file
  try {
    // Use a dummy if here to make it possible all following if statements can
    // use `else if(...)`
    if (numSupportedTypes == 0) {
    }

    // Now check for each supported file type
#ifdef CORTIDQCT_WITH_IMAGESTACK
    else if (extension == "bst") {
      IO::loadFromBST(filename, closure);
    }
#endif
    else {
      // If got here there must have been a programming error in the code above.
      Ensures(false);
    }

    Ensures(volumeSize_.linear() > 0);
    Ensures(!voxelData_.empty());

  } catch (std::runtime_error const &e) {
    throw std::invalid_argument("Failed to load voxel data from file: "s +
                                e.what());
  }

  return *this;
}

} // namespace CortidQCT
