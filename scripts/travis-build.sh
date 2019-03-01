#!/bin/bash

set -eu

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

  make && make test

else

  make && make test

fi
