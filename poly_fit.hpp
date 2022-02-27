#pragma once

#include <vector>

std::vector<float> polyFit(std::size_t N,
                           std::vector<float>::iterator b,
                           std::vector<float>::iterator e);
float calcPoly(const std::vector<float> &, float x);
