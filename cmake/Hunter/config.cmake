option(CortidQCT_GIT_SELF "Turn of if building examples or tools as stand-alone project" OFF)

if (CortidQCT_GIT_SELF)
  hunter_config(CortidQCT GIT_SELF)
endif()

hunter_config(Microsoft.GSL
  GIT_SUBMODULE "Dependencies/GSL"
  CMAKE_ARGS
    GSL_TEST=OFF
    GSL_CXX_STANDARD=17
)

hunter_config(libigl
  GIT_SUBMODULE "Dependencies/libigl"
  CMAKE_ARGS
    HUNTER_ENABLED=ON
    LIBIGL_USE_STATIC_LIBRARY=OFF
    LIBIGL_WITH_COMISO=OFF
    LIBIGL_WITH_LIM=OFF
    LIBIGL_WITH_OPENGL=OFF
    LIBIGL_WITH_OPENGL_GLFW=OFF
    LIBIGL_WITH_TETGEN=OFF
    LIBIGL_WITH_TRIANGLE=OFF
    LIBIGL_WITH_VIEWER=OFF
    LIBIGL_WITH_XML=OFF
)

hunter_config(ImageStack
  GIT_SUBMODULE "Dependencies/ImageStack"
  CMAKE_ARGS
    BUILD_TESTING=OFF
    BUILD_EXAMPLES=OFF
)

