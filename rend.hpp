#pragma once
#include <functional>
#include <vector>

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
  unsigned spectrogramPid;
  unsigned lowerPianoPid;
  unsigned upperPianoPid;
  unsigned rollingSpectrogramPid;
  int offset;
  int vertexPos3DLocation;
  unsigned vbo = 0;
  unsigned lowerPianoVbo = 0;
  unsigned upperPianoVbo = 0;
  unsigned spectrogramVbo;
  unsigned ibo = 0;
  std::vector<float> vertexData;
  std::vector<float> spectrogramData;
  std::vector<unsigned> indexData;
  int line = 0;
};
