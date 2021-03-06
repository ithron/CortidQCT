#include "CortidQCT-Common.h"
#include "Ray.h"
#include "RayMeshIntersection.h"

#include <CortidQCT/CortidQCT.h>

using namespace CortidQCT;
using namespace CortidQCT::Internal::C;

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_Mesh CQCT_createMesh() {
  return static_cast<CQCT_Mesh>(constructObject<Mesh<float>>());
}

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_Mesh CQCT_createMeshAndAllocateMemory(
    size_t nVertices, size_t nTriangles, CQCT_Error *error) {

  try {
    return static_cast<CQCT_Mesh>(
        constructObject<Mesh<float>>(nVertices, nTriangles));
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(*error);
    }
    return nullptr;
  }
}

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_Mesh CQCT_meshFromFile(const char *filename,
                                                           CQCT_Error *error) {
  return CQCT_meshFromFileWithCustomMapping(filename, nullptr, error);
}

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_Mesh CQCT_meshFromFileWithCustomMapping(
    const char *filename, CQCT_ColorToLabelMap map, CQCT_Error *error) {
  auto mesh = CQCT_createMesh();
  CQCT_loadMeshWithCustomMapping(mesh, filename, map, error);

  return static_cast<CQCT_Mesh>(CQCT_autorelease(mesh));
}

