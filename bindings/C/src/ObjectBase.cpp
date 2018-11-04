//
//  autoreleasepool.cpp
//  CortidQCT
//
//  Created by Stefan Reinhold on 03.11.18.
//


#include "CortidQCT.h"
#include "CortidQCT-Common.h"

#include <vector>

namespace {

/// Autorelease pool object
class AutoreleasePool {
  
  using ObjectBuffer = std::vector<Id>;
  std::vector<ObjectBuffer> stack;
  
public:
  
  void push() {
    stack.emplace_back();
  }
  
  void pop() {
    if (stack.empty()) {
      return;
    }
    
    for (auto &&id : stack.back()) {
      CQCT_release(id);
    }
    
    stack.pop_back();
  }
  
  void autorelease(Id id) {
    if (stack.empty()) return;
    
    stack.back().push_back(id);
  }
};

/// Get the unique autorelease pool
static AutoreleasePool &getPool() {
  static AutoreleasePool pool;

  return pool;
}
  
} // anonymous namespace

CQCT_EXTERN void CQCT_autorelease(Id obj) {
  getPool().autorelease(obj);
}

CQCT_EXTERN void CQCT_autoreleasePoolPush() {
  getPool().push();
}
  
CQCT_EXTERN void CQCT_autoreleasePoolPop() {
  getPool().pop();
}

CQCT_EXTERN void CQCT_retain(Id obj) {
  if (obj == nullptr) return;
  
  auto controlPtr = static_cast<CortidQCT::Internal::C::Control *>(obj);
  
  ++(controlPtr->retainCount);
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
