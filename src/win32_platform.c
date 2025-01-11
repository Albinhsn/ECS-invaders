#include "common.h"
#include "math.h"
#include <windows.h>

#include "image.c"
#include "pushbuffer.c"
#include "win32_platform.h"
#include "sound.c"

static u16                     GlobalScreenWidth       = 600;
static u16                     GlobalScreenHeight      = 800;
static s32                     GlobalShouldQuit        = 0;
static u16                     GlobalFramerateTargetMS = 30;
static s64                     GlobalPerfCountFrequency;



static const char*             GlobalTempPath    = "../build/lock.tmp";
static const char*             GlobalRenderCodeTempPath    = "../build/lock1.tmp";
static const char*             GlobalGameCodePath = "../build/invaders.dll";

#if RENDERER_SOFTWARE
static const char*             GlobalRenderCodePath = "../build/win32_renderer_gl.dll";
#elif RENDERER_GL
static const char*             GlobalRenderCodePath = "../build/win32_renderer_gl.dll";
#endif
static win32_thread            GlobalAudioThread;
static win32_audio             GlobalAudio;

void Win32_Deallocate(void* Memory, long Size)
{
  VirtualFree(Memory, Size, MEM_DECOMMIT);
}

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

void Win32_FreeRenderCode(win32_render_code * RenderCode, platform_renderer * PlatformRenderer){
  RenderCode->Release(PlatformRenderer);
}

void Win32_LoadRenderCode(win32_render_code * RenderCode)
{
  CopyFile(GlobalRenderCodePath, GlobalRenderCodeTempPath, FALSE);
  void* RenderCodeDLL    = Win32_LibraryLoad(GlobalRenderCodeTempPath);
  RenderCode->Library    = RenderCodeDLL;

  void* Address          = Win32_GetProcAddress(RenderCodeDLL, "BeginFrame");
  RenderCode->BeginFrame = (renderer_begin_frame*)Address;

  Address                = Win32_GetProcAddress(RenderCodeDLL, "EndFrame");
  RenderCode->EndFrame   = (renderer_end_frame*)Address;

  Address                = Win32_GetProcAddress(RenderCodeDLL, "CreateRenderer");
  RenderCode->Create     = (renderer_create*)Address;

  Address                = Win32_GetProcAddress(RenderCodeDLL, "ReleaseRenderer");
  RenderCode->Release    = (renderer_release*)Address;
}

void Win32_LoadGameCode(win32_game_code* GameCode)
{

  CopyFile(GlobalGameCodePath, GlobalTempPath, FALSE);
  void* GameCodeDLL    = Win32_LibraryLoad(GlobalTempPath);

  void* UpdateAddress  = Win32_GetProcAddress(GameCodeDLL, "GameUpdate");
  GameCode->Library    = GameCodeDLL;
  GameCode->GameUpdate = (game_update*)UpdateAddress;

  UpdateAddress  = Win32_GetProcAddress(GameCodeDLL, "GameGetSoundSamples");
  GameCode->GameGetSoundSamples = (game_get_sound_samples*)UpdateAddress;
}



static LRESULT CALLBACK Win32_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

    // Win32_RenderFramebuffer(hwnd);

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

LARGE_INTEGER Win32_GetTimeInSeconds()
{
  LARGE_INTEGER CurrentTime;
  QueryPerformanceCounter(&CurrentTime);

  return CurrentTime;
}
f32 Win32_GetMillisecondsElapsedF(LARGE_INTEGER Start, LARGE_INTEGER End, s64 PerfCountFrequency)
{

  return 1000.0f * ((End.QuadPart - Start.QuadPart) / (f32)PerfCountFrequency);
}

u32 Win32_GetMillisecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End, s64 PerfCountFrequency)
{

  return (u32)Win32_GetMillisecondsElapsedF(Start, End, PerfCountFrequency);
}

