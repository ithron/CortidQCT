/**
 * @file      CheckExtension.h
 *
 * @brief     This file contains some utility functions to check if a file path
 * has a specific extension.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <algorithm>
#include <string>

namespace CortidQCT {

/// Namespace for IO helper functions
namespace IO {

inline std::string extension(std::string const &filename,
                             bool toLower = false) {

  std::string extension = "";
  if (auto const dotPos = filename.find_last_of('.');
      dotPos != std::string::npos) {
    extension = filename.substr(dotPos + 1);
  }

  if (toLower) {
    std::transform(begin(extension), end(extension), begin(extension), tolower);
  }
  return extension;
}

template <class Extensions>
bool checkExtensions(std::string const &filename, Extensions &&extensions) {
  using std::begin;
  using std::end;
  using std::find;

  auto const ext = extension(filename, true);

  return find_if(begin(extensions), end(extensions), [&ext](auto const &rhs) {
           std::string query = rhs;
           std::transform(begin(query), end(query), begin(query), tolower);
           return query == ext;
         }) != end(extensions);
}

} // namespace IO

} // namespace CortidQCT
