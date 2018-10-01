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
    auto const &densityNode = node["density"];

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
    if (!densityNode) {
      throw std::invalid_argument("Missing 'density' in " + filename);
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
    auto dataStorage = DataStorage{};
    for (auto &&datNode : densityNode) {
      auto const &labelNode = datNode["label"];
      if (!labelNode) {
        throw std::invalid_argument("Missing 'density.label' in " + filename);
      }
      auto const &dataNode = datNode["data"];
      if (!dataNode) {
        throw std::invalid_argument("Missing 'density.data' in " + filename);
      }
      auto const &scaleNode = datNode["scale"];
      if (!scaleNode) {
        throw std::invalid_argument("Missing 'density.scale' in " + filename);
      }

      auto const label = labelNode.as<unsigned int>();
      auto const scale = scaleNode.as<double>();
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

      auto storage = std::vector<double>(nSamples);

      // Copy binary data to storage, reinterpreting bytes as doubles.
      // It'd probably better to use std::bit_cast here, but that's not
      // available until c++20.
      std::memcpy(storage.data(), binaryData.data(), binaryData.size());

      Ensures(storage.size() == nRequiredSamples);

      VOIData voiData;
      voiData.label = label;
      voiData.scale = scale;
      voiData.data = std::move(storage);

      if (auto const &voiNameNode = datNode["name"]) {
        voiData.name = datNode.as<std::string>();
      }

      dataStorage.emplace(std::piecewise_construct,
                          std::forward_as_tuple(label),
                          std::forward_as_tuple(std::move(voiData)));
    }

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
