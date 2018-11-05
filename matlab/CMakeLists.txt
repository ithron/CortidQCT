cmake_minimum_required(VERSION 3.8)

############################
# Project

project(
  CortidQCT-MATLAB
  VERSION 1.1.0
  LANGUAGES C
)

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

add_library(CortidQCT::matlab ALIAS matlab)

