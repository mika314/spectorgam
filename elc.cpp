#include <algorithm>
#include <cmath>
#include <vector>

auto elcK(float freq) -> float
{
  // Equal Loudness Curve on 60 dB
  // Frequency-Phon lookup table
  const std::vector<std::pair<float, float>> elcTable = {
    {20, 109},  {25, 104},  {31, 99},   {40, 94},    {50, 89},    {63, 85},    {80, 82},    {100, 78},
    {125, 75},  {160, 72},  {200, 69},  {250, 67},   {315, 65},   {400, 63},   {500, 62},   {630, 60},
    {800, 59},  {1000, 60}, {1250, 62}, {1600, 63},  {2000, 59},  {2500, 57},  {3150, 56},  {4000, 57},
    {5000, 60}, {6300, 66}, {8000, 71}, {10000, 73}, {12500, 68}, {16000, 68}, {20000, 104}};

  auto it = std::lower_bound(
    std::begin(elcTable), std::end(elcTable), freq, [](const auto &p, float f) { return p.first < f; });

  if (it == std::begin(elcTable))
    return std::pow(10.0f, (60 - elcTable.front().second) / 20.0f);
  else if (it == std::end(elcTable))
    return std::pow(10.0f, (60 - elcTable.back().second) / 20.0f);
  else
  {
    const auto &prevEntry = *(it - 1);
    const auto &nextEntry = *it;

    const auto prevFreq = prevEntry.first;
    const auto nextFreq = nextEntry.first;

    const auto prevPhon = prevEntry.second;
    const auto nextPhon = nextEntry.second;

    const auto alpha = (freq - prevFreq) / (nextFreq - prevFreq);
    const auto interpolatedPhon = (1.0f - alpha) * prevPhon + alpha * nextPhon;

    return std::pow(10.0f, (60 - interpolatedPhon) / 20.0f);
  }
}
