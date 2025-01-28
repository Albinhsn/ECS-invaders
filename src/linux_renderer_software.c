#include "common.h"
#include "renderer_software.c"
#include "linux_platform.h"
#include "linux_renderer_software.h"

void Linux_Deallocate(void * Memory, u64 Size)
{
  munmap(Memory, Size);
}

void * Linux_Allocate(u64 Size)
{
  void * Result = mmap(0, Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  return Result;
}


void BeginFrame(platform_renderer * PlatformRenderer, pushbuffer* Pushbuffer)
{
  Pushbuffer_Reset(Pushbuffer);
}
void EndFrame(platform_renderer * PlatformRenderer, pushbuffer* Pushbuffer)
{
  linux_renderer_software * Renderer = (linux_renderer_software*)PlatformRenderer;

  software_renderer SoftwareRenderer = {};
  SoftwareRenderer.Width  = Renderer->Width;
  SoftwareRenderer.Height = Renderer->Height;
  SoftwareRenderer.Buffer = (u32*)Renderer->Image->data;

  Software_Renderer_Render(&SoftwareRenderer, Pushbuffer);
  XPutImage(Renderer->display, Renderer->Backbuffer, Renderer->Context, Renderer->Image, 0, 0, 0, 0, Renderer->Width, Renderer->Height);
  XCopyArea(Renderer->display, Renderer->Backbuffer, Renderer->window,
            DefaultGC(Renderer->display, Renderer->Screen),
            0, 0, Renderer->Width, Renderer->Height, 0, 0);
  XFlush(Renderer->display);
}

platform_renderer * CreateRenderer(u32 ScreenWidth, u32 ScreenHeight, void * Data)
{
  linux_display_and_window * DisplayAndWindow = (linux_display_and_window*)Data;

  u64 RendererMemorySize = sizeof(u32) * ScreenWidth * ScreenHeight;
  void * Memory = Linux_Allocate(RendererMemorySize + sizeof(linux_renderer_software));
  linux_renderer_software *Renderer = (linux_renderer_software*)Memory;

  Memory = (void*)((u8*)Memory + sizeof(linux_renderer_software));

  Renderer->window  = DisplayAndWindow->window;
  Renderer->display = DisplayAndWindow->display;
  Renderer->Screen = DisplayAndWindow->Screen;
  Renderer->Width = ScreenWidth;
  Renderer->Height = ScreenHeight;

  Renderer->Context = XCreateGC(Renderer->display, Renderer->window, 0, NULL);
  Renderer->Image = XCreateImage(Renderer->display, DefaultVisual(Renderer->display, Renderer->Screen),
                                 DefaultDepth(Renderer->display, Renderer->Screen), ZPixmap, 0, NULL,
                                 Renderer->Width, Renderer->Height, 32, 0);
  Renderer->Image->data = Memory;

  Renderer->Backbuffer = XCreatePixmap(Renderer->display, Renderer->window, ScreenWidth, ScreenHeight, DefaultDepth(Renderer->display, Renderer->Screen));

  return (platform_renderer*)Renderer;
}

void ReleaseRenderer(platform_renderer * PlatformRenderer)
{

  linux_renderer_software * Renderer = (linux_renderer_software*)PlatformRenderer;

  XFreeGC(Renderer->display, Renderer->Context);

  u64 RendererMemorySize = sizeof(u32) * Renderer->Width * Renderer->Height;
  Linux_Deallocate(Renderer, RendererMemorySize + sizeof(linux_renderer_software));
}
