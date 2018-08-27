/**
 * @file      LoadFromBST.h
 *
 * @brief     Definition of function to load voxel volume from BST files
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#pragma once

#include "VolumeSize.h"
#include "VoxelSize.h"

#include <Eigen/Core>
#include <ImageStack/ImageStack.h>
#include <ImageStack/ImageStackLoaderBST.h>
#include <ImageStack/ResolutionDecorator.h>
#include <gsl/gsl>

namespace CortidQCT {

namespace IO {

/**
 * @brief Loads a BST file and calles the given functional with the loaded
 * data.
 *
 * The functional is called with a pointer to the raw data as the first
 * argument, the 3d size of the loaded data as the second argument a with the
 * 3d resolution (i.e. voxel size) as the third argument.
 *
 * @tparam F functional to be called by the function
 * @param filename Path to the file to be loaded
 * @param f functional that is beeing called
 * @throw std::runtime_error if the loading failed
 */
template <class F> void loadFromBST(std::string const &filename, F &&f) {
  using Decorator = ImageStack::ResolutionDecorator;
  using Img = ImageStack::ImageStack<float, ImageStack::HostStorage, Decorator>;
  using Loader = ImageStack::ImageStackLoaderBST<Img>;

  auto const img = Img(Loader(filename));

  auto const mapping = img.map();
  auto const size = img.size().template cast<std::size_t>().eval();
  auto const resol = img.resolution.template cast<float>().eval();
  VoxelSize voxelSize{resol.x(), resol.y(), resol.z()};
  VolumeSize volumeSize{size.x(), size.y(), size.z()};

  f(mapping.data(), volumeSize, voxelSize);
}

} // namespace IO

} // namespace CortidQCT
