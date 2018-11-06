cmake_minimum_required(VERSION 3.8)

############################
# Project

project(
  CortidQCT-MATLAB-Bindings
  VERSION 1.1.0
  LANGUAGES C
)

# set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")

add_library(matlab-bindings SHARED
  $<TARGET_OBJECTS:C-Bindings-Objects>
  matlab-addons.c
)
target_link_libraries(matlab-bindings PRIVATE CortidQCT::Bindings::C)
set_target_properties(matlab-bindings PROPERTIES OUTPUT_NAME "CortidQCT-Matlab")

add_library(CortidQCT::Bindings::Matlab ALIAS matlab-bindings)

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

