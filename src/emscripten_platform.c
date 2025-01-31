#include "common.h"
#include "pushbuffer.c"

#include "emscripten_platform.h"


static u16                     GlobalScreenWidth       = 600;
static u16                     GlobalScreenHeight      = 800;
static s32                     GlobalShouldQuit        = 0;

game_memory GlobalGameMemory = {};
game_input  GlobalGameInput = {};
emcc_audio  GlobalAudio = {};
pushbuffer  GlobalPushbuffer = {};
platform_renderer GlobalPlatformRenderer = {};

emcc_game_code GameCode;
emcc_render_code RenderCode;
/*
	* ToDo 
	* 	Audio
  * 	Process messages
  * 	Lower the amount of memory needed
  * 	Create renderer(s)
	*/

void * EMCC_LibraryLoad(const char * Name){
	void * Library = dlopen(Name, RTLD_NOW);

	if(Library == 0)
	{
		printf("Failed to load library '%s'\n", Name);
	}

	return Library;
}

void EMCC_LibraryFree(void * Handle)
{
	dlclose(Handle);
}
void * EMCC_GetProcAddress(void * Library, const char * Name)
{
	return dlsym(Library, Name);
}


bool EMCC_FileHasChanged(u64 * FileLastChangedTimer, const char * Filename)
{
  struct stat FileStat;
  stat(Filename, &FileStat);
	printf("Statd: %llu %llu\n", FileStat.st_mtime, *FileLastChangedTimer);
  if (FileStat.st_mtime != *FileLastChangedTimer)
  {
    *FileLastChangedTimer = FileStat.st_mtime;
		printf("File changed, new timer: %llu\n", *FileLastChangedTimer);
    return true;
  }
		printf("File didn't change\n");
  return false;
}


bool EMCC_ReadFile(arena * Arena, const char * Filename, u8 **FileBuffer, u32 * Size)
{
	FILE * FD = fopen(Filename, "r");
	if(FD == 0)
	{
		printf("Failed to open file at '%s'\n", Filename);
		return false;
	}
	printf("Opened file %llu\n", (u64)FD);

	fseek(FD, 0, SEEK_END);
	*Size = ftell(FD);
	fseek(FD, 0, SEEK_SET);
	printf("Seeked, %llu\n", (u64)(*Size));

	u8* Buffer = (u8*)Arena_Allocate(Arena, *Size + 1);
	u32 Read = fread(Buffer, 1, *Size, FD);
	printf("%d\n", Read);
	if(Read != *Size)
	{
		printf("Failed to read the file! '%s'\n", Filename);
		Arena_Deallocate(Arena, *Size);
		return false;
	}

	Buffer[*Size] = '\0';

	*FileBuffer = Buffer;
	*Size = *Size + 1;

	fclose(FD);
	return true;
}

pthread_t EMCC_CreateThread(void * Procedure)
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

void EMCC_KillThread(pthread_t Thread)
{
  pthread_cancel(Thread);
  pthread_join(Thread, 0);
}


void * EMCC_Allocate(u64 Size)
{
	return malloc(Size);
}

void EMCC_Deallocate(void * Memory)
{
	free(Memory);
}

void EMCC_ProcessMessages()
{

}

void EMCC_Frame() {
	float dt = 1.0f / 60.0f;


  // ToDo Process events
  EMCC_ProcessMessages();
  // What do we do when we want to quit?
  
  GameCode.GameUpdate(&GlobalGameMemory, &GlobalGameInput, &GlobalPushbuffer);
  u32 SampleFramesToWrite = GlobalAudio.BufferFrameCount;
  GameCode.GameGetSoundSamples(&GlobalGameMemory, &GlobalAudio.GameAudio, SampleFramesToWrite);
  RenderCode.EndFrame(&GlobalPlatformRenderer, &GlobalPushbuffer);

  if(!GlobalAudio.CanStart)
  {
    GlobalAudio.CanStart = true;
    // Init audio here
  }
}

void EMCC_LoadGameCode(){}
void EMCC_LoadRenderCode(){}

int main() {
	EmscriptenWebGLContextAttributes attr;
	emscripten_webgl_init_context_attributes(&attr);
	attr.alpha = 0;
	attr.depth = 0;
	attr.stencil = 0;
	attr.antialias = 1;
	attr.preserveDrawingBuffer = 0;


  u64   GameMemorySize = Megabyte(15);
	void * Memory = EMCC_Allocate(GameMemorySize);

  arena GameArena = {};
  Arena_Create(&GameArena, Memory, GameMemorySize);

  game_memory GameMemory = {};
  GameMemory.PermanentSize        = Megabyte(5);
  GameMemory.PermanentStorage     = Arena_Allocate(&GameArena, GameMemory.PermanentSize);
  GameMemory.TemporaryStorageSize = Megabyte(5);
  GameMemory.TemporaryStorage     = Arena_Allocate(&GameArena, GameMemory.TemporaryStorageSize);
  GameMemory.ReadFile             = EMCC_ReadFile;

  // ToDo Create the renderer

  EMCC_LoadGameCode();
  EMCC_LoadRenderCode();


  u64        PushbufferMemorySize = Megabyte(1);
  void*      PushbufferMemory     = Arena_Allocate(&GameArena, PushbufferMemorySize);
  Pushbuffer_Create(&GlobalPushbuffer, PushbufferMemory, PushbufferMemorySize);

  // ToDo Init audio

	emscripten_set_canvas_element_size("#canvas", GlobalScreenWidth, GlobalScreenHeight);
	
	// emscripten_set_main_loop(EMCC_Frame, 0, true);
	return 0;
}
