#include "consts.hpp"
#include "rend.hpp"
#include <algorithm>
#include <fftw3.h>
#include <log/log.hpp>
#include <mutex>
#include <vector>

#include <sdlpp/sdlpp.hpp>

#include <GL/glew.h>

#include <SDL_opengl.h>

#include <GL/glu.h>

fftw_plan plan;
fftw_complex *input;
fftw_complex *output;
std::vector<float> spectr;
std::mutex mutex;

int main()
{
  sdl::Init init(SDL_INIT_EVERYTHING);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  sdl::Window w("Spectrogram",
                1920,
                2160,
                Width,
                Height,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_BORDERLESS);
  Rend rend(w);

  sdl::EventHandler e;
  auto done = false;
  e.quit = [&done](const SDL_QuitEvent &) { done = true; };

  auto want = []() {
    SDL_AudioSpec want;
    want.freq = SampleFreq;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = 1024;
    return want;
  }();
  SDL_AudioSpec captureHave;
  input = fftw_alloc_complex(SpectrSize);
  output = fftw_alloc_complex(SpectrSize);
  memset(input, 0, SpectrSize * sizeof(fftw_complex));
  memset(output, 0, SpectrSize * sizeof(fftw_complex));
  plan = fftw_plan_dft_1d(SpectrSize, input, output, FFTW_FORWARD, FFTW_MEASURE);

  const auto fps = 30;
  auto capture = sdl::Audio{
    nullptr, true, &want, &captureHave, 0, [](Uint8 *stream, int len) {
      static size_t pos = 0;
      static std::vector<int16_t> rawInput;
      rawInput.resize(SpectrSize);
      for (auto i = 0U; i < len / sizeof(int16_t); ++i)
      {
        rawInput[pos] = reinterpret_cast<int16_t *>(stream)[i];
        input[pos++][1] = 0;
        if (pos >= rawInput.size())
          pos = 0;
      }

      for (auto i = 0U; i < SpectrSize; ++i)
      {
        input[i][0] = expf(-0.00025f * (SpectrSize - i)) * rawInput[(pos + i) % rawInput.size()];
        input[i][1] = 0;
      }

      fftw_execute(plan);
      std::lock_guard<std::mutex> lock(mutex);
      spectr.clear();
      for (auto j = 0U; j < SpectrSize / 2; ++j)
        spectr.push_back(sqrt(output[j][0] * output[j][0] + output[j][1] * output[j][1]));
    }};
  capture.pause(false);
  float playFreq = 440;
  float playVol = 0.f;
  SDL_AudioSpec have;
  auto audio =
    sdl::Audio{nullptr, false, &want, &have, 0, [&playFreq, &playVol](Uint8 *stream, int len) {
                 static double pos = 0;
                 static float vol = 0;
                 for (auto i = 0U; i < len / sizeof(int16_t); ++i)
                 {
                   if (playVol > vol)
                     vol = playVol;
                   else
                     vol = vol - 0.001 * (vol - playVol);
                   pos += 1.f * playFreq * 2 * 3.1415926 / SampleFreq;
                   reinterpret_cast<int16_t *>(stream)[i] =
                     static_cast<int16_t>(vol * 0x8000 * (sin(pos) > 0 ? 1 : -1));
                 }
               }};
  audio.pause(false);
  e.mouseMotion = [&playVol, &playFreq](const SDL_MouseMotionEvent &e) {
    if (playVol > 0)
      playVol = expf(-0.0045f * e.y);
    playFreq = StartFreq * expf(1.f * e.x / Width * logf(1.f * EndFreq / StartFreq));
  };
  e.mouseButtonDown = [&playVol, &playFreq](const SDL_MouseButtonEvent &e) {
    playVol = expf(-0.0045f * e.y);
    playFreq = StartFreq * expf(1.f * e.x / Width * logf(1.f * EndFreq / StartFreq));
  };
  e.mouseButtonUp = [&playVol](const SDL_MouseButtonEvent &) { playVol = 0.f; };

  SDL_Keycode lastKey;
  e.keyDown = [&playVol, &playFreq, &lastKey](const SDL_KeyboardEvent &e) {
    int note = -1;
    lastKey = e.keysym.sym;
    switch (e.keysym.sym)
    {
    case SDLK_q: note = 0; break;
    case SDLK_2: note = 1; break;
    case SDLK_w: note = 2; break;
    case SDLK_3: note = 3; break;
    case SDLK_e: note = 4; break;
    case SDLK_r: note = 5; break;
    case SDLK_5: note = 6; break;
    case SDLK_t: note = 7; break;
    case SDLK_6: note = 8; break;
    case SDLK_y: note = 9; break;
    case SDLK_7: note = 10; break;
    case SDLK_u: note = 11; break;
    case SDLK_i: note = 12 + 0; break;
    case SDLK_9: note = 12 + 1; break;
    case SDLK_o: note = 12 + 2; break;
    case SDLK_0: note = 12 + 3; break;
    case SDLK_p: note = 12 + 4; break;

    case SDLK_z: note = 12 + 0; break;
    case SDLK_s: note = 12 + 1; break;
    case SDLK_x: note = 12 + 2; break;
    case SDLK_d: note = 12 + 3; break;
    case SDLK_c: note = 12 + 4; break;
    case SDLK_v: note = 12 + 5; break;
    case SDLK_g: note = 12 + 6; break;
    case SDLK_b: note = 12 + 7; break;
    case SDLK_h: note = 12 + 8; break;
    case SDLK_n: note = 12 + 9; break;
    case SDLK_j: note = 12 + 10; break;
    case SDLK_m: note = 12 + 11; break;
    case SDLK_COMMA: note = 24 + 0; break;
    case SDLK_l: note = 24 + 1; break;
    case SDLK_PERIOD: note = 24 + 2; break;
    case SDLK_SEMICOLON: note = 24 + 3; break;
    case SDLK_SLASH: note = 24 + 4; break;
    }
    if (note >= 0)
    {
      playVol = 0.05f;
      playFreq = 220 * powf(2, (note + 3) / 12.f);
    }
  };
  e.keyUp = [&playVol, &lastKey](const SDL_KeyboardEvent &e) {
    if (e.keysym.sym == lastKey)
      playVol = 0.0f;
  };

  while (!done)
  {
    const auto t1 = SDL_GetTicks();
    while (e.poll()) {}
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (!spectr.empty())
      {
        rend.rend(std::move(spectr));
        w.glSwap();
      }
    }
    const auto t2 = SDL_GetTicks();
    if (1000 / fps > t2 - t1)
      SDL_Delay(1000 / fps - (t2 - t1));
  }
  capture.pause(true);

  fftw_destroy_plan(plan);
  fftw_free(input);
  fftw_free(output);
}
