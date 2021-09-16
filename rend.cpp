#include "rend.hpp"
#include "consts.hpp"
#include "poly_fit.hpp"
#include <log/log.hpp>

#include <sdlpp/sdlpp.hpp>

#include <GL/glew.h>

#include <SDL_opengl.h>

#include <GL/glu.h>

const auto LinesNum = 10 * 30;
const auto Strade = EndFreq * SpectrSize / SampleFreq;

static auto printProgramLog(GLuint program) -> void
{
  // Make sure name is shader
  if (glIsProgram(program))
  {
    // Program log length
    int infoLogLength = 0;
    int maxLength = infoLogLength;

    // Get info string length
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    // Allocate string
    char *infoLog = new char[maxLength];

    // Get info log
    glGetProgramInfoLog(program, maxLength, &infoLogLength, infoLog);
    if (infoLogLength > 0)
    {
      // Print Log
      printf("%s\n", infoLog);
    }

    // Deallocate string
    delete[] infoLog;
  }
  else
  {
    printf("Name %d is not a program\n", program);
  }
}

static auto printShaderLog(GLuint shader) -> void
{
  // Make sure name is shader
  if (glIsShader(shader))
  {
    // Shader log length
    int infoLogLength = 0;
    int maxLength = infoLogLength;

    // Get info string length
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    // Allocate string
    char *infoLog = new char[maxLength];

    // Get info log
    glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
    if (infoLogLength > 0)
    {
      // Print Log
      printf("%s\n", infoLog);
    }

    // Deallocate string
    delete[] infoLog;
  }
  else
  {
    printf("Name %d is not a shader\n", shader);
  }
}

#define ERROR_CHECK() errorCheckImpl(__LINE__)

static auto errorCheckImpl(int line) -> void
{
  auto err = glGetError();
  if (err != GL_NO_ERROR)
  {
    std::cerr << line << " " << gluErrorString(err) << std::endl;
  }
}

static auto makeProgram(const char *vertexShaderSource, const char *fragmentShaderSource) -> unsigned
{
  // setting up Open GL
  auto programId = glCreateProgram();
  ERROR_CHECK();
  auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
  ERROR_CHECK();
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  ERROR_CHECK();
  glCompileShader(vertexShader);
  ERROR_CHECK();
  GLint vShaderCompiled = GL_FALSE;
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
  ERROR_CHECK();
  if (vShaderCompiled != GL_TRUE)
  {
    printf("Unable to compile vertex shader %d!\n", vertexShader);
    printShaderLog(vertexShader);
    throw -2;
  }
  glAttachShader(programId, vertexShader);
  ERROR_CHECK();
  auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  ERROR_CHECK();
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  ERROR_CHECK();
  glCompileShader(fragmentShader);
  ERROR_CHECK();
  GLint fShaderCompiled = GL_FALSE;
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
  ERROR_CHECK();
  if (fShaderCompiled != GL_TRUE)
  {
    printf("Unable to compile fragment shader %d!\n", fragmentShader);
    printShaderLog(fragmentShader);
    throw -3;
  }
  glAttachShader(programId, fragmentShader);
  ERROR_CHECK();
  glLinkProgram(programId);
  ERROR_CHECK();
  GLint programSuccess = GL_TRUE;
  glGetProgramiv(programId, GL_LINK_STATUS, &programSuccess);
  ERROR_CHECK();
  if (programSuccess != GL_TRUE)
  {
    printf("Error linking program %d!\n", programId);
    printProgramLog(programId);
    throw -4;
  }
  return programId;
}

