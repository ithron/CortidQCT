/**
 * @file      CommonMath.h
 *
 * @brief     This header contains common math includes, functions and
 * definitions,
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 * You may use, distribute and modify this code under the terms of the
 * AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#if defined(_MSC_VER)
#  define _USE_MATH_DEFINES
#endif

#include <cmath>

#ifndef M_PI
#  define M_PI                                                                 \
    3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211
#endif

namespace CortidQCT {
namespace Internal {

template <class T> inline constexpr auto square(T &&x) { return x * x; }

} // namespace Internal
} // namespace CortidQCT

