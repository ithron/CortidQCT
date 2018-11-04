#include "CortidQCT-Common.h"

#include <CortidQCT/CortidQCT.h>

#include <exception>

using namespace CortidQCT;
using namespace CortidQCT::Internal::C;

struct CQCT_VoxelVolume_t {
  CortidQCT::Internal::C::GenericObjectWrapper<CortidQCT::VoxelVolume> impl;
};

CQCT_EXTERN CQCT_VoxelVolume CQCT_createVoxelVolume(const char *filename,
                                                    CQCT_Error *error) {
  
  try {
  
    return static_cast<CQCT_VoxelVolume>(constructObject<VoxelVolume>(filename));
    
  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      CQCT_release(*error);
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      CQCT_release(*error);
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
    }
  }
  
  return nullptr;
}

