#include "CortidQCT-Common.h"

#include <CortidQCT/CortidQCT.h>

#include <exception>

using namespace CortidQCT;
using namespace CortidQCT::Internal::C;

struct CQCT_MeshFitter_t {
  CortidQCT::Internal::C::GenericObjectWrapper<CortidQCT::MeshFitter> impl;
};

CQCT_EXTERN CQCT_MeshFitter CQCT_createMeshFitter(const char *filename,
                                                  CQCT_Error *error) {
  try {
    
    return static_cast<CQCT_MeshFitter>(constructObject<MeshFitter>(filename));
    
  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      CQCT_release(*error);
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
    }
  } catch (std::exception const &e) {
    CQCT_release(*error);
    *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
  }
  
  return nullptr;
}

