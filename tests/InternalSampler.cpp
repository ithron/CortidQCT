/**
 * @file      InternalSampler.cpp
 *
 * @brief     Test cases for internal Sampler type
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "tests_config.h"

#include "Sampler.h"

#include <CortidQCT/CortidQCT.h>

#include <gtest/gtest.h>

#include <limits>

using namespace CortidQCT;

#ifndef CortidQCT_DATADIR
#  error "No data dir given"
#endif

using namespace CortidQCT;

static std::string const file1 =
    std::string(CortidQCT_DATADIR) + "/testModel.yml";

TEST(InternalSampler, ModelSampler) {
  using Eigen::MatrixXf;
  using Eigen::Vector4f;
  using Eigen::VectorXf;
  auto const model = MeasurementModel::fromFile(file1);
  auto const sampler = ModelSampler{model};

  MatrixXf positions(2, 4);
  positions << 0.f, 350.f, 14.5f, 0.f, // pos 0
      4.f, 350.f, 14.5f, 0.f;          // pos 1

  VectorXf const values = sampler(positions, .0f);

  std::cout << values.transpose() << std::endl;
  std::cout << sampler(Vector4f{.0f, 100.f, 10.f, .0f}.transpose(), .0f) << std::endl;

  ASSERT_FLOAT_EQ(0.81167072, values(0));
  ASSERT_FLOAT_EQ(1.17916775, values(1));
}
