// Copyright (c) 2017-2018 Filipe Verri <filipeverri@gmail.com>

#ifndef GA_ALGORITHM_HPP
#define GA_ALGORITHM_HPP

#include <ga/meta.hpp>
#include <ga/type.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace ga
{

template <typename G> static auto draw(double rate, G& g) -> bool
{
  return std::generate_canonical<double, std::numeric_limits<double>::digits>(g) < rate;
}

template <typename T> class algorithm
{
  static_assert(meta::always_false<T>::value,
                "Problem type doesn't comply with the required concept");
};

template <meta::any_problem T> class algorithm<T>
{
public:
  using individual_type = typename T::individual_type;
  using generator_type = typename T::generator_type;
  using fitness_type = typename T::fitness_type;
  using solution_type = solution<individual_type, fitness_type>;

private:
  T problem_;
  std::vector<solution_type> population_;
  std::vector<individual_type> next_population_;
  std::vector<fitness_type> next_fitness_;
  std::size_t elite_count_;
  generator_type generator_;

public:
  algorithm(T problem, std::vector<individual_type> population,
            const std::size_t elite_count, generator_type generator)
    : problem_(std::move(problem))
    , elite_count_(elite_count)
    , generator_(std::move(generator))
  {
    if (elite_count_ >= population.size())
      throw std::invalid_argument{"invalid elite_count"};

    population_.reserve(population.size());
    next_population_.reserve(population.size() - elite_count_);
    next_fitness_.reserve(population.size() - elite_count_);

    {
      std::vector<fitness_type> first_fitness;
      first_fitness.reserve(population.size());

      if constexpr (meta::single_evaluation_problem<T>)
      {
        for (const auto& individual : population)
          first_fitness.push_back(problem_.evaluate(individual, generator_));
      }
      else
      {
        problem_.evaluate(std::as_const(population), population_, 0,
                          std::back_inserter(first_fitness), generator_);
      }

      if (first_fitness.size() != population.size() || population_.size() > 0)
        throw std::runtime_error{"evaluation step has changed expected population size"};

      auto ind_it = population.begin();
      auto fit_it = first_fitness.begin();

      const auto end = population.end();

      while (ind_it != end)
        population_.push_back({std::move(*ind_it++), std::move(*fit_it++)});
    }

    sort_population();
  }

  auto iterate() -> void
  {
    // == Mating Selection, Recombination and Mutation ==
    // We perform binary tournament selection with replacement.
    auto indexes =
      std::uniform_int_distribution<std::size_t>(0u, population_.size() - 1u);

    const auto binary_tournament = [&]() -> const individual_type& {
      const auto i = sample_from(indexes);
      const auto j = sample_from(indexes);
      return population_[i].fitness < population_[j].fitness ? population_[i].x
                                                             : population_[j].x;
    };

    const auto expected_size = population_.size();
    while (next_population_.size() < expected_size - elite_count_)
    {
      // Two binary tournament to select the parents.
      const auto& parent1 = binary_tournament();
      const auto& parent2 = binary_tournament();

      // Children are either a recombination or the parents themselves.
      auto children = problem_.recombine(parent1, parent2, generator_);

      // Mutate and put children in the new population.
      for (auto& child : children)
      {
        problem_.mutate(child, generator_);
        next_population_.push_back(std::move(child));
        if (next_population_.size() == expected_size - elite_count_)
          break;
      }
    }

    if constexpr (meta::single_evaluation_problem<T>)
    {
      for (const auto& individual : next_population_)
        next_fitness_.push_back(problem_.evaluate(individual, generator_));
    }
    else
    {
      problem_.evaluate(std::as_const(next_population_), population_, elite_count_,
                        std::back_inserter(next_fitness_), generator_);
    }

    if (population_.size() != expected_size ||
        next_population_.size() != expected_size - elite_count_ ||
        next_fitness_.size() != expected_size - elite_count_)
      throw std::runtime_error{"evaluation step has changed expected population size"};

    {
      auto ind_it = next_population_.begin();
      auto fit_it = next_fitness_.begin();

      auto sol_it = population_.begin() + elite_count_;
      const auto end = population_.end();

      while (sol_it != end)
        *sol_it++ = solution_type{std::move(*ind_it++), std::move(*fit_it++)};
    }

    next_population_.clear();
    next_fitness_.clear();

    sort_population();
  }

  auto population() const noexcept -> const std::vector<solution_type>&
  {
    return population_;
  }

  auto problem() noexcept -> T& { return static_cast<T&>(problem_); }
  auto problem() const noexcept -> const T& { return static_cast<const T&>(problem_); }

  auto generator() noexcept -> generator_type& { return generator_; }
  auto generator() const noexcept -> const generator_type& { return generator_; }

  auto elite_count() noexcept -> std::size_t& { return elite_count_; }
  [[nodiscard]] auto elite_count() const noexcept -> std::size_t { return elite_count_; }

private:
  auto sort_population() -> void
  {
    std::partial_sort(population_.begin(), population_.begin() + elite_count_,
                      population_.end(),
                      [](const solution_type& a, const solution_type& b) {
                        return a.fitness < b.fitness;
                      });
  }

  template <typename Distribution>
  auto sample_from(Distribution& dist) ->
    typename std::decay<decltype(dist(this->generator_))>::type
  {
    return dist(generator_);
  }
};

template <meta::any_problem T, typename I, typename G>
auto make_algorithm(T problem, std::vector<I> population, const std::size_t elite_count,
                    G generator) -> algorithm<T>
{
  return {std::move(problem), std::move(population), elite_count, std::move(generator)};
}

} // namespace ga

#endif // GA_ALGORITHM_HPP
