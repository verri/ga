// Copyright (c) 2017-2018 Filipe Verri <filipeverri@gmail.com>

#ifndef GA_SOLUTION_HPP
#define GA_SOLUTION_HPP

namespace ga
{

template <typename Individual, typename Fitness> struct solution
{
  Individual x;
  Fitness fitness;
};

} // namespace ga

#endif // GA_SOLUTION_HPP
