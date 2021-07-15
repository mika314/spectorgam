#include <algorithm>
#include <fftw3.h>
#include <log/log.hpp>
#include <mutex>
#include <sdlpp/sdlpp.hpp>
#include <vector>

static const auto Width = 1280;
static const auto Height = 720 / 2;

fftw_plan plan;
std::vector<int16_t> rawInput;
fftw_complex *input;
fftw_complex *output;
std::vector<float> spectr;
const auto SpectrSize = 8 * 4096;
size_t pos = 0;
std::mutex mutex;

int main()
{
  sdl::Init init(SDL_INIT_EVERYTHING);
  sdl::Window w("Spectrogram", 64, 126, Width, Height, SDL_WINDOW_SHOWN);
  sdl::Renderer r(w.get(), -1, 0);
  sdl::EventHandler e;
  auto done = false;
  e.quit = [&done](const SDL_QuitEvent &) { done = true; };

  const auto SampleFreq = 48000;

  auto want = []() {
    SDL_AudioSpec want;
    want.freq = SampleFreq;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 1024;
    return want;
  }();
  SDL_AudioSpec captureHave;
  rawInput.resize(SpectrSize);
  input = fftw_alloc_complex(SpectrSize);
  output = fftw_alloc_complex(SpectrSize);
  memset(input, 0, SpectrSize * sizeof(fftw_complex));
  memset(output, 0, SpectrSize * sizeof(fftw_complex));
  plan = fftw_plan_dft_1d(SpectrSize, input, output, FFTW_FORWARD, FFTW_MEASURE);

  const auto fps = 30;
  sdl::Audio capture(nullptr, true, &want, &captureHave, 0, [](Uint8 *stream, int len) {
    for (auto i = 0U; i < len / sizeof(int16_t); ++i)
    {
      rawInput[pos] = reinterpret_cast<int16_t *>(stream)[i];
      input[pos++][1] = 0;
      if (pos >= rawInput.size())
        pos = 0;
    }

    for (auto i = 0U; i < SpectrSize; ++i)
    {
      input[i][0] = expf(-0.00025f * (SpectrSize - i)) *  rawInput[(pos + i) % rawInput.size()];
      input[i][1] = 0;
    }

    fftw_execute(plan);
    std::lock_guard<std::mutex> lock(mutex);
    spectr.clear();
    for (auto j = 0U; j < SpectrSize / 2; ++j)
      spectr.push_back(sqrt(output[j][0] * output[j][0] + output[j][1] * output[j][1]));
  });
  capture.pause(false);
  while (!done)
  {
    const auto t1 = SDL_GetTicks();
    while (e.poll()) {}
    r.setDrawColor(0x00, 0x00, 0x00, 0x00);
    r.clear();

    bool isWhite[] = {true, false, true, true, false, true, false, true, true, false, true, false};
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (!spectr.empty())
      {
        const auto StartFreq = 55;
        const auto EndFreq = 880;
        auto max =
          std::max(*std::max_element(std::begin(spectr) + StartFreq * SpectrSize / SampleFreq,
                                     std::begin(spectr) + EndFreq * SpectrSize / SampleFreq),
                   1.5E+6f);
        for (auto x = 0; x < Width; ++x)
        {
          r.setDrawColor(0x00, 0xff, 0x00, 0x00);
          const auto freq = StartFreq * expf(logf(EndFreq / StartFreq) / Width * x);
          const auto idx = static_cast<size_t>(freq * SpectrSize / SampleFreq);
          const auto y = Height - spectr[idx] * Height / max;
          r.drawLine(x, Height, x, y);
          if (isWhite[static_cast<int>(12 * log(freq / 27.5) / log(2) + .5) %
                      (sizeof(isWhite) / sizeof(*isWhite))])
            r.setDrawColor(0xff, 0xff, 0xff, 0x00);
          else
            r.setDrawColor(0x00, 0x00, 0x00, 0x00);
          r.drawLine(x, 0, x, std::min(y, Height / 2.f));
        }
        r.setDrawColor(0x00, 0x00, 0x00, 0x00);
        const auto k = powf(2.f, 1.f / 12.f);
        for (auto freq = 1.f * StartFreq * powf(2.f, 1.f / 24.f); freq <= EndFreq; freq *= k)
        {
          const auto x = logf(freq / StartFreq) / logf(EndFreq / StartFreq) * Width;
          r.drawLine(x, Height, x, 0);
        }
      }
    }
    r.present();
    const auto t2 = SDL_GetTicks();
    if (1000 / fps > t2 - t1)
      SDL_Delay(1000 / fps - (t2 - t1));
  }
  capture.pause(true);

  fftw_destroy_plan(plan);
  fftw_free(input);
  fftw_free(output);
}
