#include "common.h"
#include <windows.h>

static s32   ScreenWidth  = 800;
static s32   ScreenHeight = 600;
static s32   ShouldQuit   = 0;

static void* Win32_Allocate(u64 size)
{
  // Align the memory?
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_DESTROY:
  {
    ShouldQuit = 1;
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

    FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
    EndPaint(hwnd, &ps);
    return 0;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

  const char* WindowClassName = "indow Class";

  WNDCLASS    wc              = {};

  wc.lpfnWndProc              = WindowProc;
  wc.hInstance                = hInstance;
  wc.lpszClassName            = WindowClassName;

  RegisterClass(&wc);

  HWND hwnd = CreateWindowEx(0,                                                       // Optional window styles
                             WindowClassName,                                         // Window class
                             "Window Text",                                           // Window text
                             WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,                     // Window style, last part removes resizing
                             CW_USEDEFAULT, CW_USEDEFAULT, ScreenWidth, ScreenHeight, // Size and position
                             NULL,                                                    // Parent Window
                             NULL,                                                    // Menu
                             hInstance,                                               // Instance handle
                             NULL                                                     // Additional application data
  );

  if (hwnd == NULL)
  {
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);

  // Allocate some amount of memory :)
  void* Memory = Win32_Allocate(Gigabyte(1));

  while (!ShouldQuit)
  {
    MSG Msg = {};
    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
    }
  }

  return 0;
}
