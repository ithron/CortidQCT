/**
 * @file      DisplacementOptimizer.h
 *
 * @brief     This file contains the definition of the DisplacementOptimizer
 * class
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 * You may use, distribute and modify this code under the terms of the
 * AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "MeasurementModel.h"
#include "MeshFitter.h"
#include "MeshHelpers.h"
#include "Sampler.h"

#include <Eigen/Core>

namespace CortidQCT {

namespace Internal {

/**
 * This class encapuslates the algorithm that estimates the optimal
 * displacments.
 *
 * Find the maximizer of the posterior log likelihood of the model displacement
 * given measurements from the inptu volume.
 */
class DisplacementOptimizer {
public:
  using DisplacementsWeightsPair = std::pair<Eigen::VectorXf, Eigen::VectorXd>;

  explicit DisplacementOptimizer(MeshFitter::Configuration const &config);

  /**
   * Compute the optimal displacement and the corresponding weights
   */
  template <class DerivedN, class DerivedL, class DerivedM>
  DisplacementsWeightsPair
  operator()(Eigen::MatrixBase<DerivedN> const &N,
             Eigen::MatrixBase<DerivedL> const &labels,
             Eigen::MatrixBase<DerivedM> const &measurements,
             std::size_t nonDecrease);

private:
  using ModelSamplingPositionMatrix = Eigen::Matrix<float, Eigen::Dynamic, 4>;

  MeshFitter::Configuration const &config_;
  MeasurementModel const &model_;
  ModelSampler modelSampler_;
  ModelSamplingPositionMatrix modelSamplingPositions_;
};

extern template DisplacementOptimizer::DisplacementsWeightsPair
DisplacementOptimizer::operator()<NormalMatrix<>, LabelVector, Eigen::VectorXf>(
    Eigen::MatrixBase<NormalMatrix<>> const &,
    Eigen::MatrixBase<LabelVector> const &,
    Eigen::MatrixBase<Eigen::VectorXf> const &, std::size_t);

} // namespace Internal

} // namespace CortidQCT
