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

struct MockVolume {
  std::array<float, 8> data{{1.f, 2.f, 11.f, 12.f, 101.f, 102.f, 111.f, 112.f}};

  template <class F> auto withUnsafeDataPointer(F &&f) const {
    return f(data.data());
  }
};

struct MockSampler : public SamplerBase<MockVolume, MockSampler> {
  using Base = SamplerBase<MockVolume, MockSampler>;
  template <class Derived>
  int toLinear(Eigen::MatrixBase<Derived> const &x) const {
    Expects(x.rows() == 1 && x.cols() == 3);

    return x(2) * 4 + x(1) * 2 + x(0);
  }

  template <class Derived>
  bool isOutside(Eigen::MatrixBase<Derived> const &x) const {
    return !(x(0) >= 0 && x(0) <= 1 && x(1) >= 0 && x(1) <= 1 && x(2) >= 0 &&
             x(2) <= 1);
  }

  template <class Derived>
  float outsideValue(Eigen::MatrixBase<Derived> const & /*unused*/) const {
    return std::numeric_limits<float>::quiet_NaN();
  }

  MockSampler(MockVolume const &vol) : Base(vol) {}
};

struct MockSampler2 : public SamplerBase<MockVolume, MockSampler2> {

  using Base = SamplerBase<MockVolume, MockSampler2>;

  MockSampler2(MockVolume const &v) : Base(v) {}

  float valueTransform(float x) const { return std::sqrt(x); }

  float inverseValueTransform(float x) const { return x * x; }

  template <class Derived>
  int toLinear(Eigen::MatrixBase<Derived> const &x) const {
    Expects(x.rows() == 1 && x.cols() == 3);

    return x(2) * 4 + x(1) * 2 + x(0);
  }

  template <class Derived>
  bool isOutside(Eigen::MatrixBase<Derived> const &x) const {
    return !(x(0) >= 0 && x(0) <= 1 && x(1) >= 0 && x(1) <= 1 && x(2) >= 0 &&
             x(2) <= 1);
  }

  template <class Derived>
  float outsideValue(Eigen::MatrixBase<Derived> const & /*unused*/) const {
    return std::numeric_limits<float>::quiet_NaN();
  }

  template <class Derived>
  auto coordTransform(Eigen::MatrixBase<Derived> const &x) const {
    return ((x.array().rowwise() *
             Eigen::Vector3f{0.5f, 0.4f, 0.2f}.transpose().array())
                .matrix())
        .eval();
  }
};

TEST(InternalSampler, NoInterpolation) {
  using Eigen::Vector3f;
  MockVolume const vol;
  MockSampler const sampler(vol);

  ASSERT_FLOAT_EQ(1.f, sampler(Vector3f::Zero().transpose())(0));
  ASSERT_FLOAT_EQ(2.f, sampler(Vector3f{1, 0, 0}.transpose())(0));
  ASSERT_FLOAT_EQ(11.f, sampler(Vector3f{0, 1, 0}.transpose())(0));
  ASSERT_FLOAT_EQ(12.f, sampler(Vector3f{1, 1, 0}.transpose())(0));
  ASSERT_FLOAT_EQ(101.f, sampler(Vector3f{0, 0, 1}.transpose())(0));
  ASSERT_FLOAT_EQ(102.f, sampler(Vector3f{1, 0, 1}.transpose())(0));
  ASSERT_FLOAT_EQ(111.f, sampler(Vector3f{0, 1, 1}.transpose())(0));
  ASSERT_FLOAT_EQ(112.f, sampler(Vector3f{1, 1, 1}.transpose())(0));
}

TEST(InternalSampler, Interpolation) {
  using Eigen::VectorXf;
  MockVolume const vol;
  MockSampler const sampler(vol);

  Eigen::MatrixXf positions(9, 3);
  positions << 0.5f, 0.f, 0.f, // pos 0
      0.f, 0.5f, 0.f,          // pos 1
      0.5f, 1.f, 0.f,          // pos 2
      0.5f, 0.5f, 0.f,         // pos 3
      0.5f, 0.f, 1.f,          // pos 4
      0.f, 0.5f, 1.f,          // pos 5
      0.5f, 1.f, 1.f,          // pos 6
      0.5f, 0.5f, 1.f,         // pos 7
      0.5f, 0.5f, 0.5f;        // pos 8

  VectorXf const values = sampler(positions);

  ASSERT_FLOAT_EQ(1.5f, values(0));
  ASSERT_FLOAT_EQ(6.f, values(1));
  ASSERT_FLOAT_EQ(11.5f, values(2));
  ASSERT_FLOAT_EQ(6.5f, values(3));
  ASSERT_FLOAT_EQ(101.5f, values(4));
  ASSERT_FLOAT_EQ(106.f, values(5));
  ASSERT_FLOAT_EQ(111.5f, values(6));
  ASSERT_FLOAT_EQ(106.5f, values(7));
  ASSERT_FLOAT_EQ(56.5f, values(8));
}

TEST(InternalSampler, Outside) {
  using Eigen::Vector3f;
  using std::isfinite;
  MockVolume const vol;
  MockSampler const sampler(vol);

  ASSERT_FALSE(isfinite(sampler(Vector3f{1.001f, 0, 0}.transpose())(0)));
  ASSERT_FALSE(isfinite(sampler(Vector3f{-0.001f, 0, 0}.transpose())(0)));

  ASSERT_FALSE(isfinite(sampler(Vector3f{0, 1.00001f, 0}.transpose())(0)));
  ASSERT_FALSE(isfinite(sampler(Vector3f{0, -0.0001f, 0}.transpose())(0)));

  ASSERT_FALSE(isfinite(sampler(Vector3f{0, 0, 1.00001f}.transpose())(0)));
  ASSERT_FALSE(isfinite(sampler(Vector3f{0, 0, -0.0001f}.transpose())(0)));
}

TEST(InternalSampler, CoordAndValueTransform) {
  using Eigen::Vector3f;
  using std::isfinite;
  MockVolume const vol;
  MockSampler2 const sampler(vol);

  ASSERT_FLOAT_EQ(107.1167587262f,
                  sampler(Vector3f{2.f, 1.25f, 5.f}.transpose())(0));
}
