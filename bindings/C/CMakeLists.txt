cmake_minimum_required(VERSION 3.8)

############################
# Project

project(
  CortidQCT-C-Bindings
  VERSION 1.0.0
  LANGUAGES CXX
)

############################
# Targets

add_library(C-Bindings
  c-bindings.cpp
)

target_include_directories(C-Bindings
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/CortidQCT/C"
)

target_link_libraries(C-Bindings PRIVATE
  CortidQCT::Core
)
set_target_properties(C-Bindings PROPERTIES OUTPUT_NAME "CortidQCT-C-Bindings")

if (NOT CLANG_TIDY_EXE AND WITH_CLANG_TIDY)
  message(WARNING "clang-tidy not found. Disable it.")
  set(CortiQCT_WITH_CLANG_TIDY OFF)
else()
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

if (WITH_CLANG_TIDY)
  set_target_properties(C-Bindings PROPERTIES C_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()

add_library(CortidQCT::Binding::C ALIAS C-Bindings)

############################
# Exports

include(GNUInstallDirs)

install(TARGETS C-Bindings EXPORT CortidQCTExport
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

