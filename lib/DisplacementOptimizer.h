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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
/**
 * This class encapuslates the algorithm that estimates the optimal
 * displacments.
 *
 * Find the maximizer of the posterior log likelihood of the model displacement
 * given measurements from the inptu volume.
 */
class DisplacementOptimizer {
public:
  using DisplacementsWeightsPair = std::pair<Eigen::VectorXf, Eigen::VectorXf>;

  explicit DisplacementOptimizer(MeshFitter::Configuration const &config);

  /**
   * @brief Compute the optimal displacement and the corresponding weights
   */
  template <class DerivedN, class DerivedL, class DerivedM>
  DisplacementsWeightsPair
  operator()(Eigen::MatrixBase<DerivedN> const &N,
             Eigen::MatrixBase<DerivedL> const &labels,
             Eigen::MatrixBase<DerivedM> const &measurements,
             std::size_t nonDecrease, float &effectiveSigmaS);

  /**
   * @brief Computes the per-veretx log likelihood of the current model.
   */
  template <class DerivedN, class DerivedL, class DerivedM>
  Eigen::Matrix<typename DerivedM::Scalar, Eigen::Dynamic, 1>
  logLikelihoodVector(Eigen::MatrixBase<DerivedN> const &N,
                      Eigen::MatrixBase<DerivedL> const &labels,
                      Eigen::MatrixBase<DerivedM> const &measurements);

  /**
   * @brief Compute the log likelihood of the current model.
   */
  template <class DerivedN, class DerivedL, class DerivedM>
  float logLikelihood(Eigen::MatrixBase<DerivedN> const &N,
                      Eigen::MatrixBase<DerivedL> const &labels,
                      Eigen::MatrixBase<DerivedM> const &measurements);

  /**
   * @brief Returns the model sampling positions used by the optimizer
   */
  inline auto const &modelSamplingPositions() const noexcept {
    return modelSamplingPositions_;
  }

private:
  using ModelSamplingPositionMatrix = Eigen::Matrix<float, Eigen::Dynamic, 4>;

  MeshFitter::Configuration const &config_;
  MeasurementModel const &model_;
  ModelSampler modelSampler_;
  ModelSamplingPositionMatrix modelSamplingPositions_;
  float currentSigma_;
};
#pragma clang diagnostic pop

extern template DisplacementOptimizer::DisplacementsWeightsPair
DisplacementOptimizer::
operator()<NormalMatrix<float>, LabelVector, Eigen::VectorXf>(
    Eigen::MatrixBase<NormalMatrix<float>> const &,
    Eigen::MatrixBase<LabelVector> const &,
    Eigen::MatrixBase<Eigen::VectorXf> const &, std::size_t, float &);

extern template DisplacementOptimizer::DisplacementsWeightsPair
DisplacementOptimizer::
operator()<NormalMatrix<double>, LabelVector, Eigen::VectorXd>(
    Eigen::MatrixBase<NormalMatrix<double>> const &,
    Eigen::MatrixBase<LabelVector> const &,
    Eigen::MatrixBase<Eigen::VectorXd> const &, std::size_t, float &);

extern template DisplacementOptimizer::DisplacementsWeightsPair
DisplacementOptimizer::operator()<
    Eigen::Transpose<const Eigen::Map<Eigen::Matrix<float, 3, Eigen::Dynamic>>>,
    Eigen::Map<LabelVector>, Eigen::Map<Eigen::VectorXf>>(
    Eigen::MatrixBase<Eigen::Transpose<
        const Eigen::Map<Eigen::Matrix<float, 3, Eigen::Dynamic>>>> const &,
    Eigen::MatrixBase<Eigen::Map<LabelVector>> const &,
    Eigen::MatrixBase<Eigen::Map<Eigen::VectorXf>> const &, std::size_t,
    float &);

extern template DisplacementOptimizer::DisplacementsWeightsPair
DisplacementOptimizer::operator()<
    Eigen::Transpose<Eigen::Map<Eigen::Matrix<float, 3, Eigen::Dynamic>>>,
    Eigen::Map<LabelVector>, Eigen::Map<Eigen::VectorXf>>(
    Eigen::MatrixBase<Eigen::Transpose<
        Eigen::Map<Eigen::Matrix<float, 3, Eigen::Dynamic>>>> const &,
    Eigen::MatrixBase<Eigen::Map<LabelVector>> const &,
    Eigen::MatrixBase<Eigen::Map<Eigen::VectorXf>> const &, std::size_t,
    float &);

extern template float DisplacementOptimizer::logLikelihood<
    Eigen::Transpose<const Eigen::Map<Eigen::Matrix<float, 3, Eigen::Dynamic>>>,
    Eigen::Map<LabelVector>, Eigen::Map<Eigen::VectorXf>>(
    Eigen::MatrixBase<Eigen::Transpose<
        const Eigen::Map<Eigen::Matrix<float, 3, Eigen::Dynamic>>>> const &,
    Eigen::MatrixBase<Eigen::Map<LabelVector>> const &,
    Eigen::MatrixBase<Eigen::Map<Eigen::VectorXf>> const &);

extern template Eigen::Matrix<float, Eigen::Dynamic, 1>
DisplacementOptimizer::logLikelihoodVector<
    Eigen::Transpose<const Eigen::Map<Eigen::Matrix<float, 3, Eigen::Dynamic>>>,
    Eigen::Map<LabelVector>, Eigen::Map<Eigen::VectorXf>>(
    Eigen::MatrixBase<Eigen::Transpose<
        const Eigen::Map<Eigen::Matrix<float, 3, Eigen::Dynamic>>>> const &,
    Eigen::MatrixBase<Eigen::Map<LabelVector>> const &,
    Eigen::MatrixBase<Eigen::Map<Eigen::VectorXf>> const &);

} // namespace Internal

} // namespace CortidQCT
