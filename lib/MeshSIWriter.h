/**
 * @file      MeshSIWriter.h
 *
 * @brief     This file contains the definition of the SIMesh writer function.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "Mesh.h"

namespace CortidQCT {
namespace Internal {

template <class T>
void writeToSIMesh(Mesh<T> const &mesh, std::string const &filename,
                   bool includeLabels = false);

extern template void writeToSIMesh<float>(Mesh<float> const &,
                                          std::string const &, bool);
extern template void writeToSIMesh<double>(Mesh<double> const &,
                                           std::string const &, bool);
} // namespace Internal
} // namespace CortidQCT
