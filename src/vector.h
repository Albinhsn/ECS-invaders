#ifndef VECTOR_H
#define VECTOR_H

#include "common.h"

typedef struct vec2i
{
  s32 X;
  s32 Y;
} vec2i;
typedef struct vec2f
{
  f32 X;
  f32 Y;
} vec2f;

typedef struct vec4f
{
  f32 X;
  f32 Y;
  f32 Z;
  f32 W;
} vec4f;

vec2i V2i(u32 X, u32 Y){
  return (vec2i){X, Y};
}
vec2f V2f(f32 X, f32 Y){
  return (vec2f){X, Y};
}

#endif
