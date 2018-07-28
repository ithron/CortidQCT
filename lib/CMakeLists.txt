cmake_minimum_required(VERSION 3.8)

############################
# Targets

add_library(Core
  CortidQCT.cpp
)

add_library(CortidQCT::Core ALIAS Core)

target_compile_features(Core
  PUBLIC
    cxx_std_17
)
target_include_directories(Core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:include>
)

set_property(
  TARGET Core
  PROPERTY CXX_EXTENSIONS OFF
)

############################
# Required Dependencies

hunter_add_package(Microsoft.GSL)
find_package(Microsoft.GSL CONFIG REQUIRED)

hunter_add_package(Eigen)
find_package(Eigen3 CONFIG REQUIRED)

hunter_add_package(libigl)
find_package(libigl CONFIG REQUIRED)

# Add to target
target_link_libraries(Core
  PUBLIC
    Microsoft.GSL::GSL
    Eigen3::Eigen
  PRIVATE
    igl::core
)


############################
# Optional Dependencies

option(WITH_OPENCL "Build with OpenCL support" ON)
if (WITH_OPENCL)
  hunter_add_package(OpenCL)
  find_package(OpenCL CONFIG REQUIRED)

  # Add to targets
  target_link_libraries(Core
    PRIVATE
      OpenCL::OpenCL
  )

endif()

############################
# Exports

install(TARGETS Core EXPORT CortidQCTConfig
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT CortidQCTConfig NAMESPACE CortidQCT::
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/igl/cmake
)

export(TARGETS ${ALL_MODULES} NAMESPACE CortidQCT:: FILE CortidQCTConfig.cmake)

