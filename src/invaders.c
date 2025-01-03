#include "platform.h"
#include "pushbuffer.c"

GAME_UPDATE(GameUpdate)
{
  Pushbuffer_PushClear(Pushbuffer, 0xFF00FF00);
}
