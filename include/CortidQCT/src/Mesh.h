#pragma once

#include <Eigen/Core>

namespace CortidQCT {

  struct Mesh {
    Eigen::MatrixXd vertices;
    Eigen::MatrixXi indices;
  };

} // namespace CortidQCT

