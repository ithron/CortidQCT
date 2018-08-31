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

  return discreteRange(min, max, stride);
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

    auto const nRequiredSamples = samplingRange_.numElements() *
                                  densityRange_.numElements() *
                                  angleRange_.numElements();

    if (nSamples != nRequiredSamples) {
      throw std::invalid_argument(
          "Inconsistent data. Got "s + std::to_string(nSamples) +
          " samples but required "s + std::to_string(nRequiredSamples));
    }

    auto dataStorage = DataStorage(nSamples);
    // Copy binary data to storage, reinterpreting bytes as doubles.
    // It'd probably better to use std::bit_cast here, but that's not available
    // until c++20.
    std::memcpy(dataStorage.data(), binaryData.data(), binaryData.size());

    Ensures(dataStorage.size() == nRequiredSamples);

    std::optional<std::string> name_, description_, author_;
    std::optional<std::string> creationDate_;

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
      creationDate_ = dateNode.as<std::string>();
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
