/**
 * @file      DisplacementOptimizer.cpp
 *
 * @brief     Implementation file for class DisplacementOptimizer
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 * You may use, distribute and modify this code under the terms of the
 * AFL 3.0 license; see LICENSE for full license details.
 */

#include "DisplacementOptimizer.h"
#include "CommonMath.h"
#include "DiscreteRangeDecorators.h"

namespace CortidQCT {
namespace Internal {

namespace {

/// @brief Convert per vertex normals to angles with z-axis in degrees
template <class DerivedN>
Eigen::VectorXf normalsToAngles(Eigen::MatrixBase<DerivedN> const &N) {
  Expects(N.rows() > 0);
  Expects(N.cols() == 3);
  Eigen::VectorXf const angles =
      N.col(2).array().abs().acos().template cast<float>() * 180.f /
      static_cast<float>(M_PI);

  return angles;
}

/**
 * @brief Updates the given model sampling positions matrix with new values
 *
 * Updates sampling position based on offset, angles based on normals and
 * densities.
 *
 * @param model MeasurementModel instance
 * @param N per-vertex normal matrix
 * @param densities densities sampled from input volume along lines through
 * vertices along normals from `N`
 * @param positionsOut position matrix to update
 */
template <class DerivedN, class DerivedL, class DerivedD, class DerivedOut>
void updateModelSamplingPositions(MeasurementModel const &model,
                                  Eigen::MatrixBase<DerivedN> const &N,
                                  Eigen::MatrixBase<DerivedL> const &labels,
                                  Eigen::MatrixBase<DerivedD> const &densities,
                                  Eigen::MatrixBase<DerivedOut> &positionsOut) {
  using Eigen::VectorXf;

  Expects(N.cols() == 3);
  Expects(N.rows() > 0);
  Expects(densities.rows() % N.rows() == 0);

  if (positionsOut.rows() != densities.rows()) {
    positionsOut.derived().resize(densities.rows(), 4);
  }

  Ensures(positionsOut.rows() == densities.rows());
  Ensures(positionsOut.cols() >= 3);

  auto const numVertices = N.rows();
  auto const numSamples = densities.rows() / N.rows();

  VectorXf const t = discreteRangeElementVector(model.samplingRange);

  auto const angles = normalsToAngles(N);
  Ensures(angles.rows() == N.rows());

  positionsOut.col(1) = densities.template cast<float>();
  positionsOut.col(2) = angles.template cast<float>().replicate(numSamples, 1);
  positionsOut.col(3) = labels.template cast<float>().replicate(numSamples, 1);
  for (auto i = 0; i < numSamples; ++i) {
    positionsOut.col(0).segment(numVertices * i, numVertices).array() = t(i);
  }
}

} // anonymous namespace

DisplacementOptimizer::DisplacementOptimizer(
    MeshFitter::Configuration const &config)
    : config_{config}, model_{config.model}, modelSampler_{config.model},
      currentSigma_{static_cast<float>(config.sigmaS * config.sigmaS)} {}

template <class DerivedN, class DerivedL, class DerivedM>
DisplacementOptimizer::DisplacementsWeightsPair DisplacementOptimizer::
operator()(Eigen::MatrixBase<DerivedN> const &N,
           Eigen::MatrixBase<DerivedL> const &labels,
           Eigen::MatrixBase<DerivedM> const &measurements,
           std::size_t nonDecrease) {
  using Eigen::Index;
  using Eigen::Map;
  using Eigen::MatrixXd;
  using Eigen::MatrixXf;
  using Eigen::VectorXd;
  using Eigen::VectorXf;

  DiscreteRange<float> const displacementRange{model_.samplingRange.min * 2,
                                               model_.samplingRange.max * 2,
                                               model_.samplingRange.stride};
  auto const displacements =
      discreteRangeElementVector(displacementRange).cast<float>();
  auto const numSamples =
      gsl::narrow<Index>(model_.samplingRange.numElements());

  updateModelSamplingPositions(model_, N, labels, measurements,
                               modelSamplingPositions_);

  // Pre-allocate matrix for observation likelihood given displacement
  MatrixXf Lzs(N.rows(), displacements.rows());

  MatrixXf tmp(N.rows() * numSamples, displacements.rows());

  VectorXf modelSamples(N.rows() * numSamples);
#pragma omp parallel for firstprivate(modelSamples)
  for (Index i = 0; i < displacements.size(); ++i) {

    modelSampler_(modelSamplingPositions_, displacements(i), modelSamples);

    // interpret modelSamples as 2K+1 x N matrix, then the observation
    // log likelihood is the colwise sum of that matrix
    Lzs.col(i) = Map<MatrixXf const>{modelSamples.data(), N.rows(), numSamples}
                     .rowwise()
                     .sum();
  }

  // To compute the posterior, the displacement prior log likelihood and the
  // total log lokelihood of observations must be computed, first.

  if (nonDecrease >= config_.minNonDecreasing) {
    currentSigma_ *= config_.decay;
  }
  auto const sigmaSq = currentSigma_;

  // Log likelihood vector of the gaussian displacement prior
  VectorXf const displacementLL =
      -0.5f * displacements.array().square() / sigmaSq;

  // Copmute the nomnator term: conditional LL + prior LL
  MatrixXf posteriorNominator = Lzs;
  posteriorNominator.rowwise() += displacementLL.transpose();
  // Scale `posteriorNominator` so that the maxCoeff is 0 to avoid overflows
  VectorXf const posteriorMaxCoeffs = posteriorNominator.rowwise().maxCoeff();
  posteriorNominator.colwise() -= posteriorMaxCoeffs;

  // The denominator of the posterior log likelihood contains an intergral
  // over all displacements, which is approximated with a sum here.
  VectorXf posteriorDenominator =
      (posteriorNominator.array().exp().rowwise().sum() *
       displacementRange.stride)
          .log()
          .matrix();
  // // Add `posteriorMaxCoeffs` to denominator term to invert the scaleing
  // posteriorDenominator += posteriorMaxCoeffs;
  posteriorDenominator.array() +=
      2 * log(static_cast<float>(displacementLL.size()));

  // Compose the posterior log likelihood term
  // Note that the scaling factor (from the model normalization) is canceled
  // out here.
  MatrixXf posteriorLL = posteriorNominator;
  posteriorLL.colwise() -= posteriorDenominator;

  // Find the displacements that maximize the posterior log likelihood
  VectorXf bestDisplacements(N.rows());
#pragma omp parallel for
  for (Index i = 0; i < N.rows(); ++i) {
    Index idx;
    posteriorLL.row(i).maxCoeff(&idx);
    bestDisplacements(i) =
        displacementRange.nThElement(gsl::narrow_cast<std::size_t>(idx) + 1);
  }

  // Compute weight vector gamma
  VectorXf gamma = posteriorLL.rowwise().maxCoeff().array().exp().matrix();
  // Set all non-finite weights to 0
  gamma.array() =
      gamma.array().isFinite().select(gamma, VectorXd::Zero(gamma.rows()));
  gamma.array() /= labels.array().unaryExpr([this](auto const l) {
    auto const scale = static_cast<float>(this->model_.densityScale(l));
    return std::isfinite(scale) ? scale : 1.0f;
  });

  return {-bestDisplacements, gamma};
}

template DisplacementOptimizer::DisplacementsWeightsPair DisplacementOptimizer::
operator()<NormalMatrix<float>, LabelVector, Eigen::VectorXf>(
    Eigen::MatrixBase<NormalMatrix<float>> const &,
    Eigen::MatrixBase<LabelVector> const &,
    Eigen::MatrixBase<Eigen::VectorXf> const &, std::size_t);

template DisplacementOptimizer::DisplacementsWeightsPair DisplacementOptimizer::
operator()<NormalMatrix<double>, LabelVector, Eigen::VectorXd>(
    Eigen::MatrixBase<NormalMatrix<double>> const &,
    Eigen::MatrixBase<LabelVector> const &,
    Eigen::MatrixBase<Eigen::VectorXd> const &, std::size_t);

} // namespace Internal

} // namespace CortidQCT
