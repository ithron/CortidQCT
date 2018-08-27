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

TEST(VoxelVolume, EmptyVolumeIsEmpty) {
  VoxelVolume const emptyVolume;

  ASSERT_TRUE(emptyVolume.isEmpty());
  ASSERT_EQ(0, emptyVolume.volumeSize().width);
  ASSERT_EQ(0, emptyVolume.volumeSize().height);
  ASSERT_EQ(0, emptyVolume.volumeSize().depth);
}
