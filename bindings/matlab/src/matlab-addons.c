#include "CortidQCT.h"
#include "cortidqct-matlab_export.h"

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

