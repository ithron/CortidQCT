/**
 * @file      VoxelVolume.cpp
 *
 * @brief     Test cases for VoxelVolume type
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "tests_config.h"

#include <CortidQCT/CortidQCT.h>

#include <gsl/gsl>
#include <gtest/gtest.h>

#ifndef CortidQCT_DATADIR
#  error "No data dir given"
#endif

using namespace CortidQCT;

static std::string const file1 =
    std::string(CortidQCT_DATADIR) + "/ascendingSlices.bst";

static constexpr auto volumeSize1 = VolumeSize{20, 40, 10};
static constexpr auto voxelSize1 = VoxelSize{0.25f, 0.5f, 1.0f};

TEST(VoxelVolume, EmptyVolumeIsEmpty) {
  VoxelVolume const emptyVolume;

  ASSERT_TRUE(emptyVolume.isEmpty());
  ASSERT_EQ(0, emptyVolume.volumeSize().width);
  ASSERT_EQ(0, emptyVolume.volumeSize().height);
  ASSERT_EQ(0, emptyVolume.volumeSize().depth);
}

TEST(VoxelVolume, LoadBST) {
  auto volume = VoxelVolume();

  ASSERT_NO_THROW(volume.loadFromFile(file1));

  ASSERT_FALSE(volume.isEmpty());

  ASSERT_EQ(volumeSize1, volume.volumeSize());

  ASSERT_FLOAT_EQ(voxelSize1.width, volume.voxelSize().width);
  ASSERT_FLOAT_EQ(voxelSize1.height, volume.voxelSize().height);
  ASSERT_FLOAT_EQ(voxelSize1.depth, volume.voxelSize().depth);

  volume.withUnsafeDataPointer([&](float const *dataPtr) {
    auto const size = volume.volumeSize();
    auto const data =
        gsl::make_span(dataPtr, gsl::narrow<std::ptrdiff_t>(size.linear()));

    auto idx = 0u;
    for (auto &&val : data) {
      auto const x = idx % size.width;
      auto const y = (idx / size.width) % size.height;
      auto const z = idx / (size.height * size.width);

      auto const ref = -2000.f + static_cast<float>(y) * 0.5f +
                       static_cast<float>(x) * 20.f +
                       static_cast<float>(z) * 400.f;

      ASSERT_FLOAT_EQ(ref, val);
      ++idx;
    }
  });
}

TEST(VoxelVolume, LoadFromFileThrowsOnUnknownFormat) {

  ASSERT_THROW(VoxelVolume{"non-existant-file.unsupportedExtension"},
               std::invalid_argument);
}

TEST(VoxelVolume, LoadFromFileThrowsOnNonExistentFile) {

  ASSERT_THROW(VoxelVolume{"non-existant-file.bst"},
               std::invalid_argument);
}

