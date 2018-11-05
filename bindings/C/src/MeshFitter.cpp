#include "CortidQCT-Common.h"

#include <CortidQCT/CortidQCT.h>

#include <exception>

namespace CortidQCT {
namespace Internal {
namespace C {

struct MeshFitterResult {
  CQCT_Mesh mesh = nullptr;

  ~MeshFitterResult() { CQCT_release(mesh); }
};

} // namespace C
} // namespace Internal
} // namespace CortidQCT

using namespace CortidQCT;
using namespace CortidQCT::Internal::C;

struct CQCT_MeshFitter_t {
  CortidQCT::Internal::C::GenericObjectWrapper<CortidQCT::MeshFitter> impl;
};

struct CQCT_MeshFitterResult_t {
  CortidQCT::Internal::C::GenericObjectWrapper<
      CortidQCT::Internal::C::MeshFitterResult>
      impl;
};

CQCT_EXTERN CQCT_MeshFitter CQCT_createMeshFitter(const char *filename,
                                                  CQCT_Error *error) {
  try {

    return static_cast<CQCT_MeshFitter>(constructObject<MeshFitter>(filename));

  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
      CQCT_autorelease(*error);
    }
  } catch (std::exception const &e) {
    *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
    CQCT_autorelease(*error);
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(*error);
    }
  }

  return nullptr;
}

CQCT_EXTERN CQCT_Mesh CQCT_meshFitterResultMesh(CQCT_MeshFitterResult result) {
  assert(result != nullptr);
  return result->impl.objPtr->mesh;
}

CQCT_EXTERN CQCT_MeshFitterResult CQCT_meshFitterFit(CQCT_MeshFitter meshFitter,
                                                     CQCT_VoxelVolume volume) {
  assert(meshFitter != nullptr);
  assert(volume != nullptr);

  auto &fitter = *(meshFitter->impl.objPtr);
  auto const &vol = *(volume->impl.objPtr);

  auto res = fitter.fit(vol);

  CQCT_Mesh mesh = nullptr;
  if (res.deformedMesh) {
    mesh = CQCT_createMesh();
    *(static_cast<CQCT_Mesh>(mesh)->impl.objPtr) = std::move(*res.deformedMesh);
  }

  auto result = static_cast<CQCT_MeshFitterResult>(
      constructObject<MeshFitterResult>(mesh));
  CQCT_autorelease(result);

  return result;
}
