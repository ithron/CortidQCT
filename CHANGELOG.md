# CortidQCT Changelog

## Next release

### Added
- CI builds for clang-7 and macOS (xcode 10.2)
- Ray-mesh intersection computation
- Interpolation of custom per-vertex attributes
- Mutable meshes in C and MATLAB API
  * Mesh constructor that reserved memory for data
  * Setters for mesh vertices, indices and labels
- Support sum of gaussian MTF modelling
- Changelog

## Removed
- Dropped support for older clang versions ( < 6.0.0 )

## Fixes
- Scaling of out-of-plane MTF
- Issue #47: CortidQCT.saveModel broken in MATLAB 2018b on linux

## Known Issues
- Xcode generator on macOS not workin
  * Workaround: use 'UNIX Makefiles' generator
- Visual Studio builds not working on Windows
  * Workaround: Use clang compiler
