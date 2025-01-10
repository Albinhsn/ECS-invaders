#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"
#include "pushbuffer.h"

typedef struct game_memory
{
  u8    IsInitialized;
  void* TemporaryStorage;
  u64   TemporaryStorageSize;

  void* PermanentStorage;
  u64   PermanentSize;

  // Idk why they're here
  f32   DeltaTime;
  u16   ScreenWidth;
  u16   ScreenHeight;
  bool (*ReadFile)(arena* Arena, const char* Filename, u8** Buffer, u32* Size);
} game_memory;

typedef struct game_audio
{
  // Circular buffer that we write 33 ms of audio for the next frame
  // from the game thread into
  f32 * Buffer;               // size is Channels * SampleFrameCount
  u32 Channels;
  u32 SampleFrameCount;
  u32 SampleIndexAudioThread; // The index that the audio thread copies from
  u32 SampleIndexGameCode;    // The index that the game code writes into
  u32 SampleFramesToWrite;         // This is number of sample frames
} game_audio;

typedef struct game_input
{
  u32 Up;
  u32 Left;
  u32 Right;
  u32 Down;
  u32 Shoot;

} game_input;

#define GAME_UPDATE(name) void name(game_memory* Memory, game_input* Input, pushbuffer* Pushbuffer)
typedef GAME_UPDATE(game_update);
#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory * Memory, game_audio * Audio)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#endif
