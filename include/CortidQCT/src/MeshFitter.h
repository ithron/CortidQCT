#pragma once

#include "MeasurementModel.h"
#include "Mesh.h"

#include "propagate_const.h"

#include <memory>

namespace CortidQCT {

class MeshFitter {
 public:
  struct Configuration {
    MeasurementModel model;
    Mesh referenceMesh;
    double sigmaE = 2.0;
    double sigmaS = 2.0;
  };

  MeshFitter(Configuration const &config);

 private:
  struct Impl;

  std::propagate_const<std::unique_ptr<Impl>> pImpl_;
};

}  // namespace CortidQCT

