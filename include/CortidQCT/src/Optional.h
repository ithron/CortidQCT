/**
 * @file      Optional.h
 *
 * @brief     This header is a wrapper for including optional depending on the
 * supported C++ standard version.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <ostream>

#if __has_include(<optional>)
#  include <optional>
#elif __has_include(<experimental/optional>)
#  include <experimental/optional>

namespace std {
using std::experimental::nullopt;
using std::experimental::nullopt_t;
using std::experimental::optional;

using std::experimental::make_optional;

template <class T>
std::ostream &operator<<(std::ostream &os, optional<T> const &opt) {
  return opt ? (os << *opt) : (os << "nullopt");
}

} // namespace std

#else
#  error C++ Standard libary has no optional type
#endif
