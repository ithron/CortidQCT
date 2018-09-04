/**
 * @file      MeshFitterConfiguration.cpp
 *
 * @brief     Implementation file for MeshFitter::Configuration
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "ColorToLabelMapIO.h"
#include "MeshFitter.h"

#include <gsl/gsl>
#include <yaml-cpp/yaml.h>

namespace CortidQCT {

namespace {

inline std::string basePath(std::string const &path) {

  // find last occurence of 'separator'
  if (auto const pos = path.find_last_of("/\\");
      pos != std::string::npos && pos > 0) {
    return path.substr(0, pos);
  }

  return "";
}

inline std::string composePath(std::string const &origPath,
                               std::string const &relPath) {
#ifdef _WIN32
  // Maybe not the best check for windos
  if (relPath[1] == ':') { return relPath; }
#else
  if (relPath[0] == '/') { return relPath; }
#endif
  return basePath(origPath) + "/" + relPath;
}

} // anonymous namespace

MeshFitter::Configuration &
MeshFitter::Configuration::loadFromFile(std::string const &filename) {

  try {

    auto const node = YAML::LoadFile(filename);

    auto const &referenceMeshNode = node["referenceMesh"];
    auto const &measurementModelNode = node["measurementModel"];

    if (!referenceMeshNode) {
      throw std::invalid_argument("Missing 'referenceMesh' in " + filename);
    }

    if (!measurementModelNode) {
      throw std::invalid_argument("Missing 'measurementModel' in " + filename);
    }

    if (!referenceMeshNode["mesh"]) {
      throw std::invalid_argument("Missing 'referenceMesh.mesh' in " +
                                  filename);
    }

    auto const meshFilename =
        composePath(filename, referenceMeshNode["mesh"].as<std::string>());
    auto const modelFilename =
        composePath(filename, measurementModelNode.as<std::string>());

    auto refMesh = Mesh<float>{};

    auto meshOrigin = referenceMeshOrigin;

    if (auto const &originNode = referenceMeshNode["origin"]) {
      if (originNode.IsScalar()) {
        if (originNode.as<std::string>() == "untouched") {
          meshOrigin = OriginType::untouched;
        } else if (originNode.as<std::string>() == "centered") {
          meshOrigin = OriginType::centered;
        } else {
          throw std::invalid_argument("Invalid origin type '" +
                                      originNode.as<std::string>() + "' in " +
                                      filename);
        }
      } else {
        auto const &typeNode = originNode["type"];
        if (!typeNode || !typeNode.IsScalar()) {
          throw std::invalid_argument("Missing or invalid origin.type in " +
                                      filename);
        }

        if (typeNode.as<std::string>() == "untouched") {
          meshOrigin = OriginType::untouched;
        } else if (typeNode.as<std::string>() == "centered") {
          meshOrigin = OriginType::centered;
        } else if (typeNode.as<std::string>() == "absolute" ||
                   typeNode.as<std::string>() == "relative") {
          auto const &xyzNode = originNode["xyz"];
          if (!xyzNode || !xyzNode.IsSequence() || xyzNode.size() != 3) {
            throw std::invalid_argument("Missing or invalid origin.xyz in " +
                                        filename);
          }

          Coordinate3D originXYZ;

          originXYZ.xyz[0] = xyzNode.begin()->as<float>();
          originXYZ.xyz[1] = (++xyzNode.begin())->as<float>();
          originXYZ.xyz[2] = (++++xyzNode.begin())->as<float>();

          if (typeNode.as<std::string>() == "relative") {
            originXYZ.type = Coordinate3D::Type::relative;
          }

          meshOrigin = originXYZ;
        }
      }
    }

    referenceMeshOrigin = meshOrigin;

    if (auto const &labelNode = referenceMeshNode["labels"]) {
      auto const labelFilename =
          composePath(filename, labelNode.as<std::string>());
      refMesh.loadFromFile(meshFilename, labelFilename);
    } else if (auto const &colorMapNode =
                   referenceMeshNode["colorToLabelMap"]) {

      if (auto const &typeNode = colorMapNode["type"]) {
        auto const type = typeNode.as<std::string>();

        if (type == "default") {

          refMesh.loadFromFile(meshFilename);
        } else if (type == "custom") {

          ColorToLabelMap<Mesh<float>::Label, double> const colorToLabelMap =
              ColorToLabelMaps::IO::loadCustomMapFromYAMLNode(colorMapNode);

          refMesh.loadFromFile(meshFilename, colorToLabelMap);

        } else {
          throw std::invalid_argument("Invalid color to label map type: '" +
                                      type + "' in " + filename);
        }
      } else {
        // custom type
        ColorToLabelMap<Mesh<float>::Label, double> const colorToLabelMap =
            ColorToLabelMaps::IO::loadCustomMapFromYAMLNode(colorMapNode);

        refMesh.loadFromFile(meshFilename, colorToLabelMap);
      }
    } else {
      throw std::invalid_argument("Missing 'referenceMesh.labels' or "
                                  "'referenceMesh.colorToLabelMap' in " +
                                  filename);
    }

    Ensures(!refMesh.isEmpty());

    auto model_ = MeasurementModel{}.loadFromFile(modelFilename);

    if (auto sigmaENode = node["sigmaE"]) {
      sigmaE = sigmaENode.as<double>(sigmaE);
    }

    if (auto sigmaSNode = node["sigmaS"]) {
      sigmaS = sigmaSNode.as<double>(sigmaS);
    }

    model = std::move(model_);
    referenceMesh = std::move(refMesh);

  } catch (YAML::Exception const &e) {
    throw std::invalid_argument("Failed to load configuration file '" +
                                filename + "': " + e.what());
  }

  return *this;
}

} // namespace CortidQCT
