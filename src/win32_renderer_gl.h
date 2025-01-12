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

  gl_texture Textures[8];
  u32 TextureCount;

  shader Shaders[4];
  u32 ShaderCount;

  arena Arena;

  u32 VAOT;
  u32 VBOT;
  u32 EBOT;

  u32 VAOText;
  u32 VBOText;
  u32 EBOText;

  u32 VAOQ;
  u32 VBOQ;
  u32 EBOQ;

  u32 ScreenWidth;
  u32 ScreenHeight;
}win32_renderer_gl;

#endif