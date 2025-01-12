#ifndef WIN32_RENDERER_OPENGL
#define WIN32_RENDERER_OPENGL


#include "renderer_software.h"
#include "windows.h"
#include "platform.h"

// Shader
// Textures

typedef struct shader
{
  u32 ID;
  string Name;
} shader;
typedef struct gl_texture
{
  u32 ID;
  string Name;
} gl_texture;

typedef struct win32_renderer_gl
{
  platform_renderer Header;
  HWND hwnd;
  HGLRC Context;
}win32_renderer_gl;

#endif