#include <CortidQCT/CortidQCT.h>

#include <gtest/gtest.h>

TEST(FitMesh, Basic) {
  auto const img = loadImageFromSomewhere();
  auto const model = loadModel();
  auto const refMesh = loadReferenceMesh();
  auto const config = loadConfig();

  auto fitter = MeshFitter{model, refMesh, config};

  auto const result = fitter.fit(img);

}
