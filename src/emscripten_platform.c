#include <emscripten.h>
#include <emscripten/html5.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include "common.h"
#include <pthread.h>
#include<dlfcn.h>


static u16                     GlobalScreenWidth       = 600;
static u16                     GlobalScreenHeight      = 800;
static s32                     GlobalShouldQuit        = 0;
static u16                     GlobalFramerateTargetMS = 30;

/*
	* ToDo 
	* 	Process Messages
	* 	Use wasm worker instead of pthread?
	* 	Audio
	*/

void * EMCC_LibraryLoad(const char * Name){
	void * Library = dlopen(Name, RTLD_NOW);

	if(Library == 0)
	{
		printf("Failed to load library '%s'\n", Name);
	}

	return Library;
}

bool EMCC_FileHasChanged(u64 * FileLastChangedTimer, const char * Filename)
{
  struct stat FileStat;
  stat(Filename, &FileStat);
  if (FileStat.st_mtime != *FileLastChangedTimer)
  {
    *FileLastChangedTimer = FileStat.st_mtime;
    return true;
  }
  return false;
}

void EMCC_LibraryFree(void * Handle)
{
	dlclose(Handle);
}

void * EMCC_GetProcAddress(void * Library, const char * Name)
{
	return dlsym(Library, Name);
}

bool EMCC_ReadFile(arena * Arena, const char * Filename, u8 **FileBuffer, u32 * Size)
{
	FILE * FD = fopen(Filename, "r");

	fseek(FD, 0, SEEK_END);
	*Size = ftell(FD);
	fseek(FD, 0, SEEK_SET);

	u8* Buffer = (u8*)Arena_Allocate(Arena, *Size + 1);
	u32 Read = fread(Buffer, 1, *Size, FD);
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

void game_loop() {
	float dt = 1.0f / (float)GlobalFramerateTargetMS;
}

int main() {
	EmscriptenWebGLContextAttributes attr;
	emscripten_webgl_init_context_attributes(&attr);
	attr.alpha = 0;
	attr.depth = 0;
	attr.stencil = 0;
	attr.antialias = 1;
	attr.preserveDrawingBuffer = 0;

	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("canvas", &attr);
	emscripten_webgl_make_context_current(context);
	emscripten_set_canvas_element_size("#canvas", GlobalScreenWidth, GlobalScreenHeight);
	
	emscripten_set_main_loop(game_loop, GlobalFramerateTargetMS, true);
	return 0;
}
