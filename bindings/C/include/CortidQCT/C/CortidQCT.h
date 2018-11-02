#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct VoxelVolume_t;
/// Voxel volume handle type
typedef struct VoxelVolume_t *VoxelVolume;

struct MeshFitter_t;
/// Mesh fitter handle type
typedef struct MeshFitter_t *MeshFitter;

struct Mesh_t;
/// Mesh handle type
typedef struct Mesh_t *Mesh;

/// Creates a voxel volume by loading from the given file
VoxelVolume createVoxelVolume(const char *filename);

/// Deletes a voxel volume
void deleteVoxelVolume(VoxelVolume vol);

/// Creates a mesh fitter given the configuration file
MeshFitter createMeshFitter(const char *filename);

/// Deletes a mesh fitter
void deleteMeshFitter(MeshFitter fitter);

/// Loads a mesh from file
Mesh loadMesh(const char *filename);

/// Loads a mesh from mesh and label file
Mesh loadMeshAndLabels(const char *meshFilename, const char *labelFilename);

/// Deletes a mesh
void deleteMesh(Mesh);

/// Return number of vertices of the given mesh
size_t meshVertexCount(Mesh mesh);

/// Return number of triangles of the given mesh
size_t meshTriangleCount(Mesh mesh);

/// Copies the mesh's vertices into the given buffer
size_t meshGetVertices(Mesh mesh, float **bufferPtr);

/// Copies the mesh's indices into the given buffer
size_t meshGetTriangles(Mesh mesh, ptrdiff_t **bufferPtr);

/// Copies the mesh's labels into the given buffer
size_t meshGetLabels(Mesh mesh, unsigned int **bufferPtr);

#ifdef __cplusplus
}
#endif
