cmake_minimum_required(VERSION 3.8)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/Init.cmake)

############################
# Project

project(
  CortidQCT-Common
  VERSION 1.0.0
  LANGUAGES CXX
)

############################
# Targets

project(
  CortidQCT-Common
  VERSION 1.0.0
  LANGUAGES CXX
)

add_library(Common INTERFACE)

target_compile_features(Common
  INTERFACE
    cxx_std_17
)

target_include_directories(Common
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(Common INTERFACE
  -Wall
  $<BUILD_INTERFACE:-Werror>
)

if($<CXX_COMPILER_ID:Clang>)
  target_compile_options(Common INTERFACE
    -Weverything
    -Wno-c++98-compat
    -fcomment-block-commands=nosubgrouping
  )
endif()

set_property(
  TARGET Common
  PROPERTY INTERFACE_CXX_EXTENSIONS OFF
)

add_library(CortidQCT::Common ALIAS Common)

############################
# Exports

install(TARGETS Common EXPORT CortidQCTExport
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(
  DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/../include/CortidQCT"
  DESTINATION
    "${CMAKE_INSTALL_INCLUDEDIR}"
)

