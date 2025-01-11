#include "common.h"
#include "platform.h"
#include "win32_renderer_software.h"
#include "renderer_software.c"

void Win32_Deallocate(void* Memory, u64 Size)
{
  VirtualFree(Memory, Size, MEM_DECOMMIT);
}

static void* Win32_Allocate(u64 size)
{
  // ToDo Align the memory?
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static void Win32_CreateRenderer(win32_software_renderer* Renderer, void * Buffer, u32 ScreenWidth, u32 ScreenHeight, HWND hwnd)
{
  Renderer->hwnd                         = hwnd;
  Renderer->Width                        = ScreenWidth;
  Renderer->Height                       = ScreenHeight;
  Renderer->Buffer                       = Buffer;
  Renderer->Info.bmiHeader.biSize        = sizeof(Renderer->Info.bmiHeader);
  Renderer->Info.bmiHeader.biWidth       = Renderer->Width;
  Renderer->Info.bmiHeader.biHeight      = -(s16)Renderer->Height;
  Renderer->Info.bmiHeader.biPlanes      = 1;
  Renderer->Info.bmiHeader.biBitCount    = 32;
  Renderer->Info.bmiHeader.biCompression = BI_RGB;
}

void Win32_RenderFramebuffer(win32_software_renderer * Renderer)
{

  HDC               hdc      = GetDC(Renderer->hwnd);
  StretchDIBits(hdc, 0, 0, Renderer->Width, Renderer->Height, 0, 0, Renderer->Width, Renderer->Height, Renderer->Buffer, &Renderer->Info, DIB_RGB_COLORS, SRCCOPY);
  ReleaseDC(Renderer->hwnd, hdc);
}

void BeginFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{
  // We don't really do anything here?
  Pushbuffer_Reset(Pushbuffer);
}

void EndFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{
  win32_software_renderer * Win32Renderer = (win32_software_renderer*)PlatformRenderer;

  software_renderer SoftwareRenderer = {};
  SoftwareRenderer.Width    = Win32Renderer->Width;
  SoftwareRenderer.Height   = Win32Renderer->Height;
  SoftwareRenderer.Buffer   = Win32Renderer->Buffer;
  Software_Renderer_Render(&SoftwareRenderer, Pushbuffer);
  Win32_RenderFramebuffer(Win32Renderer);
}

platform_renderer * CreateRenderer(u32 ScreenWidth, u32 ScreenHeight, HWND hwnd)
{
  u64 RendererMemorySize = sizeof(u32) * ScreenWidth * ScreenHeight;
  void * Memory = Win32_Allocate(RendererMemorySize + sizeof(win32_software_renderer));
  win32_software_renderer * Renderer = (win32_software_renderer*)Memory;
  Win32_CreateRenderer(Renderer, (void*)((u8*)Memory + sizeof(win32_software_renderer)), ScreenWidth, ScreenHeight, hwnd);

  return (platform_renderer*)Renderer;
}

void ReleaseRenderer(platform_renderer * PlatformRenderer)
{
  win32_software_renderer * Renderer = (win32_software_renderer*)PlatformRenderer;
  u64 RendererMemorySize = sizeof(u32) * Renderer->Width * Renderer->Height;
  Win32_Deallocate(PlatformRenderer, sizeof(win32_software_renderer) + RendererMemorySize);
}