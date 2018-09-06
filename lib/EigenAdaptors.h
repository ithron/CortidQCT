/**
 * @file      EigenAdaptors.h
 *
 * @brief     This header contains adaptor functions for Eigen types
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <Eigen/Core>
#include <gsl/gsl>

#include <array>

namespace CortidQCT {
namespace Adaptor {

namespace Detail_ {

template <class T, class I>
constexpr inline auto subscript(T &&x,
                                I i) noexcept(noexcept(std::declval<T>()[i])) {
  return x[i];
}

template <class Derived, class I>
constexpr inline auto
subscript(Eigen::MatrixBase<Derived> const &x,
          I i) noexcept(noexcept(x(std::declval<Eigen::Index>()))) {
  return x(gsl::narrow_cast<Eigen::Index>(i));
}

template <class Derived, class I>
constexpr inline auto
subscript(Eigen::MatrixBase<Derived> &&x,
          I i) noexcept(noexcept(x(std::declval<Eigen::Index>()))) {
  return x(gsl::narrow_cast<Eigen::Index>(i));
}

template <class T, std::size_t N> struct ConstructionHelper {
  template <class P, class... Args>
  inline constexpr T operator()(P &&param, Args &&... args) const
      noexcept(noexcept(ConstructionHelper<T, N - 1>{}(
          std::forward<P>(param), subscript(param, N - 1),
          std::forward<Args>(args)...))) {
    return ConstructionHelper<T, N - 1>{}(std::forward<P>(param),
                                          subscript(param, N - 1),
                                          std::forward<Args>(args)...);
  }
};

template <class T> struct ConstructionHelper<T, 0> {
  template <class P, class... Args>
  inline constexpr T operator()(P &&, Args &&... args) const
      noexcept(noexcept(T{std::forward<Args>(args)...})) {
    return T{std::forward<Args>(args)...};
  }
};

template <class T, std::size_t N>
struct ConstructionHelper<std::array<T, N>, 0> {
  template <class P, class... Args>
  inline constexpr std::array<T, N> operator()(P &&, Args &&... args) const
      noexcept(noexcept(std::array<T, N>{{std::forward<Args>(args)...}})) {
    return std::array<T, N>{{std::forward<Args>(args)...}};
  }
};

} // namespace Detail_

/// Converts an std::array based vector into an Eigen vector
template <class T, std::size_t N>
inline Eigen::Matrix<T, static_cast<Eigen::Index>(N), 1>
vec(std::array<T, N> const &arr) {
  using Result = Eigen::Matrix<T, static_cast<Eigen::Index>(N), 1>;
  return Detail_::ConstructionHelper<Result, N>{}(arr);
}

/// Converts an Eigen vector to a std::array based one
template <class Derived>
inline std::array<typename Derived::Scalar,
                  static_cast<std::size_t>(Derived::RowsAtCompileTime)>
arr(Eigen::MatrixBase<Derived> const &vec) noexcept {
  static_assert(Derived::RowsAtCompileTime != Eigen::Dynamic &&
                    Derived::ColsAtCompileTime == 1,
                "Conversion only available for fix sized vectors");

  using T = typename Derived::Scalar;
  auto constexpr N = static_cast<std::size_t>(Derived::RowsAtCompileTime);
  using Result = std::array<T, N>;
  return Detail_::ConstructionHelper<Result, N>{}(vec);
}

} // namespace Adaptor
} // namespace CortidQCT
