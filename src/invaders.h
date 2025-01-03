#ifndef INVADERS_H
#define INVADERS_H

#include "common.h"
#include "vector.h"

typedef struct game_state
{
  vec2f PlayerPosition;
  u8    Initialized;

} game_state;

#endif
