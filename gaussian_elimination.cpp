#include "gaussian_elimination.hpp"
#include <cmath>

static void elimination(Matrix &eqns)
{
  // 'eqns' is the matrix, 'rows' is no. of vars
  int rows = eqns.size(), cols = eqns[0].size();

  for (int i = 0; i < rows - 1; i++)
  {
    int pivot = i;

    for (int j = i + 1; j < rows; j++)
    {
      if (std::abs(eqns[j][i]) > std::abs(eqns[pivot][i]))
        pivot = j;
    }

    if (eqns[pivot][i] == 0.0)
      continue; // But continuing to simplify the matrix as much as possible

    if (i != pivot)                    // Swapping the rows if new row with higher maxVals is found
      std::swap(eqns[pivot], eqns[i]); // C++ swap function

    for (int j = i + 1; j < rows; j++)
    {
      float scale = eqns[j][i] / eqns[i][i];

      for (int k = i + 1; k < cols; k++)  // k doesn't start at 0, since
        eqns[j][k] -= scale * eqns[i][k]; // values before from 0 to i
                                          // are already 0
      eqns[j][i] = 0.0;
    }
  }
}

static std::vector<float> backSubs(const Matrix &eqns)
{
  // 'eqns' is matrix, 'rows' is no. of variables
  int rows = eqns.size();

  std::vector<float> ans(rows);
  for (int i = rows - 1; i >= 0; i--)
  {
    float sum = 0.0;

    for (int j = i + 1; j < rows; j++)
      sum += eqns[i][j] * ans[j];

    if (eqns[i][i] != 0)
      ans[i] = (eqns[i][rows] - sum) / eqns[i][i];
    else
      return std::vector<float>(0);
  }
  return ans;
}

namespace GaussianElimination
{
  std::vector<float> solve(Matrix val)
  {
    elimination(val);
    return backSubs(val);
  }
} // namespace GaussianElimination
