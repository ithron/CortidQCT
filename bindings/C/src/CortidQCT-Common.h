#pragma once

#include "CortidQCT.h"
#include "cortidqct-c_export.h"

#include <CortidQCT/CortidQCT.h>

#include <cstddef>
#include <cstring>

namespace CortidQCT {
namespace Internal {
namespace C {

typedef void (*Deleter)(Id);

CQCT_EXTERN struct Control {
  std::size_t retainCount = 1;
  Deleter deleter;
};

template <class T> struct GenericObjectWrapper {
  Control control;
  T *objPtr = nullptr;

  ~GenericObjectWrapper() {
    if (objPtr != nullptr) { delete objPtr; }
  }
};

template <class T> void genericDeleter(Id obj) {
  auto objWrapperPtr = static_cast<GenericObjectWrapper<T> *>(obj);
  delete objWrapperPtr;
}

template <class T, class... Args> Id constructObject(Args &&... args) {
  auto wrapperPtr = new GenericObjectWrapper<T>;

  wrapperPtr->control.deleter = &genericDeleter<T>;
  wrapperPtr->objPtr = new T{std::forward<Args>(args)...};

  return static_cast<Id>(wrapperPtr);
}

CQCT_MeshFitterState
createMeshFitterState(CQCT_MeshFitter meshFitter,
                      ::CortidQCT::MeshFitter::State const &state);

struct MeshFitterState {
  CQCT_Mesh deformedMesh = nullptr;
  CQCT_Mesh referenceMesh = nullptr;

  CortidQCT::MeshFitter::State state;

  void updateMesh();

  ~MeshFitterState() {
    CQCT_release(deformedMesh);
    CQCT_release(referenceMesh);
  }
};

} // namespace C
} // namespace Internal
} // namespace CortidQCT

struct CQCT_MeshFitterState_t {
  CortidQCT::Internal::C::GenericObjectWrapper<
      CortidQCT::Internal::C::MeshFitterState>
      impl;
};

struct CQCT_VoxelVolume_t {
  CortidQCT::Internal::C::GenericObjectWrapper<CortidQCT::VoxelVolume> impl;
};

struct CQCT_Mesh_t {
  CortidQCT::Internal::C::GenericObjectWrapper<CortidQCT::Mesh<float>> impl;
};

struct CQCT_ColorToLabelMap_t {
  CortidQCT::Internal::C::GenericObjectWrapper<
      CortidQCT::ColorToLabelMaps::CustomMap>
      impl_;
};

struct CQCT_MeshFitter_t {
  CortidQCT::Internal::C::GenericObjectWrapper<CortidQCT::MeshFitter> impl;
};

