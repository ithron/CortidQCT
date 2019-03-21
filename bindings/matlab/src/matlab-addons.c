#include "CortidQCT.h"
#include "cortidqct-matlab_export.h"

#include <assert.h>
#include <stdlib.h>

struct CORTIDQCT_MATLAB_NO_EXPORT Size {
  size_t width, height, depth;
};

struct CORTIDQCT_MATLAB_NO_EXPORT VoxelSize {
  float width, height, depth;
};

extern struct Size CQCT_voxelVolumeSize(CQCT_VoxelVolume volume);
extern struct VoxelSize CQCT_voxelVolumeVoxelSize(CQCT_VoxelVolume volume);

CORTIDQCT_MATLAB_EXPORT CQCT_EXTERN size_t
CQCT_voxelVolumeWidth(CQCT_VoxelVolume volume) {
  return CQCT_voxelVolumeSize(volume).width;
}

CORTIDQCT_MATLAB_EXPORT CQCT_EXTERN size_t
CQCT_voxelVolumeHeight(CQCT_VoxelVolume volume) {
  return CQCT_voxelVolumeSize(volume).height;
}

CORTIDQCT_MATLAB_EXPORT CQCT_EXTERN size_t
CQCT_voxelVolumeDepth(CQCT_VoxelVolume volume) {
  return CQCT_voxelVolumeSize(volume).depth;
}

CORTIDQCT_MATLAB_EXPORT CQCT_EXTERN float
CQCT_voxelVolumeVoxelWidth(CQCT_VoxelVolume volume) {
  return CQCT_voxelVolumeVoxelSize(volume).width;
}

CORTIDQCT_MATLAB_EXPORT CQCT_EXTERN float
CQCT_voxelVolumeVoxelHeight(CQCT_VoxelVolume volume) {
  return CQCT_voxelVolumeVoxelSize(volume).height;
}

CORTIDQCT_MATLAB_EXPORT CQCT_EXTERN float
CQCT_voxelVolumeVoxelDepth(CQCT_VoxelVolume volume) {
  return CQCT_voxelVolumeVoxelSize(volume).depth;
}

struct CQCT_RayMeshIntersection {
  /// Position on the mesh in barycentric coordinates
  CQCT_BarycentricPoint position;
  /// Signed distance frim the ray origin to the intersection
  float t;
  /// Explicit padding
  unsigned int : 4 * 8;
};
typedef struct CQCT_RayMeshIntersection CQCT_RayMeshIntersection;

extern size_t
CQCT_meshRayIntersections(CQCT_Mesh mesh, CQCT_Ray *raysPtr, size_t nRays,
                          CQCT_RayMeshIntersection **intersectionsOutPtr);

CORTIDQCT_MATLAB_EXPORT CQCT_EXTERN size_t CQCT_meshRayIntersectionsBuffers(
    CQCT_Mesh mesh, CQCT_Ray *raysPtr, size_t nRays, float **positionsOutPtr,
    ptrdiff_t **indicesOutPtr, float **distancesOutPtr) {

  CQCT_RayMeshIntersection *buffer = NULL;

  assert(positionsOutPtr != NULL);
  assert(indicesOutPtr != NULL);
  assert(distancesOutPtr != NULL);

  CQCT_meshRayIntersections(mesh, raysPtr, nRays, &buffer);

  if (*positionsOutPtr == NULL) {
    *positionsOutPtr = (float *)malloc(nRays * 2 * sizeof(float));
  }
  if (*indicesOutPtr == NULL) {
    *indicesOutPtr = (ptrdiff_t *)malloc(nRays * sizeof(ptrdiff_t));
  }
  if (*distancesOutPtr == NULL) {
    *distancesOutPtr = (float *)malloc(nRays * sizeof(float));
  }

  // copy data
  for (size_t i = 0; i < nRays; ++i) {
    (*positionsOutPtr)[2 * i + 0] = buffer[i].position.u;
    (*positionsOutPtr)[2 * i + 1] = buffer[i].position.v;
    (*indicesOutPtr)[i] = buffer[i].position.triangleIndex;
    (*distancesOutPtr)[i] = buffer[i].t;
  }

  free(buffer);

  return nRays * (sizeof(float) * 2 + sizeof(ptrdiff_t) + sizeof(float));
}

extern void CQCT_autoreleasePoolPush(void);
extern void CQCT_autoreleasePoolPop(void);

#ifdef _WIN32

#  include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      CQCT_autoreleasePoolPush();
      break;

    case DLL_PROCESS_DETACH:
      CQCT_autoreleasePoolPop();
      break;

    default:
      break;
  }
  return TRUE;
}

#else

CORTIDQCT_MATLAB_EXPORT void __attribute__((constructor)) CQCT_init(void);
CORTIDQCT_MATLAB_EXPORT void __attribute__((destructor)) CQCT_fini(void);

CORTIDQCT_MATLAB_EXPORT void __attribute__((constructor)) CQCT_init(void) {
  CQCT_autoreleasePoolPush();
}

CORTIDQCT_MATLAB_EXPORT void __attribute__((destructor)) CQCT_fini(void) {
  CQCT_autoreleasePoolPop();
}

#endif

