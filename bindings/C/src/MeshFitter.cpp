#include "CortidQCT-Common.h"

#include <CortidQCT/CortidQCT.h>

#include <exception>

using namespace CortidQCT;
using namespace Internal;
using namespace C;

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_MeshFitter
CQCT_createMeshFitter(const char *filename, CQCT_Error *error) {
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

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_MeshFitterResult
CQCT_meshFitterFit(CQCT_MeshFitter meshFitter, CQCT_VoxelVolume volume) {
  assert(meshFitter != nullptr);
  assert(volume != nullptr);

  auto &fitter = *(meshFitter->impl.objPtr);
  auto const &vol = *(volume->impl.objPtr);

  auto res = fitter.fit(vol);

  auto result = createMeshFitterState(meshFitter, std::move(res));
  CQCT_autorelease(result);

  return result;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int
CQCT_meshFitterFitOneIteration(CQCT_MeshFitter meshFitter,
                               CQCT_MeshFitterState state, CQCT_Error *error) {
  assert(meshFitter != nullptr);
  assert(state != nullptr);

  auto const &fitter = *meshFitter->impl.objPtr;
  auto &stateRef = state->impl.objPtr->state;

  try {

    fitter.fitOneIteration(stateRef);
    state->impl.objPtr->updateMesh();
    return true;

  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
      CQCT_autorelease(error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(error);
    }
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(error);
    }
  }

  return false;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int CQCT_meshFitterVolumeSamplingStep(
    CQCT_MeshFitter meshFitter, CQCT_MeshFitterState state, CQCT_Error *error) {
  assert(meshFitter != nullptr);
  assert(state != nullptr);

  auto const &fitter = *meshFitter->impl.objPtr;
  auto &stateRef = state->impl.objPtr->state;

  try {

    fitter.volumeSamplingStep(stateRef);
    return true;

  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
      CQCT_autorelease(error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(error);
    }
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(error);
    }
  }

  return false;
}

CORTIDQCT_C_EXPORT
CQCT_EXTERN int CQCT_meshFitterOptimalDisplacementStep(
    CQCT_MeshFitter meshFitter, CQCT_MeshFitterState state, CQCT_Error *error) {
  assert(meshFitter != nullptr);
  assert(state != nullptr);

  auto const &fitter = *meshFitter->impl.objPtr;
  auto &stateRef = state->impl.objPtr->state;

  try {

    fitter.optimalDisplacementStep(stateRef);
    return true;

  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
      CQCT_autorelease(error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(error);
    }
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(error);
    }
  }

  return false;
}

CORTIDQCT_C_EXPORT
CQCT_EXTERN int CQCT_meshFitterOptimalDeformationStep(
    CQCT_MeshFitter meshFitter, CQCT_MeshFitterState state, CQCT_Error *error) {
  assert(meshFitter != nullptr);
  assert(state != nullptr);

  auto const &fitter = *meshFitter->impl.objPtr;
  auto &stateRef = state->impl.objPtr->state;

  try {

    fitter.optimalDeformationStep(stateRef);

    state->impl.objPtr->updateMesh();

    return true;

  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
      CQCT_autorelease(error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(error);
    }
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(error);
    }
  }

  return false;
}

CORTIDQCT_C_EXPORT
CQCT_EXTERN int CQCT_meshFitterLogLikelihoodStep(CQCT_MeshFitter meshFitter,
                                                 CQCT_MeshFitterState state,
                                                 CQCT_Error *error) {
  assert(meshFitter != nullptr);
  assert(state != nullptr);

  auto const &fitter = *meshFitter->impl.objPtr;
  auto &stateRef = state->impl.objPtr->state;

  try {

    fitter.logLikelihoodStep(stateRef);
    return true;

  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
      CQCT_autorelease(error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(error);
    }
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(error);
    }
  }

  return false;
}

CORTIDQCT_C_EXPORT
CQCT_EXTERN int CQCT_meshFitterConvergenceTestStep(CQCT_MeshFitter meshFitter,
                                                   CQCT_MeshFitterState state,
                                                   CQCT_Error *error) {
  assert(meshFitter != nullptr);
  assert(state != nullptr);

  auto const &fitter = *meshFitter->impl.objPtr;
  auto &stateRef = state->impl.objPtr->state;

  try {

    fitter.convergenceTestStep(stateRef);
    return true;

  } catch (std::invalid_argument const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_InvalidArgument, e.what());
      CQCT_autorelease(error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(error);
    }
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(error);
    }
  }

  return false;
}
