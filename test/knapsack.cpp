#include "ga/algorithm.hpp"

#include <catch.hpp>

#include <algorithm>
#include <iostream>
#include <random>
#include <valarray>

class knapsack
{
public:
  using individual_type = std::valarray<bool>;
  using generator_type = std::mt19937;

  knapsack(std::array<std::valarray<double>, 2u> values, std::valarray<double> weights,
           double capacity, double mutation_rate, double recombination_rate)
    : values{std::move(values)}
    , weights{std::move(weights)}
    , capacity{capacity}
    , mutation_rate{mutation_rate}
    , recombination_rate{recombination_rate}
  {
    const auto size = this->weights.size();
    for (const auto& v : this->values)
      if (v.size() != this->weights.size())
        throw std::runtime_error{"mismatching sizes"};
  }

  auto evaluate(const individual_type& x, generator_type&) const -> std::array<double, 2u>
  {
    auto result = std::array<double, 2u>{};
    const auto overweight = weights[x].sum() > capacity;

    for (auto i = 0ul; i < 2u; ++i)
      result[i] = overweight ? 0.0 : -values[i][x].sum();

    return {{result[0], result[1]}};
  }

  auto mutate(individual_type& x, generator_type& g) const -> void
  {
    for (auto& allele : x)
      if (ga::draw(mutation_rate, g))
        allele = !allele;
  }

  auto recombine(const individual_type& parent1, const individual_type& parent2,
                 generator_type& g) const -> std::array<individual_type, 2u>
  {
    if (!ga::draw(recombination_rate, g))
      return {{parent1, parent2}};

    auto mask = individual_type(parent1.size());
    for (auto& v : mask)
      v = ga::draw(0.5, g);

    return {{
      (parent1 && mask) || (parent2 && !mask), // first child
      (parent1 && !mask) || (parent2 && mask)  // second child
    }};
  }

private:
  std::array<std::valarray<double>, 2u> values;
  std::valarray<double> weights;
  double capacity;

  double mutation_rate, recombination_rate;
};

static_assert(ga::meta::Problem<knapsack>::value,
              "Knapsack problem doesn't comply with ga::Problem concept");

template <typename F> static auto generate_random_valarray(std::size_t size, F f)
{
  auto result = std::valarray<std::result_of_t<F()>>(size);
  std::generate_n(begin(result), size, std::move(f));
  return result;
}

template <typename F> static auto generate_random_vector(std::size_t size, F f)
{
  auto result = std::vector<std::result_of_t<F()>>();
  result.reserve(size);
  std::generate_n(back_inserter(result), size, std::move(f));
  return result;
}

TEST_CASE("Knapsack", "")
{
  static constexpr auto item_count = 50u;
  static constexpr auto generation_count = 100u;
  static constexpr auto population_size = 100u;
  static constexpr auto elite_count = 5u;

  // Our pseudo-random number generator.
  auto generator = std::mt19937{17};

  // Initial population.
  auto initial_population = generate_random_vector(population_size, [&] {
    auto individual = knapsack::individual_type(item_count);
    if (individual.size() != item_count)
      std::abort();
    for (auto& allele : individual)
      allele = ga::draw(0.1, generator);
    return individual;
  });

  // Problem.
  const auto random_values = [&]() {
    return generate_random_valarray(
      item_count, [&] { return std::generate_canonical<double, 64>(generator); });
  };

  auto problem = knapsack{/* values = */ {{random_values(), random_values()}},
                          /* weights = */ random_values(),
                          /* capacity = */ 0.3 * item_count,
                          /* mutation = */ 1.0 / item_count,
                          /* crossover = */ 0.4};

  // Algorithm
  auto algorithm = ga::make_algorithm(std::move(problem), std::move(initial_population),
                                      elite_count, std::move(generator));

  // Iterate
  for (auto t = 0u; t < generation_count; ++t)
    algorithm.iterate();

  for (const auto& solution : algorithm.population())
  {
    std::cout << std::fixed << std::setprecision(4) << "x = ";
    for (auto allele : solution.x)
      std::cout << (allele ? 1 : 0);
    std::cout << "\tf(x) = [";
    std::cout << ' ' << -solution.fitness[0] << ' ' << -solution.fitness[1];
    std::cout << " ]\n";
  }
}
