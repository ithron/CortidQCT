# CortidQCT Changelog

## Next release

### Added
- Mesh now has a nRing query function (MATLAB only)
- Mesh now has an upsampling method that interpolates normals and preserves
  labels (#57, #64)
- Mesh now has a per-vertex normals property (#62)
- Matlab Mesh object has now a `transform` method to apply affine
  transformations to the mesh
- Matlab meshes can now be copied
- Option to ignore samples outside the scan `ignoreExteriorSamples`, see [#52](https://github.com/ithron/CortidQCT/issues/52))
- CI builds for clang-7 and macOS (xcode 10.2)
- Ray-mesh intersection computation (#51, #59)
- Interpolation of custom per-vertex attributes
- Mutable meshes in C and MATLAB API
  * Mesh constructor that reserves memory for vertices, indices and labels
  * Setters for mesh vertices, indices and labels
- Support sum of gaussian MTF modelling
- Maintainable version management (#56)
- Changelog

## Removed
- Dropped support for older clang versions ( < 6.0.0 )
- MeshFitter::State::vertexNormals is now depreacated, use vertex normals
  property of deformed mesh instead (#63)


## Fixes
- Scaling of out-of-plane MTF
- Issue #47: CortidQCT.saveModel broken in MATLAB 2018b on linux
- Issue #60: (MATLAB) VoxelVolume.plot off by one error

## Known Issues
- Xcode generator on macOS not working
  * Workaround: use 'UNIX Makefiles' generator
- Visual Studio builds not working on Windows
  * Workaround: Use clang compiler