bool Win32_ReadFile(arena* Arena, const char* Filename, u8** FileBuffer, u32* Size)
{

  DWORD  dwBytesRead = 0;

  HANDLE hFile       = CreateFile(Filename,              //
                                  GENERIC_READ,          //
                                  FILE_SHARE_READ,       //
                                  NULL,                  //
                                  OPEN_EXISTING,         //
                                  FILE_ATTRIBUTE_NORMAL, //
                                  NULL                   //
        );

  if (hFile == INVALID_HANDLE_VALUE)
  {
    return false;
  }

  u32        FileSize          = GetFileSize(hFile, NULL);
  u8*        Buffer            = (u8*)Arena_Allocate(Arena, FileSize + 1);

  DWORD      NumberOfBytesRead = 0;
  OVERLAPPED ol                = {};

  s32        Result            = ReadFile(hFile, Buffer, FileSize, &NumberOfBytesRead, &ol);
  DWORD      Error             = GetLastError();
  if (Result == false || FileSize != NumberOfBytesRead)
  {
    Arena_Deallocate(Arena, FileSize + 1);
    return false;
  }
  Buffer[FileSize] = '\0';
  *FileBuffer      = Buffer;
  *Size            = FileSize;

  CloseHandle(hFile);

  return true;
}

bool Win32_InitAudio(arena * Arena)
{
  HRESULT Result;

  // Init COM library
  Result = CoInitializeEx(0, COINIT_MULTITHREADED);
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to init COM\n");
    return false;
  }

  // Get the default audio endpoint
  Result = CoCreateInstance(&CLSID_MMDeviceEnumerator,      // The CLSID associated with the data and code that will be used to create the object
                            0,                              // Pointer to the aggregate object's IUnknown Interface
                            CLSCTX_ALL,                     // Context in which the code that manages the newly created object will run in (CLSCTX enum)
                            &IID_IMMDeviceEnumerator,       //  Reference to the identifier of the interface used to communicate with the object
                            (void**)&GlobalAudio.Enumerator // Address of the pointer variable that receives the interface pointer requested in riid (value above). Is 0 if it failed to create it
  );

  if (FAILED(Result))
  {
    OutputDebugStringA("CoCreateInstance failed\n");
    return false;
  }

  Result = GlobalAudio.Enumerator->lpVtbl->GetDefaultAudioEndpoint(GlobalAudio.Enumerator, // explicit "this"
                                                                   eRender,                // The data flow direction for  rendering device (otherwise capture and eCapture)
                                                                   eConsole,               // role of the endpoint device, either eConsole, eMultimedai and eCommunications
                                                                   &GlobalAudio.Device     // Pointer to address of the IMMDevice interface of the endpoint object
  );
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to get default audio endpoint\n");
    return false;
  }

  Result = GlobalAudio.Device->lpVtbl->Activate(GlobalAudio.Device, &IID_IAudioClient, CLSCTX_ALL, 0, (void**)&GlobalAudio.AudioClient);
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to create audio client\n");
    return false;
  }

  WAVEFORMATEX * WaveFormat = 0;
  Result = GlobalAudio.AudioClient->lpVtbl->GetMixFormat(GlobalAudio.AudioClient,
                                                         &WaveFormat // Pointer to the pointer to the waveformat which it will fill
  );
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to get wave format\n");
    return false;
  }
  // Extensible
  GlobalAudio.WaveFormat = (WAVEFORMATEXTENSIBLE*)WaveFormat;
  Assert(GlobalAudio.WaveFormat->Format.wFormatTag == 0xFFFE); // Is extended format
  Assert(GlobalAudio.WaveFormat->SubFormat.Data1 == 3); // Is floating point
  Assert(GlobalAudio.WaveFormat->Format.nChannels == 2);

  // Initialize audio stream
  REFERENCE_TIME BufferDuration   = GlobalFramerateTargetMS * 10000; // 33 ms? expressed in 100-nanosecond units.
  u32            AudioClientFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST;
  Result                          = GlobalAudio.AudioClient->lpVtbl->Initialize(GlobalAudio.AudioClient,
                                                                                AUDCLNT_SHAREMODE_SHARED, // Shared mode or exclusive
                                                                                AudioClientFlags,         //  Flags to control creation of the stream.
                                                                                BufferDuration,           // Duration of the buffer
                                                                                0,                        // Should always be 0 in shared mode, otherwise always nonzero
                                                                                (WAVEFORMATEX*)GlobalAudio.WaveFormat,   // Pointer to the wave format
                                                                                0 // Pointer to a session GUID, setting this to NULL is equivalent to passing ap ointer to a GUID_NULL
                           );
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to initialize audio client\n");
    return false;
  }



  Result = GlobalAudio.AudioClient->lpVtbl->GetService(GlobalAudio.AudioClient, &IID_IAudioRenderClient, (void**)&GlobalAudio.RenderClient);
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to get render client\n");
    return false;
  }

  // Get the buffer frame count
  Result = GlobalAudio.AudioClient->lpVtbl->GetBufferSize(GlobalAudio.AudioClient, &GlobalAudio.BufferFrameCount);
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to get buffer size\n!");
    return false;
  }
  Assert(GlobalFramerateTargetMS == (u32)(1000.0f * GlobalAudio.BufferFrameCount / GlobalAudio.WaveFormat->Format.nSamplesPerSec));

  GlobalAudio.RefillEvent = CreateEventEx(0,                               // pointer to SECURITY_ATTRIBUTES structure
                                          0,                               // Name of the event object, if NULL object is created without a name
                                          0,                               // includes CREATE_EVENT_INITIAL_SET and CREATE_EVENT_MANUAL_RESET
                                          EVENT_MODIFY_STATE | SYNCHRONIZE // Access mask for the event object
  );
  Result                  = GlobalAudio.AudioClient->lpVtbl->SetEventHandle(GlobalAudio.AudioClient, GlobalAudio.RefillEvent);
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to set event handle\n");
    return false;
  }

  // Set the data buffer to silent first
  // ToDo Does this actually do something?
  u8* Data = 0;
  Result   = GlobalAudio.RenderClient->lpVtbl->GetBuffer(GlobalAudio.RenderClient, GlobalAudio.BufferFrameCount, &Data);
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to get the buffer!\n");
    return false;
  }

  Result = GlobalAudio.RenderClient->lpVtbl->ReleaseBuffer(GlobalAudio.RenderClient, GlobalAudio.BufferFrameCount, AUDCLNT_BUFFERFLAGS_SILENT);
  if (FAILED(Result))
  {
    OutputDebugStringA("Failed to release the buffer!\n");
    return false;
  }

  game_audio * GameAudio = &GlobalAudio.GameAudio;
  GameAudio->Channels = GlobalAudio.WaveFormat->Format.nChannels;

  u32 NumberOfBuffers                 = 2;
  // Always write one frame worth of buffer
  // This is wrong?
  u32 SampleFramesToWrite                  = GlobalAudio.BufferFrameCount;
  GameAudio->SampleFrameCount              = SampleFramesToWrite * NumberOfBuffers;
  GameAudio->SampleFrameIndexAudioThread   = 0;
  GameAudio->SampleFrameIndexGameCode      = 0;
  GameAudio->Buffer = (f32*)Arena_Allocate(Arena, sizeof(f32) * GameAudio->SampleFrameCount * GameAudio->Channels);
  GlobalAudio.CanStartThread = false;
  return true;
}


