#include "invaders.h"
#include "platform.h"
#include "pushbuffer.c"

GAME_UPDATE(GameUpdate)
{
  Pushbuffer_PushClear(Pushbuffer, 0xFF00FFFF);
  game_state* GameState = (game_state*)Memory->TransientStorage;
  if (!GameState->Initialized)
  {
    GameState->PlayerPosition = V2f(100, 100);
    GameState->Initialized = 1;
  }

  f32 Velocity = 10.0f;
  if (Input->Up)
  {
    GameState->PlayerPosition.Y -= Velocity / 60.0f;
  }
  if (Input->Down)
  {
    GameState->PlayerPosition.Y += Velocity / 60.0f;
  }
  if (Input->Left)
  {
    GameState->PlayerPosition.X -= Velocity / 60.0f;
  }
  if (Input->Right)
  {
    GameState->PlayerPosition.X += Velocity / 60.0f;
  }

  vec2i Min = V2i((u32)(GameState->PlayerPosition.X - 50), (u32)(GameState->PlayerPosition.Y - 50));
  vec2i Max = V2i((u32)(GameState->PlayerPosition.X + 50), (u32)(GameState->PlayerPosition.Y + 50));
  Pushbuffer_Push_Rect_Color(Pushbuffer, Min, Max, 0xFFFF0000);
}
