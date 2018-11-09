#!/bin/bash

set -eu

if [[ -v TRAVIS_TAG ]]; then

  make install

  cp README.md install/
  cp LICENSE install/
  cp -r images install/
  cp -r data install/

  REL_DIR=CortidQCT-${TRAVIS_TAG}-x86_64-linux-gnu-ubuntu-18.04
  mv install ${REL_DIR}
  tar czf ${REL_DIR}.tar.gz ${REL_DIR}

fi