#include <math.h>
#define TAU (PI * 2)
void Win32_OutputSineWave(u32 SamplesPerSecond, u32 SampleCount, f32 * Samples, f32 ToneHz, f32 ToneVolume)
{
  static f64 TSine = 0;


  f32 WavePeriod = SamplesPerSecond / ToneHz;

  for(u32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++)
  {
    f32 Sine = (f32)sin(TSine);
    f32 Sample = (f32)(Sine * ToneVolume);
    *Samples++ = Sample;
    *Samples++ = Sample;

    TSine += TAU / WavePeriod;
    if(TSine >= TAU){
      TSine -= TAU;
    }

  }

}

sound GlobalSound = {};

void Win32_OutputTestSound(u32 SampleCount, f32 * Samples, f32 Volume)
{
  static u32 SoundSampleIndex = 0;

  for(u32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++)
  {
    f32 Sample0 = GlobalSound.Samples[SoundSampleIndex++];
    *Samples++ = Sample0 * Volume;

    f32 Sample1 = GlobalSound.Samples[SoundSampleIndex++];
    *Samples++ = Sample1 * Volume;
    SoundSampleIndex = (SoundSampleIndex) % (GlobalSound.SampleFrameCount * GlobalSound.Channels);
  }
}

DWORD Win32_AudioThread_Main(void* Data)
{

  u64 FramesWritten = 0;
  HRESULT Result;


  game_audio * GameAudio = &GlobalAudio.GameAudio;

  while(!GlobalAudio.CanStartThread)
  {
  }

  char          Buf[1024] = {};
  sprintf_s(Buf, ArrayCount(Buf), "Audio Thread Starts at %d\n", GlobalAudio.GameAudio.SampleFrameIndexAudioThread);
  OutputDebugStringA(Buf);
  LARGE_INTEGER Current = Win32_GetTimeInSeconds();
  bool HadFirst         = false;
  GlobalAudio.AudioClient->lpVtbl->Start(GlobalAudio.AudioClient);
  while (true)
  {
    DWORD GotEvent = WaitForSingleObject(GlobalAudio.RefillEvent, INFINITE);

    // Wait for event
    if (GotEvent == WAIT_OBJECT_0)
    {
      if(!HadFirst)
      {
        Current = Win32_GetTimeInSeconds();
        HadFirst = true;
      }
      // ToDo check for device change with guids!

      // ToDo rename?
      u32 BufferSpaceAvailable = 0;
      Result = GlobalAudio.AudioClient->lpVtbl->GetCurrentPadding(GlobalAudio.AudioClient, &BufferSpaceAvailable);
      if(Result == AUDCLNT_E_DEVICE_INVALIDATED){
        // ToDo Handle invalid device
      }

      u32 SampleCount = GlobalAudio.BufferFrameCount - BufferSpaceAvailable;
      if(SampleCount > 0){
        u8 *BufferData = 0;
        Result = GlobalAudio.RenderClient->lpVtbl->GetBuffer(GlobalAudio.RenderClient, SampleCount, &BufferData);

        f32 * Samples = (f32*)BufferData;

        Assert(GameAudio->Channels == 2);


        char          Buf[1024] = {};
        sprintf_s(Buf, ArrayCount(Buf), "Audio Thread Starting %d\n", GameAudio->SampleFrameIndexAudioThread);
        OutputDebugStringA(Buf);

        for(u32 SampleToWriteIndex = 0; SampleToWriteIndex < SampleCount; SampleToWriteIndex++)
        {
          *Samples++ = GameAudio->Buffer[GameAudio->SampleFrameIndexAudioThread * 2 + 0];
          *Samples++ = GameAudio->Buffer[GameAudio->SampleFrameIndexAudioThread * 2 + 1];
          GameAudio->SampleFrameIndexAudioThread++;
          GameAudio->SampleFrameIndexAudioThread %= GameAudio->SampleFrameCount;
        }
        LARGE_INTEGER End = Win32_GetTimeInSeconds();
        char          Buf2[1024] = {};
        sprintf_s(Buf2, ArrayCount(Buf2), "Audio Thread Ending %d, timer: %f\n", GameAudio->SampleFrameIndexAudioThread, Win32_GetMillisecondsElapsedF(Current, End, GlobalPerfCountFrequency));
        OutputDebugStringA(Buf2);
        Current = End;


        Result = GlobalAudio.RenderClient->lpVtbl->ReleaseBuffer(GlobalAudio.RenderClient, SampleCount, 0);

      }
    }
  }

  return 0;
}

