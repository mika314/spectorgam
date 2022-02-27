#include "poly_fit.hpp"
#include "gaussian_elimination.hpp"
#include <cmath>

std::vector<float> polyFit(std::size_t N, std::vector<float>::iterator b, std::vector<float>::iterator e)
{
  const auto sz = std::distance(b, e);
  Matrix m;
  for (auto i = 0U; i < N; ++i)
  {
    auto &row = m.emplace_back();
    for (auto j = 0U; j < N; ++j)
    {
      auto sum = 0.0;
      for (auto k = 0U; k < sz; ++k)
        sum += std::pow(k, j + i);
      row.push_back(sum);
    }
    {
      auto sum = 0.0;
      for (auto k = 0U; k < sz; ++k)
        sum += std::pow(k, i) * *(b + k);
      row.push_back(sum);
    }
  }
  return GaussianElimination::solve(m);
}

float calcPoly(const std::vector<float> &val, float x)
{
  auto ret = 0;
  auto pow = 1.0;
  for (auto i = 0U; i < val.size(); ++i)
  {
    ret += val[i] * pow;
    pow *= x;
  }
  return ret;
}
