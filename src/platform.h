#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"
#include "pushbuffer.h"

typedef struct game_memory
{
  void* TransientStorage;
  u64   TransientStorageSize;

  void* TemporaryStorage;
  u64   TemporarySize;

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

#define READ_FILE(name) void name(const char* Filename, void** Buffer, u32* Size)
typedef READ_FILE(read_file);

#endif
