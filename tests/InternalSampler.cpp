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
  using Eigen::VectorXd;
  auto const model = MeasurementModel::fromFile(file1);
  auto const sampler = ModelSampler{model};

  MatrixXf positions(2, 4);
  positions << 0.f, 350.f, 14.5f, 0.f, // pos 0
      4.f, 350.f, 14.5f, 0.f;          // pos 1

  VectorXd const values = sampler(positions, .0f);

  ASSERT_DOUBLE_EQ(2.2553976242465152, values(0));
  ASSERT_DOUBLE_EQ(3.2553976242465152, values(1));
}
