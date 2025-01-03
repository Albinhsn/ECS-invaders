#include "common.h"
#include <windows.h>

#include "renderer_software.c"
#include "win32_platform.h"

static u16                     GlobalScreenWidth  = 800;
static u16                     GlobalScreenHeight = 600;
static s32                     GlobalShouldQuit   = 0;
static s64                     GlobalPerfCountFrequency;

static win32_software_renderer GlobalRenderer;

static void*                   Win32_Allocate(u64 size)
{
  // ToDo Align the memory?
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static void Win32_Create_Renderer(win32_software_renderer* Renderer, arena* GameMemory)
{
  void* Buffer = Arena_Allocate(GameMemory, sizeof(u32) * GlobalScreenWidth * GlobalScreenHeight);
  Software_Renderer_Create(&Renderer->Renderer, Buffer, GlobalScreenWidth, GlobalScreenHeight);
  Renderer->Info.bmiHeader.biSize        = sizeof(Renderer->Info.bmiHeader);
  Renderer->Info.bmiHeader.biWidth       = Renderer->Renderer.Width;
  Renderer->Info.bmiHeader.biHeight      = Renderer->Renderer.Height;
  Renderer->Info.bmiHeader.biPlanes      = 1;
  Renderer->Info.bmiHeader.biBitCount    = 32;
  Renderer->Info.bmiHeader.biCompression = BI_RGB;
}

void Win32_LoadGameCode(){

}

void Win32_RenderFramebuffer(HDC hdc)
{

  software_renderer Renderer = GlobalRenderer.Renderer;
  StretchDIBits(hdc, 0, 0, GlobalScreenWidth, GlobalScreenHeight, 0, 0, Renderer.Width, Renderer.Height, Renderer.Buffer, &GlobalRenderer.Info, DIB_RGB_COLORS, SRCCOPY);
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_DESTROY:
  {
    GlobalShouldQuit = 1;
    PostQuitMessage(0);
    return 0;
  }
  case WM_CLOSE:
  {
    DestroyWindow(hwnd);
    return 0;
  }

  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC         hdc = BeginPaint(hwnd, &ps);

    Win32_RenderFramebuffer(hdc);

    EndPaint(hwnd, &ps);
    return 0;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Win32_ProcessMessages()
{
  MSG Msg = {};
  while (PeekMessage(&Msg, 0, 0, 0, PM_REMOVE))
  {
    TranslateMessage(&Msg);
    DispatchMessage(&Msg);
  }
}

LARGE_INTEGER GetTimeInMilliseconds()
{
  LARGE_INTEGER CurrentTime;
  QueryPerformanceCounter(&CurrentTime);

  return CurrentTime;
}

u32 GetMillisecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End, s64 PerfCountFrequency)
{

  return (u32)(1000.0f * ((End.QuadPart - Start.QuadPart) / (f32)PerfCountFrequency));
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

  const char* WindowClassName = "Window Class";

  WNDCLASS    wc              = {};

  wc.lpfnWndProc              = WindowProc;
  wc.hInstance                = hInstance;
  wc.lpszClassName            = WindowClassName;

  RegisterClass(&wc);

  HWND hwnd = CreateWindowEx(0,                                                                   // Optional window styles
                             WindowClassName,                                                     // Window class
                             "Window Text",                                                       // Window text
                             WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,                                 // Window style, last part removes resizing
                             CW_USEDEFAULT, CW_USEDEFAULT, GlobalScreenWidth, GlobalScreenHeight, // Size and position
                             NULL,                                                                // Parent Window
                             NULL,                                                                // Menu
                             hInstance,                                                           // Instance handle
                             NULL                                                                 // Additional application data
  );

  if (hwnd == NULL)
  {
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);

  u64   GameMemorySize = Gigabyte(1);
  void* Memory         = Win32_Allocate(GameMemorySize);

  arena GameMemory     = {};
  Arena_Create(&GameMemory, Memory, GameMemorySize);

  Win32_Create_Renderer(&GlobalRenderer, &GameMemory);

  Software_Renderer_Clear(&GlobalRenderer.Renderer, 0xFF00FFFF);

  LARGE_INTEGER PerfCountFrequencyResult;
  QueryPerformanceFrequency(&PerfCountFrequencyResult);
  GlobalPerfCountFrequency        = PerfCountFrequencyResult.QuadPart;

  LARGE_INTEGER PreviousTimer     = GetTimeInMilliseconds();
  u32           TargetFrameTimeMS = 32;
  while (!GlobalShouldQuit)
  {
    Win32_ProcessMessages();
    Software_Renderer_Clear(&GlobalRenderer.Renderer, 0x00FF00FF);

    HDC hdc = GetDC(hwnd);
    Win32_RenderFramebuffer(hdc);
    ReleaseDC(hwnd, hdc);

    LARGE_INTEGER CurrentTimer = GetTimeInMilliseconds();
    u32           FrameTimeMS  = GetMillisecondsElapsed(PreviousTimer, CurrentTimer, GlobalPerfCountFrequency);
    if (FrameTimeMS < TargetFrameTimeMS)
    {
      u32 TimeToSleep = TargetFrameTimeMS - FrameTimeMS;
      Sleep(FrameTimeMS);
    }
  }

  return 0;
}
