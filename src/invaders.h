#ifndef INVADERS_H
#define INVADERS_H

#include "common.h"
#include "entity.h"
#include "sound.h"
#include "ui.h"
#include "vector.h"

typedef struct highscore
{
  string * Name;
  u32      Score;
} highscore;

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

typedef enum game_state_type
{
  GameState_GameRunning,
  GameState_MainMenu,
  GameState_InputName,
  GameState_ShowHighscore
} game_state_type;

#define MAX_SOUND_COUNT         16
#define MAX_PLAYING_SOUND_COUNT 16
#define MAX_HIGHSCORES          5
typedef struct game_state
{

  game_state_type State;
  arena          PermanentArena;
  arena          TemporaryArena;
  highscore      Highscores[MAX_HIGHSCORES];
  u32            HighscoreCount;
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
