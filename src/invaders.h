#ifndef INVADERS_H
#define INVADERS_H

#include "common.h"
#include "entity.h"
#include "vector.h"

typedef enum command_type
{
  Command_SpawnEnemy,  // Needs the EntityManager
  Command_DecideSpawn, // Just needs the CommandBuffer
} command_type;

typedef struct command command;
struct command
{
  f32          Time;
  command_type Type;
  u32 EnemiesToSpawn;
};

typedef struct command_buffer
{
  f32      Time;
  command* Commands;
  u32      MaxCommands;
  u32      CommandCount;
} command_buffer;

typedef struct texture_node texture_node;
struct texture_node
{
  texture_node* Next;
  texture*      Texture;
  const char*   Name;
};

typedef struct game_state
{
  arena          PermanentArena;
  arena          TemporaryArena;

  entity_manager EntityManager;
  entity         PlayerEntity;
  texture*       Textures;
  command_buffer CommandBuffer;
  u32            ScreenWidth, ScreenHeight;
  u32            TextureCount;
  f32            DeltaTime;
  vec2f          PlayerPosition;
} game_state;

#endif
