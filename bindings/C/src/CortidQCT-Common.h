#pragma once

#include "CortidQCT.h"

#include <cstddef>

namespace CortidQCT {
namespace Internal {
namespace C {

typedef void(*Deleter)(Id);

CQCT_EXTERN struct Control {
  std::size_t retainCount = 1;
  Deleter deleter;
};

template<class T> struct GenericObjectWrapper {
  Control control;
  T *objPtr = nullptr;
  
  ~GenericObjectWrapper() {
    if (objPtr != nullptr) {
      delete objPtr;
    }
  }
};
  
template<class T> void genericDeleter(Id obj) {
  auto objWrapperPtr = static_cast<GenericObjectWrapper<T> *>(obj);
  delete objWrapperPtr;
}

template<class T, class ...Args> Id constructObject(Args &&...args) {
  auto wrapperPtr = new GenericObjectWrapper<T>;
  
  wrapperPtr->control.deleter = &genericDeleter<T>;
  wrapperPtr->objPtr = new T{std::forward<Args>(args)...};
  
  return static_cast<Id>(wrapperPtr);
}
    
} // namespace C
} // namespace Internal
} // namespace CortidQCT
