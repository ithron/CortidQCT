cmake_minimum_required(VERSION 3.8)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/Init.cmake)

############################
# Project

project(
  LibCortidQCT
  VERSION 1.2.1
  LANGUAGES CXX
)

############################
# Options


option(WITH_CLANG_TIDY "Use clang-tidy linter" OFF)
find_program(
  CLANG_TIDY_EXE
  NAMES "clang-tidy"
  DOC "Path to clang-tidy executable"
)
if (NOT CLANG_TIDY_EXE AND WITH_CLANG_TIDY)
  message(WARNING "clang-tidy not found. Disable it.")
  set(CortiQCT_WITH_CLANG_TIDY OFF)
else()
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

############################
# Targets

add_library(Core
  CortidQCT.cpp
  ColorToLabelMapIO.cpp
  DisplacementOptimizer.cpp
  MeasurementModel.cpp
  Mesh.cpp
  MeshFitter.cpp
  MeshFitterConfiguration.cpp
  MeshFitterImpl.cpp
  MeshSIWriter.cpp
  VoxelVolume.cpp
  WeightedARAPFitter.cpp
)
add_library(PrivateAPI INTERFACE)

set_property(TARGET Core PROPERTY POSITION_INDEPENDENT_CODE True)

target_link_libraries(Core
  PUBLIC
    Common
  PRIVATE
)
set_target_properties(Core PROPERTIES OUTPUT_NAME "CortidQCT-Core")
target_include_directories(Core PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../include/CortidQCT/src")

target_include_directories(PrivateAPI INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../include/CortidQCT/src")
target_include_directories(PrivateAPI INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../lib")

if (WITH_CLANG_TIDY)
  set_target_properties(Core PROPERTIES CXX_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()

add_library(CortidQCT::Core ALIAS Core)


############################
# Required Dependencies

hunter_add_package(Microsoft.GSL)
find_package(Microsoft.GSL CONFIG REQUIRED)

hunter_add_package(Eigen)
find_package(Eigen3 CONFIG REQUIRED)

hunter_add_package(libigl)
find_package(libigl CONFIG REQUIRED)

hunter_add_package(yaml-cpp)
find_package(yaml-cpp CONFIG REQUIRED)

# Add to target
target_link_libraries(Core
  PUBLIC
  PRIVATE
    igl::core
    Microsoft.GSL::GSL
    Eigen3::Eigen
    yaml-cpp
)
target_link_libraries(PrivateAPI
  INTERFACE
    igl::core
    Microsoft.GSL::GSL
    Eigen3::Eigen
    yaml-cpp
)


############################
# Optional Dependencies

option(WITH_OPENCL "Build with OpenCL support" ON)
if (WITH_OPENCL)
  hunter_add_package(OpenCL)
  find_package(OpenCL CONFIG REQUIRED)

  # Add to targets
  target_link_libraries(Core
    PRIVATE
      OpenCL::OpenCL
  )

endif()

option(CORTIDQCT_WITH_IMAGESTACK "Build with ImageStack support (.bst images)" ON)
if (CORTIDQCT_WITH_IMAGESTACK)
  hunter_add_package(ImageStack)
  find_package(ImageStack CONFIG REQUIRED)

  # Add to targets
  target_link_libraries(Core
    PRIVATE
      ImageStack::ImageStack
  )
  target_link_libraries(PrivateAPI
    INTERFACE
      ImageStack::ImageStack
  )
endif()

############################
# Configuration Header

configure_file(lib_config.h.in lib_config.h @ONLY)
target_include_directories(Core PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

############################
# Exports

include(GNUInstallDirs)

install(TARGETS Core EXPORT CortidQCTExport
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

