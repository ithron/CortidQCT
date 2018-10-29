@echo on

::###########################################################################
:::: Install Ninja
::::###########################################################################
if NOT EXIST ninja (
  appveyor DownloadFile https://github.com/ninja-build/ninja/releases/download/v1.6.0/ninja-win.zip -FileName ninja.zip
  7z x ninja.zip -oC:\projects\deps\ninja > nul
  rm ninja.zip
)
@set PATH=C:\projects\deps\ninja;%PATH%
ninja --version

@echo off
