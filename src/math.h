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

u32 Color_vec4fToU32(vec4f V)
{
	u32 Color = 0;
	u32 R = (u32)(V.X * 255) & 0xFF;
	u32 G = (u32)(V.Y * 255) & 0xFF;
	u32 B = (u32)(V.Z * 255) & 0xFF;
	u32 A = (u32)(V.W * 255) & 0xFF;

	Color = ((A << 24) |
					 (R << 16) |
					 (G << 8)  | 
					 (B << 0)); 
	return Color;
}

#endif
