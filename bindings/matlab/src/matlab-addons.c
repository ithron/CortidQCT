#include "CortidQCT.h"

struct Size {
  size_t width, height, depth;
};

struct VoxelSize {
  float width, height, depth;
};

extern struct Size CQCT_voxelVolumeSize(CQCT_VoxelVolume volume);
extern struct VoxelSize CQCT_voxelVolumeVoxelSize(CQCT_VoxelVolume volume);

CQCT_EXTERN size_t CQCT_voxelVolumeWidth(CQCT_VoxelVolume volume) {
  return CQCT_voxelVolumeSize(volume).width;
}

CQCT_EXTERN size_t CQCT_voxelVolumeHeight(CQCT_VoxelVolume volume) {
  return CQCT_voxelVolumeSize(volume).height;
}

CQCT_EXTERN size_t CQCT_voxelVolumeDepth(CQCT_VoxelVolume volume) {
  return CQCT_voxelVolumeSize(volume).depth;
}

