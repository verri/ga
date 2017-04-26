#ifndef SPEA2_UTILITIES_HPP
#define SPEA2_UTILITIES_HPP

#include <ga/meta.hpp>

#include <algorithm>
#include <array>
#include <vector>

#include <boost/range/irange.hpp>

namespace ga
{
namespace util
{

template <
  typename Integer1, typename Integer2,
  typename = meta::requires<std::is_integral<Integer1>, std::is_integral<Integer2>>>
auto range(Integer1 first, Integer2 last)
{
  return boost::irange<std::common_type_t<Integer1, Integer2>>(first, last);
}

template <typename... Ts, typename = std::enable_if_t<(sizeof...(Ts) > 0ul)>>
auto indexes_of(const Ts&... args)
{
  const auto sizes = std::array<std::size_t, sizeof...(Ts)>{{args.size()...}};

  if (!std::all_of(begin(sizes), end(sizes),
                   [size = sizes[0]](const auto& s) { return s == size; }))
    throw std::runtime_error{"indexes_of: container sizes mismatch"};

  return range(0u, sizes[0]);
}

} // namespace util
} // namespace ga

#endif // SPEA2_UTILITIES_HPP