void Win32_UninitAudio()
{
  // This has to be called from the same thread that created it
  GlobalAudio.RenderClient->lpVtbl->Release(GlobalAudio.RenderClient);
  CoTaskMemFree(GlobalAudio.WaveFormat);
  GlobalAudio.AudioClient->lpVtbl->Release(GlobalAudio.AudioClient);
  GlobalAudio.Device->lpVtbl->Release(GlobalAudio.Device);
  GlobalAudio.Enumerator->lpVtbl->Release(GlobalAudio.Enumerator);

  CoUninitialize();
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{




  const char* WindowClassName = "Window Class";

  WNDCLASS    wc              = {};

  wc.lpfnWndProc              = Win32_WindowProc;
  wc.hInstance                = hInstance;
  wc.lpszClassName            = WindowClassName;

  RegisterClass(&wc);


  // Calculate the client area
  RECT rect = {0, 0, GlobalScreenWidth, GlobalScreenHeight};
  AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, false);

  u32 WindowWidth = rect.right - rect.left;
  u32 WindowHeight = rect.bottom - rect.top;

  HWND hwnd = CreateWindowEx(0,                                                                   // Optional window styles
                             WindowClassName,                                                     // Window class
                             "Window Text",                                                       // Window text
                             WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,                                 // Window style, last part removes resizing
                             CW_USEDEFAULT, CW_USEDEFAULT, WindowWidth, WindowHeight, // Size and position
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

   win32_game_code GameCode = {};
  Win32_LoadGameCode(&GameCode);
  u64 GameCodeLastChanged = 0;
  Win32_FileHasChanged(&GameCodeLastChanged, "../build/invaders.dll");

  win32_render_code RenderCode = {};
  Win32_LoadRenderCode(&RenderCode);
  platform_renderer * PlatformRenderer = RenderCode.Create(GlobalScreenWidth, GlobalScreenHeight, hwnd);

  LARGE_INTEGER PerfCountFrequencyResult;
  QueryPerformanceFrequency(&PerfCountFrequencyResult);
  GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

  bool InitializedAudio = Win32_InitAudio(&GameArena);
  if (!InitializedAudio)
  {
    OutputDebugStringA("Failed to init Audio!\n");
  }else
  {
      GlobalAudioThread.Handle = CreateThread(0, 0, Win32_AudioThread_Main, 0, 0, &GlobalAudioThread.Id);
      if (GlobalAudioThread.Handle == 0)
      {
        OutputDebugStringA("Failed to create audio thread!\n");
      }else
      {
        SetThreadPriority(GlobalAudioThread.Handle, THREAD_PRIORITY_TIME_CRITICAL);
      }
  }




  pushbuffer Pushbuffer           = {};
  u64        PushbufferMemorySize = Megabyte(1);
  void*      PushbufferMemory     = Arena_Allocate(&GameArena, PushbufferMemorySize);
  Pushbuffer_Create(&Pushbuffer, PushbufferMemory, PushbufferMemorySize);

  game_memory GameMemory          = {};
  GameMemory.PermanentSize        = Megabyte(100);
  GameMemory.PermanentStorage     = Arena_Allocate(&GameArena, GameMemory.PermanentSize);
  GameMemory.TemporaryStorageSize = Megabyte(100);
  GameMemory.TemporaryStorage     = Arena_Allocate(&GameArena, GameMemory.TemporaryStorageSize);

  game_input    GameInput         = {};

  LARGE_INTEGER PreviousTimer     = Win32_GetTimeInSeconds();


  GameMemory.ReadFile             = Win32_ReadFile;
  f32 DeltaTime                   = GlobalFramerateTargetMS / 1000.0f;

  UINT   DesiredSchedulerMS = 1;
  bool SleepIsGranular    = timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR;
  f32 SleepError = 0;

  while (!GlobalShouldQuit)
  {
    RenderCode.BeginFrame(PlatformRenderer, &Pushbuffer);
    if (Win32_FileHasChanged(&GameCodeLastChanged, GlobalGameCodePath))
    {
      Win32_FreeGameCode(&GameCode);
      Win32_LoadGameCode(&GameCode);
    }

    Win32_ProcessMessages(&GameInput);
    if(GlobalShouldQuit)
    {
      break;
    }

    // Purely for debugging
    DeltaTime = Min(DeltaTime, 1.0f / GlobalFramerateTargetMS);

    GameMemory.DeltaTime    = DeltaTime;
    GameMemory.ScreenWidth  = GlobalScreenWidth;
    GameMemory.ScreenHeight = GlobalScreenHeight;
    GameCode.GameUpdate(&GameMemory, &GameInput, &Pushbuffer);
    if(GameCode.GameGetSoundSamples)
    {
      u32 SampleFramesToWrite           = GlobalAudio.BufferFrameCount;
      GameCode.GameGetSoundSamples(&GameMemory, &GlobalAudio.GameAudio, SampleFramesToWrite);
    }


    RenderCode.EndFrame(PlatformRenderer, &Pushbuffer);

    LARGE_INTEGER CurrentTimer       = Win32_GetTimeInSeconds();

    f32           FrameMinusSleep = Win32_GetMillisecondsElapsedF(PreviousTimer, CurrentTimer, GlobalPerfCountFrequency);;

    if (FrameMinusSleep < GlobalFramerateTargetMS)
    {
      f32 TargetSleepTime = GlobalFramerateTargetMS - SleepError;
      f32 TimeToSleep = TargetSleepTime - FrameMinusSleep;
      DeltaTime       = 1.0f / GlobalFramerateTargetMS;
      if(SleepIsGranular)
      {
        Sleep((u32)TimeToSleep);
      }
      f32 FrameTimeMSF       = Win32_GetMillisecondsElapsedF(PreviousTimer, CurrentTimer, GlobalPerfCountFrequency);
      while(FrameTimeMSF < TargetSleepTime)
      {
        FrameTimeMSF = Win32_GetMillisecondsElapsedF(PreviousTimer, Win32_GetTimeInSeconds(), GlobalPerfCountFrequency);
      }
      f32 Error = SleepError;
      SleepError = FrameTimeMSF - TargetSleepTime;
      char          FrameTimeBuf[1024] = {};
      sprintf_s(FrameTimeBuf, ArrayCount(FrameTimeBuf), "Slept for: %.4f, prev error %f, Frame: %f, Sleep: %f, target: %f\n", FrameTimeMSF, Error, FrameMinusSleep, TimeToSleep, TargetSleepTime);
      OutputDebugStringA(FrameTimeBuf);
    }
    else
    {
      Assert(0 && "Missed Frame!");
    }


    CurrentTimer  = Win32_GetTimeInSeconds();
    PreviousTimer = CurrentTimer;
    char          FrameTimeBuf[1024] = {};
    sprintf_s(FrameTimeBuf, ArrayCount(FrameTimeBuf), "Frame: %.4f\n", FrameMinusSleep);
    OutputDebugStringA(FrameTimeBuf);

    GlobalAudio.CanStartThread = true;

  }

  bool Result = TerminateThread(GlobalAudioThread.Handle, 0);
  if (!Result)
  {
    OutputDebugStringA("Failed to kill thread? GL\n");
  }
  Win32_UninitAudio();
  Win32_FreeGameCode(&GameCode);
  Win32_FreeRenderCode(&RenderCode, PlatformRenderer);

  return 0;
}
