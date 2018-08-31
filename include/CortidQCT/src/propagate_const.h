#pragma once

#if __has_include(<propagate_const>)
#  include <propagate_const>
#elif __has_include(<experimental/propagate_const>)

#  include <experimental/propagate_const>
namespace std {
using experimental::propagate_const;
} // namespace std

#else

namespace std {
namespace experimental {
inline namespace fundamentals_v2 {
template <class T> class propagate_const {
public:
  typedef remove_reference_t<decltype(*declval<T &>())> element_type;

  // [propagate_const.ctor], constructors
  constexpr propagate_const() = default;
  propagate_const(const propagate_const &p) = delete;
  constexpr propagate_const(propagate_const &&p) = default;
  template <class U>
  see below constexpr propagate_const(propagate_const<U> &&pu);
  template <class U> see below constexpr propagate_const(U &&u);

  // [propagate_const.assignment], assignment
  propagate_const &operator=(const propagate_const &p) = delete;
  constexpr propagate_const &operator=(propagate_const &&p) = default;
  template <class U>
  constexpr propagate_const &operator=(propagate_const<U> &&pu);
  template <class U> constexpr propagate_const &operator=(U &&u);

  // [propagate_const.const_observers], const observers
  explicit constexpr operator bool() const;
  constexpr const element_type *operator->() const;
  constexpr operator const element_type *() const; // Not always defined
  constexpr const element_type &operator*() const;
  constexpr const element_type *get() const;

  // [propagate_const.non_const_observers], non-const observers
  constexpr element_type *operator->();
  constexpr operator element_type *(); // Not always defined
  constexpr element_type &operator*();
  constexpr element_type *get();

  // [propagate_const.modifiers], modifiers
  constexpr void swap(propagate_const &pt) noexcept(see below);

private:
  T t_; // exposition only
};

// [propagate_const.relational], relational operators
template <class T>
constexpr bool operator==(const propagate_const<T> &pt, nullptr_t);
template <class T>
constexpr bool operator==(nullptr_t, const propagate_const<T> &pu);

template <class T>
constexpr bool operator!=(const propagate_const<T> &pt, nullptr_t);
template <class T>
constexpr bool operator!=(nullptr_t, const propagate_const<T> &pu);

template <class T, class U>
constexpr bool operator==(const propagate_const<T> &pt,
                          const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator!=(const propagate_const<T> &pt,
                          const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator<(const propagate_const<T> &pt,
                         const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator>(const propagate_const<T> &pt,
                         const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator<=(const propagate_const<T> &pt,
                          const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator>=(const propagate_const<T> &pt,
                          const propagate_const<U> &pu);

template <class T, class U>
constexpr bool operator==(const propagate_const<T> &pt, const U &u);
template <class T, class U>
constexpr bool operator!=(const propagate_const<T> &pt, const U &u);
template <class T, class U>
constexpr bool operator<(const propagate_const<T> &pt, const U &u);
template <class T, class U>
constexpr bool operator>(const propagate_const<T> &pt, const U &u);
template <class T, class U>
constexpr bool operator<=(const propagate_const<T> &pt, const U &u);
template <class T, class U>
constexpr bool operator>=(const propagate_const<T> &pt, const U &u);

template <class T, class U>
constexpr bool operator==(const T &t, const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator!=(const T &t, const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator<(const T &t, const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator>(const T &t, const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator<=(const T &t, const propagate_const<U> &pu);
template <class T, class U>
constexpr bool operator>=(const T &t, const propagate_const<U> &pu);

// [propagate_const.algorithms], specialized algorithms
template <class T>
constexpr void swap(propagate_const<T> &pt,
                    propagate_const<T> &pt2) noexcept(see below);

// [propagate_const.underlying], underlying pointer access
template <class T>
constexpr const T &get_underlying(const propagate_const<T> &pt) noexcept;
template <class T> constexpr T &get_underlying(propagate_const<T> &pt) noexcept;
} //  end namespace fundamentals_v2
} //  end namespace experimental

// [propagate_const.hash], hash support
template <class T> struct hash;
template <class T>
struct hash<experimental::fundamentals_v2::propagate_const<T>>;

// [propagate_const.comparison_function_objects], comparison function objects
template <class T> struct equal_to;
template <class T>
struct equal_to<experimental::fundamentals_v2::propagate_const<T>>;
template <class T> struct not_equal_to;
template <class T>
struct not_equal_to<experimental::fundamentals_v2::propagate_const<T>>;
template <class T> struct less;
template <class T>
struct less<experimental::fundamentals_v2::propagate_const<T>>;
template <class T> struct greater;
template <class T>
struct greater<experimental::fundamentals_v2::propagate_const<T>>;
template <class T> struct less_equal;
template <class T>
struct less_equal<experimental::fundamentals_v2::propagate_const<T>>;
template <class T> struct greater_equal;
template <class T>
struct greater_equal<experimental::fundamentals_v2::propagate_const<T>>;

using experimental::propagate_const;
} // end namespace std

#endif
