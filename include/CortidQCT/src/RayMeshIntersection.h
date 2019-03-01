/**
 * @file      RayMeshIntersection.h
 *
 * @brief     This file contains the definition of the RayMeshIntersection type
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2019 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "Mesh.h"
#include "Padding.h"

#include <limits>

namespace CortidQCT {

template <class T> struct alignas(8) RayMeshIntersection {
  using Position = BarycentricPoint<T, typename Mesh<T>::Index>;
  Position position;
  T signedDistance = std::numeric_limits<T>::infinity();
  /// Add explicit padding to ensure copatibility with C
  CQCT_PADDING(sizeof(Position) + sizeof(T), 8);
};

} // namespace CortidQCT
