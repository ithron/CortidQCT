/**
 * @file      bindings/Matlab/include/CortidQCT/Matlab/CortidQCT.h
 *
 * @brief     This file contains the Matlab API definition.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

/**
 * @defgroup Matlab-API Matlab API
 * @brief Matlab API for CortidQCT.
 *
 * @{
 */
#ifndef CORTIDQCT_MATLAB_BINDINGS_CORTIDQCT_H__
#  define CORTIDQCT_MATLAB_BINDINGS_CORTIDQCT_H__

#  ifdef __cplusplus
extern "C" {

#    define CQCT_EXTERN extern "C"
#  else
#    define CQCT_EXTERN
#  endif

#  include <stddef.h>

// MARK: -
// MARK: Object Type / Generic Functions
/// @name Generic Functions / Object Type
/// @{

/// Generic 'object' type
typedef void *Id;

/// Retains an object, i.e. increments its retain count
CQCT_EXTERN Id CQCT_retain(Id obj);

/// Releases an object, i.e. decrements its retain count and releases
/// the memory held by the object iff the retain count reaces zero.
CQCT_EXTERN void CQCT_release(Id obj);

/// Adds the object to the current autorelease pool. It's released when the
/// pool is released.
CQCT_EXTERN Id CQCT_autorelease(Id obj);

/// Adds a new autorelease pool to the stack
CQCT_EXTERN void CQCT_autoreleasePoolPush();

/// Removes the current autorelease pool from the stack and releases all of
/// its objects
CQCT_EXTERN void CQCT_autoreleasePoolPop();

/// @}

// MARK: -
// MARK: Error Type
/**
 * @name Error type
 * @{
 */

/// Error types/Ids
enum CQCT_ErrorId { CQCT_ErrorId_Unknown, CQCT_ErrorId_InvalidArgument };

struct CQCT_Error_t;
/// Error type
typedef struct CQCT_Error_t *CQCT_Error;

/// Creates an error object
CQCT_EXTERN CQCT_Error CQCT_createError(enum CQCT_ErrorId id,
                                        const char *message);

/// Returns the error id
CQCT_EXTERN enum CQCT_ErrorId CQCT_errorType(CQCT_Error error);

/// Returns the error message
CQCT_EXTERN const char *CQCT_errorMessage(CQCT_Error error);

/// @}

// MARK: -
// MARK: VoxelVolume Type
/**
 * @name VoxelVolume type
 * @{
 */

struct CQCT_VoxelVolume_t;
/// Voxel volume handle type
typedef struct CQCT_VoxelVolume_t *CQCT_VoxelVolume;

/// Creates a voxel volume by loading from the given file
CQCT_EXTERN CQCT_VoxelVolume CQCT_createVoxelVolume(const char *filename,
                                                    CQCT_Error *error);

/// Returns the width of the voxel volume
CQCT_EXTERN size_t CQCT_voxelVolumeWidth(CQCT_VoxelVolume volume);

/// Returns the height of the voxel volume
CQCT_EXTERN size_t CQCT_voxelVolumeHeight(CQCT_VoxelVolume volume);

/// Returns the depth of the voxel volume
CQCT_EXTERN size_t CQCT_voxelVolumeDepth(CQCT_VoxelVolume volume);

/// Returns the width of a voxel
CQCT_EXTERN float CQCT_voxelVolumeVoxelWidth(CQCT_VoxelVolume volume);

/// Returns the height of a voxel
CQCT_EXTERN float CQCT_voxelVolumeVoxelHeight(CQCT_VoxelVolume volume);

/// Returns the depth of a voxel
CQCT_EXTERN float CQCT_voxelVolumeVoxelDepth(CQCT_VoxelVolume volume);

/// Copies the voxel data to the given buffer
CQCT_EXTERN size_t CQCT_voxelVolumeCopyVoxels(CQCT_VoxelVolume volume,
                                              float **buffer);

/// @}

// MARK: -
// MARK: Mesh Type
/**
 * @name Mesh type
 * @{
 */
struct CQCT_Mesh_t;
/// Mesh handle type
typedef struct CQCT_Mesh_t *CQCT_Mesh;

/// Creates an empty mesh
CQCT_EXTERN CQCT_Mesh CQCT_createMesh();

/**
 * @brief Creates and loads a mesh from file.
 *
 * @param[in] filename path to the file to load the mesh from
 * @param[out] error In case of an error, an error object is copied to `*error`.
 *   NULL may be passed here to not get any error object.
 * @return A newly created Mesh object.
 * @note The ownershop is NOT transfered to the caller.
 * @see CortidQCT::Mesh<float>::loadFromFile()
 */
CQCT_EXTERN CQCT_Mesh CQCT_meshFromFile(const char *filename,
                                        CQCT_Error *error);

/**
 * @brief Creates and loads a mesh from a mesh and a label file.
 *
 * @param[in] meshFilename path to the file to load the mesh from
 * @param[in] labelFilename path to the file to load the labels from
 * @param[out] error In case of an error, an error object is copied to `*error`.
 *   NULL may be passed here to not get any error object.
 * @return A newly created Mesh object.
 * @note The ownershop is NOT transfered to the caller.
 * @see CortidQCT::Mesh<float>::loadFromFile()
 */
CQCT_EXTERN CQCT_Mesh CQCT_meshAndLabelsFromFile(const char *meshFilename,
                                                 const char *labelFilename,
                                                 CQCT_Error *error);

/// Loads a mesh from file
CQCT_EXTERN int CQCT_loadMesh(CQCT_Mesh mesh, const char *filename,
                              CQCT_Error *error);

/// Loads a mesh from mesh and label file
CQCT_EXTERN int CQCT_loadMeshAndLabels(CQCT_Mesh mesh, const char *meshFilename,
                                       const char *labelFilename,
                                       CQCT_Error *error);

/// Writes mesh and labels to seperate files
CQCT_EXTERN int CQCT_meshAndLabelsWriteToFile(CQCT_Mesh mesh,
                                              const char *meshFilename,
                                              const char *labelsFilename,
                                              CQCT_Error *error);

/// Return number of vertices of the given mesh
CQCT_EXTERN size_t CQCT_meshVertexCount(CQCT_Mesh mesh);

/// Return number of triangles of the given mesh
CQCT_EXTERN size_t CQCT_meshTriangleCount(CQCT_Mesh mesh);

/**
 * @brief Copies the mesh's vertices into the given buffer.
 *
 * If the pointer pointed to by bufferPtr (i.e. `*bufferPtr`) is NULL,
 * memory is allocated, otherwise the pointed to memory is used.

 * @param mesh Mesh object to inspect
 * @param bufferPtr Pointer to a pointer to the start of a memory buffer, that
 is
 * large enough to hold `3 * N * sizeof(float)` (N = number of verices) bytes.
 * Alternativly, bufferPtr can point to a NULL pointer. In that case the
 * required memory is allocated by the function.
 * @return Number of copied bytes. In case `*bufferPtr == NULL` it's the size of
 * the allocated buffer.
 * @note The caller is responsible to release the memory of the buffer, even if
 * the memory was allocated by the function (when `*bufferPtr == NULL`).
 */
CQCT_EXTERN size_t CQCT_meshCopyVertices(CQCT_Mesh mesh, float **bufferPtr);

/**
 * @brief Copies the mesh's indices into the given buffer
 * @see CQCT_meshCopyVertices()
 */
CQCT_EXTERN size_t CQCT_meshCopyTriangles(CQCT_Mesh mesh,
                                          ptrdiff_t **bufferPtr);

/**
 * @brief Copies the mesh's labels into the given buffer
 * @see CQCT_meshCopyVertices()
 */
CQCT_EXTERN size_t CQCT_meshCopyLabels(CQCT_Mesh mesh,
                                       unsigned int **bufferPtr);

/// @}

// MARK: -
// MARK: MeshFitter Type
/**
 * @name MeshFitter object type
 * @{
 */

struct CQCT_MeshFitter_t;
/// Mesh fitter handle type
typedef struct CQCT_MeshFitter_t *CQCT_MeshFitter;

struct CQCT_MeshFitterResult_t;
/// Mesh fitter result object
typedef struct CQCT_MeshFitterResult_t *CQCT_MeshFitterResult;

/// Creates a mesh fitter given the configuration file
CQCT_EXTERN CQCT_MeshFitter CQCT_createMeshFitter(const char *filename,
                                                  CQCT_Error *error);

/// Returns the result mesh
CQCT_EXTERN CQCT_Mesh CQCT_meshFitterResultMesh(CQCT_MeshFitterResult result);

/// Fits the reference mesh to the given voxel volume
CQCT_EXTERN CQCT_MeshFitterResult CQCT_meshFitterFit(CQCT_MeshFitter meshFitter,
                                                     CQCT_VoxelVolume volume);

/// @}

#  ifdef __cplusplus
}
#  endif

#endif

/// @}
