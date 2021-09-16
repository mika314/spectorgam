#pragma once
#include <functional>

namespace sdl
{
  class Window;
}

class Rend
{
public:
  Rend(sdl::Window &);
  auto rend(std::vector<float> spectr, bool smartScale) -> void;

private:
  void *ctx;
  unsigned programId;
  unsigned pianoProgramId;
  unsigned spectrogramProgramId;
  int offset;
  int vertexPos3DLocation;
  unsigned vbo = 0;
  unsigned pianoVbo = 0;
  unsigned spectrogramVbo;
  unsigned ibo = 0;
  std::vector<float> vertexData;
  std::vector<float> spectrogramData;
  std::vector<unsigned> indexData;
  int line = 0;
};
