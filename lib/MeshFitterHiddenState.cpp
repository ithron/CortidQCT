/**
 * @file      MeshFitterHiddenState.cpp
 *
 * @brief     Implementation of the MeshFitter HiddenState type
 *
 * @author    Stefan Reinhold
 * @copyright Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.
 *            You may use, distribute and modify this code under the terms of
 *            the AFL 3.0 license; see LICENSE for full license details.
 */

#include "MeshFitterHiddenState.h"

namespace CortidQCT {

using namespace Internal;

/**************
 * MeshFitter::State implementations
 */
// MARK: -
// MARK: MeshFitter::State

MeshFitter::State::State(State const &rhs)
    : Result(rhs), hiddenState_{
                       std::make_unique<HiddenState>(*rhs.hiddenState_)} {}

MeshFitter::State::State(Result const &rhs)
    : Result(rhs), hiddenState_{nullptr} {}

MeshFitter::State::State(Result &&rhs)
    : Result(std::move(rhs)), hiddenState_{nullptr} {}

MeshFitter::State::~State(){};

MeshFitter::State &MeshFitter::State::operator=(State const &rhs) {
  static_cast<Result &>(*this) = rhs;
  hiddenState_ = std::make_unique<HiddenState>(*rhs.hiddenState_);

  return *this;
}

} // namespace CortidQCT
