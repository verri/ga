# Genetic Algorithm

This is a header-only, generic C++ implementation of a standard genetic algorithm.

## Requirements

To use the library, you will need a C++11-compliant compiler.

## Documentation

This library is header-only.  Thus, the only thing you need is to include
the headers in your project.  The core of the library is the class `ga::algorithm` which expects
a user-defined optimization problem as a template argument.  Such feature enables a generic, optimized
library with no virtual calls.

The user-defined problem must comply with the following class:
```c++
class problem
{
public:
  using individual_type = /* ... */;
  using generator_type = /* ... */;

  auto evaluate(individual_type&, generator_type&)
    -> /* see below */;

  auto recombine(const individual_type&, const individual_type&, generator_type&)
    -> /* see below */;

  auto mutate(individual_type&, generator_type&)
    -> void;

   /* ... */
};
```

The aliases (or nested classes) `problem::individual_type` and `problem::generator_type`
are the type of the individuals and a
[random number engine](http://en.cppreference.com/w/cpp/numeric/random), respectively.

The function members `problem::evaluate`, `problem::recombine`, and `problem::mutate`
define the functioning of the algorithm.

`problem::evaluate` is called *exactly once* per individual during the
execution of the algorithm. It receives the individual to evaluate and a random
engine, and it must return the fitness value. The type of the fitness value
must be sortable. That is, given two fitness values `a` and `b`, `a < b` must
be a valid expression. The algorithm will try to minimize such objectives.

`problem::recombine` is called for every two selected (by binary tournament)
individuals from the archive to populate the next generation.  It receives
references to the individuals and a random engine, and it must return a range
of any number of individuals. The new individuals (which can be the same, if
the user wants to) go the next population up to the maximum allowed number.
Extra individuals will be discarded.

`problem::mutate` is called *exactly once* per individual returned by `problem::recombine`.
It receives the individual and a random engine.

The class doesn't necessarily need to have exactly the same function-member signatures.
Parameters type must be compatible.  If the user-defined
class doesn't comply with the requirement, a `static_assert` is triggered, avoiding nasty
compiler cries.

Once you define the problem class, you can use the algorithm:
```c++
int main()
{
  // ===== Prepare input and instanciate the algorithm ===== //

  problem myproblem = /* ... */
  std::vector<problem::individual_type> initial_population = /* ... */;
  std::size_t elite_count = /* ... */
  problem::generator_type generator;

  ga::algorithm<myproblem> algorithm(std::move(myproblem), std::move(initial_population), elite_count, std::move(generator));
  // or
  // auto algorithm = ga::make_algorithm(std::move(myproblem), std::move(initial_population), elite_count, std::move(generator));

  // ===== Iterate the algorithm ===== //

  const unsigned generation_count = 100u;
  for (unsigned i = 0u; i < 100; ++i)
    algorithm.iterate();

  // ===== Retrieve solution information ===== //

  // Every candidate solution in the population sorted by fitness.
  for (const ga::algorithm<problem>::solution_type& solution : algorithm.population())
  {
    const problem::individual_type& x = solution.x;
    const auto& fitness = solution.fitness;
  }

  // ===== Other helpers ===== //

  problem::generator_type& generator = algorithm.engine();
  problem& problem = algorithm.problem();
}
```

## Example

To illustrate the usage, let's implement a multi-objective
[Knapsack problem](https://en.wikipedia.org/wiki/Knapsack_problem).
It will use lexicographical comparison for selecting the best
solutions.

First, we include the required headers.
```c++
#include <ga/algorithm> // ga::make_algorithm, ga::draw
#include <random>       // std::mt19937
#include <valarray>     // std::valarray
```

Then, we define the problem class.
```c++
class knapsack
{
public:
  using individual_type = std::valarray<bool>;
  using generator_type = std::mt19937;

  knapsack(std::array<std::valarray<double>, 2u> values,
           std::valarray<double> weights, double capacity, double mutation_rate,
           double recombination_rate)
  {
    /* ... */
  }

  auto evaluate(const individual_type& x, generator_type&) const
  {
    auto result = std::array<double, 2u>{};
    const auto overweight = weights[x].sum() > capacity;

    for (auto i = 0ul; i < 2u; ++i)
      result[i] = overweight ? 0.0 : -values[i][x].sum();

    return result;
  }

  auto mutate(individual_type& x, generator_type& g) const
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
      (parent1 && mask) || (parent2 && !mask),
      (parent1 && !mask) || (parent2 && mask)
    }};
  }

private:
  std::array<std::valarray<double>, 2u> values;
  std::valarray<double> weights;
  double capacity;

  double mutation_rate, recombination_rate;
};
```

This problem describes a two-objectives knapsack problem.  We chose std::mt19937 as the random number
engine.  The genotype representation is a sequence of boolean values where each value indicates
whether an item is in the bag or not.  Our problem instance stores:

- `knapsack::values`: The values of each item for each objective.
- `knapsack::weights`:  The weight of each item.
- `knapsack::capacity`: The bag maximum capacity.
- `knapsack::mutation_rate`: The chance of each allele being flipped.
- `knapsack::recombination_rate`: The chance of crossover.

Since we want to maximize the value of the bag, and `ga::algorithm` minimizes the objective
function, `knapsack::evaluate` returns the arithmetic inverse of the total value.  If the bag
is storing more than it can carry, the result is 0 for all objectives.

`knapsack::mutate` flips every allele with chance `knapsack::mutation_rate`.  The utility
function `ga::draw(double rate, generator&)` returns `true` with chance `rate`.

`knapsack::recombine` applies uniform crossover with chance `knapsack::recombination_rate`.
Otherwise, it forwards the parents.

For the complete source code see [knapsack.cpp](https://github.com/verri/ga/blob/master/test/knapsack.cpp).

## Contributing

The library should be flexible and efficient enough to work in most of the common cases,
but if it doesn't fit into your problem, please let me know (or send me a pull request).
