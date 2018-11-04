#pragma once

#ifdef __cplusplus
extern "C" {
  
#define CQCT_EXTERN extern "C"
#else
#define CQCT_EXTERN
#endif
  

#include <stddef.h>

typedef void *Id;
  
enum CQCT_ErrorId {
  CQCT_ErrorId_Unknown,
  CQCT_ErrorId_InvalidArgument
};
  
CQCT_EXTERN void CQCT_retain(Id obj);
CQCT_EXTERN void CQCT_release(Id obj);
CQCT_EXTERN void CQCT_autorelease(Id obj);
  
CQCT_EXTERN void CQCT_autorelasePoolPush();
CQCT_EXTERN void CQCT_autoreleasePoolPop();
  
struct CQCT_Error_t;
/// Error type
typedef struct CQCT_Error_t *CQCT_Error;
  
struct CQCT_VoxelVolume_t;
/// Voxel volume handle type
typedef struct CQCT_VoxelVolume_t *CQCT_VoxelVolume;

struct CQCT_MeshFitter_t;
/// Mesh fitter handle type
typedef struct CQCT_MeshFitter_t *CQCT_MeshFitter;

struct CQCT_Mesh_t;
/// Mesh handle type
typedef struct CQCT_Mesh_t *CQCT_Mesh;

/// Creates an error object
CQCT_EXTERN CQCT_Error CQCT_createError(CQCT_ErrorId id,
                                        const char *message);
  
/// Returns the error id
CQCT_EXTERN CQCT_ErrorId CQCT_errorType(CQCT_Error error);
/// Returns the error message
CQCT_EXTERN const char *CQCT_errorMessage(CQCT_Error error);
  
/// Creates a voxel volume by loading from the given file
CQCT_EXTERN CQCT_VoxelVolume CQCT_createVoxelVolume(const char *filename,
                                                    CQCT_Error *error);

/// Creates a mesh fitter given the configuration file
CQCT_EXTERN CQCT_MeshFitter CQCT_createMeshFitter(const char *filename,
                                                  CQCT_Error *error);
  
/// Creates an empty mesh
CQCT_EXTERN CQCT_Mesh CQCT_createMesh();
  
/// Creates and loads a mesh from file
/// Returns autoreleased object
CQCT_EXTERN CQCT_Mesh CQCT_meshFromFile(const char *filename,
                                        CQCT_Error *error);
  
/// Creates and loads a mesh from mesh and label file
/// Returns autoreleased object
CQCT_EXTERN CQCT_Mesh CQCT_meshAndLabelsFromFile(const char *meshFilename,
                                                 const char *labelFilename,
                                                 CQCT_Error *error);
  
/// Loads a mesh from file
CQCT_EXTERN bool CQCT_loadMesh(CQCT_Mesh mesh,
                               const char *filename,
                               CQCT_Error *error);

/// Loads a mesh from mesh and label file
CQCT_EXTERN bool CQCT_loadMeshAndLabels(CQCT_Mesh mesh,
                                        const char *meshFilename,
                                        const char *labelFilename,
                                        CQCT_Error *error);

/// Return number of vertices of the given mesh
CQCT_EXTERN size_t CQCT_meshVertexCount(CQCT_Mesh mesh);

/// Return number of triangles of the given mesh
CQCT_EXTERN size_t CQCT_meshTriangleCount(CQCT_Mesh mesh);

/// Copies the mesh's vertices into the given buffer
CQCT_EXTERN size_t CQCT_meshCopyVertices(CQCT_Mesh mesh, float **bufferPtr);

/// Copies the mesh's indices into the given buffer
CQCT_EXTERN size_t CQCT_meshCopyTriangles(CQCT_Mesh mesh, ptrdiff_t **bufferPtr);

/// Copies the mesh's labels into the given buffer
CQCT_EXTERN size_t CQCT_meshCopyLabels(CQCT_Mesh mesh, unsigned int **bufferPtr);

#ifdef __cplusplus
}
#endif
