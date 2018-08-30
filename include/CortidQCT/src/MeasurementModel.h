/**
 * @file      MeasurementModel.h
 *
 * @brief     This header contains the definition of the MeasurementModel type.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "DiscreteRange.h"
#include "Optional.h"

#include <chrono>
#include <string>
#include <vector>

namespace CortidQCT {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
/**
 * Type representing the measurement model.
 *
 * A MeasurementModel can currently only be loaded from file.
 * @nosubgrouping .
 */
class MeasurementModel {
  /// Type used to store data samples
  using DataStorage = std::vector<double>;

public:
  /// @name Properties
  /// @{

  /// Optional model name
  std::optional<std::string> name;
  /// Optional model description
  std::optional<std::string> description;
  /// Optional model author
  std::optional<std::string> author;
  /// Optional model creation date
  /// @note Date is saved as a string. This may change is the future.
  std::optional<std::string> creationDate;

  /// in-plane kernel width (σ of gaussian kernel)
  float kernelSigma = 1.f;
  /// slice thickness
  float sliceSpacing = 1.f;

  /// sampling range for the line sampling
  DiscreteRange<float> samplingRange{-2.f, 2.f, 0.1f};
  /// range for the density sampling
  DiscreteRange<float> densityRange{-1000.f, 2000.f, 10.f};
  /// range for the angle sampling
  DiscreteRange<float> angleRange{0.f, 90.f, 1.f};

  /// @}

  /// @name Construction
  /// @{

  /// @brief Creates an empty measurement model
  ///
  /// - `kernelSigma` defaults to 1,
  /// - `sliceSpacing` default to 1,
  /// - `samplingRange` defaults to [-2, 2] with a stride of 0.1,
  /// - `densityRange` defaults to [-1000, 3000] with a stride of 10
  /// - `angleRange` defaults to [0, 90] with a stride of 1.
  ///
  /// @throws noexcept iff `noexcept(DataStorage())`
  inline MeasurementModel() noexcept(noexcept(DataStorage())) {}

  /// @brief Creates a measurement model by loading from a file
  ///
  /// @param filename path to model file
  /// @see loadFromFile
  explicit MeasurementModel(std::string const &filename);

  /// @}

  /// @name IO
  /// @{

  /// Loads the model from the given file
  ///
  /// @param filename path to the model file
  /// @throws std::invalid_argument if the model failed to load
  /// @return Reference to the loaded model
  MeasurementModel &loadFromFile(std::string const &filename);

  /// @}

  /// @name Accessors
  /// @{

  /// Returns true iff to model contains to data
  inline bool isEmpty() const noexcept { return data_.empty(); }

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
   * @brief Calls the given functional with an unsafe pointer to the raw sample
   * data storage.
   *
   * Sample data are stored in order t, φ, ρ.
   *
   * @tparam F function that accepts a `double const *` pointer as the only
   * argument.
   * @throws noexcept(conditional) iff `f(double const *)` is noexcept
   * @return The return value of the functional
   */
  template <class F>
  inline auto withUnsafeDataPointer(F &&f) const
      noexcept(noexcept(f(std::declval<DataStorage>().data()))) {
    return f(data_.data());
  }

  /// @}

private:
  /// Storage for data samples
  DataStorage data_;
};
#pragma clang diagnostic pop

} // namespace CortidQCT
