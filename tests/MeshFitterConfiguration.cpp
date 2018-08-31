/**
 * @file      MeshFitterConfiguration.cpp
 *
 * @brief     Test cases for MeshFitter::Configuration
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
    std::string(CortidQCT_DATADIR) + "/testConfig.yml";

TEST(MeshFitterConfiguration, DefaultParameters) {
  auto const config = MeshFitter::Configuration{};

  ASSERT_DOUBLE_EQ(2.0, config.sigmaE);
  ASSERT_DOUBLE_EQ(2.0, config.sigmaS);

  ASSERT_TRUE(config.model.isEmpty());
  ASSERT_TRUE(config.referenceMesh.isEmpty());
}

TEST(MeshFitterConfiguration, LoadFromFile) {

  auto config = MeshFitter::Configuration{};

  ASSERT_NO_THROW(config.loadFromFile(file1));

  ASSERT_DOUBLE_EQ(3.1415, config.sigmaE);
  ASSERT_DOUBLE_EQ(1.5, config.sigmaS);

  ASSERT_FALSE(config.model.isEmpty());
  ASSERT_FALSE(config.referenceMesh.isEmpty());
}
