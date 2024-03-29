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
enum CQCT_ErrorId {
  CQCT_ErrorId_Unknown,
  CQCT_ErrorId_InvalidArgument,
  CQCT_ErrorId_OutOfRange
};

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

/// Calibrates the voxel volume
CQCT_EXTERN CQCT_VoxelVolume CQCT_voxelVolumeCalibrate(CQCT_VoxelVolume volume,
                                                       float slope,
                                                       float intercept);

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

/// Barycentric point type
struct CQCT_BarycentricPoint_t {
  float u;                 /// first barycentric coordinate
  float v;                 /// second barycentric coordinate
  ptrdiff_t triangleIndex; /// triangle index
};
typedef struct CQCT_BarycentricPoint_t CQCT_BarycentricPoint;

/// Ray type
struct CQCT_Ray_t {
  /// Ray origin in cartesian coordinates
  float x0, y0, z0;
  /// Ray direction in cartesian coordinates
  float dx, dy, dz;
};
typedef struct CQCT_Ray_t CQCT_Ray;

struct CQCT_RayMeshIntersection {
  /// Position on the mesh in barycentric coordinates
  CQCT_BarycentricPoint position;
  /// Signed distance frim the ray origin to the intersection
  float t;
  /// Explicit padding
  unsigned int : 4 * 8;
};
typedef struct CQCT_RayMeshIntersection CQCT_RayMeshIntersection;

/// Creates an empty mesh
CQCT_EXTERN CQCT_Mesh CQCT_createMesh();

/**
 * @brief Creates an mesh and reserve space for vertices and indices.
 *
 * Creates a mesh data structure and allocate memory for vertices, indices and
 * labels.
 *
 * @param[in] nVertices Number of vertices to allocate memory for
 * @param[in] nTriangles Number of triangles to allocate memory for
 * @param[out] error In case of an error, an error object is copied to
 * `*error`. NULL may be passed here to not get any error objects.
 * @return A newly created CQCT_Mesh opbject or NULL on error.
 * @note The ownership of the mesh object is transfered to the caller.
 * @note The ownership of the error object is NOT transfered to teh caller.
 * @note It is very unlikely that this function returns an error. The only
 * reason for failure is when the system is unable to allocate enough memory.
 * It is therefore kind of save to pass NULL for `error` here.
 */
CQCT_EXTERN CQCT_Mesh CQCT_createMeshAndAllocateMemory(size_t nVertices,
                                                       size_t nTriangles,
                                                       CQCT_Error *error);

/**
 * @brief Creates and loads a mesh from file.
 *
 * @param[in] filename path to the file to load the mesh from
 * @param[out] error In case of an error, an error object is copied to
 * `*error`. NULL may be passed here to not get any error object.
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
 * @brief Copies the vertices from the given buffer into the mesh data
 * structure.
 *
 * @note Vertices are expected to have the oder [v1x, v1y, v1z, v2x, v2y, v2z,
 * ...]
 * @param[in,out] mesh mesh to set the vertices for
 * @param[in] buffer buffer to copy the vertices from
 */
CQCT_EXTERN void CQCT_meshSetVertices(CQCT_Mesh mesh, float const *buffer);

/**
 * @brief Copies the mesh's per-vertex normals into the given buffer.
 *
 * If the pointer pointed to by bufferPtr (i.e. `*bufferPtr`) is NULL,
 * memory is allocated, otherwise the pointed to memory is used.

 * @param mesh Mesh object to inspect
 * @param bufferPtr Pointer to a pointer to the start of a memory buffer, that
 * is large enough to hold `3 * N * sizeof(float)` (N = number of verices)
 * bytes.* Alternativly, bufferPtr can point to a NULL pointer. In that case the
 * required memory is allocated by the function.
 * @return Number of copied bytes. In case `*bufferPtr == NULL` it's the size of
 * the allocated buffer.
 * @note The caller is responsible to release the memory of the buffer, even if
 * the memory was allocated by the function (when `*bufferPtr == NULL`).
 */
CQCT_EXTERN size_t CQCT_meshCopyVertexNormals(CQCT_Mesh mesh,
                                              float **bufferPtr);

/**
 * @brief Copies the per-vertex normals from the given buffer into the mesh data
 * structure.
 *
 * @note Normals are expected to have the oder [n1x, n1y, n1z, n2x, n2y, n2z,
 * ...]
 * @param[in,out] mesh mesh to set the vertex normals for
 * @param[in] buffer buffer to copy the normals from
 */
CQCT_EXTERN void CQCT_meshSetVertexNormals(CQCT_Mesh mesh, float const *buffer);

/**
 * @brief Copies the mesh's indices into the given buffer
 * @see CQCT_meshCopyVertices()
 */
CQCT_EXTERN size_t CQCT_meshCopyTriangles(CQCT_Mesh mesh,
                                          ptrdiff_t **bufferPtr);

/**
 * @brief Copies the triangle indices from the given buffer into the mesh data
 * structure.
 *
 * @note Vertices are expected to have the oder [i11, i12, i13, i21, i22, i23,
 * ...], where im, denotes the m-th index of the n-th triangle.
 * @param[in,out] mesh mesh to set the indices for
 * @param[in] buffer buffer to copy the indices from
 */
CQCT_EXTERN void CQCT_meshSetTriangles(CQCT_Mesh mesh, ptrdiff_t const *buffer);

/**
 * @brief Copies the mesh's labels into the given buffer
 * @see CQCT_meshCopyVertices()
 */
