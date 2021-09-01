#pragma once

#include <vector>

using Matrix = std::vector<std::vector<float>>;

namespace GaussianElimination
{
  std::vector<float> solve(Matrix);
}
