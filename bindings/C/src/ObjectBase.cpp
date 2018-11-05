//
//  autoreleasepool.cpp
//  CortidQCT
//
//  Created by Stefan Reinhold on 03.11.18.
//

#include "CortidQCT-Common.h"
#include "CortidQCT.h"

#include <vector>

namespace {

/// Autorelease pool object
class AutoreleasePool {

  using ObjectBuffer = std::vector<Id>;
  std::vector<ObjectBuffer> stack;

public:
  void push() { stack.emplace_back(); }

  void pop() {
    if (stack.empty()) { return; }

    for (auto &&id : stack.back()) { CQCT_release(id); }

    stack.pop_back();
  }

  void autorelease(Id id) {
    if (stack.empty()) return;

    stack.back().push_back(id);
  }

  bool empty() const { return stack.empty(); }
};

static AutoreleasePool *gPool = nullptr;

/// Get the unique autorelease pool
static AutoreleasePool &getPool() {

  if (gPool == nullptr) { gPool = new AutoreleasePool; }

  return *gPool;
}

} // anonymous namespace

CQCT_EXTERN Id CQCT_autorelease(Id obj) {
  getPool().autorelease(obj);

  return obj;
}

CQCT_EXTERN void CQCT_autoreleasePoolPush() { getPool().push(); }

CQCT_EXTERN void CQCT_autoreleasePoolPop() {
  auto &pool = getPool();
  if (pool.empty()) {
    delete gPool;
  } else {
    getPool().pop();
  }
}

CQCT_EXTERN Id CQCT_retain(Id obj) {
  if (obj == nullptr) return obj;

  auto controlPtr = static_cast<CortidQCT::Internal::C::Control *>(obj);

  ++(controlPtr->retainCount);

  return obj;
}

CQCT_EXTERN void CQCT_release(Id obj) {
  if (obj == nullptr) return;

  auto controlPtr = static_cast<CortidQCT::Internal::C::Control *>(obj);

  if (controlPtr->retainCount == 1) {
    controlPtr->deleter(obj);
  } else {
    --(controlPtr->retainCount);
  }
}
