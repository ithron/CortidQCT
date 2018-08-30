/**
 * @file      MeasurementModel.cpp
 *
 * @brief     Test cases for MeasurementModel type
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
    std::string(CortidQCT_DATADIR) + "/testModel.yml";

TEST(MeasurementModel, EmptyModelIsEmpty) {
  auto const emptyModel = MeasurementModel{};

  ASSERT_TRUE(emptyModel.isEmpty());
}

TEST(MeasurementModel, TestDefaultParameters) {
  auto const emptyModel = MeasurementModel{};

  ASSERT_TRUE(!emptyModel.name);
  ASSERT_TRUE(!emptyModel.description);
  ASSERT_TRUE(!emptyModel.author);
  ASSERT_TRUE(!emptyModel.creationDate);

  ASSERT_FLOAT_EQ(1.f, emptyModel.kernelSigma);
  ASSERT_FLOAT_EQ(1.f, emptyModel.sliceSpacing);

  ASSERT_FLOAT_EQ(-2.f, emptyModel.samplingRange.min);
  ASSERT_FLOAT_EQ(2.f, emptyModel.samplingRange.max);
  ASSERT_FLOAT_EQ(0.1f, emptyModel.samplingRange.stride);

  ASSERT_FLOAT_EQ(-1000.f, emptyModel.densityRange.min);
  ASSERT_FLOAT_EQ(2000.f, emptyModel.densityRange.max);
  ASSERT_FLOAT_EQ(1.f, emptyModel.densityRange.stride);

  ASSERT_FLOAT_EQ(0.f, emptyModel.angleRange.min);
  ASSERT_FLOAT_EQ(90.f, emptyModel.angleRange.max);
  ASSERT_FLOAT_EQ(1.f, emptyModel.angleRange.stride);
}

TEST(MeasurementModel, LoadFromFile) {
  using namespace std::string_literals;

  auto model = MeasurementModel{};

  ASSERT_NO_THROW(model.loadFromFile(file1));

  ASSERT_FALSE(!model.name);
  ASSERT_EQ("Test Model"s, *model.name);

  ASSERT_FALSE(!model.description);
  ASSERT_EQ("A simple test model", *model.description);

  ASSERT_FALSE(!model.author);
  ASSERT_EQ("Stefan Reinhold", *model.author);

  ASSERT_FALSE(!model.creationDate);
  ASSERT_EQ("2018-08-30 15:37:00 +02", *model.creationDate);

  ASSERT_FLOAT_EQ(0.68f, model.kernelSigma);
  ASSERT_FLOAT_EQ(0.7f, model.sliceSpacing);

  ASSERT_FLOAT_EQ(-4.f, model.samplingRange.min);
  ASSERT_FLOAT_EQ(4.f, model.samplingRange.max);
  ASSERT_FLOAT_EQ(4.f, model.samplingRange.stride);

  ASSERT_FLOAT_EQ(-500.f, model.densityRange.min);
  ASSERT_FLOAT_EQ(1500.f, model.densityRange.max);
  ASSERT_FLOAT_EQ(600.f, model.densityRange.stride);

  ASSERT_FLOAT_EQ(10.f, model.angleRange.min);
  ASSERT_FLOAT_EQ(85.f, model.angleRange.max);
  ASSERT_FLOAT_EQ(18.f, model.angleRange.stride);

  auto const nSamples = model.samplingRange.numElements();
  auto const nDensities = model.densityRange.numElements();
  auto const nAngles = model.angleRange.numElements();

  // Check data
  model.withUnsafeDataPointer([&](double const *ptr) {
    double const *val = ptr;
    for (auto z = 1u; z <= nAngles; ++z) {
      for (auto y = 1u; y <= nDensities; ++y) {
        for (auto x = 1u; x <= nSamples; ++x) {
          auto const refVal = static_cast<double>(x) +
                              static_cast<double>(y) * 0.1 +
                              static_cast<double>(0.01) * z;
          ASSERT_DOUBLE_EQ(refVal, *val);
          ++val;
        }
      }
    }
  });
}
