/**
 * @file      CustomColorToLabelMap.cpp
 *
 * @brief     Test cases for custom color to label map
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "tests_config.h"

#include <CortidQCT/CortidQCT.h>

#include <gtest/gtest.h>

#ifndef CortidQCT_DATADIR
#  error "No data dir given"
#endif

using namespace CortidQCT;

static std::string const file1 =
    std::string(CortidQCT_DATADIR) + "/testColorMap.yml";

static std::string const file2 =
    std::string(CortidQCT_DATADIR) + "/testColorMapExt.yml";

constexpr static ColorRGB testColors[] = {
    {{0, 0, 0}},   {{255, 0, 0}},   {{0, 255, 0}},   {{255, 255, 0}},
    {{0, 0, 255}}, {{255, 0, 255}}, {{0, 255, 255}}, {{255, 255, 255}}};

inline constexpr std::array<float, 3> colorToFlt(ColorRGB const &c) {
  return {{static_cast<float>(c[0]) / 255.f, static_cast<float>(c[1]) / 255.f,
           static_cast<float>(c[2]) / 255.f}};
}

TEST(CustomColorToLabelMap, DefaultMap) {
  auto const map = ColorToLabelMaps::CustomMap{};

  ASSERT_EQ(0, map.undefinedLabel);

  auto const c0 = colorToFlt(testColors[1]);
  auto const c1 = colorToFlt(testColors[2]);
  auto const c2 = colorToFlt(testColors[3]);
  auto const c3 = colorToFlt(testColors[4]);
  auto const c4 = colorToFlt(testColors[5]);
  auto const c5 = colorToFlt(testColors[6]);

  ColorToLabelMap<int, float> const castedMap = map;
  auto const callMap = [&](auto const &c) {
    return castedMap(c[0], c[1], c[2]);
  };

  ASSERT_EQ(0, callMap(c0));
  ASSERT_EQ(1, callMap(c1));
  ASSERT_EQ(2, callMap(c2));
  ASSERT_EQ(3, callMap(c3));
  ASSERT_EQ(4, callMap(c4));
  ASSERT_EQ(5, callMap(c5));
}

TEST(CustomColorToLabelMap, LoadFromFile) {
  auto map = ColorToLabelMaps::CustomMap{};

  ASSERT_NO_THROW(map.loadFromFile(file1));

  ColorToLabelMap<int, float> const castedMap = map;

  ASSERT_EQ(0, map.undefinedLabel);

  ASSERT_EQ(42, castedMap(1.f, 0.f, 0.f));
  ASSERT_EQ(23, castedMap(128.f / 255.f, 128.f / 255.f, 1.f / 255.f));

  ASSERT_EQ(0, castedMap(0.5f, 0.4f, 0.3f));
}

TEST(CustomColorToLabelMap, LoadFromFileExt) {
  auto map = ColorToLabelMaps::CustomMap{};

  try {
    map.loadFromFile(file2);
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  ASSERT_NO_THROW(map.loadFromFile(file2));

  ColorToLabelMap<int, float> const castedMap = map;

  ASSERT_EQ(5, map.undefinedLabel);

  ASSERT_EQ(42, castedMap(1.f, 0.f, 0.f));
  ASSERT_EQ(23, castedMap(128.f / 255.f, 128.f / 255.f, 1.f / 255.f));

  ASSERT_EQ(5, castedMap(0.5f, 0.4f, 0.3f));
}
