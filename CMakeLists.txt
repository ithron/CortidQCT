cmake_minimum_required(VERSION 3.8)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Init.cmake)

include(GNUInstallDirs)

# Create project
project(
  CortidQCT
  VERSION 1.0.0
  LANGUAGES CXX
)

add_subdirectory(common)
add_subdirectory(lib)

OPTION(BUILD_TESTS "Build tests" OFF)
OPTION(BUILD_TOOLS "Build tools" OFF)

############################
# Exports

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/CortidQCTConfig.cmake.in
  ${CMAKE_BINARY_DIR}/CortidQCTConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/CortidQCT/cmake
)

export(EXPORT CortidQCTExport
  FILE "${CMAKE_BINARY_DIR}/CortidQCTExport.cmake"
)
install(EXPORT CortidQCTExport NAMESPACE CortidQCT::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/CortidQCT/cmake
)

############################
# Optional Sub-projects

if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if (BUILD_TOOLS)
  add_subdirectory(tools)
endif()

