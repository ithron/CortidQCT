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
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace CortidQCT {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
/**
 * @brief Type representing the measurement model.
 *
 * A MeasurementModel can currently only be loaded from file.
 * @nosubgrouping .
 */
class MeasurementModel {
public:
  /// Label type
  using Label = unsigned int;

private:
  /// VOI data structure
  struct VOIData {
    Label label;
    double scale;
    std::string name;
    std::vector<float> data;
  };
  /// Type used to store data samples
  using DataStorage = std::unordered_map<Label, VOIData>;

public:
  struct Version {
    int major, minor, patch;
  };
  /// @name Properties
  /// @{

  Version version = {1, 0, 0};
  /// Optional model name
  std::optional<std::string> name;
  /// Optional model description
  std::optional<std::string> description;
  /// Optional model author
  std::optional<std::string> author;
  /// Optional model creation date
  /// @note Date is saved as a string. This may change is the future.
  std::optional<std::string> creationDate;

  /// in-plane kernel width (sigma of gaussian kernel)
  std::optional<float> kernelSigma;
  /// name of the reconstruction kernel (since version 2.0.0)
  std::optional<std::string> kernelName;
  /// scanner name (since version 2..0.0)
  std::optional<std::string> scannerName;
  /// slice thickness
  float sliceSpacing = 1.f;

  /// sampling range for the line sampling
  DiscreteRange<float> samplingRange{-2.f, 2.f, 0.1f};
  /// range for the density sampling
  DiscreteRange<float> densityRange{-1000.f, 2000.f, 1.f};
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
  /// - `densityRange` defaults to [-1000, 3000] with a stride of 1
  /// - `angleRange` defaults to [0, 90] with a stride of 1.
  ///
  /// @throws noexcept iff `noexcept(DataStorage())`
  inline MeasurementModel() noexcept(noexcept(DataStorage())) {}

  /// @brief Creates a measurement model by loading from a file
  ///
  /// @param filename path to model file
  /// @see loadFromFile
  explicit MeasurementModel(std::string const &filename);

  /// Returns a model loaded from file
  /// @see loadFromFile
  inline static MeasurementModel fromFile(std::string const &filename) {
    return MeasurementModel(filename);
  }

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

  /// Returns all valid labels of the model
  inline auto labels() const {
    std::set<Label> labels;
    for (auto &&entry : data_) { labels.insert(entry.first); }

    return labels;
  }

  /// Returns the number of valid labels of the model
  inline std::size_t labelCount() const noexcept { return data_.size(); }

  /// Returns the density scale parameter of the given VOI
  inline double densityScale(Label const &label) const {
    if (auto const voiIt = data_.find(label); voiIt != data_.end()) {
      return voiIt->second.scale;
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

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
   * Sample data are stored in order rho, phi, t.
   *
   * @tparam F function that accepts a `double const *` pointer as the only
   * argument.
   * @param label Label to retrieve the data pointer for
   * @param f functional that is called with the raw data pointer as an
   * argument,
   * @return The return value of the functional
   */
  template <class F>
  inline auto withUnsafeDataPointer(Label label, F &&f) const {
    if (auto const store = data_.find(label); store != data_.end()) {
      return f(store->second.data.data());
    }

    throw std::invalid_argument("Label " + std::to_string(label) +
                                " not found");

    // Should never be called, but GCC complains if nothing is returned.
    return f(nullptr);
  }

  /// @}

private:

  /// Reorders data to optimize cache locality
  void reorderData();

  /// Storage for data samples
  DataStorage data_;
};
#pragma clang diagnostic pop

} // namespace CortidQCT
