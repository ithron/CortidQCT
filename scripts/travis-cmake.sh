#!/bin/bash

set -eu

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

  set GENERATOR="Xcode"

else

  set GENERATOR="Unix Makefiles"

fi

cmake \
  -G"${GENERATOR}" \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DBUILD_TOOLS=ON \
  -DBUILD_TESTS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DWITH_CLANG_TIDY=OFF \
  -DBUILD_DOC=OFF .

