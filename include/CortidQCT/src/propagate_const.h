#pragma once

#if __has_include(<propagate_const>)
#  include <propagate_const>
#elif __has_include(<experimental/propagate_const>)

#  include <experimental/propagate_const>
namespace std {
using experimental::propagate_const;
} // namespace std

#else

#define CORTIDQCT_NO_PROPAGATE_CONST

#endif
