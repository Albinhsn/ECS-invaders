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
  u32 placeholder;

} game_input;

#define GAME_UPDATE(name) void name(game_memory* Memory, game_input* Input, pushbuffer* Pushbuffer)
typedef GAME_UPDATE(game_update);

#endif