Rend::Rend(sdl::Window &window) : ctx(SDL_GL_CreateContext(window.get()))
{
  if (!ctx)
  {
    LOG("SDL could not get GL context. SDL Error:", SDL_GetError());
    throw -2;
  }
  glewExperimental = GL_TRUE;
  auto glewError = glewInit();
  if (glewError != GLEW_OK)
  {
    LOG("Error initilizing GLEW.", glewGetErrorString(glewError));
    throw -3;
  }

  programId = makeProgram(R"(
    #version 140

    in vec3 LVertexPos3D;

    out vec4 color;
    void main()
    {
      float StartFreq = 55;
      float EndFreq = 2 * 880;
      float freq = LVertexPos3D.x;
      float x = log(freq / StartFreq) / log(EndFreq / StartFreq) * 2 - 1;
      gl_Position = vec4(x, LVertexPos3D.y / 4 - 0.75, 0, 1);

      float fNote = log(freq / 55 * 2) / log(2.0) * 12.0 + 0.5;
      int note = int(fNote);
      fNote = abs((fNote - note - 0.5) * 2);
      float v = LVertexPos3D.z;
      vec4 c;
      switch (note % 12)
      {
        case 3: c = vec4(1, 0, 0, 1); break;
        case 10: c = vec4(1, 0.5, 0, 1); break;
        case 5: c = vec4(1, 1, 0, 1); break;
        case 0: c = vec4(0.5, 1, 0, 1); break;
        case 7: c = vec4(0, 1, 0, 1); break;
        case 2: c = vec4(0, 1, 0.5, 1); break;
        case 9: c = vec4(0, 1, 1, 1); break;
        case 4: c = vec4(0, 0.5, 1, 1); break;
        case 11: c = vec4(0, 0, 1, 1); break;
        case 6: c = vec4(0.5, 0, 1, 1); break;
        case 1: c = vec4(1, 0, 1, 1); break;
        case 8: c = vec4(1, 0, 0.5, 1); break;
      }
      color = vec4(c.r * v * (1 - fNote) + v * fNote, c.g * v * (1 - fNote) + v * fNote, c.b * v * (1 - fNote) + v * fNote, c.a);
    }
  )",
                          R"(
    #version 140
    in vec4 color;
    out vec4 LFragment;
    void main()
    {
      LFragment = color;
    }
  )");

  spectrogramProgramId = makeProgram(R"(
    #version 140

    in vec3 LVertexPos3D;
    uniform float offset;
    float LinesNum = 10 * 30;

    out vec4 color;
    void main()
    {
      float StartFreq = 55;
      float EndFreq = 2 * 880;
      float freq = LVertexPos3D.x;
      float x = log(freq / StartFreq) / log(EndFreq / StartFreq) * 2 - 1;
      float y = (LVertexPos3D.y - offset) * 2 * 0.75 - 0.5;

      float fNote = log(freq / 55 * 2) / log(2.0) * 12.0 + 0.5;
      int note = int(fNote);
      fNote = abs((fNote - note - 0.5) * 2);
      float v = pow(LVertexPos3D.z, 2 - (freq - StartFreq) / (EndFreq - StartFreq));
      vec4 c;
      switch (note % 12)
      {
        case 3: c = vec4(1, 0, 0, 1); break;
        case 10: c = vec4(1, 0.5, 0, 1); break;
        case 5: c = vec4(1, 1, 0, 1); break;
        case 0: c = vec4(0.5, 1, 0, 1); break;
        case 7: c = vec4(0, 1, 0, 1); break;
        case 2: c = vec4(0, 1, 0.5, 1); break;
        case 9: c = vec4(0, 1, 1, 1); break;
        case 4: c = vec4(0, 0.5, 1, 1); break;
        case 11: c = vec4(0, 0, 1, 1); break;
        case 6: c = vec4(0.5, 0, 1, 1); break;
        case 1: c = vec4(1, 0, 1, 1); break;
        case 8: c = vec4(1, 0, 0.5, 1); break;
      }
      color = vec4(c.r * v * (1 - fNote) + v * fNote, c.g * v * (1 - fNote) + v * fNote, c.b * v * (1 - fNote) + v * fNote, c.a);

      if (y < -0.5)
        y += 2 * 0.75;

      if (y < -0.5 + 2.0 / LinesNum && y >= -0.5 || y > 1.0 - 2.0 / LinesNum)
        color = vec4(0, 0, 0, 0);

      gl_Position = vec4(x, y, 0, 1);
    }
  )",
                                     R"(
    #version 140
    in vec4 color;
    out vec4 LFragment;
    void main()
    {
      LFragment = color;
    }
  )");

  pianoProgramId = makeProgram(R"(
    #version 140

    in vec3 LVertexPos3D;

    out vec4 color;
    void main()
    {
      float StartFreq = 55;
      float EndFreq = 2 * 880;
      float freq = LVertexPos3D.x;
      float x = log(freq / StartFreq) / log(EndFreq / StartFreq) * 2 - 1;
      gl_Position = vec4(x, LVertexPos3D.y / 4 - 0.75, 0, 1);
      color = vec4(LVertexPos3D.z, LVertexPos3D.z, LVertexPos3D.z, 1.0);
    }
  )",
                               R"(
    #version 140
    in vec4 color;
    out vec4 LFragment;
    void main()
    {
      LFragment = color;
    }
  )");

  // Get vertex attribute location
  vertexPos3DLocation = glGetAttribLocation(programId, "LVertexPos3D");
  ERROR_CHECK();
  if (vertexPos3DLocation == -1)
  {
    printf("LVertexPos3D is not a valid glsl program variable!\n");
    throw -5;
  }

  offset = glGetUniformLocation(spectrogramProgramId, "offset");
  ERROR_CHECK();
  if (offset == -1)
  {
    LOG("Offset is not a valid glsl program variable!");
  }

  // Initialize clear color
  glClearColor(0.f, 0.f, 0.f, 1.f);
  ERROR_CHECK();

  // Create VBO
  glGenBuffers(1, &vbo);
  ERROR_CHECK();

  // Create VBO
  glGenBuffers(1, &pianoVbo);
  ERROR_CHECK();

  glBindBuffer(GL_ARRAY_BUFFER, pianoVbo);
  ERROR_CHECK();
  bool isWhite[] = {true, false, true, true, false, true, false, true, true, false, true, false};
  std::vector<float> pianoData;
  for (int i = 0; i < 49 + 12; ++i)
  {
    const auto freq1 = 55 * powf(2, (i - .46f) / 12.f);
    pianoData.push_back(freq1);
    pianoData.push_back(0);
    pianoData.push_back(isWhite[i % 12] ? 1 : 0);
    pianoData.push_back(freq1);
    pianoData.push_back(1);
    pianoData.push_back(isWhite[i % 12] ? 1 : 0);

    const auto freq2 = 55 * powf(2, (i + .46f) / 12.f);
    pianoData.push_back(freq2);
    pianoData.push_back(0);
    pianoData.push_back(isWhite[i % 12] ? 1 : 0);
    pianoData.push_back(freq2);
    pianoData.push_back(1);
    pianoData.push_back(isWhite[i % 12] ? 1 : 0);

    const auto freq3 = 55 * powf(2, (i + .50f) / 12.f);
    pianoData.push_back(freq3);
    pianoData.push_back(0);
    pianoData.push_back(0);
    pianoData.push_back(freq3);
    pianoData.push_back(1);
    pianoData.push_back(0);
  }
  glBufferData(GL_ARRAY_BUFFER, pianoData.size() * sizeof(GLfloat), pianoData.data(), GL_STATIC_DRAW);
  ERROR_CHECK();

  glGenBuffers(1, &spectrogramVbo);
  ERROR_CHECK();
  glBindBuffer(GL_ARRAY_BUFFER, spectrogramVbo);
  ERROR_CHECK();
  for (int j = 0; j < LinesNum; ++j)
    for (int i = 0; i < Strade; ++i)
    {
      const auto freq = 1.f * i * SampleFreq / SpectrSize;
      spectrogramData.push_back(freq);
      spectrogramData.push_back(1.f * j / LinesNum);
      spectrogramData.push_back(0);
      spectrogramData.push_back(freq);
      //      spectrogramData.push_back(1.f * (j + 1) / LinesNum);
      spectrogramData.push_back(1.f * (j + 1) / LinesNum);
      spectrogramData.push_back(0);
    }

  // Create IBO
  glGenBuffers(1, &ibo);
  ERROR_CHECK();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  ERROR_CHECK();

  for (int j = 0; j < LinesNum; ++j)
    for (int i = 0; i < Strade - 2; ++i)
    {
      indexData.push_back(2 * j * Strade + i * 2 + 0);
      indexData.push_back(2 * j * Strade + i * 2 + 3);
      indexData.push_back(2 * j * Strade + i * 2 + 1);
      indexData.push_back(2 * j * Strade + i * 2 + 0);
      indexData.push_back(2 * j * Strade + i * 2 + 2);
      indexData.push_back(2 * j * Strade + i * 2 + 3);
    }

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexData.size() * sizeof(indexData[0]), indexData.data(), GL_STATIC_DRAW);
  ERROR_CHECK();
}

