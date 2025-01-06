#ifndef INVADERS_H
#define INVADERS_H

#include "common.h"
#include "entity.h"
#include "vector.h"

typedef enum command_type
{
  Command_SpawnEnemy,  // Needs the EntityManager
  Command_DecideSpawn, // Just needs the CommandBuffer
  Command_EnemyShoot   // Needs the EntityManager
} command_type;

typedef struct command command;
struct command
{
  f32          Time;
  command_type Type;
  entity       Entity; // Used for shoot!
};

typedef struct command_buffer
{
  command* Commands;
  u32      MaxCommands;
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
  u32 ScreenWidth, ScreenHeight;
  u32            TextureCount;
  f32            DeltaTime;
  vec2f          PlayerPosition;
} game_state;

#endif
