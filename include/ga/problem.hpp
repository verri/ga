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

template <typename T> class problem<T, meta::requires<meta::Problem<T>>> : private T {
public:
  using T::mutate;
  using T::recombine;
  using T::evaluate;

  operator const T&() const { return *this; }
  operator T&() { return *this; }

  problem(const T& t) : T(t) {}
  problem(T&& t) : T(std::move(t)) {}
};

} // namespace detail
} // namespace ga

#endif // GA_PROBLEM_HPP