void Rend::rend(std::vector<float> spectr, bool smartScale)
{
  glClear(GL_COLOR_BUFFER_BIT);
  ERROR_CHECK();

  glEnable(GL_BLEND);
  ERROR_CHECK();
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  ERROR_CHECK();

  glUseProgram(pianoProgramId);
  ERROR_CHECK();
  glBindBuffer(GL_ARRAY_BUFFER, pianoVbo);
  ERROR_CHECK();
  glVertexAttribPointer(vertexPos3DLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
  ERROR_CHECK();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  ERROR_CHECK();
  glEnableVertexAttribArray(vertexPos3DLocation);
  ERROR_CHECK();
  glDrawElements(GL_TRIANGLES, 3 * (49 + 12) * 6, GL_UNSIGNED_INT, NULL);
  ERROR_CHECK();
  glDisableVertexAttribArray(vertexPos3DLocation);
  ERROR_CHECK();

  glUseProgram(programId);
  ERROR_CHECK();

  // VBO data
  vertexData.clear();

  // Set vertex data
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  ERROR_CHECK();
  glVertexAttribPointer(vertexPos3DLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
  ERROR_CHECK();

  const auto startIdx = StartFreq * SpectrSize / SampleFreq;
  const auto endIdx = EndFreq * SpectrSize / SampleFreq;

  std::vector<float> poly;
  auto max = 0.0f;
  auto max2 = 0.0f;
  if (smartScale)
  {
    const auto c = polyFit(4, std::begin(spectr) + startIdx, std::begin(spectr) + endIdx);
    for (int i = startIdx; i < endIdx; ++i)
    {
      auto tmp2 = calcPoly(c, i - startIdx);
      poly.push_back(tmp2);
      {
        if (max2 < tmp2)
          max2 = tmp2;
      }
    }

    for (int i = startIdx; i < endIdx; ++i)
    {
      auto tmp2 = poly[i - startIdx];
      const auto tmp = spectr[i] / std::max(0.01f, (tmp2 + 0.5f * max2));
      if (max < tmp)
        max = tmp;
    }
  }
  else
  {
    max = std::max(
      *std::max_element(std::begin(spectr) + StartFreq * SpectrSize / SampleFreq, std::begin(spectr) + EndFreq * SpectrSize / SampleFreq), 1.5E+6f);
    max2 = 0;
    for (int i = startIdx; i < endIdx; ++i)
      poly.push_back(1);
  }

  auto i = 0;

  for (auto a : spectr)
  {
    const auto freq = 1.f * i * SampleFreq / SpectrSize;
    const auto tmp2 = (i >= startIdx && i < endIdx) ? poly[i - startIdx] : 1.0f;
    const auto y = a / std::max(0.01f, (tmp2 + 0.5f * max2)) / max;

    vertexData.push_back(freq);
    vertexData.push_back(-1.f);

    vertexData.push_back(y);

    vertexData.push_back(freq);
    vertexData.push_back(y * 2.f - 1.f);
    vertexData.push_back(y);

    if (i < Strade)
    {
      spectrogramData[line * Strade * 6 + i * 6 + 2] = y;
      spectrogramData[line * Strade * 6 + i * 6 + 5] = y;
    }

    ++i;
  }
  line = (line + LinesNum - 1) % LinesNum;

  glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);
  ERROR_CHECK();

  // Enable vertex position
  glEnableVertexAttribArray(vertexPos3DLocation);
  ERROR_CHECK();

  // Set index data and render
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  ERROR_CHECK();
  glDrawElements(GL_TRIANGLES, (spectr.size() - 1) * 6, GL_UNSIGNED_INT, NULL);
  ERROR_CHECK();

  // Disable vertex position
  glDisableVertexAttribArray(vertexPos3DLocation);
  ERROR_CHECK();

  glUseProgram(spectrogramProgramId);
  ERROR_CHECK();

  if (offset >= 0)
  {
    glUniform1f(offset, 1.f * line / LinesNum);
    ERROR_CHECK();
  }

  glBindBuffer(GL_ARRAY_BUFFER, spectrogramVbo);
  ERROR_CHECK();
  glVertexAttribPointer(vertexPos3DLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), NULL);
  ERROR_CHECK();

  glBufferData(GL_ARRAY_BUFFER, spectrogramData.size() * sizeof(GLfloat), spectrogramData.data(), GL_STATIC_DRAW);
  ERROR_CHECK();

  // Enable vertex position
  glEnableVertexAttribArray(vertexPos3DLocation);
  ERROR_CHECK();

  // Set index data and render
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
  ERROR_CHECK();
  glDrawElements(GL_TRIANGLES, indexData.size(), GL_UNSIGNED_INT, NULL);
  ERROR_CHECK();

  // Disable vertex position
  glDisableVertexAttribArray(vertexPos3DLocation);
  ERROR_CHECK();

  // Unbind program
  glUseProgram(NULL);
  ERROR_CHECK();
}
