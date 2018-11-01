#!/bin/bash

set -eu

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

  GENERATOR="Xcode"

else

  GENERATOR="Unix Makefiles"

fi

BUILD_DOC=OFF
if [[ ${GENERATE_DOCS} = 'yes' ]]; then
  BUILD_DOC=ON
fi


cmake \
  -G"${GENERATOR}" \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DBUILD_TOOLS=ON \
  -DBUILD_TESTS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DWITH_CLANG_TIDY=OFF \
  -DWITH_OPENCL=OFF \
  -DMatlab_ROOT_DIR=$ENV{Matlab_ROOT_DIR} \
  -DBUILD_DOC=${BUILD_DOC} .

