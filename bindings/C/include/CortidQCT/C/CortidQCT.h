/**
 * @file      bindings/C/include/CortidQCT/C/CortidQCT.h
 *
 * @brief     This file contains the C API definition.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

/**
 * @defgroup C-API C API
 * @brief C API for CortidQCT.
 *
 * The C API uses a reference counting object concept. Each 'object' has a
 * retain count. If the user wants to retain / keep an object the retain count
 * have to be incremented by called `CQCT_retain()`. Once the object is no
 * longer required, `CQCT_release()` should be called to decrement the retain
 * count. Once the retain count reaces zero the object is destroyed and the
 * allocated memory is released. THIS MECHANISM IS NOT THREAD SAFE!
 *
 * **Naming Conventions**
 * * All API functions are prefixed with `CQCT_`.
 * * Functions starting with `CQCT_create` transfer the ownership of the
 * returned object to the caller, i.e. the caller is responsible to calling
 * `CQCT_release()` on the object.
 *
 * **Memory Managfement**
 * * `CQCT_autoreleasePoolPush()` must be called before any other API call.
 * * `CQCT_autoreleasePoolPop()` must be called after the last API call.
 * * If functions that create autoreleased objects are called rapidely (e.g. in
 * a loop), those calls can be surrounded by an additional autorelease pool
 * layer.
 * * To keep and object around call `CQCT_retain()`.
 * * If a retained object is no longer required release it by called
 * `CQCT_release()`.
 * * Objects returned from `CQCT_create*` functions are automatically owned by
 * the caller, so no retain is required.
 * * Release all owned object by calling `CQCT_release` when they are no longer
 * needed.
 *
 * **Example**
 *
 * An example can be found in bindings/C/examples/cli.c:
 * @include examples/cli.c
 * @{
 */
#ifndef CORTIDQCT_C_BINDINGS_CORTIDQCT_H__
#  define CORTIDQCT_C_BINDINGS_CORTIDQCT_H__

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

/// VoxelVolume size
typedef struct {
  size_t width, heigth, depth;
} CQCT_VoxelVolumeSize;

/// VoxelVolume voxel size
typedef struct {
  float width, heigth, depth;
} CQCT_VoxelVolumeVoxelSize;

struct CQCT_VoxelVolume_t;
/// Voxel volume handle type
typedef struct CQCT_VoxelVolume_t *CQCT_VoxelVolume;

/// Creates an empty voxel volume
CQCT_EXTERN CQCT_VoxelVolume CQCT_createVoxelVolume();

/// Loads the voxel volume from file
CQCT_EXTERN int CQCT_voxelVolumeLoadFromFile(CQCT_VoxelVolume volume,
                                             const char *filename,
                                             CQCT_Error *error);

/// Returns the size of the voxel volume
CQCT_EXTERN CQCT_VoxelVolumeSize CQCT_voxelVolumeSize(CQCT_VoxelVolume volume);

/// Returns the size of a voxel
CQCT_EXTERN CQCT_VoxelVolumeVoxelSize
CQCT_voxelVolumeVoxelSize(CQCT_VoxelVolume volume);

/// Copies the voxel data to the given buffer
CQCT_EXTERN size_t CQCT_voxelVolumeCopyVoxels(CQCT_VoxelVolume volume,
                                              float **buffer);

/// @}

/** @name Custom Color To Label Map
 * @{
 */

struct CQCT_ColorToLabelMap_t;
/// ColorToLabelMap handle type
typedef struct CQCT_ColorToLabelMap_t *CQCT_ColorToLabelMap;

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
 * @brief Creates and loads a mesh from file using the given custom color to
 * label mapping.
 *
 * @param[in] filename path to the file to load the mesh from
 * @param[in] map custom color to label map object
 * @param[out] error In case of an error, an error object is copied to `*error`.
 *   NULL may be passed here to not get any error object.
 * @return A newly created Mesh object.
 * @note The ownershop is NOT transfered to the caller.
 * @see CortidQCT::Mesh<float>::loadFromFile()
 */
CQCT_EXTERN CQCT_Mesh CQCT_meshFromFileWithCustomMapping(
    const char *filename, CQCT_ColorToLabelMap map, CQCT_Error *error);

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

