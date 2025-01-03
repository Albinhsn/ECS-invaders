
#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

#include "renderer_software.h"
#include "platform.h"
#include "windows.h"

typedef struct win32_software_renderer
{
  software_renderer Renderer;
  BITMAPINFO Info;
} win32_software_renderer;

typedef struct win32_game_code{
  game_update * GameUpdate;
  void * Library;
}win32_game_code;

#endif
