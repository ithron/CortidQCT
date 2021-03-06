cmake_minimum_required(VERSION 3.8)

############################
# Project

project(
  CortidQCT-MATLAB
  VERSION ${CortidQCT_VERSION}
  LANGUAGES C
)

if (BUILD_MEX)
  find_package(Matlab COMPONENTS MX_LIBRARY MAT_LIBRARY)

  if (NOT MATLAB_FOUND)
    message(FATAL_ERROR "MATLAB could not be found. Please install MATLAB MCR from https://www.mathworks.com/products/compiler/matlab-runtime.html")
  endif()

  matlab_add_mex(
    NAME
      matlab
    SRC
      +CortidQCT/floatVector2base64.c
    OUTPUT_NAME
      floatVector2base64
  )

  set_target_properties(matlab PROPERTIES COMPILE_FLAGS "-fvisibility=default")

  add_library(CortidQCT::matlab ALIAS matlab)

  set(Faddeeva_Toolbox_ROOT "${CortidQCT_Toolbox_ROOT}/toolbox/+CortidQCT")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../Dependencies/Faddeeva" "${CMAKE_CURRENT_BINARY_DIR}/Faddeeva")
endif()

##########################
# Package configuration

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CortidQCT.prj.in CortidQCT.prj @ONLY)


############################
# Exports

include(GNUInstallDirs)

if (BUILD_MEX)
  install(TARGETS matlab EXPORT CortidQCTExport
    LIBRARY DESTINATION ${CortidQCT_Toolbox_ROOT}/toolbox/+CortidQCT
    RUNTIME DESTINATION ${CortidQCT_Toolbox_ROOT}/toolbox/+CortidQCT
  )
endif()

install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/+CortidQCT"
  DESTINATION
    "${CortidQCT_Toolbox_ROOT}/toolbox"
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CortidQCT.prj"
  DESTINATION
    ${CortidQCT_Toolbox_ROOT}
)