CORTIDQCT_C_EXPORT CQCT_EXTERN CQCT_Mesh CQCT_meshAndLabelsFromFile(
    const char *meshFilename, const char *labelFilename, CQCT_Error *error) {
  auto mesh = CQCT_createMesh();
  CQCT_loadMeshAndLabels(mesh, meshFilename, labelFilename, error);

  return static_cast<CQCT_Mesh>(CQCT_autorelease(mesh));
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int
CQCT_loadMesh(CQCT_Mesh mesh, const char *filename, CQCT_Error *error) {
  return CQCT_loadMeshWithCustomMapping(mesh, filename, nullptr, error);
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int
CQCT_loadMeshWithCustomMapping(CQCT_Mesh mesh, const char *filename,
                               CQCT_ColorToLabelMap map, CQCT_Error *error) {
  assert(mesh != nullptr);
  try {

    if (map == nullptr) {
      mesh->impl.objPtr->loadFromFile(filename);
    } else {
      mesh->impl.objPtr->loadFromFile(filename, *map->impl_.objPtr);
    }
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
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(*error);
    }
  }

  return false;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int
CQCT_loadMeshAndLabels(CQCT_Mesh mesh, const char *meshFilename,
                       const char *labelsFilename, CQCT_Error *error) {
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
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(*error);
    }
  }

  return false;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int
CQCT_meshAndLabelsWriteToFile(CQCT_Mesh mesh, const char *meshFilename,
                              const char *labelsFilename, CQCT_Error *error) {
  assert(mesh != nullptr);

  try {
    mesh->impl.objPtr->writeToFile(meshFilename, labelsFilename);
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
  } catch (...) {
    if (error != nullptr) {
      *error =
          CQCT_createError(CQCT_ErrorId_Unknown, "Unknown exception caught");
      CQCT_autorelease(*error);
    }
  }

  return false;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t CQCT_meshVertexCount(CQCT_Mesh mesh) {
  assert(mesh != nullptr);

  return mesh->impl.objPtr->vertexCount();
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t CQCT_meshTriangleCount(CQCT_Mesh mesh) {
  assert(mesh != nullptr);

  return mesh->impl.objPtr->triangleCount();
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t CQCT_meshCopyVertices(CQCT_Mesh mesh,
                                                            float **bufferPtr) {
  assert(mesh != nullptr);
  assert(bufferPtr != nullptr);

  auto const &meshRef = *(mesh->impl.objPtr);
  auto const size = 3 * meshRef.vertexCount() * sizeof(float);

  if (*bufferPtr == nullptr) {
    *bufferPtr = static_cast<float *>(malloc(size));
  }

  meshRef.withUnsafeVertexPointer(
      [bufferPtr, size](const float *src) { memcpy(*bufferPtr, src, size); });

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN void CQCT_meshSetVertices(CQCT_Mesh mesh,
                                                         float const *buffer) {

  assert(mesh != nullptr);
  assert(buffer != nullptr);

  auto &obj = *(mesh->impl.objPtr);
  auto const size = obj.vertexCount() * 3;
  obj.withUnsafeVertexPointer(
      [buffer, size](float *dest) { std::copy(buffer, buffer + size, dest); });
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t CQCT_meshCopyVertexNormals(CQCT_Mesh mesh,
                                                            float **bufferPtr) {
  assert(mesh != nullptr);
  assert(bufferPtr != nullptr);

  auto const &meshRef = *(mesh->impl.objPtr);
  auto const size = 3 * meshRef.vertexCount() * sizeof(float);

  if (*bufferPtr == nullptr) {
    *bufferPtr = static_cast<float *>(malloc(size));
  }

  meshRef.withUnsafeVertexNormalPointer(
      [bufferPtr, size](const float *src) { memcpy(*bufferPtr, src, size); });

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN void CQCT_meshSetVertexNormals(CQCT_Mesh mesh,
                                                         float const *buffer) {

  assert(mesh != nullptr);
  assert(buffer != nullptr);

  auto &obj = *(mesh->impl.objPtr);
  auto const size = obj.vertexCount() * 3;
  obj.withUnsafeVertexNormalPointer(
      [buffer, size](float *dest) { std::copy(buffer, buffer + size, dest); });
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_meshCopyTriangles(CQCT_Mesh mesh, ptrdiff_t **bufferPtr) {
  assert(mesh != nullptr);
  assert(bufferPtr != nullptr);

  auto const &meshRef = *(mesh->impl.objPtr);
  auto const size = 3 * meshRef.triangleCount() * sizeof(ptrdiff_t);

  if (*bufferPtr == nullptr) {
    *bufferPtr = static_cast<ptrdiff_t *>(malloc(size));
  }

  meshRef.withUnsafeIndexPointer([bufferPtr, size](const ptrdiff_t *src) {
    memcpy(*bufferPtr, src, size);
  });

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN void
CQCT_meshSetTriangles(CQCT_Mesh mesh, ptrdiff_t const *buffer) {

  assert(mesh != nullptr);
  assert(buffer != nullptr);

  auto &obj = *(mesh->impl.objPtr);
  auto const size = obj.triangleCount() * 3;
  obj.withUnsafeIndexPointer([buffer, size](ptrdiff_t *dest) {
    std::copy(buffer, buffer + size, dest);
  });
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_meshCopyLabels(CQCT_Mesh mesh, unsigned int **bufferPtr) {
  assert(mesh != nullptr);
  assert(bufferPtr != nullptr);

  auto const &meshRef = *(mesh->impl.objPtr);
  auto const size = meshRef.vertexCount() * sizeof(unsigned int);

  if (*bufferPtr == nullptr) {
    *bufferPtr = static_cast<unsigned int *>(malloc(size));
  }

  meshRef.withUnsafeLabelPointer([bufferPtr, size](const unsigned int *src) {
    memcpy(*bufferPtr, src, size);
  });

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN void
CQCT_meshSetLabels(CQCT_Mesh mesh, unsigned int const *buffer) {

  assert(mesh != nullptr);
  assert(buffer != nullptr);

  auto &obj = *(mesh->impl.objPtr);
  auto const size = obj.vertexCount();
  obj.withUnsafeLabelPointer([buffer, size](unsigned int *dest) {
    std::copy(buffer, buffer + size, dest);
  });
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int CQCT_meshBarycentricToCartesian(
    CQCT_Mesh mesh, CQCT_BarycentricPoint const *barycentricPtr, size_t nPoints,
    float **bufferPtr, CQCT_Error *error) {

  if (nPoints == 0 || barycentricPtr == nullptr) { return 0; };

  using BP = BarycentricPoint<float, typename Mesh<float>::Index>;

  assert(barycentricPtr != nullptr);
  assert(mesh != nullptr);

  if (*bufferPtr == nullptr) {
    *bufferPtr = (float *)malloc(3 * nPoints * sizeof(float));
  }

  auto const input = reinterpret_cast<BP const *>(barycentricPtr);

  auto const &meshObj = *(mesh->impl.objPtr);

  try {
    meshObj.cartesianRepresentation(input, input + nPoints, *bufferPtr);
    return 0;
  } catch (std::out_of_range const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_OutOfRange, e.what());
      CQCT_autorelease(*error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(*error);
    }
  }

  return -1;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN int CQCT_meshBarycentricInterpolation(
    CQCT_Mesh mesh, CQCT_BarycentricPoint const *barycentricPtr, size_t nPoints,
    float const *attributePtr, size_t attributeDimensions, float **bufferPtr,
    CQCT_Error *error) {

  if (nPoints == 0 || barycentricPtr == nullptr) { return 0; };

  using BP = BarycentricPoint<float, typename Mesh<float>::Index>;

  assert(barycentricPtr != nullptr);
  assert(mesh != nullptr);

  if (*bufferPtr == nullptr) {
    *bufferPtr = (float *)malloc(attributeDimensions * nPoints * sizeof(float));
  }

  auto const input = reinterpret_cast<BP const *>(barycentricPtr);

  auto const &meshObj = *(mesh->impl.objPtr);

  try {
    meshObj.barycentricInterpolation(input, input + nPoints, attributePtr,
                                     *bufferPtr, attributeDimensions);
    return 0;
  } catch (std::out_of_range const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_OutOfRange, e.what());
      CQCT_autorelease(*error);
    }
  } catch (std::exception const &e) {
    if (error != nullptr) {
      *error = CQCT_createError(CQCT_ErrorId_Unknown, e.what());
      CQCT_autorelease(*error);
    }
  }

  return -1;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN size_t
CQCT_meshRayIntersections(CQCT_Mesh mesh, CQCT_Ray *raysPtr, size_t nRays,
                          CQCT_RayMeshIntersection **intersectionsOutPtr) {
  if (nRays == 0) return 0;

  assert(mesh != nullptr);
  assert(raysPtr != nullptr);
  assert(intersectionsOutPtr != nullptr);

  auto const size = nRays * sizeof(CQCT_RayMeshIntersection);
  if (*intersectionsOutPtr == nullptr) {
    *intersectionsOutPtr =
        static_cast<CQCT_RayMeshIntersection *>(malloc(size));
  }

  auto const raysBegin = reinterpret_cast<Ray<float> const *>(raysPtr);
  auto const raysEnd = raysBegin + nRays;
  auto const intersectionsOut =
      reinterpret_cast<RayMeshIntersection<float> *>(*intersectionsOutPtr);

  mesh->impl.objPtr->rayIntersections(raysBegin, raysEnd, intersectionsOut);

  return size;
}

CORTIDQCT_C_EXPORT CQCT_EXTERN void CQCT_meshUpsample(CQCT_Mesh mesh,
                                                      size_t nTimes) {
  if (nTimes == 0) return;

  assert(mesh != nullptr);

  mesh->impl.objPtr->upsample(nTimes);
}

CORTIDQCT_C_EXPORT CQCT_EXTERN void
CQCT_meshUpdatePerVertexNormals(CQCT_Mesh mesh) {

  assert(mesh != nullptr);

  mesh->impl.objPtr->updatePerVertexNormals();
}

