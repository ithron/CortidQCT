
if "%APPVEYOR_REPO_TAG%" == "true" (

  ninja install

  rename install CortidQCT-%APPVEYOR_REPO_TAG_NAME%-win64

  cp README.md "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-win64\\"
  cp LICENSE "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-win64\\"
  cp -r "%APPVEYOR_BUILD_FOLDER%\\images" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-win64\\"
  cp -r "%APPVEYOR_BUILD_FOLDER%\\data" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-win64\\"

  7z a -tzip CortidQCT-%APPVEYOR_REPO_TAG_NAME%-win64.zip CortidQCT-%APPVEYOR_REPO_TAG_NAME%-win64
  appveyor PushArtifact CortidQCT-%APPVEYOR_REPO_TAG_NAME%-win64.zip
)

