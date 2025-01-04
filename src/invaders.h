#ifndef INVADERS_H
#define INVADERS_H

#include "common.h"
#include "vector.h"

typedef struct texture_node texture_node;
struct texture_node
{
  texture_node* Next;
  texture*      Texture;
  const char*   Name;
};

typedef struct game_state
{
  arena          GameArena;
  pool_allocator TexturePool;
  vec2f          PlayerPosition;
} game_state;

#endif
