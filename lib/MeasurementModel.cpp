/**
 * @file      MeasurementModel.cpp
 *
 * @brief     Implementation file for MeasurementModel type.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "MeasurementModel.h"

#include <date/date.h>
#include <gsl/gsl>
#include <yaml-cpp/yaml.h>

#include <sstream>
#include <string>

namespace CortidQCT {

namespace {

template <class T> DiscreteRange<T> parseRange(YAML::Node const &node) {
  auto const &minNode = node["min"];
  auto const &maxNode = node["max"];
  auto const &strideNode = node["stride"];

  if (!minNode) { throw std::runtime_error("Missing 'min' in range"); }
  if (!maxNode) { throw std::runtime_error("Missing 'max' in range"); }
  if (!strideNode) { throw std::runtime_error("Missing 'stride' in range"); }

  auto const min = minNode.as<T>();
  auto const max = maxNode.as<T>();
  auto const stride = strideNode.as<T>();

  return DiscreteRange(min, max, stride);
}

} // anonymous namespace

MeasurementModel::MeasurementModel(std::string const &filename)
    : MeasurementModel() {
  loadFromFile(filename);
}

MeasurementModel &MeasurementModel::loadFromFile(std::string const &filename) {
  using namespace std::string_literals;

  try {
    auto const node = YAML::LoadFile(filename);

    // Parse required tags

    auto const &kernelNode = node["kernel"];
    auto const &sliceSpacingNode = node["sliceSpacing"];
    auto const &samplingRangeNode = node["samplingRange"];
    auto const &densityRangeNode = node["densityRange"];
    auto const &angleRangeNode = node["angleRange"];
    auto const &dataNode = node["data"];

    if (!kernelNode) {
      throw std::invalid_argument("Missing 'kernel' in " + filename);
    }
    if (!sliceSpacingNode) {
      throw std::invalid_argument("Missing 'sliceSpacing' in " + filename);
    }
    if (!samplingRangeNode) {
      throw std::invalid_argument("Missing 'samplingRange' in " + filename);
    }
    if (!densityRangeNode) {
      throw std::invalid_argument("Missing 'densityRange' in " + filename);
    }
    if (!angleRangeNode) {
      throw std::invalid_argument("Missing 'angleRange' in " + filename);
    }
    if (!dataNode) {
      throw std::invalid_argument("Missing 'data' in " + filename);
    }
    auto const &kernelSigmaNode = kernelNode["sigma"];

    if (!kernelSigmaNode) {
      throw std::invalid_argument("Missing 'kernel.sigma' in " + filename);
    }

    auto const kernelSigma_ = kernelSigmaNode.as<float>();
    auto const sliceSpacing_ = sliceSpacingNode.as<float>();

    auto const samplingRange_ = parseRange<float>(samplingRangeNode);
    auto const densityRange_ = parseRange<float>(densityRangeNode);
    auto const angleRange_ = parseRange<float>(angleRangeNode);

    // load  data
    auto const binaryData = dataNode.as<YAML::Binary>();
    auto const nSamples = binaryData.size() / sizeof(double);

    // Note: C++ core guideline forbids the use of reinterpret_cast, but since
    // here is no way around it, disable linter for the specific line.
    auto const dataSpan = gsl::make_span(
        reinterpret_cast<double const *>(binaryData.data()), // NOLINT
        gsl::narrow<std::ptrdiff_t>(nSamples));
    auto dataStorage = DataStorage(dataSpan.begin(), dataSpan.end());

    std::optional<std::string> name_, description_, author_;
    std::optional<std::chrono::time_point<std::chrono::system_clock>>
        creationDate_;

    if (auto const &nameNode = node["name"]) {
      name_ = nameNode.as<std::string>();
    }

    if (auto const descriptionNode = node["description"]) {
      description_ = descriptionNode.as<std::string>();
    }

    if (auto const authorNode = node["author"]) {
      author_ = authorNode.as<std::string>();
    }

    if (auto const dateNode = node["creationDate"]) {
      auto const dateString = dateNode.as<std::string>();
      std::istringstream iss{dateString};
      date::sys_time<std::chrono::seconds> timePoint;
      iss >> date::parse("%F%t%T%t%z", timePoint);

      if (iss.fail()) {
        throw std::invalid_argument("Failed to parse date '" + dateString +
                                    "' in file " + filename);
      }

      creationDate_ = timePoint;
    }

    // write loaded data to *this
    name = name_;
    description = description_;
    author = author_;
    creationDate = creationDate_;
    kernelSigma = kernelSigma_;
    sliceSpacing = sliceSpacing_;
    samplingRange = samplingRange_;
    densityRange = densityRange_;
    angleRange = angleRange_;
    data_ = std::move(dataStorage);

  } catch (YAML::Exception const &e) {
    throw std::invalid_argument("Failed to load model file: "s + e.what());
  } catch (std::runtime_error const &e) {
    throw std::invalid_argument("Failed to load model file '"s + filename +
                                "':"s + e.what());
  }

  return *this;
}

} // namespace CortidQCT
