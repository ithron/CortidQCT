# CortidQCT Changelog

## 1.3.0

### Added
- Support for volume calibration
- Support for measurement model version 2.0.0
- Docker support
- Support for modeling complex PSF function (PR [#67](https://github.com/ithron/CortidQCT/pull/67))
- Mesh now has a nRing query function (MATLAB only) ([#65](https://github.com/ithron/CortidQCT/issues/65))
- Mesh now has an upsampling method that interpolates normals and preserves
  labels ([#57](https://github.com/ithron/CortidQCT/issues/57), [#64](https://github.com/ithron/CortidQCT/issues/64))
- Mesh now has a per-vertex normals property ([#62](https://github.com/ithron/CortidQCT/issues/62))
- Matlab Mesh object has now a `transform` method to apply affine
  transformations to the mesh
- Matlab meshes can now be copied
- Option to ignore samples outside the scan `ignoreExteriorSamples`, see [#52](https://github.com/ithron/CortidQCT/issues/52))
- CI builds for clang-7 and macOS (xcode 10.2)
- Ray-mesh intersection computation ([#51](https://github.com/ithron/CortidQCT/issues/51), [#59](https://github.com/ithron/CortidQCT/issues/59))
- Interpolation of custom per-vertex attributes
- Mutable meshes in C and MATLAB API
  * Mesh constructor that reserves memory for vertices, indices and labels
  * Setters for mesh vertices, indices and labels
- Support sum of gaussian MTF modelling
- Support cosine modulated sum of gaussians PSF modelling
- Maintainable version management ([#56](https://github.com/ithron/CortidQCT/issues/56))
- Changelog
- Inheritance for configuration files (rudimentary)

## Removed
- OpenCL support
- Dropped support for older clang versions ( < 6.0.0 )
- MeshFitter::State::vertexNormals is now depreacated, use vertex normals
  property of deformed mesh instead ([#63](https://github.com/ithron/CortidQCT/issues/63))
- OpenCL support


## Fixes

- Labels were not included when saving meshes to SIMesh format
- Issue [#70](https://github.com/ithron/CortidQCT/issues/70):Crash when using MeshFitter.fit(volume) from MATLAB 2019b
- Issue [#69](https://github.com/ithron/CortidQCT/issues/68): Failed postcondition when loading highly non-convex meshes 
- Compatibility with newer MATLAB versions
- Scaling of out-of-plane MTF
- Issue [#47](https://github.com/ithron/CortidQCT/issues/47): CortidQCT.saveModel broken in MATLAB 2018b on linux
- Issue [#60](https://github.com/ithron/CortidQCT/issues/60): (MATLAB) VoxelVolume.plot off by one error
- Issue [#70](https://github.com/ithron/CortidQCT/issues/70): Crash when using MeshFitter.fit(volume) from MATLAB or C
- Issue [#71](https://github.com/ithron/CortidQCT/issues/70): CortidQCT\_CLI crashes when build in debug mode
- Indexing issue where the zero planes of a volume were beein ignored by the
  fitter
- Labels were not included when saving meshes to SIMesh format

## Known Issues
- Xcode generator on macOS not working
  * Workaround: use 'UNIX Makefiles' generator
- Visual Studio builds not working on Windows
  * Workaround: Use clang compiler
