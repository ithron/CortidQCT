#!/bin/bash

set -e

if [[ "$TRAVIS_OS_NAME" = "linux" ]]
then
  MCR_URL="http://ssd.mathworks.com/supportfiles/downloads/R2017b/deployment_files/R2017b/installers/glnxa64/MCR_R2017b_glnxa64_installer.zip"
elif [[ "$TRAVIS_OS_NAME" = "osx" ]]
then
  MCR_URL="http://ssd.mathworks.com/supportfiles/downloads/R2017b/deployment_files/R2017b/installers/maci64/MCR_R2017b_maci64_installer.dmg.zip"
fi

MCR_ROOT=${HOME}/MCR

if [[ ! -f ${MCR_ROOT} ]]
then
  echo "Downloading Matlab Compiler Runtime"
  curl ${MCR_URL} > MCR.zip
  unzip MCR.zip
  mkdir MCR-install
  unzip MCR.zip -d MCR-install/
  cd MCR-install

  echo "Installing Matlab Compiler Runtime"
  if [[ "$TRAVIS_OS_NAME" = "linux" ]]
  then
    ./install -mode silent -agreeToLicense yes -destinationFolder ${MCR_ROOT}

  elif [[ "$TRAVIS_OS_NAME" = "osx" ]]
  then

    CUR_DIR=`pwd`
    hdiutil attach MCR_R2017b_maci64_installer.dmg && cd /Volumes/MCR_R2017b_maci64_installer
    InstallForMacOSX.app/Contents/MacOS/InstallForMacOSX -mode silent -agreeToLicense yes -destinationFolder ${MCR_ROOT}
    cd ${CUR_DIR}

  else
    echo "Unsupported OS"
    exit -1
  fi
  cd ..

  export Matlab_ROOT_DIR=${MCR_ROOT}/v93

  echo "Matlab_ROOT_DIR={$Matlab_ROOT_DIR}"
fi

