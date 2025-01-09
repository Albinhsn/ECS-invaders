#include "vector.h"
#include "math.h"




vec2f Vec2f_NormalizeSafe(vec2f Vector){
  float Magnitude = sqrtf(Vector.X * Vector.X + Vector.Y * Vector.Y);
  if(Magnitude != 0){
    Vector.X /= Magnitude;
    Vector.Y /= Magnitude;
  }

  return Vector;
}
vec2f Vec2f_Add(vec2f A, vec2f B){
  vec2f Result = {};
  Result.X = A.X + B.X;
  Result.Y = A.Y + B.Y;

  return Result;
}
vec2f Vec2f_Sub(vec2f A, vec2f B){
  vec2f Result = {};
  Result.X = A.X - B.X;
  Result.Y = A.Y - B.Y;

  return Result;
}

f32 Vec2f_Length(vec2f A){
  return sqrtf(A.X * A.X + A.Y * A.Y);
}

f32 Vec2f_Dot(vec2f A, vec2f B){
  return A.X * B.X + A.Y * B.Y;
}
vec2f Vec2f_Scale(vec2f A, f32 Scale){
  A.X *= Scale;
  A.Y *= Scale;
  return A;
}

vec2f Vec2f_Reflect(vec2f V, vec2f Normal)
{

  // V - Normal * (2 * Dot(V, Normal))
  return Vec2f_Sub(V, Vec2f_Scale(Normal, (2 * Vec2f_Dot(V, Normal))));;

}