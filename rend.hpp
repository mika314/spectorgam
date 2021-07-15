#pragma once
#include <functional>

namespace sdl
{
  class Renderer;
}

class Rend
{
public:
  Rend(sdl::Renderer &);
  void rend(std::vector<float> spectr);

private:
  std::reference_wrapper<sdl::Renderer> r;
};
