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