CQCT_EXTERN size_t CQCT_meshCopyLabels(CQCT_Mesh mesh,
                                       unsigned int **bufferPtr);

/**
 * @brief Copies the vertex labels from the given buffer into the mesh data
 * structure.
 *
 * @param[in,out] mesh mesh to set the indices for
 * @param[in] buffer buffer to copy the indices from
 */
CQCT_EXTERN void CQCT_meshSetLabels(CQCT_Mesh mesh, unsigned int const *buffer);

/**
 * @brief Converts the given list of barycentric points in cartesian
 * coordinates.
 *
 * @param[in] mesh Mesh object
 * @param[in] barycentricPtr pointer to list of points in barycentric
 * representation
 * @param[in] nPoints number of points to convert
 * @param[out] bufferPtr Pointer to the output buffer. The buffer must be able
 * to hold `3 * N` floats. Alternatively, `bufferPtr` can point to a `NULL`
 * pointer. In this case the required memory is allocated by the function. Note
 * that in both variants the caller is responsible for releasing the memory.
 * @param[out] error pointer to error object where an error is stored in case
 * of an error. Or `NULL` if no error should be returned.
 * @return 0 on success, a negative value on error
 * @pre `barycentricPtr != NULL || nPoints == 0`
 * @pre `bufferPtr != NULL`
 */
CQCT_EXTERN int CQCT_meshBarycentricToCartesian(
    CQCT_Mesh mesh, CQCT_BarycentricPoint const *barycentricPtr, size_t nPoints,
    float **bufferPtr, CQCT_Error *error);

/**
 * @brief Interpolates attribute values given at mesh vertices at arbitrary
 * points on the surface of the mesh.
 *
 * @param[in] mesh Mesh object
 * @param[in] barycentricPtr pointer to a list of points in barycentric
 * representation
 * @param[in] nPoints number of points
 * @param[in] attributePtr pointer to attribute buffer. Note that this buffer
 * must hold `attributeDimensions * N` values, where `N` is the number of
 * vertices in the mesh.
 * @param[in] attributeDimensions number of attribute dimensions
 * @param[out] bufferPtr Pointer where the interpolated values are stored. Must
 * be able to hold `nPoints * attributeDimensions` values. Alternatively, if
 * `*bufferPtr == NULL`, the required memory is allocated by the function and
 * the pointer is returned in `bufferPtr`. The caller is responisble for
 * releasing the memory, in both cases.
 * @param[out] error pointer to an error object. If `NULL` errors are ignored.
 * @return 0 on success, a negative value on error.
 * @pre `barycentricPtr != NULL || nPoint == 0`
 * @pre `bufferPtr != NULL || nPoints == 0`
 * @pre 'attributePtr != NULL || nPoints == 0'
 */
CQCT_EXTERN int CQCT_meshBarycentricInterpolation(
    CQCT_Mesh mesh, CQCT_BarycentricPoint const *barycentricPtr, size_t nPoints,
    float const *attributePtr, size_t attributeDimensions, float **bufferPtr,
    CQCT_Error *error);

/**
 * @brief Computes the intersection of a set of rays with the mesh.
 *
 * If for any ray no intersection can be found, its signed distance `t` is set
 * to infinity.
 *
 * @param[in] mesh Mesh object
 * @param[in] raysPtr pointer to array of CQCT_Ray objects
 * @param[in] nRays Number of rays
 * @param[out] intersectionsOutPtr Pointer that contains the memory address
 * where the intersection object should be stored. the underlying memory must
 * be able to hold `nRays * sizeof(CQCT_RayMeshIntersection)` bytes.
 * Alternatively, the address contained in `intersectionsOutPtr` can be set to
 * NULL. In this case the memory is allocated by the function. The caller is
 * responsible for relasing the memory, in all cases.
 * @pre `mesh != NULL || nRays == 0`
 * @pre `raysPtr != NULL || nRays == 0`
 * @pre `intersectionsOutPtr != NULL || nRays == 0`
 * @return number of bytes copied to `*intersecionsOutPtr`
 */
CQCT_EXTERN size_t
CQCT_meshRayIntersections(CQCT_Mesh mesh, CQCT_Ray *raysPtr, size_t nRays,
                          CQCT_RayMeshIntersection **intersectionsOutPtr);

/**
 * @brief Upsamples the given mesh `nTimes` without touchting the original
 * vertices
 *
 * @param nTimes number of upsample iterations
 * @pre `mesh != null || nTimes == 0`
 */
CQCT_EXTERN void CQCT_meshUpsample(CQCT_Mesh mesh, size_t nTimes);

/**
 * @brief Re-computes per-vertex normals
 *
 * @pre `mesh != NULL`
 * @param[in] mesh Mesh object
 */
CQCT_EXTERN void CQCT_meshUpdatePerVertexNormals(CQCT_Mesh mesh);

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
/// @deprecated Deprecated since version 1.3, will be removed in 2.0. Use the
/// per-vertex normals property of the defomed mesh instead.
CQCT_EXTERN size_t CQCT_meshFitterResultCopyVertexNormals(
    CQCT_MeshFitterResult result, float **buffer)
    __attribute__((deprecated("Use normals stored in deformedMesh instead.")));

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

/// Copies the current per-vertex log likelihood vector of the model given the
/// input volume to the given buffer
CQCT_EXTERN float
CQCT_meshFitterResultCopyPerVertexLogLikelihood(CQCT_MeshFitterResult result,
                                                float **buffer);

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
