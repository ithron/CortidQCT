cmake_minimum_required(VERSION 3.8)

############################
# Project

project(
  CortidQCT-C-Bindings
  VERSION 1.2.0
  LANGUAGES CXX
)

option(CortidQCT_WITH_C_BINDINGS_EXAMPLES "Build C-Bindings exampled" ON)

include(GenerateExportHeader)

############################
# Targets

add_library(C-Bindings-Objects OBJECT
  src/ColorToLabelMap.cpp
  src/Error.cpp
  src/Mesh.cpp
  src/MeshFitter.cpp
  src/ObjectBase.cpp
  src/VoxelVolume.cpp
)
set_property(TARGET C-Bindings-Objects PROPERTY POSITION_INDEPENDENT_CODE True)

add_library(C-Bindings $<TARGET_OBJECTS:C-Bindings-Objects>)

target_include_directories(C-Bindings
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/CortidQCT/C"
)

target_link_libraries(C-Bindings PRIVATE
  CortidQCT::Core
)

target_include_directories(C-Bindings-Objects
  PRIVATE
    $<TARGET_PROPERTY:C-Bindings,INCLUDE_DIRECTORIES>
)
target_compile_definitions(C-Bindings-Objects
  PRIVATE
    $<TARGET_PROPERTY:C-Bindings,COMPILE_DEFINITIONS>
)
target_compile_features(C-Bindings-Objects
  PRIVATE
    $<TARGET_PROPERTY:C-Bindings,COMPILE_FEATURES>
)

if(WIN32)
  if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    target_compile_options(C-Bindings PUBLIC /w /W0 /MTd)
    target_compile_options(C-Bindings-PUBLIC INTERFACE /w /W0 /MTd)
  else()
    target_compile_options(C-Bindings PUBLIC /w /W0 /MT)
    target_compile_options(C-Bindings-Objects PUBLIC /w /W0 /MT)
  endif()
endif()


set_target_properties(C-Bindings PROPERTIES OUTPUT_NAME "CortidQCT-C")

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
generate_export_header(C-Bindings
  BASE_NAME cortidqct-c
)

if (NOT CLANG_TIDY_EXE AND WITH_CLANG_TIDY)
  message(WARNING "clang-tidy not found. Disable it.")
  set(CortiQCT_WITH_CLANG_TIDY OFF)
else()
  set(DO_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

if (WITH_CLANG_TIDY)
  set_target_properties(C-Bindings PROPERTIES C_CLANG_TIDY "${DO_CLANG_TIDY}")
endif()

add_library(CortidQCT::Bindings::C ALIAS C-Bindings)


############################
# Examples
if (CortidQCT_WITH_C_BINDINGS_EXAMPLES)
  add_subdirectory(examples)
endif()

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

