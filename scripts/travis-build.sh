#!/bin/bash

set -eu

if [[ $TRAVIS_OS_NAME == 'osx' ]]; then

  xcodebuild -scheme ALL_BUILD && xcodebuild -scheme RUN_TESTS

else

  make && make test

fi
