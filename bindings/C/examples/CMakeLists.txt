cmake_minimum_required(VERSION 3.8)

include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/Init.cmake)


# Create project
project(
  CortidQCT-C-Bindings-Examples
  VERSION 1.0.0
  LANGUAGES C
)

############################
# Targets

add_executable(C_CLI
  cli.c
)

target_link_libraries(C_CLI PRIVATE
  CortidQCT::Bindings::C
)

if(WIN32)
  if (CMAKE_BUILD_TYPE MATCHES DEBUG)
    target_compile_options(C_CLI INTERFACE /w /W0 /MTd)
  else()
    target_compile_options(C_CLI INTERFACE /w /W0 /MT)
  endif()
endif()

