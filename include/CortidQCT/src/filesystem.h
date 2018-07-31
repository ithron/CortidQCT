/**
 * @file      filesystem.h
 *
 * @brief     Wrapper for including the C++17 filesystem header.
 *
 * @author    Stefan Reinhold
 * @copyright
 *            Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#if __has_include(<filesystem>)
#  include <filesystem>
#elif __has_include(<experimental/filesystem>)

#  include <experimental/filesystem>

namespace std {
namespace filesystem = experimental::filesystem;
}

#else

#  error "CortidQCT requires a standard library with filesystem support"

#endif
