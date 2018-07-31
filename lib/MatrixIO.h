/**
 * @file      MatrixIO.h
 *
 * @brief     Contains a wrapper to enable IO on Eigen matrices.
 *
 * @author    Stefan Reinhold
 * @copyright
 *            Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include <Eigen/Core>

#include <iostream>

namespace CortidQCT {

/**
 * @brief Wrapper for Eigen matrices to support IO
 */
template <class MatrixType> struct MartrixIOWrapper {

  inline MartrixIOWrapper(Eigen::MatrixBase<MatrixType> &matrix)
      : matrix_(matrix.derived()) {}

  /// operator<< overload for io wrapped matrices
  friend inline std::ostream &operator<<(std::ostream &os,
                                         MartrixIOWrapper const &wrapper) {
    return os << wrapper.matrix_;
  }

  /// operator>> overload for io wrapped matrices
  friend inline std::istream &operator>>(std::istream &is,
                                         MartrixIOWrapper &&wrapper) {

    for (auto i = 0; i < wrapper.matrix_.rows(); ++i)
      for (auto j = 0; j < wrapper.matrix_.cols(); ++j)
        is >> wrapper.matrix_(i, j);

    return is;
  }

private:
  MatrixType &matrix_;
};

/**
 * Wrappes an Eigen matrix for IO support
 * @tparam MatrixType Eigen matrix type
 * @param matrix Eigen matrix
 * @return Wrapper object
 */
template <class MatrixType>
inline MartrixIOWrapper<MatrixType> io(Eigen::MatrixBase<MatrixType> &matrix) {
  return MartrixIOWrapper<MatrixType>(matrix);
}

/**
 * Wrappes an Eigen matrix for IO support
 * @tparam MatrixType Eigen matrix type
 * @param matrix Eigen matrix
 * @return Wrapper object (const)
 */
template <class MatrixType>
inline MartrixIOWrapper<MatrixType>
io(Eigen::MatrixBase<MatrixType> const &matrix) {
  return MartrixIOWrapper<MatrixType const>(matrix);
}

} // namespace CortidQCT
