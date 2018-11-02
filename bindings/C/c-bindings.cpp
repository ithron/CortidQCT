#include "CortidQCT.h"

#include <CortidQCT/CortidQCT.h>

#include <stdio.h>

extern "C" {

VoxelVolume createVoxelVolume(const char *filename) {
  try {

    auto vv = new CortidQCT::VoxelVolume{filename};
    return static_cast<VoxelVolume>(static_cast<void *>(vv));

  } catch (std::exception const &e) {
    fprintf(stderr, "Failed loading voxel volume from file '%s': %s", filename,
            e.what());
    return nullptr;
  }
}

void deleteVoxelVolume(VoxelVolume vol) {
  if (vol == nullptr) return;
  auto vv = static_cast<CortidQCT::VoxelVolume *>(static_cast<void *>(vol));

  delete vv;
}

MeshFitter createMeshFitter(const char *filename) {
  try {

    auto mf = new CortidQCT::MeshFitter{filename};
    return static_cast<MeshFitter>(static_cast<void *>(mf));

  } catch (std::exception const &e) {
    fprintf(stderr,
            "Failed creating MeshFitter form configuration file '%s': %s",
            filename, e.what());
    return nullptr;
  }
}

void deleteMeshFitter(MeshFitter fitter) {
  if (fitter == nullptr) return;
  auto mf = static_cast<CortidQCT::MeshFitter *>(static_cast<void *>(fitter));
  delete mf;
}

Mesh loadMesh(const char *filename) {
  try {
    auto mesh = new CortidQCT::Mesh<float>{};
    mesh->loadFromFile(filename);

    return static_cast<Mesh>(static_cast<void *>(mesh));

  } catch (std::exception const &e) {
    fprintf(stderr, "Failed to load mesh form file '%s': %s", filename,
            e.what());
    return nullptr;
  }
}

Mesh loadMeshAndLabels(const char *meshFilename, const char *labelFilename) {
  try {
    auto mesh = new CortidQCT::Mesh<float>{};
    mesh->loadFromFile(meshFilename, labelFilename);

    return static_cast<Mesh>(static_cast<void *>(mesh));

  } catch (std::exception const &e) {
    fprintf(stderr, "Failed to load mesh form file ('%s', '%s'): %s",
            meshFilename, labelFilename, e.what());
    return nullptr;
  }
}

void deleteMesh(Mesh mesh) {
  auto mPtr = static_cast<CortidQCT::Mesh<float> *>(static_cast<void *>(mesh));
  delete mPtr;
}

size_t meshVertexCount(Mesh mesh) {
  auto meshPtr =
      static_cast<CortidQCT::Mesh<float> *>(static_cast<void *>(mesh));
  return meshPtr->vertexCount();
}

size_t meshTriangleCount(Mesh mesh) {
  auto meshPtr =
      static_cast<CortidQCT::Mesh<float> *>(static_cast<void *>(mesh));
  return meshPtr->triangleCount();
}

size_t meshGetVertices(Mesh mesh, float **bufferPtr) {
  if (bufferPtr == nullptr || mesh == nullptr) return 0;

  auto const meshPtr =
      static_cast<CortidQCT::Mesh<float> *>(static_cast<void *>(mesh));

  auto const size = meshPtr->vertexCount() * sizeof(float) * 3;

  *bufferPtr = static_cast<float *>(malloc(size));
  meshPtr->withUnsafeVertexPointer([bufferPtr, size](float const *srcPtr) {
    memcpy(*bufferPtr, srcPtr, size);
  });

  return meshPtr->vertexCount();
}

size_t meshGetTriangles(Mesh mesh, ptrdiff_t **bufferPtr) {
  if (bufferPtr == nullptr || mesh == nullptr) return 0;

  auto const meshPtr =
      static_cast<CortidQCT::Mesh<float> *>(static_cast<void *>(mesh));

  auto const size = meshPtr->triangleCount() * sizeof(ptrdiff_t) * 3;

  *bufferPtr = static_cast<ptrdiff_t *>(malloc(size));
  meshPtr->withUnsafeIndexPointer([bufferPtr, size](ptrdiff_t const *srcPtr) {
    memcpy(*bufferPtr, srcPtr, size);
  });

  return meshPtr->triangleCount();
}

size_t meshGetLabels(Mesh mesh, unsigned int **bufferPtr) {
  if (bufferPtr == nullptr || mesh == nullptr) return 0;

  auto const meshPtr =
      static_cast<CortidQCT::Mesh<float> *>(static_cast<void *>(mesh));

  auto const size = meshPtr->vertexCount() * sizeof(unsigned int);

  *bufferPtr = static_cast<unsigned int *>(malloc(size));
  meshPtr->withUnsafeLabelPointer(
      [bufferPtr, size](unsigned int const *srcPtr) {
        memcpy(*bufferPtr, srcPtr, size);
      });

  return meshPtr->vertexCount();
}

} // extern "C"
