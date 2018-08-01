#include "tests_config.h"

#include <CortidQCT/CortidQCT.h>

#include <gtest/gtest.h>

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
