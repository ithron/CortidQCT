cmake_minimum_required(VERSION 3.8)

############################
# Project

project(
  CortidQCT-MATLAB-Bindings
  VERSION 1.1.0
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
set_target_properties(matlab-bindings PROPERTIES OUTPUT_NAME "CortidQCT-Matlab")

add_library(CortidQCT::Bindings::Matlab ALIAS matlab-bindings)

include(GenerateExportHeader)
generate_export_header(matlab-bindings
  BASE_NAME cortidqct-matlab
)

############################
# Exports

include(GNUInstallDirs)

install(TARGETS matlab-bindings EXPORT CortidQCTExport
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/include/CortidQCT"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cortidqct-matlab_export.h"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}/CortidQCT/Matlab"
)

