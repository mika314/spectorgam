#include "rend.hpp"
#include "consts.hpp"
#include <sdlpp/sdlpp.hpp>

Rend::Rend(sdl::Renderer &renderer) : r(renderer) {}
void Rend::rend(std::vector<float> spectr)
{
  r.get().setDrawColor(0x00, 0x00, 0x00, 0x00);
  r.get().clear();
  const auto StartFreq = 55;
  const auto EndFreq = 880;
  auto max = std::max(*std::max_element(std::begin(spectr) + StartFreq * SpectrSize / SampleFreq,
                                        std::begin(spectr) + EndFreq * SpectrSize / SampleFreq),
                      1.5E+6f);
  bool isWhite[] = {true, false, true, true, false, true, false, true, true, false, true, false};
  for (auto x = 0; x < Width; ++x)
  {
    r.get().setDrawColor(0x00, 0xff, 0x00, 0x00);
    const auto freq = StartFreq * expf(logf(EndFreq / StartFreq) / Width * x);
    const auto idx = static_cast<size_t>(freq * SpectrSize / SampleFreq);
    const auto y = Height - spectr[idx] * Height / max;
    r.get().drawLine(x, Height, x, y);
    if (isWhite[static_cast<int>(12 * log(freq / 27.5) / log(2) + .5) %
                (sizeof(isWhite) / sizeof(*isWhite))])
      r.get().setDrawColor(0xff, 0xff, 0xff, 0x00);
    else
      r.get().setDrawColor(0x00, 0x00, 0x00, 0x00);
    r.get().drawLine(x, 0, x, std::min(y, Height / 2.f));
  }
  r.get().setDrawColor(0x00, 0x00, 0x00, 0x00);
  const auto k = powf(2.f, 1.f / 12.f);
  for (auto freq = 1.f * StartFreq * powf(2.f, 1.f / 24.f); freq <= EndFreq; freq *= k)
  {
    const auto x = logf(freq / StartFreq) / logf(EndFreq / StartFreq) * Width;
    r.get().drawLine(x, Height, x, 0);
  }
  r.get().present();
}
