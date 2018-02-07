#include "ga/algorithm.hpp"

class problem
{
public:
  using individual_type = int;
  using generator_type = std::mt19937;
  using fitness_type = double;

  auto evaluate(const std::vector<int>& new_individuals,
                std::vector<ga::solution<int, double>>&, std::size_t,
                std::back_insert_iterator<std::vector<double>> fit_out,
                generator_type&) const -> void
  {
    for (const auto individual : new_individuals)
      *fit_out++ = static_cast<double>(individual);
  }

  auto mutate(int& x, generator_type&) const -> void { x <<= 1; }

  auto recombine(int a, int b, generator_type&) const -> std::array<int, 2u>
  {
    return {{a ^ b, b + b}};
  };
};

static auto assert_throw(bool assertion, const char* msg) -> void
{
  if (!assertion)
    throw std::runtime_error{msg};
}

int main()
{
  auto population = std::vector<int>();
  population.resize(10);
  std::iota(population.begin(), population.end(), 0);

  bool failed = false;
  try
  {
    ga::make_algorithm(problem{}, std::vector<int>{}, 1u, std::mt19937{});
  }
  catch (const std::invalid_argument&)
  {
    failed = true;
  }

  assert_throw(failed, "wrong elitism check");

  auto model = ga::make_algorithm(problem{}, std::move(population), 1u, std::mt19937{17});

  assert_throw(model.elite_count() == 1u, "wrong internal state");
  assert_throw(model.population().size() == 10, "wrong population size");

  for (auto t = 0u; t < 3; ++t)
  {
    model.iterate();
    assert_throw(model.population().size() == 10, "wrong population size");
  }
}
