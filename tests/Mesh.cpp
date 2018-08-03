#include "tests_config.h"

#include <CortidQCT/CortidQCT.h>

#include <gsl/gsl>
#include <gtest/gtest.h>

#include <cstdio>

using namespace CortidQCT;

#ifndef CortidQCT_DATADIR
#  error "No data dir given"
#endif

static std::string const mesh1 =
    std::string(CortidQCT_DATADIR) + "/SimpleVertebra.off";
static std::string const labels1 =
    std::string(CortidQCT_DATADIR) + "/SimpleVertebra-labels.txt";

TEST(Mesh, EmptyMeshIsMepty) {
  Mesh<double> const mesh;

  ASSERT_TRUE(mesh.isEmpty());
  ASSERT_EQ(0, mesh.vertexCount());
  ASSERT_EQ(0, mesh.triangleCount());
}

TEST(Mesh, LoadFromFileThrowsIfFileDesNotExist) {

  Mesh<double> mesh;

  // Generate temp file name
  auto const file = std::string{"some-non-existent-file.off"};

  ASSERT_THROW(mesh.loadFromFile(file, file), std::invalid_argument);
  ASSERT_THROW(mesh.loadFromFile(file), std::invalid_argument);
}

TEST(Mesh, WriteToFileDoesNotThrowOnEmptyMesh) {
  Mesh<double> const mesh;

  ASSERT_NO_THROW(mesh.writeToFile("does-not-matter.obj", "dummy.dat"));
  ASSERT_NO_THROW(mesh.writeToFile("does-not-matter.obj"));
}

TEST(Mesh, MeashLoadFromFileDoesNotThrowOnSuccess) {

  Mesh<double> mesh;

  ASSERT_NO_THROW(mesh.loadFromFile(mesh1, labels1));
  ASSERT_NO_THROW(mesh.loadFromFile(mesh1));
}

TEST(Mesh, MeshLoadSaveLoadPreservesMesh) {

  using namespace std::string_literals;
  using gsl::make_span;

  Mesh<double> meshA, meshB;

  // load meshes
  ASSERT_NO_THROW(meshA.loadFromFile(mesh1, labels1));
  ASSERT_NO_THROW(meshB.loadFromFile(mesh1));

  std::string const filenameAMesh = std::tmpnam(nullptr) + ".off"s;
  std::string const filenameALabel = std::tmpnam(nullptr) + ".txt"s;
  std::string const filenameBMesh = std::tmpnam(nullptr) + ".off"s;

  // save meshes
  ASSERT_NO_THROW(meshA.writeToFile(filenameAMesh, filenameALabel));
  ASSERT_NO_THROW(meshB.writeToFile(filenameBMesh));

  Mesh<double> meshC, meshD;

  // load meshes
  ASSERT_NO_THROW(meshC.loadFromFile(filenameAMesh, filenameALabel));
  ASSERT_NO_THROW(meshD.loadFromFile(filenameBMesh));

  // assert equality of counts
  ASSERT_EQ(meshA.vertexCount(), meshC.vertexCount());
  ASSERT_EQ(meshA.triangleCount(), meshC.triangleCount());
  ASSERT_EQ(meshB.vertexCount(), meshD.vertexCount());
  ASSERT_EQ(meshB.triangleCount(), meshD.triangleCount());
  ASSERT_EQ(meshA.vertexCount(), meshD.vertexCount());

  // assert equality of vertices
  auto const nFloats = 3 * meshA.vertexCount();
  meshC.withUnsafeVertexPointer([&meshA, nFloats](auto const pV) {
    meshA.withUnsafeVertexPointer([pV, nFloats](auto const pU) {
      auto pvi = pV;
      auto const spanA = make_span(pU, nFloats);

      for (auto &&ui : spanA) {
        ASSERT_DOUBLE_EQ(ui, *pvi);
        ++pvi;
      }
    });
  });

  meshD.withUnsafeVertexPointer([&meshB, nFloats](auto const pV) {
    meshB.withUnsafeVertexPointer([pV, nFloats](auto const pU) {
      auto pvi = pV;
      auto const spanA = make_span(pU, nFloats);

      for (auto &&ui : spanA) {
        ASSERT_DOUBLE_EQ(ui, *pvi);
        ++pvi;
      }
    });
  });

  // assert equality of triangles
  ASSERT_TRUE(meshC.withUnsafeIndexPointer([&meshA](auto const pI) {
    return meshA.withUnsafeIndexPointer(
        [pI, size = meshA.triangleCount() * 3](auto const pJ) {
          return std::equal(pI, pI + size, pJ);
        });
  }));
  ASSERT_TRUE(meshD.withUnsafeIndexPointer([&meshB](auto const pI) {
    return meshB.withUnsafeIndexPointer(
        [pI, size = meshB.triangleCount() * 3](auto const pJ) {
          return std::equal(pI, pI + size, pJ);
        });
  }));

  // assert equality of labels
  ASSERT_TRUE(meshC.withUnsafeLabelPointer([&meshA](auto const pI) {
    return meshA.withUnsafeLabelPointer(
        [pI, size = meshA.vertexCount()](auto const pJ) {
          return std::equal(pI, pI + size, pJ);
        });
  }));
  ASSERT_TRUE(meshD.withUnsafeLabelPointer([&meshB](auto const pI) {
    return meshB.withUnsafeLabelPointer(
        [pI, size = meshB.vertexCount()](auto const pJ) {
          return std::equal(pI, pI + size, pJ);
        });
  }));
}
