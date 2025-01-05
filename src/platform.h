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
  bool (*ReadFile)(arena* Arena, const char* Filename, u8** Buffer, u32* Size);
} game_memory;

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

#endif
