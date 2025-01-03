#include "common.h"
#include <windows.h>

#include "renderer_software.c"
#include "win32_platform.h"

static u16                     GlobalScreenWidth  = 800;
static u16                     GlobalScreenHeight = 600;
static s32                     GlobalShouldQuit   = 0;
static s64                     GlobalPerfCountFrequency;

static win32_software_renderer GlobalRenderer;

static const char*             GlobalTempPath    = "../build/lock.tmp";
static const char*             GlobalLibraryPath = "../build/invaders.dll";

static void*                   Win32_Allocate(u64 size)
{
  // ToDo Align the memory?
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
u32 Win32_FileHasChanged(u64* FileLastChangedTimer, const char* Filename)
{

  WIN32_FILE_ATTRIBUTE_DATA AttributeData;
  if (GetFileAttributesEx(Filename, GetFileExInfoStandard, &AttributeData))
  {
    FILETIME LastFileTimeChanged = {};
    LastFileTimeChanged          = AttributeData.ftLastWriteTime;
    long LastChanged             = LastFileTimeChanged.dwLowDateTime | (((uint64_t)AttributeData.ftLastWriteTime.dwHighDateTime) << 32);
    if (*FileLastChangedTimer != LastChanged)
    {
      *FileLastChangedTimer = LastChanged;
      return 1;
    }
  }
  return 0;
}

static void Win32_Create_Renderer(win32_software_renderer* Renderer, arena* GameMemory)
{
  void* Buffer = Arena_Allocate(GameMemory, sizeof(u32) * GlobalScreenWidth * GlobalScreenHeight);
  Software_Renderer_Create(&Renderer->Renderer, Buffer, GlobalScreenWidth, GlobalScreenHeight);
  Renderer->Info.bmiHeader.biSize        = sizeof(Renderer->Info.bmiHeader);
  Renderer->Info.bmiHeader.biWidth       = Renderer->Renderer.Width;
  Renderer->Info.bmiHeader.biHeight      = -Renderer->Renderer.Height;
  Renderer->Info.bmiHeader.biPlanes      = 1;
  Renderer->Info.bmiHeader.biBitCount    = 32;
  Renderer->Info.bmiHeader.biCompression = BI_RGB;
}

void* Win32_LibraryLoad(const char* LibraryName)
{
  return (void*)LoadLibraryA(LibraryName);
}
void Win32_LibraryFree(void* LibraryHandle)
{
  (void)FreeLibrary((HMODULE)LibraryHandle);
}
void* Win32_GetProcAddress(void* LibraryHandle, const char* ProcName)
{
  return (void*)GetProcAddress((HMODULE)LibraryHandle, ProcName);
}

void Win32_FreeGameCode(win32_game_code* GameCode)
{
  Win32_LibraryFree(GameCode->Library);
  GameCode->Library    = 0;
  GameCode->GameUpdate = 0;
}
void Win32_LoadGameCode(win32_game_code* GameCode)
{

  CopyFile(GlobalLibraryPath, GlobalTempPath, FALSE);
  void* GameCodeDLL    = Win32_LibraryLoad(GlobalTempPath);

  void* UpdateAddress  = Win32_GetProcAddress(GameCodeDLL, "GameUpdate");
  GameCode->Library    = GameCodeDLL;
  GameCode->GameUpdate = (game_update*)UpdateAddress;
}

void Win32_RenderFramebuffer(HWND hwnd)
{

  HDC               hdc      = GetDC(hwnd);
  software_renderer Renderer = GlobalRenderer.Renderer;
  StretchDIBits(hdc, 0, 0, GlobalScreenWidth, GlobalScreenHeight, 0, 0, Renderer.Width, Renderer.Height, Renderer.Buffer, &GlobalRenderer.Info, DIB_RGB_COLORS, SRCCOPY);
  ReleaseDC(hwnd, hdc);
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

    Win32_RenderFramebuffer(hwnd);

    EndPaint(hwnd, &ps);
    return 0;
  }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Win32_ProcessMessages(game_input* Input)
{
  MSG Msg = {};
  while (PeekMessage(&Msg, 0, 0, 0, PM_REMOVE))
  {
    switch (Msg.message)
    {
    case WM_KEYUP:
    case WM_KEYDOWN:
    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
    {
      WORD VKCode  = LOWORD(Msg.wParam);
      u32  WasDown = ((Msg.lParam & (1 << 30)) != 0);
      u32  IsDown  = ((Msg.lParam & (1 << 31)) == 0);
      switch (VKCode)
      {
      case VK_SPACE:
      {
        Input->Shoot = IsDown;
        break;
      }
      case 'W':
      {
        Input->Up = IsDown;
        break;
      }
      case 'A':
      {
        Input->Left = IsDown;
        break;
      }
      case 'S':
      {
        Input->Down = IsDown;
        break;
      }
      case 'D':
      {
        Input->Right = IsDown;
        break;
      }
      }
      break;
    }
    default:
    {

      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
      break;
    }
    }
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

  u64   GameMemorySize = Megabyte(205);
  void* Memory         = Win32_Allocate(GameMemorySize);

  arena GameArena      = {};
  Arena_Create(&GameArena, Memory, GameMemorySize);

  Win32_Create_Renderer(&GlobalRenderer, &GameArena);

  LARGE_INTEGER PerfCountFrequencyResult;
  QueryPerformanceFrequency(&PerfCountFrequencyResult);
  GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

  win32_game_code GameCode = {};
  Win32_LoadGameCode(&GameCode);
  u64 GameCodeLastChanged = 0;
  Win32_FileHasChanged(&GameCodeLastChanged, "../build/invaders.dll");

  pushbuffer Pushbuffer           = {};
  u64        PushbufferMemorySize = Megabyte(1);
  void*      PushbufferMemory     = Arena_Allocate(&GameArena, PushbufferMemorySize);
  Pushbuffer_Create(&Pushbuffer, PushbufferMemory, PushbufferMemorySize);

  game_memory GameMemory          = {};
  GameMemory.TemporarySize        = Megabyte(100);
  GameMemory.TemporaryStorage     = Arena_Allocate(&GameArena, GameMemory.TemporarySize);
  GameMemory.TransientStorageSize = Megabyte(100);
  GameMemory.TransientStorage     = Arena_Allocate(&GameArena, GameMemory.TransientStorageSize);

  game_input    GameInput         = {};

  LARGE_INTEGER PreviousTimer     = GetTimeInMilliseconds();
  u32           TargetFrameTimeMS = 33;
  while (!GlobalShouldQuit)
  {
    if (Win32_FileHasChanged(&GameCodeLastChanged, GlobalLibraryPath))
    {
      Win32_FreeGameCode(&GameCode);
      Win32_LoadGameCode(&GameCode);
    }

    Win32_ProcessMessages(&GameInput);
    Pushbuffer_Reset(&Pushbuffer);

    GameCode.GameUpdate(&GameMemory, &GameInput, &Pushbuffer);

    Software_Renderer_Render(&GlobalRenderer.Renderer, &Pushbuffer);
    Win32_RenderFramebuffer(hwnd);

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
