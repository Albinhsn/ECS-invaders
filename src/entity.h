#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"
#include "vector.h"

typedef struct HealthComponent
{
  s32 Health;
  u8  DamageTaken;
} HealthComponent;

typedef struct PositionComponent
{
  vec2f Position;
  float Rotation;
} PositionComponent;

typedef struct VelocityComponent
{
  vec2f Velocity;
} VelocityComponent;

typedef struct RenderComponent
{
  texture* Texture;
  float    Alpha;
} RenderComponent;

typedef struct ColliderComponent
{
  vec2f Extents;
} ColliderComponent;

#endif
