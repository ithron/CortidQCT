cmake_minimum_required(VERSION 3.8)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/Init.cmake)

include(GNUInstallDirs)

# Create project
project(
  CortidQCTApplication
  VERSION 1.1.0
  LANGUAGES CXX
)

############################
# Required Dependencies

option(CortidQCT_GIT_SELF "Turn of if building examples or tools as stand-alone project" ON)

if (NOT TARGET CortidQCT::Core)
  hunter_add_package(CortidQCT)
  find_package(CortidQCT CONFIG REQUIRED)
endif()

############################
# Targets

add_executable(CortidQCT_CLI CLI.cpp)

target_link_libraries(CortidQCT_CLI
  PRIVATE
    CortidQCT::Core
)

############################
# Exports

include(GNUInstallDirs)

install(TARGETS CortidQCT_CLI EXPORT CortidQCTExport
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

