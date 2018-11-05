#include "CortidQCT-Common.h"

#include <CortidQCT/CortidQCT.h>

#include <exception>

using namespace CortidQCT;
using namespace CortidQCT::Internal::C;

CQCT_EXTERN CQCT_VoxelVolume CQCT_createVoxelVolume(const char *filename,
                                                    CQCT_Error *error) {

  try {

    return static_cast<CQCT_VoxelVolume>(
        constructObject<VoxelVolume>(filename));

  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
      CQCT_autorelease(*error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(*error);
    }
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(*error);
    }
  }

  return nullptr;
}

CQCT_EXTERN CQCT_VoxelVolumeSize CQCT_voxelVolumeSize(CQCT_VoxelVolume volume) {
  assert(volume != nullptr);

  auto const size = volume->impl.objPtr->size();
  return CQCT_VoxelVolumeSize{size.width, size.height, size.depth};
}

CQCT_EXTERN CQCT_VoxelVolumeVoxelSize
CQCT_voxelVolumeVoxelSize(CQCT_VoxelVolume volume) {
  assert(volume != nullptr);

  auto const size = volume->impl.objPtr->voxelSize();
  return CQCT_VoxelVolumeVoxelSize{size.width, size.height, size.depth};
}

CQCT_EXTERN size_t CQCT_voxelVolumeCopyVoxels(CQCT_VoxelVolume volume,
                                              float **buffer) {
  assert(volume != nullptr);
  assert(buffer != nullptr);

  auto const &vv = *volume->impl.objPtr;
  auto const size = vv.size().linear() * sizeof(float);

  if (*buffer == nullptr) { *buffer = static_cast<float *>(malloc(size)); }

  vv.withUnsafeDataPointer(
      [buffer, size](float const *srcPtr) { memcpy(*buffer, srcPtr, size); });

  return size;
}
