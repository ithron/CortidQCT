#!/usr/local/bin/bash

set -eu

if [[ -v TRAVIS_TAG ]]; then

  make install

  cp README.md install/
  cp LICENSE install/
  cp -r images install/
  cp -r data install/

  if [[ "${TRAVIS_OS_NAME}" = "linux" ]]; then
    REL_DIR=CortidQCT-${TRAVIS_TAG}-x86_64-linux-gnu-ubuntu-18.04
  elif [[ "${TRAVIS_OS_NAME}" = "osx" ]]; then
    REL_DIR=CortidQCT-${TRAVIS_TAG}-macOS-Mojave
  fi

  mv install ${REL_DIR}

  if [[ "${TRAVIS_OS_NAME}" = "linux" ]]; then
    tar czf ${REL_DIR}.tar.gz ${REL_DIR}
  elif [[ "${TRAVIS_OS_NAME}" = "osx" ]]; then
    zip -r ${REL_DIR}.zip ${REL_DIR}
  fi

fi

