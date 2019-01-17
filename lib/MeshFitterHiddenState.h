/**
 * @file      MeshFitterHiddenState.h
 *
 * @brief     Definitions for the HiddenState type of MeshFitter
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "DisplacementOptimizer.h"
#include "MeshFitter.h"
#include "MeshHelpers.h"
#include "WeightedARAPFitter.h"

namespace CortidQCT {

struct MeshFitter::State::HiddenState {
  VoxelVolume volume;
  Internal::DisplacementOptimizer displacementOptimizer;
  Internal::WeightedARAPFitter<float> meshFitter;
  Internal::FacetMatrix F;
  Eigen::MatrixXf volumeSamplesMatrix;

  HiddenState(VoxelVolume const &v, Internal::DisplacementOptimizer const &opt,
              Internal::WeightedARAPFitter<float> const &fitter,
              Internal::FacetMatrix const &f)
      : volume{v}, displacementOptimizer{opt}, meshFitter{fitter}, F{f} {}
};

namespace Internal {

struct PrivateStateAccessor {

  static MeshFitter::State::HiddenState const &
  hiddenState(MeshFitter::State const &state) {
    return *state.hiddenState_;
  }
};

} // namespace Internal

} // namespace CortidQCT
