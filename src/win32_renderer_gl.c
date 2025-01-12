#include "win32_renderer_gl.h"
#include "pushbuffer.c"
#include "common.h"
#include "wingdi.h"
#include "opengl.c"
#include <gl/gl.h>
#include <gl/glext.h>


void* Win32_GLGetProcAddress(const char* ProcName)
{
  return (void*)wglGetProcAddress((LPCSTR)ProcName);
}

void Win32_Render(win32_renderer_gl * Renderer, pushbuffer* Pushbuffer)
{
  while(Pushbuffer->ReadOffset < Pushbuffer->AllocatedOffset)
  {
    pushbuffer_entry_type EntryType = Pushbuffer_ReadEntryType(Pushbuffer);
    switch(EntryType)
    {
      case Pushbuffer_Entry_Clear:
      {
        pushbuffer_entry_clear Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_clear);
        u32 Color = Entry.Color;
        f32 A     = ((Color >> 24) & 255) / 255.0f;
        f32 R     = ((Color >> 16) & 255) / 255.0f;
        f32 G     = ((Color >> 8) & 255) / 255.0f;
        f32 B     = ((Color >> 0) & 255) / 255.0f;
        glClearColor(R, G, B, A);
        break;
      }
      case Pushbuffer_Entry_Text:
      {
        pushbuffer_entry_text Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_text);
        Entry.Size = 5;
        break;
      }
      case Pushbuffer_Entry_Rect_Color:
      {
        pushbuffer_entry_rect_color Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_color);
        Entry.Color = 5;
        break;
      }
      case Pushbuffer_Entry_Rect_Texture:
      {
        pushbuffer_entry_rect_texture Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_texture);
        Entry.Texture  = 0;
        break;
      }
    }
  }
}

void Win32_Deallocate(void* Memory, u64 Size)
{
  VirtualFree(Memory, Size, MEM_DECOMMIT);
}

static void* Win32_Allocate(u64 size)
{
  // ToDo Align the memory?
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}


void BeginFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{
  Pushbuffer_Reset(Pushbuffer);
  glClear(GL_COLOR_BUFFER_BIT);
}

void EndFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{
  win32_renderer_gl * Renderer = (win32_renderer_gl*)PlatformRenderer;
  Win32_Render(Renderer, Pushbuffer);

  HDC hdc = GetDC(Renderer->hwnd);
  SwapBuffers(hdc);
  ReleaseDC(Renderer->hwnd, hdc);
}

platform_renderer * CreateRenderer(u32 ScreenWidth, u32 ScreenHeight, HWND hwnd)
{
  HDC hdc = GetDC(hwnd);

  PIXELFORMATDESCRIPTOR Format  ={};
  Format.nSize = sizeof(Format);
  Format.nVersion = 1;
  Format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  Format.iPixelType = PFD_TYPE_RGBA;
  Format.cColorBits = 24;
  Format.cDepthBits = 32;
  Format.iLayerType = PFD_MAIN_PLANE;

  int PixelFormat = ChoosePixelFormat(hdc, &Format);
  SetPixelFormat(hdc, PixelFormat, &Format);

  win32_renderer_gl *Renderer = Win32_Allocate(sizeof(win32_renderer_gl));
  Renderer->hwnd    = hwnd;
  Renderer->Context = wglCreateContext(hdc);


  wglMakeCurrent(hdc, Renderer->Context);

  #if 0
  s32 MajorVersion, MinorVersion;
  glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);
  #endif

  GL_LoadExtensions(Win32_GLGetProcAddress);

  // Load shaders!
  

  ReleaseDC(hwnd, hdc);

  return (platform_renderer*)Renderer;
}

void ReleaseRenderer(platform_renderer * PlatformRenderer)
{
  win32_renderer_gl * Renderer = (win32_renderer_gl*)PlatformRenderer;
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(Renderer->Context);
  DestroyWindow(Renderer->hwnd);
}