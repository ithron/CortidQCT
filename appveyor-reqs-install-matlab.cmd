@echo on

if NOT EXIST C:\projects\deps\MCR (
  echo "Downloading MCR"
  appveyor DownloadFile http://ssd.mathworks.com/supportfiles/downloads/R2017b/deployment_files/R2017b/installers/win64/MCR_R2017b_win64_installer.exe -FileName MCR.exe
  7z x MCR.exe -oC:\projects\deps\MCR-install
  echo "Installing MCR"
  cd C:\projects\deps\MCR-install
  dir
  setup -mode silent -agreeToLicense yes
  cd %APPVEYOR_BUILD_FOLDER%
) ELSE (
  echo "Using cached MCR installation."
)

@echo off
