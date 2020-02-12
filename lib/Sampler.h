/**
 * @file      Sampler.h
 *
 * @brief     This private head contains the definition of the Sampler type
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "MeasurementModel.h"
#include "VoxelVolume.h"

#include <Eigen/Core>
#include <gsl/gsl>

namespace CortidQCT {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
class VolumeSampler {

public:
  VoxelVolume::ValueType outside;

  inline explicit VolumeSampler(
      VoxelVolume const &vol,
      VoxelVolume::ValueType outside_ = VoxelVolume::ValueType{0}) noexcept
      : outside{outside_}, volume_{vol} {}

  template <class Derived, class DerivedOut>
  inline void operator()(Eigen::MatrixBase<Derived> const &positions,
                         Eigen::MatrixBase<DerivedOut> &values) const {
    using Eigen::Vector3f;

    Expects(values.rows() == positions.rows());
    Expects(positions.cols() == 3);

    Vector3f const scale{1.f / volume_.voxelSize().width,
                         1.f / volume_.voxelSize().height,
                         1.f / volume_.voxelSize().depth};

    volume_.withUnsafeDataPointer(
        [this, &positions, &values, scale](auto const *ptr) {

#pragma omp parallel for
          for (Eigen::Index i = 0; i < positions.rows(); ++i) {

            values(i) = this->interpolate(
                (positions.row(i).array() * scale.array().transpose())
                    .matrix()
                    .transpose(),
                gsl::make_not_null(ptr));
          }
        });
  }

  template <class Derived>
  inline Eigen::Matrix<VoxelVolume::ValueType, Eigen::Dynamic, 1>
  operator()(Eigen::MatrixBase<Derived> const &positions) const {
    Eigen::Matrix<VoxelVolume::ValueType, Eigen::Dynamic, 1> values(
        positions.rows());

    operator()(positions, values);

    return values;
  }

private:
  template <class Derived>
  inline VoxelVolume::ValueType
  at(Eigen::MatrixBase<Derived> const &pos,
     gsl::not_null<VoxelVolume::ValueType const *> ptr) const {
    auto const &size = volume_.size();

    auto const isInside = (pos.array() >= 0).all() &&
                          pos(0) < gsl::narrow_cast<int>(size.width) &&
                          pos(1) < gsl::narrow_cast<int>(size.height) &&
                          pos(2) < gsl::narrow_cast<int>(size.depth);

    auto const index =
        gsl::narrow_cast<std::size_t>(pos(2)) * size.width * size.height +
        gsl::narrow_cast<std::size_t>(pos(1)) * size.width +
        gsl::narrow_cast<std::size_t>(pos(0));

    return isInside ? ptr.get()[index] : outside;
  }

  template <class Derived>
  inline auto
  interpolate(Eigen::MatrixBase<Derived> const &pos,
              gsl::not_null<VoxelVolume::ValueType const *> ptr) const {

    using Value = VoxelVolume::ValueType;
    using Eigen::Vector3f;
    using Eigen::Vector3i;
    using gsl::make_not_null;

    Vector3i const x0 = pos.array().floor().matrix().template cast<int>();
    Vector3i const x1 = pos.array().ceil().matrix().template cast<int>();
    Vector3f const xd = pos - x0.cast<float>();
    Vector3f const xn = Vector3f::Ones() - xd;

    // get values of lattice points
    Value const c000 = at(x0, ptr);
    Value const c001 = at(Vector3i{x0(0), x0(1), x1(2)}, ptr);
    Value const c010 = at(Vector3i{x0(0), x1(1), x0(2)}, ptr);
    Value const c011 = at(Vector3i{x0(0), x1(1), x1(2)}, ptr);
    Value const c100 = at(Vector3i{x1(0), x0(1), x0(2)}, ptr);
    Value const c101 = at(Vector3i{x1(0), x0(1), x1(2)}, ptr);
    Value const c110 = at(Vector3i{x1(0), x1(1), x0(2)}, ptr);
    Value const c111 = at(x1, ptr);

    auto const c00 = c000 * xn(0) + c100 * xd(0);
    auto const c01 = c001 * xn(0) + c101 * xd(0);
    auto const c10 = c010 * xn(0) + c110 * xd(0);
    auto const c11 = c011 * xn(0) + c111 * xd(0);

    auto const c0 = c00 * xn(1) + c10 * xd(1);
    auto const c1 = c01 * xn(1) + c11 * xd(1);

    auto const c = c0 * xn(2) + c1 * xd(2);

    return c;
  }

  VoxelVolume const &volume_;
};

class ModelSampler {

public:
  inline explicit ModelSampler(MeasurementModel const &model) noexcept
      : model_(model) {}

  template <class DerivedIn, class VectorOut>
  inline void operator()(Eigen::MatrixBase<DerivedIn> const &positions,
                         float offset, VectorOut &&values) const {
    using Eigen::Dynamic;
    using Eigen::Matrix;
    using Eigen::Vector3f;

    Expects(values.rows() == positions.rows());
    Expects(positions.cols() == 4);
    Expects(values.cols() == 1);

    Vector3f const min{model_.samplingRange.min, model_.densityRange.min,
                       model_.angleRange.min};
    Vector3f const scale{1.f / model_.samplingRange.stride,
                         1.f / model_.densityRange.stride,
                         1.f / model_.angleRange.stride};

    for (auto &&label : model_.labels()) {
      model_.withUnsafeDataPointer(label, [this, &positions, &values, min,
                                           scale, offset,
                                           label](float const *ptr) {
        for (Eigen::Index i = 0; i < positions.rows(); ++i) {
          if (static_cast<MeasurementModel::Label>(positions(i, 3)) != label) {
            continue;
          }

          Vector3f const position{positions(i, 0) + offset, positions(i, 1),
                                  positions(i, 2)};
          values(i) = this->interpolate(
              ((position - min).array() * scale.array()).matrix(), ptr);
        }
      });
    }
  }

  template <class Derived>
  inline Eigen::Matrix<float, Eigen::Dynamic, 1>
  operator()(Eigen::MatrixBase<Derived> const &positions, float offset) const {
    Eigen::Matrix<float, Eigen::Dynamic, 1> values(positions.rows());

    operator()(positions, offset, values);

    return values;
  }

private:
  inline float at(int x, int y, int z, Eigen::Vector3i const &sizeI,
                  float const *ptr) const {

    auto const index = x * sizeI.y() * sizeI.z() + z * sizeI.y() + y;
    return ptr[index];
  }

  template <class Derived>
  inline float interpolate(Eigen::MatrixBase<Derived> const &pos,
                           float const *ptr) const {
    using Eigen::Vector3f;
    using Eigen::Vector3i;
    using std::clamp;
    using std::log;

    Vector3i const sizeI{static_cast<int>(model_.samplingRange.numElements()),
                         static_cast<int>(model_.densityRange.numElements()),
                         static_cast<int>(model_.angleRange.numElements())};

    auto const x00 = clamp(static_cast<int>(pos(0)), 0, sizeI.x() - 1);
    auto const x01 = clamp(static_cast<int>(pos(2) + 0.5f), 0, sizeI.z() - 1);
    auto const x0 = static_cast<int>(pos(1));
    auto const x1 = x0 + 1;
    auto const xd = pos(1) - static_cast<float>(x0);
    auto const xn = 1.f - xd;

    auto const c0 = at(x00, clamp(x0, 0, sizeI.y() - 1), x01, sizeI, ptr);
    auto const c1 = at(x00, clamp(x1, 0, sizeI.y() - 1), x01, sizeI, ptr);

    auto const c = c0 * xn + c1 * xd;

    return log(c);
  }

  MeasurementModel const &model_;
};
#pragma clang diagnostic pop

} // namespace CortidQCT
