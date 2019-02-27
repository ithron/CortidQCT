/**
 * @file      cli.c
 *
 * @brief     This file contains an example C only implementation of the CLI to
 *            CortidQCT.
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include <CortidQCT/C/CortidQCT.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

  if (argc != 4) {
    fprintf(stderr,
            "Usage: %s <configurationFile> <inputVolume> <outputMesh> "
            "[outputLabels]",
            argv[0]);
    return EXIT_FAILURE;
  }

  int exitCode = EXIT_FAILURE;

  // Top level autorelease pool, this is always required.
  CQCT_autoreleasePoolPush();

  CQCT_Error error = NULL;

  // Load voxel volume from file
  CQCT_VoxelVolume volume = CQCT_createVoxelVolume();
  CQCT_voxelVolumeLoadFromFile(volume, argv[2], &error);
  if (volume == NULL) {
    fprintf(stderr, "Error: %s\n", CQCT_errorMessage(error));
    goto EXIT;
  }

  // Load MeshFitter (configuration) from file
  CQCT_MeshFitter fitter = CQCT_createMeshFitter(argv[1], &error);
  if (fitter == NULL) {
    fprintf(stderr, "Error: %s\n", CQCT_errorMessage(error));
    goto EXIT;
  }

  // Call fitting function
  CQCT_MeshFitterResult result = CQCT_meshFitterFit(fitter, volume);

  // Ger result deformed mesh
  CQCT_Mesh deformedMesh = CQCT_meshFitterResultMesh(result);
  if (deformedMesh == NULL) {
    fprintf(stderr, "Fitting failed with unkown error\n");
    goto EXIT;
  }

  const char *labelFilename = argc == 5 ? argv[4] : "/dev/null";

  // Write result mehr to file
  if (!CQCT_meshAndLabelsWriteToFile(deformedMesh, argv[3], labelFilename,
                                     &error)) {
    fprintf(stderr, "Failed to write result mesh: %s\n",
            CQCT_errorMessage(error));
    goto EXIT;
  }

  // Everything went fine
  exitCode = EXIT_SUCCESS;

EXIT:
  // Release all created objects, doesn't do anything when passed NULL.
  CQCT_release(volume);
  CQCT_release(fitter);

  // Release all autoreleased objects
  CQCT_autoreleasePoolPop();

  return exitCode;
}
