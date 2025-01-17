#ifndef MATH_H
#define MATH_H
#include "common.h"
#include "vector.h"

typedef struct rect2
{
  vec2f Min, Max;
} rect2;

bool Rect_PointInside(rect2 Rect, vec2f Point)
{
  return (
      Rect.Min.X <= Point.X &&
      Rect.Max.X >= Point.X && 
      Rect.Min.Y <= Point.Y &&
      Rect.Max.Y >= Point.Y
    );

}

f32 Min(f32 a, f32 b)
{
  return a < b ? a : b;
}
f32 Max(f32 a, f32 b)
{
  return a > b ? a : b;
}
s32 MinI(s32 a, s32 b)
{
  return a < b ? a : b;
}
s32 MaxI(s32 a, s32 b)
{
  return a > b ? a : b;
}
f32 Abs(f32 x)
{
  return x < 0 ? -x : x;
}

#endif
