#include "linux_platform.h"
#include "pushbuffer.c"
#include "common.h"
#include <alsa/asoundlib.h>
#include <limits.h>


static u16 GlobalScreenWidth  = 600;
static u16 GlobalScreenHeight = 800;
static u16 GlobalFramerateTargetMS = 30;
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

void Linux_Deallocate(void * Memory, u64 Size)
{
  munmap(Memory, Size);
}

void * Linux_Allocate(u64 Size)
{
  void * Result = mmap(0, Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  return Result;
}

pthread_t Linux_CreateThreadCritical(void * Procedure)
{
  pthread_t Thread = 0;
  pthread_attr_t Attributes;

  pthread_attr_init(&Attributes);

  s32 MaxPriority = sched_get_priority_max(SCHED_FIFO);

  struct sched_param Param = {}; 
  Param.sched_priority = MaxPriority; 
  
  pthread_attr_setschedparam(&Attributes, &Param);
  pthread_attr_setinheritsched(&Attributes, PTHREAD_EXPLICIT_SCHED);
  
  pthread_create(&Thread, &Attributes, Procedure, 0);

  pthread_attr_destroy(&Attributes);
  return Thread;
}


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

typedef struct linux_audio
{
  snd_pcm_t * Handle;
  game_audio GameAudio;
  snd_pcm_uframes_t BufferFrameCount;
  snd_pcm_uframes_t PeriodSize;
  u32 Channels;
  bool CanStart;
  

}linux_audio;

linux_audio GlobalAudio;


void Linux_DeinitALSA()
{
  snd_pcm_drain(GlobalAudio.Handle);
  snd_pcm_close(GlobalAudio.Handle);
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
  void* Library = dlopen(Name, RTLD_NOW);
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

void Linux_InitALSA(arena * Arena)
{
  s32 Error;
  snd_pcm_hw_params_t* HWParams;

  Error = snd_pcm_open(&GlobalAudio.Handle, "default", SND_PCM_STREAM_PLAYBACK, 0);

  if(Error < 0)
  {
    Assert(0 && "Failed to open audio device!");
  }

  Error = snd_pcm_hw_params_malloc(&HWParams);
  if(Error < 0)
  {
    Assert(0 && "Failed allocate hw params!");
  }

  Error = snd_pcm_hw_params_any(GlobalAudio.Handle, HWParams);
  if(Error < 0)
  {
    Assert(0 && "Failed allocate hw params!");
  }

  Error = snd_pcm_hw_params_set_access(GlobalAudio.Handle, HWParams, SND_PCM_ACCESS_RW_INTERLEAVED);
  if(Error < 0)
  {
    Assert(0 && "Failed allocate hw params!");
  }

  Error = snd_pcm_hw_params_set_format(GlobalAudio.Handle, HWParams, SND_PCM_FORMAT_FLOAT_LE);
  if(Error < 0)
  {
    Assert(0 && "Failed allocate hw params!");
  }


  u32 SamplingRate = 44100;
  Error = snd_pcm_hw_params_set_rate_near(GlobalAudio.Handle, HWParams, &SamplingRate, 0);
  if(Error < 0 || SamplingRate != 44100)
  {
    Assert(0 && "Failed allocate hw params!");
  }

  GlobalAudio.Channels = 2;
  Error = snd_pcm_hw_params_set_channels(GlobalAudio.Handle, HWParams, GlobalAudio.Channels);
  if(Error < 0)
  {
    Assert(0 && "Failed set channels!");
  }


  u32 BufferTime = GlobalFramerateTargetMS * 2 * 1000;
  Error = snd_pcm_hw_params_set_buffer_time_near(GlobalAudio.Handle, HWParams, &BufferTime, 0);
  if(Error < 0)
  {
    Assert(0 && "Failed set buffer tike!");
  }

  GlobalAudio.BufferFrameCount = (u32)(SamplingRate * (float)(GlobalFramerateTargetMS  * 2.0f / 1000.0f));
  snd_pcm_uframes_t BufferSize = 0;
  Error = snd_pcm_hw_params_get_buffer_size(HWParams, &BufferSize);
  if(Error < 0)
  {
    Assert(0 && "Failed set buffer size!");
  }
  printf("Got a buffer of %ld\n", BufferSize);

  snd_pcm_uframes_t PeriodSize = BufferSize / 4;
  Error = snd_pcm_hw_params_set_period_size_near(GlobalAudio.Handle, HWParams, &PeriodSize, 0);
  if(Error < 0)
  {
    Assert(0 && "Failed set period size!");
  }
  printf("Got Period size %ld\n", PeriodSize);

  GlobalAudio.PeriodSize = PeriodSize;

  game_audio  *GameAudio = &GlobalAudio.GameAudio;
  u32 NumberOfBuffers                      = 2;
  u32 SampleFramesToWrite                  = GlobalAudio.BufferFrameCount;
  GameAudio->SampleFrameCount              = SampleFramesToWrite * NumberOfBuffers;
  GameAudio->SampleFrameIndexAudioThread   = 0;
  GameAudio->SampleFrameIndexGameCode      = 0;
  GameAudio->Buffer = (f32*)Arena_Allocate(Arena, sizeof(f32) * GameAudio->SampleFrameCount * GameAudio->Channels);

  // This applies the parameters we wanted
  Error = snd_pcm_hw_params(GlobalAudio.Handle, HWParams);
  if(Error < 0)
  {
    Assert(0 && "Failed set channels!");
  }
  snd_pcm_hw_params_free(HWParams);

  snd_pcm_sw_params_t* SWParams;

  snd_pcm_sw_params_malloc(&SWParams);
  if(Error < 0)
  {
    Assert(0 && "Failed set channels!");
  }
  snd_pcm_sw_params_current(GlobalAudio.Handle, SWParams);

  snd_pcm_sw_params_set_start_threshold(GlobalAudio.Handle, SWParams, INT_MAX);
  if(Error < 0)
  {
    Assert(0 && "Failed set channels!");
  }

  Error = snd_pcm_sw_params(GlobalAudio.Handle, SWParams);
  if(Error < 0)
  {
    Assert(0 && "Failed set channels!");
  }
  snd_pcm_sw_params_free(SWParams);

}

void Linux_RunAudioThread()
{

  // Allocate two buffers worth!
  u64 BufferSize  = sizeof(f32) * GlobalAudio.Channels * GlobalAudio.BufferFrameCount;
  printf("Allocated buffer of size %ld\n", BufferSize);
  f32 * Buffer    = (f32*)Linux_Allocate(BufferSize);

  game_audio * GameAudio = &GlobalAudio.GameAudio;

  snd_pcm_uframes_t PeriodSize = GlobalAudio.PeriodSize;
  while(!GlobalAudio.CanStart)
  {
  }



  snd_pcm_start(GlobalAudio.Handle);
  while(true)
  {
    snd_pcm_sframes_t FramesToWrite;
    s32 Error;

    Error = snd_pcm_wait(GlobalAudio.Handle, 100);
    if(Error < 0)
    {
      printf("Poll failed: '%s'\n", snd_strerror(Error));
      break;
    }

    FramesToWrite = snd_pcm_avail_update(GlobalAudio.Handle);

    if(FramesToWrite < 0)
    {
      if(FramesToWrite == -EPIPE)
      {
        printf("An xrun occured!!\n");
      }else
      {
        printf("Unknown error?!!\n");
      }
      break;
    }

    if(FramesToWrite < PeriodSize)
    {
      printf("Wanted to write %ld\n", FramesToWrite);
      continue;
    }
    FramesToWrite = PeriodSize;


    // Write the frames into the buffer
    f32 * Frames = Buffer;
    for(s32 FrameToWriteIndex = 0; FrameToWriteIndex < FramesToWrite; FrameToWriteIndex++)
    {
      *Frames++ = GameAudio->Buffer[GameAudio->SampleFrameIndexAudioThread * 2 + 0];
      *Frames++ = GameAudio->Buffer[GameAudio->SampleFrameIndexAudioThread * 2 + 1];
      GameAudio->SampleFrameIndexAudioThread++;
      GameAudio->SampleFrameIndexAudioThread %= GameAudio->SampleFrameCount;
    }

    Error = snd_pcm_writei(GlobalAudio.Handle, Buffer, FramesToWrite);
    if(Error < 0){
      printf("Error writing the frames!\n");
      break;
    }
    printf("AUDIO: %ld, %d\n", FramesToWrite, GameAudio->SampleFrameIndexAudioThread);
  }

  Linux_Deallocate((void*)Buffer, BufferSize);

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

  u64 GameCodeLastChanged = 0;
  Linux_FileHasChanged(&GameCodeLastChanged, GlobalGameCodePath);

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

  // Init audio thread
  Linux_InitALSA(&GameArena);
  pthread_t AudioThread = Linux_CreateThreadCritical((void*)Linux_RunAudioThread);

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
      u32 SampleFramesToWrite = GlobalAudio.BufferFrameCount; // ToDo implement!
      GameCode.GameGetSoundSamples(&GameMemory, &GlobalAudio.GameAudio, SampleFramesToWrite);
    }
    RenderCode.EndFrame(PlatformRenderer, &Pushbuffer);

  
    struct timespec EndOfFrame = Linux_GetTimeInSeconds();
    f32 FrameMinusSleep = Linux_GetMillisecondsElapsedF(StartOfFrame, EndOfFrame);

    f32 TargetSleepTime =GlobalFramerateTargetMS - SleepError;
    if(FrameMinusSleep < TargetSleepTime)
    {
      f32 TimeToSleep = TargetSleepTime - FrameMinusSleep;
      Linux_Sleep((u32)TimeToSleep);
      f32 FrameTimeMSF       = Linux_GetMillisecondsElapsedF(StartOfFrame, Linux_GetTimeInSeconds());
      printf("Time after first sleep %.2lf\n", FrameTimeMSF);
      while(FrameTimeMSF < TargetSleepTime)
      {
        FrameTimeMSF = Linux_GetMillisecondsElapsedF(StartOfFrame, Linux_GetTimeInSeconds());
      }
      SleepError = FrameTimeMSF - TargetSleepTime;
      printf("Frame took %.2lf, Error %.2lf, Target: %.2lf, Frame: %.2lf, Slept for %.2lf\n", FrameTimeMSF, SleepError, TargetSleepTime, FrameMinusSleep, TimeToSleep);
    }

    // Figure out better way for this!
    GlobalAudio.CanStart = true;
  }

  XDestroyWindow(display, window);
  XCloseDisplay(display);
  Linux_KillThread(AudioThread);
  Linux_DeinitALSA();

  return 0;

}
