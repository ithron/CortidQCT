option(CortidQCT_GIT_SELF "Turn of if building examples or tools as stand-alone project" OFF)

hunter_config(yaml-cpp
  GIT_SUBMODULE "Dependencies/yaml-cpp"
  CMAKE_ARGS
    YAML_CPP_BUILD_TESTS=OFF
    YAML_CPP_BUILD_TOOLS=OFF
    YAML_CPP_BUILD_CONTRIB=OFF
)

if (CortidQCT_GIT_SELF)
  hunter_config(CortidQCT GIT_SELF)
endif()

hunter_config(GTest
  GIT_SUBMODULE "Dependencies/googletest"
  CMAKE_ARGS
    BUILD_GMOCK=OFF 
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

hunter_config(Microsoft.GSL
  GIT_SUBMODULE "Dependencies/Microsoft.GSL"
)

