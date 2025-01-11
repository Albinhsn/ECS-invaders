#include "win32_renderer_gl.h"
#include "pushbuffer.c"
#include "common.h"
#include "platform.h"


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

}

void EndFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{

}

platform_renderer * CreateRenderer(u32 ScreenWidth, u32 ScreenHeight, HWND hwnd)
{
  return 0;
}

void ReleaseRenderer(platform_renderer * PlatformRenderer)
{
}