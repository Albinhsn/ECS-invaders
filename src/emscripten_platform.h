#ifndef EMSCRIPTEN_PLATFORM
#define EMSCRIPTEN_PLATFORM


#include <emscripten.h>
#include <emscripten/wasm_worker.h>
#include <emscripten/html5.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <GLES2/gl2.h>
#include<dlfcn.h>
#include <stdio.h>
#include "platform.h"

typedef struct emcc_audio
{
  bool CanStart; 
  game_audio GameAudio;
  u32 BufferFrameCount;
} emcc_audio;

typedef struct emcc_game_code
{
  game_update* GameUpdate;
  game_get_sound_samples * GameGetSoundSamples;
} emcc_game_code;
typedef struct emcc_render_code
{
  renderer_begin_frame* BeginFrame;
  renderer_end_frame* EndFrame;
  renderer_create* Create;
  renderer_release* Release;
} emcc_render_code;

#endif
