#ifndef RENDERER_SOFTWARE_H
#define RENDERER_SOFTWARE_H

#include "common.h"

typedef struct software_renderer
{
  // This should be a pool?
  pool_allocator TexturePool;
  u32*     Buffer;
  u16      Width;
  u16      Height;
} software_renderer;

#endif
