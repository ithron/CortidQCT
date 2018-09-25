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

using namespace CortidQCT;

int main(int argc, char **argv) {

  if (argc != 4) {
    std::cerr << "Usage: " << argv[0]
              << " configurationFile inputVolume outputMesh" << std::endl;
    return EXIT_FAILURE;
  }

  try {
    auto const volume = VoxelVolume{argv[2]};
    auto fitter = MeshFitter{argv[1]};

    auto const result = fitter.fit(volume);

    if (result.deformedMesh) {
      auto const &mesh = *result.deformedMesh;

      mesh.writeToFile(argv[3], "/dev/null");
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
