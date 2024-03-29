/**
 * @file      VoxelVolume.h
 *
 * @brief     This header contains the definition of the VoxelVolume type.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "VolumeSize.h"
#include "VoxelSize.h"

#include <cassert>
#include <string>
#include <vector>

namespace CortidQCT {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
/**
 * @brief Type representing a voxel volume
 *
 * A 3D voxel volume where each voxel has a floating point density value.
 * Voxel size can be anisotropic.
 * @nosubgrouping .
 */
class VoxelVolume {
public:
  /// Voxel value type
  using ValueType = float;

private:
  /// Voxel data type
  using VoxelData = std::vector<ValueType>;

public:
  /// @name Construction
  /// @{

  /// Constructs an empty volume
  inline VoxelVolume() noexcept {}

  /// Constructs a volume by reading its data from a file
  /// @see loadFromFile
  inline VoxelVolume(std::string const &filename) : VoxelVolume() {
    loadFromFile(filename);
  }

  /// @}

  /// @name IO
  /// @{

  /// @brief Loads the volume data from file using format auto detection
  ///
  /// Supported file formats are: bst
  ///
  /// @param filename Path to the file to load the volume from
  /// @return Reference to the loaded volume
  /// @throw std::invalid_argument if the volume could not be loaded from the
  /// file given by filename
  /// @throw std::invalid_argument if the file format could not be recognized
  VoxelVolume &loadFromFile(std::string const &filename);

  /// @}

  /// @name Accessors
  /// @{

  /// Returns the volume size
  inline VolumeSize const &size() const noexcept { return volumeSize_; }

  /// Returns the voxel size
  inline VoxelSize const &voxelSize() const noexcept { return voxelSize_; }

  /// Returns true iff the volume is empty
  inline bool isEmpty() const noexcept { return volumeSize_.linear() == 0; }

  /// @}

  /// @name Claibration
  /// @{

  /// @brief Calibrates the voxel volume to BMD equivalent using the given 
  ///   calibration parameters
  ///
  /// The calibration is done using the equation BMD = slope * HU + intercept.
  ///
  /// @param slope Slope
  /// @param intercept Intercept
  /// @return Reference to `*this`.
  VoxelVolume &calibrate(ValueType slope, ValueType intercept);

  /// @}

  /**
   * @name Raw Data Access
   * The methods in this section all call a functional with a pointer to raw
   * data as its argument. The pointer is only guaranteed to be valid within
   * the call to the given functional.
   *
   * @attention Do not return the row pointer or save it any other way!
   * @{
   */

  /**
   * @brief Calls the given functional with an unsafe pointer to the raw voxel
   * storage.
   *
   * Voxel data are stored in column major order: y, x then z.
   *
   * @tparam F function that accepts a `ValueType const *` pointer as the only
   * argument.
   * @throws noexcept(conditional) iff `f(ValueType const *)` is noexcept
   * @return The return value of the functional
   */
  template <class F>
  inline auto withUnsafeDataPointer(F &&f) const
      noexcept(noexcept(f(std::declval<VoxelData>().data()))) {
    return f(voxelData_.data());
  }

  /// @}

private:
  /// The raw voxel data
  VoxelData voxelData_;

  /// The volume size
  VolumeSize volumeSize_;

  /// The voxel size
  VoxelSize voxelSize_;
};
#pragma clang diagnostic pop

} // namespace CortidQCT
