
if "%APPVEYOR_REPO_TAG%" == "true" (
  md CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64
  md "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\bin"
  md "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\lib"
  md "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\matlab"
  cp app\CortidQCT_CLI.exe "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\bin"
  cp lib\CortidQCT-Core.lib "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\lib"
  cp bindings\C\CortidQCT-C.lib "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\lib"
  cp README.md "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\"
  cp LICENSE "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\"
  cp -r "%APPVEYOR_BUILD_FOLDER%\\matlab\\+CortidQCT" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\matlab\\"
  cp -r "%APPVEYOR_BUILD_FOLDER%\\data" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\"
  cp -r "%APPVEYOR_BUILD_FOLDER%\\include" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\"
  cp -r "%APPVEYOR_BUILD_FOLDER%\\bindings\\C\\include" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\"
  cp -r "%APPVEYOR_BUILD_FOLDER%\\images" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\"
  cp "matlab\\floatVector2base64.mexw64" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\matlab\\+CortidQCT\\"
  7z a -tzip CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64.zip CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64
  appveyor PushArtifact CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64.zip
)

