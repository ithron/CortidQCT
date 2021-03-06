cmake_minimum_required(VERSION 3.8)

# Setup Hunter
include(${CMAKE_CURRENT_LIST_DIR}/HunterGate.cmake)

HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.23.34.tar.gz"
  SHA1 "70287b1ffa810ee4e952052a9adff9b4856d0d54"
  FILEPATH "${CMAKE_CURRENT_LIST_DIR}/Hunter/config.cmake"
)

set(CPP_STANDARD_LIBRARY "stdc++" CACHE STRING
  "Standard library used (e.g. stdc++ or c++)"
)

set_property(CACHE CPP_STANDARD_LIBRARY PROPERTY STRINGS c++ stdc++)

if ("${CPP_STANDARD_LIBRARY}" STREQUAL "c++")
  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS "7")
    set(CortidQCT_FILESYSTEM_LIBRARY c++experimental)
  else()
    set(CortidQCT_FILESYSTEM_LIBRARY c++fs)
  endif()
else()
    set(CortidQCT_FILESYSTEM_LIBRARY stdc++fs)
endif()

