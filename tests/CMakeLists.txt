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

if (NOT TARGET CortidQCT::Core)
  hunter_add_package(CortidQCT)
  find_package(CortidQCT CONFIG REQUIRED)
endif()

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)


############################
# Targets

add_executable(TestMesh Mesh.cpp)
target_link_libraries(TestMesh
  PRIVATE
    GTest::main
    CortidQCT::Core
)
add_test(TestMesh TestMesh)

# add_executable(TestFitMesh FitMesh.cpp)
# target_link_libraries(TestFitMesh
#   PRIVATE
#     GTest::main
#     CortidQCT::Core
# )
# add_test(TestFitMesh TestFitMesh)

