cmake_minimum_required(VERSION 3.8)

option(CortidQCT_C_BINDINGS "Build C bindings" ON)

if (CortidQCT_C_BINDINGS)
  add_subdirectory(C)
endif()

