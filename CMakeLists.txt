cmake_minimum_required(VERSION 3.8)

# Setup Hunter
include(${CMAKE_SOURCE_DIR}/cmake/HunterGate.cmake)

HunterGate(
  URL "https://github.com/ruslo/hunter/archive/v0.22.36.tar.gz"
  SHA1 "66b28c3e88dd81833851181149df1075c62e8790"
  LOCAL
)

include(GNUInstallDirs)

# Create project
project(
  LibCortidQCT
  VERSION 1.0.0
  LANGUAGES CXX
)



add_subdirectory(lib)

OPTION(BUILD_TESTS "Build tests" OFF)
OPTION(BUILD_TOOLS "Build tools" OFF)

if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if (BUILD_TOOLS)
  add_subdirectory(tools)
endif()

