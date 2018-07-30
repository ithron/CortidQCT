cmake_minimum_required(VERSION 3.2)

project(
  CortidQCT-Examples
  VERSION 1.0.0
)

i############################
# Required Dependencies

option(CortidQCT_GIT_SELF "Turn of if building examples or tools as stand-alone project" ON)

if (NOT TARGET CortidQCT::Core)
  hunter_add_package(CortidQCT)
  find_package(CortidQCT CONFIG REQUIRED)
endif()

hunter_add_package(Microsoft.GSL)
find_package(Microsoft.GSL CONFIG REQUIRED)


############################
# Optional Dependencies

option(WITH_OPENCL "Build with OpenCL support" ON)
if (WITH_OPENCL)
  hunter_add_package(OpenCL)
  find_package(OpenCL CONFIG REQUIRED)
endif()

if (WITH_OPENCL)
  add_executable(ListOpenCLStats listOpenCLStats.cpp)

  target_link_libraries(ListOpenCLStats
    PRIVATE
      OpenCL::OpenCL
      Microsoft.GSL::GSL
  )
endif()


############################
# Targets

