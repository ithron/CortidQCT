#include "CortidQCT-Common.h"

#include <CortidQCT/CortidQCT.h>

#include <string>
#include <cassert>

namespace CortidQCT {
namespace Internal {
namespace C {
  
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
struct Error {
  CQCT_ErrorId id;
  std::string message;
};
#pragma clang diagnostic pop
  
} // namespace C
} // namespace Internal
} // namespace CortidQCT

using namespace CortidQCT;
using namespace CortidQCT::Internal::C;

struct CQCT_Error_t {
  CortidQCT::Internal::C::GenericObjectWrapper<Error> impl;
};

CQCT_EXTERN CQCT_Error CQCT_createError(CQCT_ErrorId id, const char *message) {
  return static_cast<CQCT_Error>(constructObject<Error>(id, message));
}

CQCT_EXTERN CQCT_ErrorId CQCT_errorType(CQCT_Error error) {
  assert(error != nullptr);
  
  return error->impl.objPtr->id;
}

CQCT_EXTERN const char *CQCT_errorMessage(CQCT_Error error) {
  assert(error != nullptr);
  
  return error->impl.objPtr->message.c_str();
}
