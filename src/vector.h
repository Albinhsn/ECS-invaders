#ifndef VECTOR_H
#define VECTOR_H

#include "common.h"

typedef struct vec2i
{
  u32 X;
  u32 Y;
} vec2i;

vec2i V2(u32 X, u32 Y){
  return (vec2i){X, Y};
}

#endif
