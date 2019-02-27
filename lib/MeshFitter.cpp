/**
 * @file      MeshFitter.cpp
 *
 * @brief     Implementation file for MeshFitter
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "MeshFitter.h"
#include "MeshFitterImpl.h"

namespace CortidQCT {

MeshFitter::~MeshFitter() = default;

MeshFitter::MeshFitter(MeshFitter const &other)
    : configuration(other.configuration),
      pImpl_(std::make_unique<Impl>(*other.pImpl_, *this)) {}

MeshFitter::MeshFitter(MeshFitter &&other) noexcept
    : configuration(std::move(other.configuration)),
      pImpl_(std::move(other.pImpl_)) {
  // Update back reference
  pImpl_->fitter_ = *this;
}

MeshFitter &MeshFitter::operator=(MeshFitter const &rhs) {
  configuration = rhs.configuration;
  pImpl_ = std::make_unique<Impl>(*rhs.pImpl_, *this);

  return *this;
}

MeshFitter &MeshFitter::operator=(MeshFitter &&rhs) noexcept {
  configuration = std::move(rhs.configuration);
  pImpl_ = std::move(rhs.pImpl_);
  pImpl_->fitter_ = *this;

  return *this;
}

MeshFitter::MeshFitter(Configuration config)
    : configuration(std::move(config)), pImpl_(std::make_unique<Impl>(*this)) {}

void MeshFitter::swap(MeshFitter &rhs) noexcept {
  using std::swap;

  swap(configuration, rhs.configuration);
  pImpl_.swap(rhs.pImpl_);

  rhs.pImpl_->fitter_ = rhs;
  pImpl_->fitter_ = *this;
}

MeshFitter::Result MeshFitter::fit(VoxelVolume const &volume) const {
  return pImpl_->fit(volume);
}

MeshFitter::State MeshFitter::init(VoxelVolume const &volume) const {
  return pImpl_->init(volume);
}

void MeshFitter::fitOneIteration(MeshFitter::State &state) const {
  return pImpl_->fitOneIteration(state);
}

void MeshFitter::volumeSamplingStep(MeshFitter::State &state) const {
  return pImpl_->sampleVolume(state);
}

void MeshFitter::optimalDisplacementStep(MeshFitter::State &state) const {
  return pImpl_->findOptimalDisplacements(state);
}

void MeshFitter::optimalDeformationStep(MeshFitter::State &state) const {
  return pImpl_->findOptimalDeformation(state);
}

void MeshFitter::logLikelihoodStep(MeshFitter::State &state) const {
  return pImpl_->computeLogLikelihood(state);
}

void MeshFitter::convergenceTestStep(MeshFitter::State &state) const {
  return pImpl_->checkConvergence(state);
}

} // namespace CortidQCT
