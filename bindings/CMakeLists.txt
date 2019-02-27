cmake_minimum_required(VERSION 3.8)

option(CortidQCT_C_BINDINGS "Build C bindings" ON)
option(CortidQCT_MATLAB_BINDINGS "Build Matlab bindings" ON)

if (CortidQCT_C_BINDINGS)
  add_subdirectory(C)
endif()

if (CortidQCT_MATLAB_BINDINGS)
  add_subdirectory(matlab)
endif()

