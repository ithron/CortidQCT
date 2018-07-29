cmake_minimum_required(VERSION 3.8)

hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

if(NOT TARGET CortidQCT::Core)
  message(FATAL_ERROR "NO TARGET!")
endif()

add_executable(TestFitMesh FitMesh.cpp)
target_link_libraries(TestFitMesh
  PRIVATE
    GTest::main
    CortidQCT::Core
)
add_test(TestFitMesh TestFitMesh)

