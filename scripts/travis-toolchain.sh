#!/bin/sh

set -eu

if [[ "$TRAVIS_OS_NAME" = "linux" && "$COMPILER" = "clang" ]]
then

  CLANG_DIR="${HOME}/clang-${CLANG_VERSION}"
  CLANG_URL="http://releases.llvm.org/${CLANG_VERSION}/clang+llvm-${CLANG_VERSION}-x86_64-linux-gnu-ubuntu-14.04.tar.xz"

  mkdir ${CLANG_DIR}
  # Download clang
  curl ${CLANG_URL} | tar xJ -C ${CLANG_DIR} --strip-components 1
  export CXXFLAGS="-stdlib=libc++"
  export LD_LIBRARY_PATH=${CLANG_DIR}/lib:${LD_LIBRARY_PATH}
  export PATH=${CLANG_DIR}/bin:${PATH}

  export CC=clang
  export CXX=clang++

elif [[ "$TRAVIS_ON_NAME" = "linux" && "$COMPILER" = "gcc" ]]

  export CC=gcc-${GCC_VERSION}
  export CXX=g++-${GCC_VERSION}

fi

