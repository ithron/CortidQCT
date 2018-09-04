/**
 * @file      MeshFitterImpl.cpp
 *
 * @brief     Implementation file for MeshFitter::Impl
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "MeshFitter.h"

namespace CortidQCT {

MeshFitter::Result MeshFitter::Impl::fit(VoxelVolume const & /*unused*/) {
  // Step:
  // 1. Move reference mesh to center
  // 2. Set deformedMesh = refMesh
  // 3. Compute normals of deformedMesh
  // 4. Sample volume along lines through the vertices of refMesh along surface
  //    normals
  // 5. For every displacement s in [-s0, +s0] compute the log likelihood using
  //    trilinear sampling of the measurement PDF
  // 6. Find optimal displacements
  // 7. Copmute target vertex positions using optimal displacements
  // 8. Fit mesh to target vertices
  // 9. Check for convergence, if not converged go to 3.
  return MeshFitter::Result{};
}

} // namespace CortidQCT
