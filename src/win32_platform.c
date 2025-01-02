#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>
#include <stdio.h>

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
  case WM_DESTROY:
  {
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

    const wchar_t WindowClassName[]  = L"Sample Window Class";

  WNDCLASS    wc              = {};

  wc.lpfnWndProc              = WindowProc;
  wc.hInstance                = hInstance;
  wc.lpszClassName            = WindowClassName;

  RegisterClass(&wc);

  HWND hwnd                   = CreateWindowEx(0,                                                          // Optional window styles
                                               WindowClassName,                                            // Window class
                                               L"Window Text",                                              // Window text
                                               WS_OVERLAPPEDWINDOW,                                        // Window style
                                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Size and position
                                               NULL,                                                       // Parent Window
                                               NULL,                                                       // Menu
                                               hInstance,                                                  // Instance handle
                                               NULL                                                        // Additional application data
                    );

  if (hwnd == NULL)
  {
    return 0;
  }

  ShowWindow(hwnd, nCmdShow);

  MSG msg = {};
  while (GetMessage(&msg, NULL, 0, 0) > 0)
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}
