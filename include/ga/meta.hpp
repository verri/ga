// Copyright (c) 2017-2020 Filipe Verri <filipeverri@gmail.com>

#ifndef GA_META_HPP
#define GA_META_HPP

#include <ga/type.hpp>

#include <array>
#include <type_traits>
#include <vector>

namespace ga::meta
{
template <typename T> struct always_false : std::false_type
{
};

template <typename T, typename U> concept convertible_to = std::is_convertible_v<T, U>;
template <typename T, typename U> concept same_as = std::is_same_v<T, U>;

using std::begin;
using std::end;

// clang-format off
template <typename T, typename U> concept iterable_of = requires(T& container)
{
  typename T::value_type;
  convertible_to<typename T::value_type, U>;
  { *begin(container) } -> convertible_to<U>;
  { *end(container) } -> convertible_to<U>;
};
// clang-format on

namespace detail
{
// clang-format off
template <typename T> concept partial_problem = requires
{
  typename T::individual_type;
  typename T::generator_type;
  typename T::fitness_type;
} && requires(T& problem, typename T::individual_type& candidate,
              typename T::generator_type& gen, typename T::fitness_type fitness)
{
  { problem.mutate(candidate, gen) } -> same_as<void>;
  { problem.recombine(std::as_const(candidate), std::as_const(candidate), gen) } -> iterable_of<typename T::individual_type>;
};
// clang-format on
} // namespace detail

// clang-format off
template <typename T>
concept single_evaluation_problem = detail::partial_problem<T> &&
  requires(T& problem, const typename T::individual_type& candidate, typename T::generator_type& gen)
{
  { problem.evaluate(candidate, gen) } -> convertible_to<typename T::fitness_type>;
};

template <typename T>
concept multi_evaluation_problem = detail::partial_problem<T> &&
  requires(T& problem, const std::vector<typename T::individual_type>& new_individuals,
           std::vector<ga::solution<typename T::individual_type, typename T::fitness_type>>& individuals,
           std::size_t elite_count, std::back_insert_iterator<std::vector<typename T::fitness_type>> fit_out,
           typename T::generator_type& gen)
{
  { problem.evaluate(new_individuals, individuals, elite_count, fit_out, gen) } -> same_as<void>;
};

template <typename T> concept any_problem = single_evaluation_problem<T> || multi_evaluation_problem<T>;
// clang-format on

} // namespace ga::meta

#endif // GA_META_HPP
