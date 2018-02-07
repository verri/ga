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

template <typename T>
class problem<T, meta::requires<meta::Problem<T>, meta::SingleEvaluation<T>>> : private T
{
public:
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

  auto evaluate(
    const std::vector<typename T::individual_type>& new_individuals,
    std::vector<::ga::solution<typename T::individual_type, typename T::fitness_type>>&,
    std::size_t, std::back_insert_iterator<std::vector<typename T::fitness_type>> fit_out,
    typename T::generator_type& g) -> void
  {
    for (const auto& individual : new_individuals)
      *fit_out++ = static_cast<T&>(*this).evaluate(individual, g);
  }
};

template <typename T>
class problem<T, meta::requires<meta::Problem<T>, meta::MultiEvaluation<T>>> : private T
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
