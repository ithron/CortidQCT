/**
 * @file      MeshAdaptors.h
 *
 * @brief     This file contains mesh <-> Eigen matrix adaptors
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "Mesh.h"

#include <Eigen/Core>
#include <gsl/gsl>

namespace CortidQCT {
namespace Internal {
namespace Adaptor {

template <class T>
inline Eigen::Map<Eigen::Matrix<T, 3, Eigen::Dynamic> const>
vertexMap(Mesh<T> const &mesh) {
  return mesh.withUnsafeVertexPointer([&mesh](auto const *ptr) {
    return Eigen::Map<Eigen::Matrix<T, 3, Eigen::Dynamic> const>{
        ptr, 3, gsl::narrow_cast<Eigen::Index>(mesh.vertexCount())};
  });
}

template <class T>
inline Eigen::Map<Eigen::Matrix<T, 3, Eigen::Dynamic>>
vertexMap(Mesh<T> &mesh) {
  return mesh.withUnsafeVertexPointer([&mesh](auto *ptr) {
    return Eigen::Map<Eigen::Matrix<T, 3, Eigen::Dynamic>>{
        ptr, 3, gsl::narrow_cast<Eigen::Index>(mesh.vertexCount())};
  });
}

template <class T>
inline Eigen::Map<Eigen::Matrix<T, 3, Eigen::Dynamic> const>
vertexNormalMap(Mesh<T> const &mesh) {
  return mesh.withUnsafeVertexNormalPointer([&mesh](auto const *ptr) {
    return Eigen::Map<Eigen::Matrix<T, 3, Eigen::Dynamic> const>{
        ptr, 3, gsl::narrow_cast<Eigen::Index>(mesh.vertexCount())};
  });
}

template <class T>
inline Eigen::Map<Eigen::Matrix<T, 3, Eigen::Dynamic>>
vertexNormalMap(Mesh<T> &mesh) {
  return mesh.withUnsafeVertexNormalPointer([&mesh](auto *ptr) {
    return Eigen::Map<Eigen::Matrix<T, 3, Eigen::Dynamic>>{
        ptr, 3, gsl::narrow_cast<Eigen::Index>(mesh.vertexCount())};
  });
}

template <class T>
inline Eigen::Map<
    Eigen::Matrix<typename Mesh<T>::Index, 3, Eigen::Dynamic> const>
indexMap(Mesh<T> const &mesh) {
  using S = typename Mesh<T>::Index;
  return mesh.withUnsafeIndexPointer([&mesh](auto const *ptr) {
    return Eigen::Map<Eigen::Matrix<S, 3, Eigen::Dynamic> const>{
        ptr, 3, gsl::narrow_cast<Eigen::Index>(mesh.triangleCount())};
  });
}

template <class T>
inline Eigen::Map<Eigen::Matrix<typename Mesh<T>::Index, 3, Eigen::Dynamic>>
indexMap(Mesh<T> &mesh) {
  using S = typename Mesh<T>::Index;
  return mesh.withUnsafeIndexPointer([&mesh](auto *ptr) {
    return Eigen::Map<Eigen::Matrix<S, 3, Eigen::Dynamic>>{
        ptr, 3, gsl::narrow_cast<Eigen::Index>(mesh.triangleCount())};
  });
}

template <class T>
inline Eigen::Map<
    Eigen::Matrix<typename Mesh<T>::Label, Eigen::Dynamic, 1> const>
labelMap(Mesh<T> const &mesh) {
  using S = typename Mesh<T>::Label;
  return mesh.withUnsafeLabelPointer([&mesh](auto const *ptr) {
    return Eigen::Map<Eigen::Matrix<S, Eigen::Dynamic, 1> const>{
        ptr, gsl::narrow_cast<Eigen::Index>(mesh.vertexCount())};
  });
}

template <class T>
inline Eigen::Map<Eigen::Matrix<typename Mesh<T>::Label, Eigen::Dynamic, 1>>
labelMap(Mesh<T> &mesh) {
  using S = typename Mesh<T>::Label;
  return mesh.withUnsafeLabelPointer([&mesh](auto *ptr) {
    return Eigen::Map<Eigen::Matrix<S, Eigen::Dynamic, 1>>{
        ptr, gsl::narrow_cast<Eigen::Index>(mesh.vertexCount())};
  });
}

} // namespace Adaptor
} // namespace Internal
} // namespace CortidQCT
