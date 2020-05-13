cmake_minimum_required(VERSION 3.2)

project(
  CortidQCT-Tools
  VERSION ${CortidQCT_VERSION}
)

############################
# Required Dependencies

option(CortidQCT_GIT_SELF "Turn of if building examples or tools as stand-alone project" ON)

if (NOT TARGET CortidQCT::Core)
  hunter_add_package(CortidQCT)
  find_package(CortidQCT CONFIG REQUIRED)
endif()

hunter_add_package(Microsoft.GSL)
find_package(Microsoft.GSL CONFIG REQUIRED)


############################
# Optional Dependencies


############################
# Targets

add_executable(MeshConvert meshConvert.cpp)
target_link_libraries(MeshConvert PRIVATE CortidQCT::Core PrivateAPI)

set_property(TARGET MeshConvert PROPERTY OUTPUT_NAME CortidQCT_MeshConvert)

############################
# Exports

include(GNUInstallDirs)

install(TARGETS MeshConvert EXPORT CortidQCTExport
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

