cmake_minimum_required(VERSION 3.8)

include(${CMAKE_CURRENT_LIST_DIR}/../../../cmake/Init.cmake)


# Create project
project(
  CortidQCT-C-Bindings-Examples
  VERSION ${CortidQCT_VERSION}
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

