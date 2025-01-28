#include "linux_platform.h"
#include "common.h"


static u16 GlobalScreenWidth  = 600;
static u16 GlobalScreenHeight = 800;
static bool GlobalRunning = true;
static s64 GlobalPerfCountFrequency = 0;
Atom GlobalDeleteAtom;

/*
  ToDo:
  * Platform
    * ALSA  (audio)
    * Load game code
    * Load render code
    * Parse normal input
  * Renderer
    * Allocate memory
    * CreateRenderer
    * Render
    * Begin Frame
    * End Frame
    * Release Renderer
*/


pthread_t Linux_CreateThread(void * Procedure)
{
  pthread_t Thread = 0;
  pthread_create(&Thread, 0, Procedure, 0);
  return Thread;
}
struct timespec Linux_GetTimeInSeconds()
{
  Assert(GlobalPerfCountFrequency != 0); 
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
  struct timespec TimeSpec;
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

void* Linux_LoadLibrary(const char* Name)
{
  void* Library = dlopen(Name, RTLD_NOW | RTLD_GLOBAL);
  if (Library == 0)
  {
    fputs(dlerror(), stderr);
    return 0;
  }
  return Library;
}
void Linux_FreeLibrary(void* Handle)
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
    return false;
  }

  struct stat FileStat;
  fstat(FD, &FileStat);

  *Size= FileStat.st_size;

  void * Memory = Arena_Allocate(Arena, *Size);

  void* Buf = mmap(Memory, *Size, PROT_READ, MAP_PRIVATE | MAP_FIXED, FD, 0);

  bool FailedToMap = Buf == MAP_FAILED;
  if(FailedToMap)
  {
    Arena_Deallocate(Arena, *Size);
  }
  close(FD);

  return !FailedToMap;
}

void Linux_HandleEvents(Display * display)
{

  XEvent event;
  while(XPending(display) > 0)
  {
    XNextEvent(display, &event);
    if(event.type == ClientMessage && event.xclient.data.l[0] == GlobalDeleteAtom)
    {
      GlobalRunning = false;
    }
  }
}

int main()
{
  Display * display = XOpenDisplay(0);
  if(display == 0)
  {
    return 1;
  }

  u32 Screen = DefaultScreen(display);

  Window RootWindow = RootWindow(display, Screen);
  Window window = XCreateSimpleWindow(display, RootWindow, 10, 10, GlobalScreenWidth, GlobalScreenHeight, 1, BlackPixel(display, Screen), WhitePixel(display, Screen));

  XStoreName(display, window, "Invaders");

  // ToDO more events?
  XSelectInput(display, window, ExposureMask | KeyPressMask);

  // Actually display the window
  XMapWindow(display, window);

  GlobalDeleteAtom = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &GlobalDeleteAtom, 1);

  while(GlobalRunning)
  {
    Linux_HandleEvents(display);

  }

  XDestroyWindow(display, window);
  XCloseDisplay(display);

  return 0;

}
