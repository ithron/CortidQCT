cmake_minimum_required(VERSION 3.8)

############################
# Project

project(
  CortidQCT-MATLAB-Bindings
  VERSION ${CortidQCT_VERSION}
  LANGUAGES C
)

if (NOT CortidQCT_C_BINDINGS)
  message(FATAL_ERROR "Cannot build Matlab binding without C bindings. Set CortidQCT_C_BINDINGS=ON")
endif()

add_library(matlab-bindings SHARED
  $<TARGET_OBJECTS:C-Bindings-Objects>
  src/matlab-addons.c
)

target_include_directories(matlab-bindings
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/CortidQCT/Matlab>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(matlab-bindings PRIVATE CortidQCT::Bindings::C)
set_target_properties(matlab-bindings PROPERTIES
  OUTPUT_NAME "CortidQCT-Matlab"
  WINDOWS_EXPORT_ALL_SYMBOLS ON
)

add_library(CortidQCT::Bindings::Matlab ALIAS matlab-bindings)

include(GenerateExportHeader)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
generate_export_header(matlab-bindings
  BASE_NAME cortidqct-matlab
)

############################
# Exports

include(GNUInstallDirs)

install(TARGETS matlab-bindings EXPORT CortidQCTExport
  LIBRARY DESTINATION "${CortidQCT_Toolbox_ROOT}/toolbox/+CortidQCT/+lib/lib"
  RUNTIME DESTINATION "${CortidQCT_Toolbox_ROOT}/toolbox/+CortidQCT/+lib/lib"
)

install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/include/CortidQCT"
  DESTINATION
    "${CortidQCT_Toolbox_ROOT}/toolbox/+CortidQCT/+lib/include"
)

install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/+CortidQCT"
  DESTINATION
    "${CortidQCT_Toolbox_ROOT}/toolbox"
)

