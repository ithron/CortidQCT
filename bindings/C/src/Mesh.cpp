#include "CortidQCT-Common.h"

#include <CortidQCT/CortidQCT.h>

using namespace CortidQCT;
using namespace CortidQCT::Internal::C;

CQCT_EXTERN CQCT_Mesh CQCT_createMesh() {
  return static_cast<CQCT_Mesh>(constructObject<Mesh<float>>());
}

CQCT_EXTERN CQCT_Mesh CQCT_meshFromFile(const char *filename,
                                        CQCT_Error *error) {
  auto mesh = CQCT_createMesh();
  CQCT_loadMesh(mesh, filename, error);
  
  return static_cast<CQCT_Mesh>(CQCT_autorelease(mesh));
}

CQCT_EXTERN CQCT_Mesh CQCT_meshAndLabelsFromFile(const char *meshFilename,
                                                 const char *labelFilename,
                                                 CQCT_Error *error) {
  auto mesh = CQCT_createMesh();
  CQCT_loadMeshAndLabels(mesh, meshFilename, labelFilename, error);
  
  return static_cast<CQCT_Mesh>(CQCT_autorelease(mesh));
}

CQCT_EXTERN bool CQCT_loadMesh(CQCT_Mesh mesh,
                               const char *filename,
                               CQCT_Error *error) {
  assert(mesh != nullptr);
  try {
    
    mesh->impl.objPtr->loadFromFile(filename);
    return true;
    
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
  }
  
  return false;
}

CQCT_EXTERN bool CQCT_loadMeshAndLabels(CQCT_Mesh mesh,
                                        const char *meshFilename,
                                        const char *labelsFilename,
                                        CQCT_Error *error) {
  assert(mesh != nullptr);
  try {
    
    mesh->impl.objPtr->loadFromFile(meshFilename, labelsFilename);
    return true;
    
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
  }
  
  return false;
}

CQCT_EXTERN size_t CQCT_meshVertexCount(CQCT_Mesh mesh) {
  assert(mesh != nullptr);
  
  return mesh->impl.objPtr->vertexCount();
}

CQCT_EXTERN size_t CQCT_meshTriangleCount(CQCT_Mesh mesh) {
  assert(mesh != nullptr);
  
  return mesh->impl.objPtr->triangleCount();
}

CQCT_EXTERN size_t CQCT_meshCopyVertices(CQCT_Mesh mesh, float **bufferPtr) {
  assert(mesh != nullptr);
  assert(bufferPtr != nullptr);
  
  auto const &meshRef = *(mesh->impl.objPtr);
  auto const size = 3 * meshRef.vertexCount() * sizeof(float);
  
  
  if (*bufferPtr == nullptr) {
    *bufferPtr = static_cast<float *>(malloc(size));
  }
  
  meshRef.withUnsafeVertexPointer([bufferPtr, size] (const float *src) {
    memcpy(*bufferPtr, src, size);
  });
  
  return size;
}

CQCT_EXTERN size_t CQCT_meshCopyTriangles(CQCT_Mesh mesh, ptrdiff_t **bufferPtr) {
  assert(mesh != nullptr);
  assert(bufferPtr != nullptr);
  
  auto const &meshRef = *(mesh->impl.objPtr);
  auto const size = 3 * meshRef.triangleCount() * sizeof(ptrdiff_t);
  
  
  if (*bufferPtr == nullptr) {
    *bufferPtr = static_cast<ptrdiff_t *>(malloc(size));
  }
  
  meshRef.withUnsafeIndexPointer([bufferPtr, size] (const ptrdiff_t *src) {
    memcpy(*bufferPtr, src, size);
  });
  
  return size;
}

CQCT_EXTERN size_t CQCT_meshCopyLabels(CQCT_Mesh mesh, unsigned int **bufferPtr) {
  assert(mesh != nullptr);
  assert(bufferPtr != nullptr);
  
  auto const &meshRef = *(mesh->impl.objPtr);
  auto const size = meshRef.vertexCount() * sizeof(unsigned int);
  
  
  if (*bufferPtr == nullptr) {
    *bufferPtr = static_cast<unsigned int *>(malloc(size));
  }
  
  meshRef.withUnsafeLabelPointer([bufferPtr, size] (const unsigned int *src) {
    memcpy(*bufferPtr, src, size);
  });
  
  return size;
}
