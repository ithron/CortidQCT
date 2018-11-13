/**
 * @file      MeshFitterImpl.h
 *
 * @brief     This private header contains the definition of MeshFitter::Impl
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "MeshFitter.h"

namespace CortidQCT {

class MeshFitter::Impl {

  friend class MeshFitter;

public:
  inline explicit Impl(MeshFitter &meshFitter) : fitter_(meshFitter) {}

  ~Impl() noexcept = default;

  Impl(Impl const &) = delete;
  inline Impl(Impl &&) noexcept = delete;

  Impl &operator=(Impl const &) = delete;
  Impl &operator=(Impl &&) = delete;

  inline Impl(Impl const & /*unused*/, MeshFitter &rhsFitter)
      : fitter_(rhsFitter) {}
  inline Impl(Impl && /*unused*/, MeshFitter &rhsFitter) noexcept
      : fitter_(rhsFitter) {}

protected:
  MeshFitter::Result fit(VoxelVolume const &volume);
  MeshFitter::State init(VoxelVolume const &volume);
  void fitOneIteration(MeshFitter::State &state);

  void findOptimalDisplacements(MeshFitter::State &state);
  void findOptimalDeformation(MeshFitter::State &state);
  void sampleVolume(MeshFitter::State &state);
  void computeLogLikelihood(MeshFitter::State &state);
  void checkConvergence(MeshFitter::State &state);

  MeshFitter &fitter_;

  std::optional<Result> result_;
};

} // namespace CortidQCT
