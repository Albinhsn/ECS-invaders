#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"
#include "pushbuffer.h"

#define MOUSE_BUTTON_LEFT 0

typedef struct os_event
{
  u8 Key;
  u8 WasDown;
  u8 IsDown;
} os_event;

typedef struct game_memory
{
  u8    IsInitialized;
  void* TemporaryStorage;
  u64   TemporaryStorageSize;

  void* PermanentStorage;
  u64   PermanentSize;

  // Idk why they're here
  f32 DeltaTime;
  u16 ScreenWidth;
  u16 ScreenHeight;
  bool (*ReadFile)(arena* Arena, const char* Filename, u8** Buffer, u32* Size);
} game_memory;

typedef struct game_audio
{
  // Circular buffer that we write 33 ms of audio for the next frame
  // from the game thread into
  f32* Buffer; // size is Channels * SampleFrameCount
  u32  Channels;
  u32  SampleFrameCount;
  u32  SampleFrameIndexAudioThread; // The index that the audio thread copies from
  u32  SampleFrameIndexGameCode;    // The index that the game code writes into
} game_audio;

typedef struct game_input
{
  vec2f      MousePosition;
  os_event   Events[32];
  u32        EventCount;
  u32        Up;
  u32        Left;
  u32        Right;
  u32        Down;
  u32        Shoot;
} game_input;

typedef struct platform_renderer
{
  u32 Padding;
} platform_renderer;

#define GAME_UPDATE(name) void name(game_memory* Memory, game_input* Input, pushbuffer* Pushbuffer)
typedef GAME_UPDATE(game_update);
#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory* Memory, game_audio* Audio, u32 SampleFramesToWrite)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#define RENDERER_BEGIN_FRAME(name) void name(platform_renderer* PlatformRenderer, pushbuffer* Pushbuffer)
typedef RENDERER_BEGIN_FRAME(renderer_begin_frame);

#define RENDERER_END_FRAME(name) void name(platform_renderer* PlatformRenderer, pushbuffer* Pushbuffer)
typedef RENDERER_END_FRAME(renderer_end_frame);

#define RENDERER_CREATE(name) platform_renderer* name(u32 ScreenWidth, u32 ScreenHeight, void * Window)
typedef RENDERER_CREATE(renderer_create);

#define RENDERER_RELEASE(name) void name(platform_renderer* PlatformRenderer)
typedef RENDERER_RELEASE(renderer_release);

#endif
