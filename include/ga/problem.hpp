// Copyright (c) 2018 Filipe Verri <filipeverri@gmail.com>

#include <ga/meta.hpp>

#ifndef GA_PROBLEM_HPP
#define GA_PROBLEM_HPP

namespace ga
{
namespace detail
{

template <typename T, typename E = void> class problem
{
  static_assert(meta::always_false<T>::value,
                "Problem type doesn't comply with the required concept");
};

template <typename T> class problem<T, meta::requires<meta::Problem<T>>> : private T
{
public:
  using T::evaluate;
  using T::mutate;
  using T::recombine;

  constexpr operator const T&() const noexcept { return *this; }
  operator T&() noexcept { return *this; }

  constexpr problem(const T& t)
    : T(t)
  {
  }
  constexpr problem(T&& t) noexcept
    : T(std::move(t))
  {
  }
};

} // namespace detail
} // namespace ga

#endif // GA_PROBLEM_HPP
