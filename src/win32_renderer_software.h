#ifndef WIN32_SOFTWARE_RENDERER
#define WIN32_SOFTWARE_RENDERER

#include "renderer_software.h"
#include "windows.h"

typedef struct win32_software_renderer
{
  platform_renderer Header;
  HWND hwnd;
  BITMAPINFO  Info;
  u32*     Buffer;
  u32      Width;
  u32      Height;
} win32_software_renderer;

#endif