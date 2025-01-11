#ifndef INVADERS_H
#define INVADERS_H

#include "common.h"
#include "entity.h"
#include "ui.h"
#include "sound.h"
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
  u32          EnemiesToSpawn;
};

typedef struct command_buffer
{
  f32      Time;
  command* Commands;
  u32      MaxCommands;
  u32      CommandCount;
} command_buffer;

#define MAX_SOUND_COUNT         16
#define MAX_PLAYING_SOUND_COUNT 16
typedef struct game_state
{
  arena          PermanentArena;
  arena          TemporaryArena;

  entity_manager EntityManager;
  entity         PlayerEntity;
  texture*       Textures;
  u32            TextureCount;
  command_buffer CommandBuffer;
  u32            ScreenWidth, ScreenHeight;
  f32            DeltaTime;
  vec2f          PlayerPosition;
  f32            Score;

  msdf_font      Font;
  string         ScoreString;
  playing_sound  PlayingSounds[MAX_PLAYING_SOUND_COUNT];
  u32            PlayingSoundCount;
  sound          Sounds[MAX_SOUND_COUNT];
  u32            SoundCount;

} game_state;

#endif
