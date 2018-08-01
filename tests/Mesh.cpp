#include <CortidQCT/CortidQCT.h>

#include <gtest/gtest.h>

using namespace CortidQCT;

TEST(Mesh, EmptyMeshIsMepty) {
  Mesh<double> const mesh;

  ASSERT_TRUE(mesh.isEmpty());
  ASSERT_EQ(0, mesh.vertexCount());
  ASSERT_EQ(0, mesh.triangleCount());
}

