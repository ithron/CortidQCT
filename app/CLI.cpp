/**
 * @file      CLI.cpp
 *
 * @brief     Implementation file for the CortidQCT Command Line Interface
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include <CortidQCT/CortidQCT.h>

#include <exception>
#include <iostream>
#include <string>

using namespace CortidQCT;

int main(int argc, char **argv) {
  using namespace std::string_literals;

  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << " <configurationFile> <inputVolume> <outputMesh> [outputLabels]" << std::endl;
    return EXIT_FAILURE;
  }

  try {
    auto const volume = VoxelVolume{argv[2]};
    auto fitter = MeshFitter{argv[1]};

    auto const result = fitter.fit(volume);

    if (result.success) {
      auto const &mesh = result.deformedMesh;
      auto outLabels = "/dev/null"s;

      if (argc == 5) { outLabels = argv[4]; }

      mesh.writeToFile(argv[3], outLabels);
    } else {
      std::cerr << "Fitting failed with unknown error!" << std::endl;
      return EXIT_FAILURE;
    }

  } catch (std::invalid_argument const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
