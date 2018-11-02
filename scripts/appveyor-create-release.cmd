
if "%APPVEYOR_REPO_TAG%" == "true" (
  md CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64
  md "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\matlab"
  cp app/CortidQCT_CLI.exe "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\"
  cp -r "%APPVEYOR_BUILD_FOLDER%\\matlab\\+CortidQCT" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\matlab\\"
  cp -r "%APPVEYOR_BUILD_FOLDER%\\data" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\"
  cp "matlab\\floatVector2base64.mexw64" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64\\matlab\\+CortidQCT\\" 
  7z a -tzip CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64.zip CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64
  appveyor PushArtifact CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64.zip
)