/// Loads a mesh from file using a custom color to label map
CQCT_EXTERN int CQCT_loadMeshWithCustomMapping(CQCT_Mesh mesh,
                                               const char *filename,
                                               CQCT_ColorToLabelMap map,
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
// MARK: ColorToLabelMap Type

/**
 * @name ColorToLabelMap object type
 * @{
 */

/// Creates a default color to label map
CQCT_EXTERN CQCT_ColorToLabelMap CQCT_createColorToLabelMap();

/// Loads the mapping from YAML file
CQCT_EXTERN int CQCT_colorToLabelMapLoadFromFile(CQCT_ColorToLabelMap map,
                                                 const char *filename,
                                                 CQCT_Error *error);

/// Returns number of entries of the map
CQCT_EXTERN size_t CQCT_colorToLabelMapEntryCount(CQCT_ColorToLabelMap map);

/// @brief Copies the entries of the map into the given buffer.
///
/// If `*bufferPtr == NULL` the memory for the buffer is allocated by the
/// function and the caller is responisble for releasing it.
/// Entries are in the follwing order: [R0, G0, B0, L0, R1, G1, B1, L1, ...].
CQCT_EXTERN size_t CQCT_colorToLabelMapCopyEntries(CQCT_ColorToLabelMap map,
                                                   unsigned int **bufferPtr);

/// @brief Sets the entries of the mapping
CQCT_EXTERN void CQCT_colorToLabelMapSetEntries(CQCT_ColorToLabelMap map,
                                                size_t count,
                                                const unsigned int *entries);

/// @}

// MARK: -
// MARK: MeshFitterState Type
/**
 * @name MeshFitterState object type
 * @{
 */

struct CQCT_MeshFitter_t;
/// Mesh fitter handle type
typedef struct CQCT_MeshFitter_t *CQCT_MeshFitter;

struct CQCT_MeshFitterState_t;
/// Mesh fitter state handle type
typedef struct CQCT_MeshFitterState_t *CQCT_MeshFitterState;

/// Mesh fitter result object
typedef CQCT_MeshFitterState CQCT_MeshFitterResult;

/// Creates an MeshFitter optimization state object
CQCT_EXTERN CQCT_MeshFitterState
CQCT_createMeshFitterState(CQCT_MeshFitter fitter, CQCT_VoxelVolume volume);

/// Returns the result reference mesh
CQCT_EXTERN CQCT_Mesh
CQCT_meshFitterResultReferenceMesh(CQCT_MeshFitterResult result);

/// Returns the result mesh
CQCT_EXTERN CQCT_Mesh CQCT_meshFitterResultMesh(CQCT_MeshFitterResult result);

/// Copies the displacement vector
CQCT_EXTERN size_t CQCT_meshFitterResultCopyDisplacementVector(
    CQCT_MeshFitterResult result, float **buffer);

/// Sets the displacement vector
CQCT_EXTERN void
CQCT_meshFitterResultSetDisplacementVector(CQCT_MeshFitterResult result,
                                           float const *buffer);

/// Copies the weight vector
CQCT_EXTERN size_t
CQCT_meshFitterResultCopyWeights(CQCT_MeshFitterResult result, float **buffer);

/// Sets the weight vector
CQCT_EXTERN void CQCT_meshFitterResultSetWeights(CQCT_MeshFitterResult result,
                                                 float const *buffer);

/// Copies the vertex normals
CQCT_EXTERN size_t CQCT_meshFitterResultCopyVertexNormals(
    CQCT_MeshFitterResult result, float **buffer);

/// Returns the number of volume sampling positions
CQCT_EXTERN size_t
CQCT_meshFitterResultVolumeSamplingPositionsCount(CQCT_MeshFitterResult result);

/// Copies the volume sampling positions
CQCT_EXTERN size_t CQCT_meshFitterResultCopyVolumeSamplingPositions(
    CQCT_MeshFitterResult result, float **buffer);

/// Copies the volume samples
CQCT_EXTERN size_t CQCT_meshFitterResultCopyVolumeSamples(
    CQCT_MeshFitterResult result, float **buffer);

/// Returns the minimum norm of the displacement vector
CQCT_EXTERN float
CQCT_meshFitterResultMinimumDisplacementNorm(CQCT_MeshFitterResult result);

/// Returns the current log likelihood of the model given the input volume
CQCT_EXTERN float
CQCT_meshFitterResultLogLikelihood(CQCT_MeshFitterResult result);

/// Sets the current log likelihood of the model given the input volume
CQCT_EXTERN void
CQCT_meshFitterResultSetLogLikelihood(CQCT_MeshFitterResult result, float ll);

/// Returns the current effective sigmaS (after applying decay)
CQCT_EXTERN float
CQCT_meshFitterResultEffectiveSigmaS(CQCT_MeshFitterResult result);

/// Returns the iteration count
CQCT_EXTERN size_t
CQCT_meshFitterResultIterationCount(CQCT_MeshFitterResult result);

/// Sets the iteration count
CQCT_EXTERN void
CQCT_meshFitterResultSetIterationCount(CQCT_MeshFitterResult result,
                                       size_t count);

/// Returns whether the optimization has converged
CQCT_EXTERN int CQCT_meshFitterResultHasConverged(CQCT_MeshFitterResult result);

/// Sets whether the optimization has converged
CQCT_EXTERN void
CQCT_meshFitterResultSetHasConverged(CQCT_MeshFitterResult result,
                                     int converged);

/// Returns whether the optimization was successfull
CQCT_EXTERN int CQCT_meshFitterResultSuccess(CQCT_MeshFitterResult result);

/// Sets whether the optimization was successfull
CQCT_EXTERN void CQCT_meshFitterResultSetSuccess(CQCT_MeshFitterResult result,
                                                 int success);

/// Returns the current non decreasing count
CQCT_EXTERN size_t
CQCT_meshFitterResultNonDecreasingCount(CQCT_MeshFitterResult result);

/// Sets the current non decreasing count
CQCT_EXTERN void
CQCT_meshFitterResultSetNonDecreasingCount(CQCT_MeshFitterResult result,
                                           size_t count);

/// Copies the model sampling positions
CQCT_EXTERN size_t CQCT_meshFitterStateCopyModelSamplingPositions(
    CQCT_MeshFitterState state, float **buffer);

/// @}

// MARK: -
// MARK: MeshFitter Type
/**
 * @name MeshFitter object type
 * @{
 */

/// Creates a mesh fitter given the configuration file
CQCT_EXTERN CQCT_MeshFitter CQCT_createMeshFitter(const char *filename,
                                                  CQCT_Error *error);

/// Fits the reference mesh to the given voxel volume
CQCT_EXTERN CQCT_MeshFitterResult CQCT_meshFitterFit(CQCT_MeshFitter meshFitter,
                                                     CQCT_VoxelVolume volume);

/// Runs one iteration of the fitting algorithm
CQCT_EXTERN int CQCT_meshFitterFitOneIteration(CQCT_MeshFitter meshFitter,
                                               CQCT_MeshFitterState state,
                                               CQCT_Error *error);

/// Runs the volume sampling step
CQCT_EXTERN int CQCT_meshFitterVolumeSamplingStep(CQCT_MeshFitter meshFitter,
                                                  CQCT_MeshFitterState state,
                                                  CQCT_Error *error);

/// Runs the displacement optimization step
CQCT_EXTERN int CQCT_meshFitterOptimalDisplacementStep(
    CQCT_MeshFitter meshFitter, CQCT_MeshFitterState state, CQCT_Error *error);

/// Runs the deformation optimization step
CQCT_EXTERN int CQCT_meshFitterOptimalDeformationStep(
    CQCT_MeshFitter meshFitter, CQCT_MeshFitterState state, CQCT_Error *error);

/// Runs the log likelihood computation step
CQCT_EXTERN int CQCT_meshFitterLogLikelihoodStep(CQCT_MeshFitter meshFitter,
                                                 CQCT_MeshFitterState state,
                                                 CQCT_Error *error);

/// Runs the convergence testing step
CQCT_EXTERN int CQCT_meshFitterConvergenceTestStep(CQCT_MeshFitter meshFitter,
                                                   CQCT_MeshFitterState state,
                                                   CQCT_Error *error);

/// @}

#  ifdef __cplusplus
}
#  endif

#endif

/// @}

