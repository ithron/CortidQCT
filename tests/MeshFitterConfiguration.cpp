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
static std::string const file2 =
    std::string(CortidQCT_DATADIR) + "/testConfig2.yml";

TEST(MeshFitterConfiguration, DefaultParameters) {
  auto const config = MeshFitter::Configuration{};

  ASSERT_DOUBLE_EQ(5.4, config.sigmaE);
  ASSERT_DOUBLE_EQ(2.0, config.sigmaS);

  ASSERT_TRUE(config.model.isEmpty());
  ASSERT_TRUE(config.referenceMesh.isEmpty());

  ASSERT_TRUE(std::holds_alternative<MeshFitter::Configuration::OriginType>(
      config.referenceMeshOrigin));
  ASSERT_EQ(MeshFitter::Configuration::OriginType::untouched,
            std::get<MeshFitter::Configuration::OriginType>(
                config.referenceMeshOrigin));
}

TEST(MeshFitterConfiguration, LoadFromFile) {

  auto config = MeshFitter::Configuration{};

  ASSERT_NO_THROW(config.loadFromFile(file1));

  ASSERT_DOUBLE_EQ(3.1415, config.sigmaE);
  ASSERT_DOUBLE_EQ(1.5, config.sigmaS);

  ASSERT_FALSE(config.model.isEmpty());
  ASSERT_FALSE(config.referenceMesh.isEmpty());

  ASSERT_TRUE(std::holds_alternative<MeshFitter::Configuration::OriginType>(
      config.referenceMeshOrigin));
  ASSERT_EQ(MeshFitter::Configuration::OriginType::centered,
            std::get<MeshFitter::Configuration::OriginType>(
                config.referenceMeshOrigin));
}

TEST(MeshFitterConfiguration, LoadFromFileWithColorMap) {

  auto config = MeshFitter::Configuration{};

  ASSERT_NO_THROW(config.loadFromFile(file2));

  ASSERT_DOUBLE_EQ(3.1415, config.sigmaE);
  ASSERT_DOUBLE_EQ(1.5, config.sigmaS);

  ASSERT_FALSE(config.model.isEmpty());
  ASSERT_FALSE(config.referenceMesh.isEmpty());

  constexpr std::array<int, 5> refCounts{{1036, 1072, 392, 192, 0}};
  std::array<int, 5> counts{{0, 0, 0, 0, 0}};

  auto const mesh = config.referenceMesh;

  mesh.withUnsafeLabelPointer([&mesh, &counts](auto const *pLabels) {
    for (auto &&label : gsl::make_span(
             pLabels, gsl::narrow<std::ptrdiff_t>(mesh.vertexCount()))) {
      ASSERT_GE(label, 0);
      ASSERT_LE(label, 4);

      ++counts[gsl::narrow<std::size_t>(label)];
    }
  });

  ASSERT_EQ(refCounts, counts);

  ASSERT_TRUE(std::holds_alternative<MeshFitter::Configuration::OriginType>(
      config.referenceMeshOrigin));

  auto const origin = std::get<MeshFitter::Configuration::OriginType>(
      config.referenceMeshOrigin);

  ASSERT_EQ(MeshFitter::Configuration::OriginType::centered, origin);
}
