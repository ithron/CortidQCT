/**
 * @file      SIMesh.h
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

template <class T>
Mesh<T> readFromSIMesh(std::string const &filename, bool withLabels = true);

extern template void writeToSIMesh<float>(Mesh<float> const &,
                                          std::string const &, bool);
extern template void writeToSIMesh<double>(Mesh<double> const &,
                                           std::string const &, bool);

extern template Mesh<float> readFromSIMesh<float>(std::string const &, bool);
extern template Mesh<double> readFromSIMesh<double>(std::string const &, bool);
} // namespace Internal
} // namespace CortidQCT
