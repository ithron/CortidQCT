#pragma once


// clang-format off
#if __has_include(<propagate_const>)
  #include <propagate_const>
#elif __has_include(<experimental/propagate_const>)

  #include <experimental/propagate_const> 
  namespace std {
    using experimental::propagate_const;
  } // namespace std

#else

  #error "CortidQCT requires a standard library with propagate_const support"

#endif
// clang-format on
