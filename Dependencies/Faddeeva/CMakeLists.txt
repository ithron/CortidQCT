cmake_minimum_required(VERSION 3.8)

############################
# Project

project(
  Faddeeva-MATLAB
  VERSION 1.0.0
  LANGUAGES CXX
)

find_package(Matlab COMPONENTS MX_LIBRARY MAT_LIBRARY)

if (NOT MATLAB_FOUND)
  message(FATAL_ERROR "MATLAB could not be found. Please install MATLAB MCR from https://www.mathworks.com/products/compiler/matlab-runtime.html")
endif()

matlab_add_mex(
  NAME
    faddeeva_erf
  SRC
    Faddeeva.cc
    Faddeeva_erf_mex.cc
  OUTPUT_NAME
    erf
)

matlab_add_mex(
  NAME
    faddeeva_erfi
  SRC
    Faddeeva.cc
    Faddeeva_erfi_mex.cc
  OUTPUT_NAME
    erfi
)

set_target_properties(matlab PROPERTIES COMPILE_FLAGS "-fvisibility=default")

add_library(Faddeeva::erf ALIAS faddeeva_erf)
add_library(Faddeeva::erfi ALIAS faddeeva_erfi)


include(GNUInstallDirs)

if (BUILD_MEX)
  install(TARGETS faddeeva_erf faddeeva_erfi EXPORT FaddeevaExport
    LIBRARY DESTINATION ${Faddeeva_Toolbox_ROOT}/+Faddeeva
    RUNTIME DESTINATION ${Faddeeva_Toolbox_ROOT}/+Faddeeva
  )
endif()

install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/+Faddeeva"
  DESTINATION
    "${Faddeeva_Toolbox_ROOT}/"
)

