#include "tests_config.h"

#include <CortidQCT/CortidQCT.h>

#include <Eigen/Core>
#include <gsl/gsl>
#include <gtest/gtest.h>
#include <igl/per_vertex_normals.h>

#include <cstdio>
#include <iterator>

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

  using IdxType = typename gsl::span<double>::index_type;

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
  auto const nFloats = gsl::narrow_cast<IdxType>(3 * meshA.vertexCount());
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

TEST(Mesh, NormalsOrientedOutwards) {
  using Eigen::Index;
  using Eigen::Map;
  using Eigen::MatrixXf;
  using Eigen::Vector3f;
  using Eigen::VectorXf;
  using FMatrix =
      Eigen::Matrix<Mesh<float>::Index, Eigen::Dynamic, Eigen::Dynamic>;

  Mesh<float> mesh;
  ASSERT_NO_THROW(mesh.loadFromFile(mesh1));

  MatrixXf const V = mesh.withUnsafeVertexPointer([&](float const *vPtr) {
    return Map<MatrixXf const>{vPtr, 3, gsl::narrow<Index>(mesh.vertexCount())}
        .transpose();
  });

  FMatrix const F = mesh.withUnsafeIndexPointer([&](auto const *iPtr) {
    return Map<FMatrix const>{iPtr, 3, gsl::narrow<Index>(mesh.triangleCount())}
        .transpose();
  });

  // compute centroid
  Vector3f const centroid = V.transpose().rowwise().mean();

  MatrixXf N;
  igl::per_vertex_normals(V, F, igl::PER_VERTEX_NORMALS_WEIGHTING_TYPE_ANGLE,
                          N);

  MatrixXf const dotProductMatrix =
      N.array() * (V.rowwise() - centroid.transpose()).array();
  VectorXf const dotProducts =
      dotProductMatrix.array().rowwise().sum() /
      dotProductMatrix.array().square().rowwise().sum();

  ASSERT_TRUE((dotProducts.array() > -0.25f).all());
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wconversion"

template <class T> class MeshQueriesTest : public ::testing::Test {
protected:
  Mesh<T> mesh;

  void SetUp() override {
    mesh = Mesh<T>{4, 4};

    // Build a regular tetrahedron
    // set vertices
    mesh.withUnsafeVertexPointer([](auto *vertices) {
      vertices[0] = T{0.9428090416}; // sqrt(8/9)
      vertices[1] = T{0};
      vertices[2] = T{-0.3333333333}; // -1/3

      vertices[3] = T{-0.4714045208}; // -sqrt(2/9)
      vertices[4] = T{0.8164965809};  // sqrt(2/3)
      vertices[5] = T{-0.3333333333}; // -1/3

      vertices[6] = T{-0.4714045208}; // -sqrt(2/9)
      vertices[7] = T{-0.8164965809}; // -sqrt(2/3)
      vertices[8] = T{-0.3333333333}; // -1/3

      vertices[9] = T{0};
      vertices[10] = T{0};
      vertices[11] = T{1};
    });

    // set indices
    mesh.withUnsafeIndexPointer([](auto *indices) {
      indices[0] = 0;
      indices[1] = 3;
      indices[2] = 1;

      indices[3] = 1;
      indices[4] = 3;
      indices[5] = 2;

      indices[6] = 0;
      indices[7] = 2;
      indices[8] = 3;

      indices[9] = 0;
      indices[10] = 1;
      indices[11] = 2;
    });
  }
};

typedef ::testing::Types<float, double> MeshTypes;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
TYPED_TEST_CASE(MeshQueriesTest, MeshTypes);
#pragma clang diagnostic pop

TYPED_TEST(MeshQueriesTest, BarycentricToCartesianSingleCall) {
  using T = TypeParam;
  using BC = BarycentricPoint<T, typename Mesh<T>::Index>;

  constexpr T oneThird = 1.0 / 3.0;
  constexpr std::array<T, 3> refPoint = {T{0.1571348403}, T{0.272165527},
                                         T{0.1111111111}};

  auto const center = BC{{oneThird, oneThird}, 0};

  auto const testPoint = this->mesh.cartesianRepresentation(center);

  ASSERT_NEAR(refPoint[0], testPoint[0], 1e-6);
  ASSERT_NEAR(refPoint[1], testPoint[1], 1e-6);
  ASSERT_NEAR(refPoint[2], testPoint[2], 1e-6);
}

TYPED_TEST(MeshQueriesTest, BarycentricToCartesianSequenceCall) {
  using T = TypeParam;
  using BC = BarycentricPoint<T, typename Mesh<T>::Index>;

  constexpr T oneThird = 1.0 / 3.0;

  std::vector<BC> const query = {
      BC{{oneThird, oneThird}, 0}, BC{{oneThird, oneThird}, 1},
      BC{{oneThird, oneThird}, 2}, BC{{oneThird, oneThird}, 3}};

  std::vector<std::array<T, 3>> const refPoints = {
      {0.1571348403, 0.272165527, 0.1111111111},
      {-0.3142696805, 0, 0.1111111111},
      {0.1571348403, -0.272165527, 0.1111111111},
      {0, 0, -0.3333333333}};

  auto const testPoints = this->mesh.cartesianRepresentation(query);

  ASSERT_EQ(refPoints.size(), testPoints.size());
  for (auto i = 0u; i < testPoints.size(); ++i) {
    auto const &rp = refPoints[i];
    auto const &tp = testPoints[i];

    ASSERT_NEAR(rp[0], tp[0], 1e-6);
    ASSERT_NEAR(rp[1], tp[1], 1e-6);
    ASSERT_NEAR(rp[2], tp[2], 1e-6);
  }
}

TYPED_TEST(MeshQueriesTest, RayMeshIntersectionSingleCall) {
  using T = TypeParam;
  using R = Ray<T>;

  R const ray = {{0, 0, -1}, {0, 0, 1}};

  auto const intersection = this->mesh.rayIntersection(ray);

  ASSERT_TRUE(std::isfinite(intersection.signedDistance));
  ASSERT_NEAR(0.666666666666, intersection.signedDistance, 1e-6);
  ASSERT_EQ(3, intersection.position.triangleIndex);
  ASSERT_NEAR(0.33333333333, intersection.position.uv[0], 1e-6);
  ASSERT_NEAR(0.33333333333, intersection.position.uv[1], 1e-6);
}

TYPED_TEST(MeshQueriesTest, RayMeshIntersectionSequence) {
  using T = TypeParam;
  using R = Ray<T>;

  std::array<R, 2> const rays = {
      {{{0, 0, -1}, {0, 0, 1}}, {{4, 0, -1}, {0, 0, 1}}}};

  std::vector<RayMeshIntersection<T>> intersections;

  this->mesh.rayIntersections(rays.cbegin(), rays.cend(),
                              std::back_inserter(intersections));

  ASSERT_EQ(2, intersections.size());
  ASSERT_TRUE(std::isfinite(intersections[0].signedDistance));
  ASSERT_FALSE(std::isfinite(intersections[1].signedDistance));
  ASSERT_NEAR(0.666666666666, intersections[0].signedDistance, 1e-6);
  ASSERT_EQ(3, intersections[0].position.triangleIndex);
  ASSERT_NEAR(0.33333333333, intersections[0].position.uv[0], 1e-6);
  ASSERT_NEAR(0.33333333333, intersections[0].position.uv[1], 1e-6);
}

#pragma clang diagnostic pop

