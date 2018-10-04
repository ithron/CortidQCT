cmake_minimum_required(VERSION 3.8)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/Init.cmake)

############################
# Project
project(
  CortidQCT-Tests
  VERSION 1.0.0
  LANGUAGES CXX
)

############################
# Required Dependencies

option(CortidQCT_GIT_SELF "Turn of if building examples or tools as stand-alone project" ON)

if (NOT CortidQCT_GIT_SELF)
  option(CortifQCT_BUILD_PRIVATE_TESTS "Build private API tests" ON)
endif()

if (NOT TARGET CortidQCT::Core)
  hunter_add_package(CortidQCT)
  find_package(CortidQCT CONFIG REQUIRED)
endif()

hunter_add_package(libigl)
find_package(libigl CONFIG REQUIRED)

hunter_add_package(Eigen)
find_package(Eigen3 CONFIG REQUIRED)

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

set(CortidQCT_DATADIR "${CMAKE_CURRENT_LIST_DIR}/../data")

configure_file(tests_config.h.in tests_config.h @ONLY)


############################
# Targets

add_library(TestCommon INTERFACE)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  target_compile_options(TestCommon INTERFACE
    -Wno-exit-time-destructors
    -Wno-global-constructors
  )

  # -Wzero-as-null-pointer-constant is available since clang 5.0
  if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL "5.0") OR
      ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang" AND "${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL "9.1"))
    target_compile_options(TestCommon INTERFACE
      -Wno-zero-as-null-pointer-constant
    )
  endif()

  if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    target_compile_options(TestCommon INTERFACE
      -Wno-deprecated-declarations
    )
endif()

endif()

add_library(TestInternalCommon INTERFACE)
target_link_libraries(TestInternalCommon INTERFACE TestCommon PrivateAPI)


add_executable(TestMesh Mesh.cpp)
target_link_libraries(TestMesh
  PRIVATE
    GTest::main
    CortidQCT::Core
    igl::core
    Eigen3::Eigen
    TestCommon
)
target_include_directories(TestMesh PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_test(TestMesh TestMesh)


add_executable(TestVoxelVolume VoxelVolume.cpp)
target_link_libraries(TestVoxelVolume
  PRIVATE
    GTest::main
    CortidQCT::Core
    TestCommon
)
target_include_directories(TestVoxelVolume PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_test(TestVoxelVolume TestVoxelVolume)

add_executable(TestMeasurementModel MeasurementModel.cpp)
target_link_libraries(TestMeasurementModel
  PRIVATE
    GTest::main
    CortidQCT::Core
    TestCommon
)
target_include_directories(TestMeasurementModel PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_test(TestMeasurementModel TestMeasurementModel)

add_executable(TestMeshFitterConfiguration MeshFitterConfiguration.cpp)
target_link_libraries(TestMeshFitterConfiguration
  PRIVATE
    GTest::main
    CortidQCT::Core
    TestCommon
)
target_include_directories(TestMeshFitterConfiguration PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_test(TestMeshFitterConfiguration TestMeshFitterConfiguration)

add_executable(TestCustomColorToLabelMap CustomColorToLabelMap.cpp)
target_link_libraries(TestCustomColorToLabelMap
  PRIVATE
    GTest::main
    CortidQCT::Core
    TestCommon
)
target_include_directories(TestCustomColorToLabelMap PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
add_test(TestCustomColorToLabelMap TestCustomColorToLabelMap)

if (CortifQCT_BUILD_PRIVATE_TESTS)

  add_executable(TestInternalSampler InternalSampler.cpp)
  target_link_libraries(TestInternalSampler
    PRIVATE
      GTest::main
      CortidQCT::Core
      TestInternalCommon
  )
  target_include_directories(TestInternalSampler PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  add_test(TestInternalSampler TestInternalSampler)

endif(CortifQCT_BUILD_PRIVATE_TESTS)

# add_executable(TestFitMesh FitMesh.cpp)
# target_link_libraries(TestFitMesh
#   PRIVATE
#     GTest::main
#     CortidQCT::Core
# )
# add_test(TestFitMesh TestFitMesh)

