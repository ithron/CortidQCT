#include "CortidQCT-Common.h"

#include <CortidQCT/CortidQCT.h>

#include <algorithm>
#include <exception>

using namespace CortidQCT;
using namespace CortidQCT::Internal::C;

namespace CortidQCT {
namespace Internal {
namespace C {

CQCT_MeshFitterState
createMeshFitterState(CQCT_MeshFitter meshFitter,
                      ::CortidQCT::MeshFitter::State const &state) {
  assert(meshFitter != nullptr);

  auto stateObj = static_cast<CQCT_MeshFitterState>(
      constructObject<MeshFitterState>(nullptr, nullptr, state));

  return stateObj;
}

void MeshFitterState::updateMesh() {
  assert(deformedMesh != nullptr);
  using std::copy;

  auto &targetMesh = *deformedMesh->impl.objPtr;

  targetMesh.withUnsafeVertexPointer([this](float *destPtr) {
    this->state.deformedMesh.withUnsafeVertexPointer(
        [this, destPtr](float const *srcPtr) {
          auto const size = static_cast<std::ptrdiff_t>(
              this->state.deformedMesh.vertexCount() * 3);
          copy(srcPtr, srcPtr + size, destPtr);
        });
  });
}

} // namespace C
} // namespace Internal
} // namespace CortidQCT

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_MeshFitterState CQCT_createMeshFitterState(
    CQCT_MeshFitter meshFitter, CQCT_VoxelVolume volume) {
  assert(meshFitter != nullptr);
  assert(volume != nullptr);

  auto internalState = meshFitter->impl.objPtr->init(*volume->impl.objPtr);

  return CortidQCT::Internal::C::createMeshFitterState(meshFitter,
                                                       internalState);
}

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_Mesh
CQCT_meshFitterResultMesh(CQCT_MeshFitterResult result) {
  assert(result != nullptr);

  auto &obj = *result->impl.objPtr;

  if (obj.deformedMesh == nullptr) {
    obj.deformedMesh = static_cast<CQCT_Mesh>(
        constructObject<CortidQCT::Mesh<float>>(obj.state.deformedMesh));
  }

  return obj.deformedMesh;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_Mesh
CQCT_meshFitterResultReferenceMesh(CQCT_MeshFitterResult result) {
  assert(result != nullptr);

  auto &obj = *result->impl.objPtr;

  if (obj.referenceMesh == nullptr) {
    obj.referenceMesh = static_cast<CQCT_Mesh>(
        constructObject<CortidQCT::Mesh<float>>(obj.state.referenceMesh));
  }

  return obj.referenceMesh;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_meshFitterResultCopyDisplacementVector(CQCT_MeshFitterResult result,
                                            float **buffer) {
  assert(result != nullptr);
  assert(buffer != nullptr);

  auto const &state = result->impl.objPtr->state;

  auto const size = state.displacementVector.size() * sizeof(float);

  if (*buffer == nullptr) { *buffer = static_cast<float *>(malloc(size)); }

  std::copy(state.displacementVector.cbegin(), state.displacementVector.cend(),
            *buffer);

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_meshFitterResultCopyWeights(CQCT_MeshFitterResult result, float **buffer) {
  assert(result != nullptr);
  assert(buffer != nullptr);

  auto const &state = result->impl.objPtr->state;

  auto const size = state.weights.size() * sizeof(float);

  if (*buffer == nullptr) { *buffer = static_cast<float *>(malloc(size)); }

  std::copy(state.weights.cbegin(), state.weights.cend(), *buffer);

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t CQCT_meshFitterResultCopyVertexNormals(
    CQCT_MeshFitterResult result, float **buffer) {
  assert(result != nullptr);
  assert(buffer != nullptr);

  auto const &state = result->impl.objPtr->state;

  auto const size = state.vertexNormals.size() * 3 * sizeof(float);

  if (*buffer == nullptr) { *buffer = static_cast<float *>(malloc(size)); }

  memcpy(*buffer, reinterpret_cast<float const *>(state.vertexNormals.data()),
         size);

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_meshFitterResultVolumeSamplingPositionsCount(
    CQCT_MeshFitterResult result) {
  assert(result != nullptr);

  return result->impl.objPtr->state.volumeSamplingPositions.size();
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_meshFitterResultCopyVolumeSamplingPositions(CQCT_MeshFitterResult result,
                                                 float **buffer) {
  assert(result != nullptr);
  assert(buffer != nullptr);

  auto const &state = result->impl.objPtr->state;

  auto const size = state.volumeSamplingPositions.size() * 3 * sizeof(float);

  if (*buffer == nullptr) { *buffer = static_cast<float *>(malloc(size)); }

  memcpy(*buffer,
         reinterpret_cast<float const *>(state.volumeSamplingPositions.data()),
         size);

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t CQCT_meshFitterResultCopyVolumeSamples(
    CQCT_MeshFitterResult result, float **buffer) {
  assert(result != nullptr);
  assert(buffer != nullptr);

  auto const &state = result->impl.objPtr->state;

  auto const size = state.volumeSamples.size() * sizeof(float);

  if (*buffer == nullptr) { *buffer = static_cast<float *>(malloc(size)); }

  std::copy(state.volumeSamples.cbegin(), state.volumeSamples.cend(), *buffer);

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN float
CQCT_meshFitterResultMinimumDisplacementNorm(CQCT_MeshFitterResult result) {
  assert(result != nullptr);

  auto const &state = result->impl.objPtr->state;

  return state.minDisNorm;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN float
CQCT_meshFitterResultLogLikelihood(CQCT_MeshFitterResult result) {
  assert(result != nullptr);
  auto const &state = result->impl.objPtr->state;

  return state.logLikelihood;
}

/// Returns the current effective sigmaS (after applying decay)
CORTIDQCT_C_EXPORT CQCT_EXTERN float
CQCT_meshFitterResultEffectiveSigmaS(CQCT_MeshFitterResult result) {
  assert(result != nullptr);
  auto const &state = result->impl.objPtr->state;

  return state.effectiveSigmaS;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_meshFitterResultIterationCount(CQCT_MeshFitterResult result) {
  assert(result != nullptr);

  auto const &state = result->impl.objPtr->state;

  return state.iteration;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int
CQCT_meshFitterResultHasConverged(CQCT_MeshFitterResult result) {
  assert(result != nullptr);

  auto const &state = result->impl.objPtr->state;

  return state.converged;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int
CQCT_meshFitterResultSuccess(CQCT_MeshFitterResult result) {
  assert(result != nullptr);

  auto const &state = result->impl.objPtr->state;

  return state.success;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_meshFitterResultNonDecreasingCount(CQCT_MeshFitterResult result) {
  assert(result != nullptr);

  auto const &state = result->impl.objPtr->state;

  return state.nonDecreasing;
}

