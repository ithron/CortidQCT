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

template <class... Args> struct DisplayType;

template <class T, class DerivedSampler> class SamplerBase {

public:
  template <class Derived>
  inline auto operator()(Eigen::MatrixBase<Derived> const &positions) const {
    return interpolate(
        static_cast<DerivedSampler const &>(*this).coordTransform(positions));
  }

protected:
  template <class Val> inline auto valueTransform(Val &&x) const { return x; }

  template <class Val> inline auto inverseValueTransform(Val &&x) const {
    return x;
  }

  // Must be implemented by the derived class
  // template <class Derived>
  // inline auto outsideValue(Eigen::MatrixBase<Derived> const &pos) const;

  // Must be implemented by the derived class
  // template <class Derived>
  // inline bool isOutside(Eigen::MatrixBase<Derived> const &pos) const ;

  template <class Derived>
  inline decltype(auto) coordTransform(Eigen::MatrixBase<Derived> const &x) const {
    return x;
  }

  template <class Derived, class ValType>
  inline auto at(Eigen::MatrixBase<Derived> const &pos,
                 ValType const *ptr) const {
    Eigen::Matrix<int, Derived::RowsAtCompileTime, 1> linearIndices(pos.rows());

    Eigen::Matrix<ValType, Derived::RowsAtCompileTime, 1> values(pos.rows());

#pragma omp parallel for
    for (auto i = 0; i < pos.rows(); ++i) {
      ValType val;
      if (!static_cast<DerivedSampler const &>(*this).isOutside(pos.row(i))) {

        auto const linIndex =
            static_cast<DerivedSampler const &>(*this).toLinear(
                pos.template cast<int>().row(i));
        val = ptr[linIndex];
      } else {
        val =
            static_cast<DerivedSampler const &>(*this).outsideValue(pos.row(i));
      }

      values(i) =
          static_cast<DerivedSampler const &>(*this).inverseValueTransform(val);
    }

    return values;
  }

  template <class Derived>
  inline auto interpolate(Eigen::MatrixBase<Derived> const &positions) const {
    using std::ceil;
    using std::floor;

    using PosScalar = typename Derived::Scalar;
    using PosMatrix = Eigen::Matrix<PosScalar, Derived::RowsAtCompileTime, 3>;

    return volume_.withUnsafeDataPointer([&positions, this](auto const *ptr) {
      using ValueType =
          std::remove_const_t<std::remove_pointer_t<decltype(ptr)>>;
      using ValueVector =
          Eigen::Matrix<ValueType, Derived::RowsAtCompileTime, 1>;

      auto const N = positions.rows();

      ValueVector samples(N, 1);

      PosMatrix const X0 = positions.array().floor().matrix();
      PosMatrix const X1 = positions.array().ceil().matrix();
      PosMatrix const Xd = positions - X0;

      // get values of lattice points
      ValueVector const c000 = at(X0, ptr);
      ValueVector const c001 = at(
          (PosMatrix(N, 3) << X0.col(0), X0.col(1), X1.col(2)).finished(), ptr);
      ValueVector const c010 = at(
          (PosMatrix(N, 3) << X0.col(0), X1.col(1), X0.col(2)).finished(), ptr);
      ValueVector const c011 = at(
          (PosMatrix(N, 3) << X0.col(0), X1.col(1), X1.col(2)).finished(), ptr);
      ValueVector const c100 = at(
          (PosMatrix(N, 3) << X1.col(0), X0.col(1), X0.col(2)).finished(), ptr);
      ValueVector const c101 = at(
          (PosMatrix(N, 3) << X1.col(0), X0.col(1), X1.col(2)).finished(), ptr);
      ValueVector const c110 = at(
          (PosMatrix(N, 3) << X1.col(0), X1.col(1), X0.col(2)).finished(), ptr);
      ValueVector const c111 = at(X1, ptr);

      ValueVector const c00 = (c000.array() * (1 - Xd.col(0).array()) +
                               c100.array() * Xd.col(0).array())
                                  .matrix();
      ValueVector const c01 = (c001.array() * (1 - Xd.col(0).array()) +
                               c101.array() * Xd.col(0).array())
                                  .matrix();
      ValueVector const c10 = (c010.array() * (1 - Xd.col(0).array()) +
                               c110.array() * Xd.col(0).array())
                                  .matrix();
      ValueVector const c11 = (c011.array() * (1 - Xd.col(0).array()) +
                               c111.array() * Xd.col(0).array())
                                  .matrix();

      ValueVector const c0 = (c00.array() * (1 - Xd.col(1).array()) +
                              c10.array() * Xd.col(1).array())
                                 .matrix();
      ValueVector const c1 = (c01.array() * (1 - Xd.col(1).array()) +
                              c11.array() * Xd.col(1).array())
                                 .matrix();

      ValueVector const c =
          (c0.array() * (1 - Xd.col(2).array()) +
           c1.array() * Xd.col(2).array())
              .matrix()
              .unaryExpr([this](auto const &x) {
                return static_cast<DerivedSampler const &>(*this)
                    .valueTransform(x);
              });

      return c;
    });
  }

  inline SamplerBase(T const &volume) : volume_(volume) {}

  T const &volume_;
};

class VolumeSampler : public SamplerBase<VoxelVolume, VolumeSampler> {

  using Base = SamplerBase<VoxelVolume, VolumeSampler>;

  friend Base;

public:
  inline explicit VolumeSampler(VoxelVolume const &vol) : Base(vol) {}

private:
  template <class Derived>
  inline auto toLinear(Eigen::MatrixBase<Derived> const &x) const {
    Expects(x.rows() == 1 && x.cols() == 3);

    auto const sliceSize = static_cast<std::ptrdiff_t>(volume_.size().width *
                                                       volume_.size().height);
    return x(2) * sliceSize +
           x(1) * static_cast<std::ptrdiff_t>(volume_.size().width) + x(0);
  }
};

} // namespace CortidQCT
