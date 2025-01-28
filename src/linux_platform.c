#include "linux_platform.h"
#include "pushbuffer.c"
#include "common.h"


static u16 GlobalScreenWidth  = 600;
static u16 GlobalScreenHeight = 800;
static u16 GlobalFramerateTargetMS = 16;
static bool GlobalRunning = true;


#if RENDERER_SOFTWARE
static const char * GlobalRenderCodePath      = "../build/linux_renderer_software.so";
#elif RENDERER_GL
static const char * GlobalRenderCodePath      = "../build/linux_renderer_gl.so";
#endif

static const char * GlobalTempRenderCodePath  = "../build/lock_renderer.tmp";
static const char * GlobalGameCodePath        = "../build/invaders.so";
static const char * GlobalTempGameCodePath    = "../build/lock_game.tmp";
Atom GlobalDeleteAtom;

/*
  ToDo:
  * Platform
    * ALSA  (audio)
*/


pthread_t Linux_CreateThread(void * Procedure)
{
  pthread_t Thread = 0;
  pthread_create(&Thread, 0, Procedure, 0);
  return Thread;
}
struct timespec Linux_GetTimeInSeconds()
{
  struct timespec Result = {};
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &Result);

  return Result;
}

f32 Linux_GetMillisecondsElapsedF(struct timespec PreviousTimer, struct timespec CurrentTimer)
{
  
  struct timespec Diff;
  if ((CurrentTimer.tv_nsec - PreviousTimer.tv_nsec)<0) {
      Diff.tv_sec  = CurrentTimer.tv_sec - PreviousTimer.tv_sec-1;
      Diff.tv_nsec = 1000000000+CurrentTimer.tv_nsec - PreviousTimer.tv_nsec;
  } else {
      Diff.tv_sec  = CurrentTimer.tv_sec - PreviousTimer.tv_sec;
      Diff.tv_nsec = CurrentTimer.tv_nsec - PreviousTimer.tv_nsec;
  }
    return (float)Diff.tv_sec * 1000.0f + (float)Diff.tv_nsec / 1000000.0f;
}

void Linux_Sleep(u32 Milliseconds)
{
  struct timespec TimeSpec = {};
  TimeSpec.tv_sec = Milliseconds / 1000;
  TimeSpec.tv_nsec = (Milliseconds % 1000) * 1000000;

  nanosleep(&TimeSpec, 0);
}

void Linux_KillThread(pthread_t Thread)
{
  Assert(Thread != 0 && "Trying to kill a non running thread?");
  pthread_cancel(Thread);
  pthread_join(Thread, 0);
}


bool Linux_FileHasChanged(u64 * FileLastChangedTimer, const char * Filename)
{
  int FD = open(Filename, O_RDWR);
  if (FD == -1)
  {
    return false;
  }
  if (flock(FD, LOCK_EX | LOCK_NB) == -1)
  {
    return false;
  }
  struct stat FileStat;
  fstat(FD, &FileStat);
  flock(FD, LOCK_UN);
  close(FD);
  if (FileStat.st_mtime != *FileLastChangedTimer)
  {
    *FileLastChangedTimer = FileStat.st_mtime;
    return true;
  }
  return false;
}

void* Linux_LibraryLoad(const char* Name)
{
  void* Library = dlopen(Name, RTLD_NOW | RTLD_GLOBAL);
  if (Library == 0)
  {
    fputs(dlerror(), stderr);
  }
  return Library;
}
void Linux_LibraryFree(void* Handle)
{
  dlclose(Handle);
}

void * Linux_GetProcAddress(void * Library, const char * Name)
{
  return dlsym(Library, Name);
}

void Linux_Deallocate(void * Memory, u64 Size)
{
  munmap(Memory, Size);
}

