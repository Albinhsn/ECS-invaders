#include "platform.h"
#include "pushbuffer.c"

GAME_UPDATE(GameUpdate)
{
  Pushbuffer_PushClear(Pushbuffer, 0xFF00FFFF);

  vec2i Min = V2(100, 100);
  vec2i Max = V2(200, 200);
  Pushbuffer_Push_Rect_Color(Pushbuffer, Min, Max, 0xFFFF0000);
}
