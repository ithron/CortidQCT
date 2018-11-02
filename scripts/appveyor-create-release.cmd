
if "%APPVEYOR_REPO_TAG%" == "true" (
  md CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windown-x64
  cp app/CortidQCT_CLI.exe "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windown-x64\\"
  cp "%APPVEYOR_BUILD_FOLDER%\\matlab" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windown-x64\\"
  cp "matlab\\floatVector2base64.mexw64" "CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windown-x64\\matlab\\+CortidQCT\\" 
  7z a -tzip CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64.zip CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windown-x64
  appveyor PushArtifact CortidQCT-%APPVEYOR_REPO_TAG_NAME%-Windows-x64.zip
)

