cmake_minimum_required(VERSION 3.9)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/Init.cmake)

############################
# Project

project(
  CortidQCT-Common
  VERSION 1.2.0
  LANGUAGES CXX
)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

if (supported)
  option(CortidQCT_LINK_TIME_OPTIMIZATION "Enable link time optimization if supported by compiler" OFF)
  message(STATUS "Link time optimization is supported by compiler")
else()
  message(STATUS "Link time optimization is NOT supported by compiler")
endif()

option(CortidQCT_WITH_WERROR "Treat warnings as errors" ON)

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

if (CortidQCT_LINK_TIME_OPTIMIZATION)
  set_property(TARGET Common PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

target_include_directories(Common
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(Common INTERFACE
  $<BUILD_INTERFACE:$<IF:$<CXX_COMPILER_ID:MSVC>,,-Wall>>
)

if (CortidQCT_WITH_WERROR)
  message(STATUS "Treating warnings as errors")
  target_compile_options(Common INTERFACE
    -Werror
  )
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  target_compile_options(Common INTERFACE
    -Weverything
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -fcomment-block-commands=nosubgrouping
  )
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_compile_options(Common INTERFACE
    -Wno-unknown-pragmas
  )
endif()

if(WIN32)
  if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    target_compile_options(Common INTERFACE /w /W0 /MTd)
  else()
    target_compile_options(Common INTERFACE /w /W0 /MT)
  endif()
endif()

set_property(
  TARGET Common
  PROPERTY INTERFACE_CXX_EXTENSIONS OFF
)

if (CortidQCT_WITH_OPENMP)
  target_compile_options(Common INTERFACE ${OpenMP_CXX_FLAGS})
  target_link_libraries(Common INTERFACE ${OpenMP_CXX_LIBRARIES})
endif()

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

