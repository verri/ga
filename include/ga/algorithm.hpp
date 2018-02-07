// Copyright (c) 2017-2018 Filipe Verri <filipeverri@gmail.com>

#ifndef GA_ALGORITHM_HPP
#define GA_ALGORITHM_HPP

#include <ga/meta.hpp>
#include <ga/problem.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <type_traits>
#include <vector>

namespace ga
{

template <typename G> static auto draw(double rate, G& g) -> bool
{
  return std::generate_canonical<double, std::numeric_limits<double>::digits>(g) < rate;
}

template <typename T, typename E = void> class algorithm
{
  static_assert(meta::always_false<T>::value,
                "Problem type doesn't comply with the required concept");
};

template <typename T> class algorithm<T, meta::requires<meta::Problem<T>>>
{
  using individual_type = typename T::individual_type;
  using generator_type = typename T::generator_type;
  using fitness_type = meta::evaluate_result<T>;

public:
  struct solution_type
  {
    individual_type x;
    fitness_type fitness;
  };

private:
  detail::problem<T> problem_;
  std::vector<solution_type> population_, next_population_;
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

    std::transform(population.begin(), population.end(), back_inserter(population_),
                   [&](individual_type& x) -> solution_type {
                     const auto fitness = problem_.evaluate(x, generator_);
                     return {std::move(x), fitness};
                   });

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

    while (next_population_.size() < population_.size() - elite_count_)
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
        const auto fitness = problem_.evaluate(child, generator_);
        next_population_.push_back({std::move(child), fitness});
        if (next_population_.size() == population_.size() - elite_count_)
          break;
      }
    }

    // Copy new individual preserving the elite_count_ best ones.
    std::move(next_population_.begin(), next_population_.end(),
              population_.begin() + elite_count_);
    next_population_.clear();

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

  auto elite_count() const noexcept -> std::size_t { return elite_count_; }

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

template <typename T, typename I, typename G, typename = meta::requires<meta::Problem<T>>>
auto make_algorithm(T problem, std::vector<I> population, const std::size_t elite_count,
                    G generator) -> algorithm<T>
{
  return {std::move(problem), std::move(population), elite_count, std::move(generator)};
}

template <typename T, typename... Args, typename = meta::fallback<meta::Problem<T>>>
auto make_algorithm(T, Args&&...) -> void
{
  static_assert(meta::always_false<T>::value,
                "Problem type doesn't comply with the required concept");
}

} // namespace ga

#endif // GA_ALGORITHM_HPP
