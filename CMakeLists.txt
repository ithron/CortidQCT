cmake_minimum_required(VERSION 3.8)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Init.cmake)

include(GNUInstallDirs)

# Create project
project(
  CortidQCT
  VERSION 1.2.2.49
  LANGUAGES C CXX
)


find_package(OpenMP)
if (OpenMP_FOUND)
  option(CortidQCT_WITH_OPENMP "Build with OpenMP support" ON)
endif()

add_subdirectory(common)
add_subdirectory(lib)

OPTION(BUILD_TESTS "Build tests" ON)
OPTION(BUILD_TOOLS "Build tools" ON)
OPTION(BUILD_APP "Build applications" ON)
OPTION(BUILD_MEX "Build MATLAB mex filed" ON)

set(CortidQCT_Toolbox_ROOT "${CMAKE_INSTALL_PREFIX}/matlab"
  CACHE STRING
  "Path to the root directory where the matlab files shoudl be placed into"
)

############################
# Exports

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/CortidQCTConfig.cmake.in
  ${CMAKE_BINARY_DIR}/CortidQCTConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/CortidQCT/cmake
)

export(EXPORT CortidQCTExport NAMESPACE CortidQCT::
  FILE "${CMAKE_BINARY_DIR}/CortidQCTExport.cmake"
)
install(EXPORT CortidQCTExport NAMESPACE CortidQCT::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/CortidQCT/cmake
)

install(
  FILES
    "${CMAKE_BINARY_DIR}/CortidQCTExport.cmake"
    "${CMAKE_BINARY_DIR}/CortidQCTConfig.cmake"
  DESTINATION
    ${CMAKE_INSTALL_DATADIR}/CortidQCT/cmake
)

############################
# Optional Sub-projects

if (BUILD_APP)
  add_subdirectory(app)
endif()

if (BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if (BUILD_TOOLS)
  add_subdirectory(tools)
endif()

add_subdirectory(matlab)
add_subdirectory(bindings)

############################
# Documentation

option(BUILD_DOC "Build documentation" OFF)
if (BUILD_DOC)
  find_package(Doxygen 1.8.3 REQUIRED OPTIONAL_COMPONENTS dot)

  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
  set(DOXYGEN_EXAMPLE_PATH ${PROJECT_SOURCE_DIR}/bindings/C)
  doxygen_add_docs(
    doc
    README.md
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/bindings
    COMMENT "Generating documentation"
  )
endif()

