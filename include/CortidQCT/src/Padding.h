/**
 * @file      Padding.h
 *
 * @brief     In this header the Padding helper type is defined
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2019 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <cstdint>
#include <type_traits>

namespace CortidQCT {
namespace Internal {

constexpr std::size_t bitPadding(std::size_t size, std::size_t alignment) {
  return (size % alignment) * 8;
}

#define CQCT_PADDING(Size, Alignment)                                          \
  unsigned int : ::CortidQCT::Internal::bitPadding((Size), (Alignment))

} // namespace Internal
} // namespace CortidQCT
