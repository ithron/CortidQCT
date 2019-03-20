# CortidQCT Changelog

## Next release

### Added
- Add perVertexNormals query to Mesh object
- Matlab Mesh object has now a `transform` method to apply affine
  transformations to the mesh
- Option to ignore samples outside the scan `ignoreExteriorSamples`, see [#52](https://github.com/ithron/CortidQCT/issues/52))
- CI builds for clang-7 and macOS (xcode 10.2)
- Ray-mesh intersection computation
- Interpolation of custom per-vertex attributes
- Mutable meshes in C and MATLAB API
  * Mesh constructor that reserves memory for vertices, indices and labels
  * Setters for mesh vertices, indices and labels
- Support sum of gaussian MTF modelling
- Changelog

## Removed
- Dropped support for older clang versions ( < 6.0.0 )

## Fixes
- Scaling of out-of-plane MTF
- Issue #47: CortidQCT.saveModel broken in MATLAB 2018b on linux

## Known Issues
- Xcode generator on macOS not working
  * Workaround: use 'UNIX Makefiles' generator
- Visual Studio builds not working on Windows
  * Workaround: Use clang compiler
