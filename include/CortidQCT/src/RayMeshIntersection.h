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

#include <limits>

namespace CortidQCT {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
template <class T> struct alignas(8) RayMeshIntersection {
  BarycentricPoint<T, typename Mesh<T>::Index> position;
  T signedDistance = std::numeric_limits<T>::infinity();
};
#pragma clang diagnostic pop

} // namespace CortidQCT
