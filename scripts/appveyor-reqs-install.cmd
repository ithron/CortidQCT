@echo on

::###########################################################################
:::: Install Ninja
::::###########################################################################
@set PATH=C:\projects\deps\ninja;%PATH%
if NOT EXIST ninja (
  appveyor DownloadFile https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-win.zip -FileName ninja.zip
  7z x ninja.zip -oC:\projects\deps\ninja > nul
  rm ninja.zip
)
ninja --version

::###########################################################################
:::: Install Matlab when tag is present
::::###########################################################################
if "%APPVEYOR_REPO_TAG%" == "true" (
  if NOT EXIST C:\projects\deps\MCR (
    echo "Downloading MCR"
    appveyor DownloadFile https://ssd.mathworks.com/supportfiles/downloads/R2021a/Release/0/deployment_files/installer/complete/glnxa64/MATLAB_Runtime_R2021a_glnxa64.zip -FileName MCR.exe
    7z x MCR.exe -oC:\projects\deps\MCR-install
    echo "Installing MCR"
    cd C:\projects\deps\MCR-install
    dir
    setup -mode silent -agreeToLicense yes
    cd %APPVEYOR_BUILD_FOLDER%
  ) ELSE (
    echo "Using cached MCR installation."
  )
)

@echo off
