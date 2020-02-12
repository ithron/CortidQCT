cmake_minimum_required(VERSION 3.8)

############################
# Project

project(
  Faddeeva-MATLAB
  VERSION 1.0.0
  LANGUAGES CXX
)

find_package(Matlab 9.4 COMPONENTS MX_LIBRARY MAT_LIBRARY)

if (NOT MATLAB_FOUND)
  message(FATAL_ERROR "MATLAB could not be found. Please install MATLAB MCR from https://www.mathworks.com/products/compiler/matlab-runtime.html")
endif()

find_package(OpenMP)
if (OpenMP_FOUND)
  message(STATUS "OpenMP support enabled.")
  set(FADDEEVA_LINK_LIBRARIES ${OpenMP_CXX_LIBRARIES})
endif()


matlab_add_mex(
  NAME
    faddeeva_erf
  SRC
    Faddeeva.cc
    Faddeeva_erf_mex.cc
  LINK_TO
    "${FADDEEVA_LINK_LIBRARIES}"
  OUTPUT_NAME
    erf
)

matlab_add_mex(
  NAME
    faddeeva_erfi
  SRC
    Faddeeva.cc
    Faddeeva_erfi_mex.cc
  LINK_TO
    "${FADDEEVA_LINK_LIBRARIES}"
  OUTPUT_NAME
    erfi
)

if (OpenMP_FOUND)
  target_compile_options(faddeeva_erf PRIVATE ${OpenMP_CXX_FLAGS})
  target_compile_options(faddeeva_erfi PRIVATE ${OpenMP_CXX_FLAGS})
endif()

target_compile_definitions(faddeeva_erf PRIVATE MATLAB_DEFAULT_RELEASE=R2018a)
target_compile_definitions(faddeeva_erfi PRIVATE MATLAB_DEFAULT_RELEASE=R2018a)

set_target_properties(faddeeva_erf PROPERTIES COMPILE_FLAGS "-fvisibility=default")
set_target_properties(faddeeva_erfi PROPERTIES COMPILE_FLAGS "-fvisibility=default")

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

