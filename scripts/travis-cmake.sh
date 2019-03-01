#!/bin/bash

set -euvx

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

  GENERATOR="Unix Makefiles"

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
  -DBUILD_MEX=${BUILD_MEX} \
  -DMatlab_ROOT_DIR=${Matlab_ROOT_DIR} \
  -DCMAKE_PREFIX_PATH=`pwd`/install \
  -DBUILD_DOC=${BUILD_DOC} .