void * Linux_Allocate(u64 Size)
{
  void * Result = mmap(0, Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  return Result;
}


bool Linux_ReadFile(arena * Arena, const char * Filename, u8** FileBuffer, u32 * Size)
{
  int FD = open(Filename, O_RDONLY);
  if(FD == -1)
  {
    printf("Failed to find '%s'\n", Filename);
    return false;
  }

  struct stat FileStat;
  fstat(FD, &FileStat);

  *Size= FileStat.st_size;


  void* Buf = mmap(0, *Size, PROT_READ, MAP_SHARED, FD, 0);
  bool FailedToMap = Buf == MAP_FAILED;
  if(FailedToMap)
  {
    printf("Failed to map the file!!\n");
  }else
  {
    void * Memory = Arena_Allocate(Arena, *Size + 1);
    Memcpy((u8*)Memory, (u8*)Buf, *Size);
    ((u8*)Memory)[*Size] = '\0';

    munmap(Buf, *Size);
    *FileBuffer = (u8*)Memory;
  }

  close(FD);

  return !FailedToMap;
}

void Linux_HandleEvents(Display * display, game_input * Input)
{

  XEvent event;
  while(XPending(display) > 0)
  {
    XNextEvent(display, &event);
    if(event.type == ClientMessage && event.xclient.data.l[0] == GlobalDeleteAtom)
    {
      GlobalRunning = false;
    }
    if(event.type == KeyPress)
    {
      switch(event.xkey.keycode)
      {
        case 25:
          {
            Input->Up = 1;
            break;
          }
        case 38:
          {
            Input->Left = 1;
            break;
          }
        case 39:
          {
            Input->Down = 1;
            break;
          }
        case 40:
          {
            Input->Right = 1;
            break;
          }
        case 65:
          {
            Input->Shoot = 1;
            break;
          }
      }

    }
    if(event.type == KeyRelease)
    {
      switch(event.xkey.keycode)
      {
        case 25:
          {
            Input->Up = 0;
            break;
          }
        case 38:
          {
            Input->Left = 0;
            break;
          }
        case 39:
          {
            Input->Down = 0;
            break;
          }
        case 40:
          {
            Input->Right = 0;
            break;
          }
        case 65:
          {
            Input->Shoot = 0;
            break;
          }
      }
    }
  }
}

void Linux_CopyFile(const char * Source, const char * Destination)
{

  u32 SrcFD = open(Source, O_RDONLY);
  Assert(SrcFD != -1);

  u32 DestFD = open(Destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  Assert(DestFD != -1);

  struct stat FileStat;
  u32 Result = fstat(SrcFD, &FileStat);
  Assert(Result != -1);

  off_t Offset = 0;
  ssize_t BytesSent;
  while((BytesSent = sendfile(DestFD, SrcFD, &Offset, FileStat.st_size)) > 0)
  {

  }

  close(SrcFD);
  close(DestFD);


}
void Linux_LoadRenderCode(linux_render_code *RenderCode)
{
  Linux_CopyFile(GlobalRenderCodePath, GlobalTempRenderCodePath);
  void * Library = Linux_LibraryLoad(GlobalTempRenderCodePath);
  RenderCode->Library = Library;

  void * Proc = Linux_GetProcAddress(Library, "BeginFrame");
  RenderCode->BeginFrame = (renderer_begin_frame*)Proc;

  Proc = Linux_GetProcAddress(Library, "EndFrame");
  RenderCode->EndFrame = (renderer_end_frame*)Proc;

  Proc = Linux_GetProcAddress(Library, "CreateRenderer");
  RenderCode->Create   = (renderer_create*)Proc;

  Proc = Linux_GetProcAddress(Library, "ReleaseRenderer");
  RenderCode->Release  = (renderer_release*)Proc;
}

void Linux_FreeGameCode(linux_game_code* GameCode)
{
  Linux_LibraryFree(GameCode->Library);
  GameCode->Library    = 0;
  GameCode->GameUpdate = 0;
  GameCode->GameGetSoundSamples = 0;
}

void Linux_LoadGameCode(linux_game_code *GameCode)
{
  Linux_CopyFile(GlobalGameCodePath, GlobalTempGameCodePath);
  void * Library = Linux_LibraryLoad(GlobalTempGameCodePath);
  Assert(Library != 0);
  GameCode->Library = Library;

  void * Proc = Linux_GetProcAddress(Library, "GameUpdate");
  GameCode->GameUpdate = (game_update*)Proc;

  Proc = Linux_GetProcAddress(Library, "GameGetSoundSamples");
  GameCode->GameGetSoundSamples = (game_get_sound_samples*)Proc;
}

int main()
{
  Display * display = XOpenDisplay(0);
  if(display == 0)
  {
    return 1;
  }

  u32 Screen = DefaultScreen(display);

  Window rootWindow = RootWindow(display, Screen);
  Window window = XCreateSimpleWindow(display, rootWindow, 10, 10, GlobalScreenWidth, GlobalScreenHeight, 1, BlackPixel(display, Screen), WhitePixel(display, Screen));

  XStoreName(display, window, "Invaders");

  // ToDO more events?
  XSelectInput(display, window, KeyReleaseMask | KeyPressMask);

  // Actually display the window
  XMapWindow(display, window);

  GlobalDeleteAtom = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &GlobalDeleteAtom, 1);

  linux_game_code GameCode = {};
  Linux_LoadGameCode(&GameCode);
  Linux_FreeGameCode(&GameCode);
  Linux_LoadGameCode(&GameCode);

  u64 GameCodeLastChanged = 0;
  Linux_FileHasChanged(&GameCodeLastChanged, "../build/invaders.so");

  linux_render_code RenderCode = {};
  Linux_LoadRenderCode(&RenderCode);

  game_input GameInput = {};

  u64   GameMemorySize = Megabyte(205);
  void* Memory         = Linux_Allocate(GameMemorySize);

  arena GameArena      = {};
  Arena_Create(&GameArena, Memory, GameMemorySize);
  
  game_memory GameMemory          = {};
  GameMemory.PermanentSize        = Megabyte(100);
  GameMemory.PermanentStorage     = Arena_Allocate(&GameArena, GameMemory.PermanentSize);
  GameMemory.TemporaryStorageSize = Megabyte(100);
  GameMemory.TemporaryStorage     = Arena_Allocate(&GameArena, GameMemory.TemporaryStorageSize);
  GameMemory.ReadFile             = Linux_ReadFile;

  game_audio GameAudio = {};
  linux_display_and_window DisplayAndWindow = {};
  DisplayAndWindow.display  = display;
  DisplayAndWindow.window   = window;
  DisplayAndWindow.Screen   = Screen;
  platform_renderer * PlatformRenderer = RenderCode.Create(GlobalScreenWidth, GlobalScreenHeight, (void*)&DisplayAndWindow);

  // Create pushbuffer
  pushbuffer Pushbuffer           = {};
  u64        PushbufferMemorySize = Megabyte(1);
  void*      PushbufferMemory     = Arena_Allocate(&GameArena, PushbufferMemorySize);
  Pushbuffer_Create(&Pushbuffer, PushbufferMemory, PushbufferMemorySize);


  f32 DeltaTime = GlobalFramerateTargetMS / 1000.0f;
  f32 SleepError = 0;
  while(GlobalRunning)
  {

    struct timespec StartOfFrame = Linux_GetTimeInSeconds();
    RenderCode.BeginFrame(PlatformRenderer, &Pushbuffer);

    if(Linux_FileHasChanged(&GameCodeLastChanged, GlobalGameCodePath))
    {
      Linux_FreeGameCode(&GameCode);
      Linux_LoadGameCode(&GameCode);
    }

    Linux_HandleEvents(display, &GameInput);
    if(!GlobalRunning){
      break;
    }

    GameMemory.DeltaTime    = DeltaTime;
    GameMemory.ScreenWidth  = GlobalScreenWidth;
    GameMemory.ScreenHeight = GlobalScreenHeight;

    GameCode.GameUpdate(&GameMemory, &GameInput, &Pushbuffer);
    if(GameCode.GameGetSoundSamples)
    {
      u32 SampleFramesToWrite = 0; // ToDo implement!
      // GameCode.GameGetSoundSamples(&GameMemory, &GameAudio, SampleFramesToWrite);
    }
    RenderCode.EndFrame(PlatformRenderer, &Pushbuffer);

  
    struct timespec EndOfFrame = Linux_GetTimeInSeconds();
    f32 FrameMinusSleep = Linux_GetMillisecondsElapsedF(StartOfFrame, EndOfFrame);

    if(FrameMinusSleep < GlobalFramerateTargetMS - SleepError)
    {
      f32 TargetSleepTime = GlobalFramerateTargetMS - SleepError;
      f32 TimeToSleep = TargetSleepTime - FrameMinusSleep;
      Linux_Sleep((u32)TimeToSleep);
      f32 FrameTimeMSF       = Linux_GetMillisecondsElapsedF(StartOfFrame, Linux_GetTimeInSeconds());
      while(FrameTimeMSF < GlobalFramerateTargetMS)
      {
        FrameTimeMSF = Linux_GetMillisecondsElapsedF(StartOfFrame, Linux_GetTimeInSeconds());
      }
      SleepError = FrameTimeMSF - GlobalFramerateTargetMS;
    }
  }

  XDestroyWindow(display, window);
  XCloseDisplay(display);

  return 0;

}
