#ifndef WIN32_RENDERER_D3D
#define WIN32_RENDERER_D3D

#include "windows.h"
#include "platform.h"
#include <d3d11.h> // D3D interface 
#include <dxgi.h>  // DirectX driver interface
#include <d3dcompiler.h> // Shader Compiler

typedef struct shader
{
  ID3D11VertexShader * Vertex;
  ID3D11PixelShader * Pixel;
  string Name;
} shader;

typedef struct d3d_texture
{
  u32 ID;
  string Name;
} d3d_texture;

typedef struct win32_renderer_d3d
{
  platform_renderer Platform;
  HWND hwnd;
  ID3D11Device *Device;
  ID3D11DeviceContext* Context;
  IDXGISwapChain *SwapChain;
  ID3D11RenderTargetView* RenderTargetView;
  arena Arena;

  d3d_texture Textures[8];
  u32         TextureCount;

  shader Shaders[4];
  u32 ShaderCount;

  ID3D11Buffer * TextureBuffer, TextBuffer, QuadBuffer;

  u32 ScreenWidth, ScreenHeight;

}win32_renderer_d3d;


#endif
