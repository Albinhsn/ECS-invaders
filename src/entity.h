#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"
#include "vector.h"

#define HEALTH_ID     0
#define POSITION_ID   1
#define VELOCITY_ID   2
#define RENDER_ID     3
#define COLLIDER_ID   4
#define TYPE_ID       5
#define SHOOT_ID      6

#define HEALTH_MASK   (1 << HEALTH_ID)
#define POSITION_MASK (1 << POSITION_ID)
#define VELOCITY_MASK (1 << VELOCITY_ID)
#define RENDER_MASK   (1 << RENDER_ID)
#define COLLIDER_MASK (1 << COLLIDER_ID)
#define TYPE_MASK     (1 << TYPE_ID)
#define SHOOT_MASK    (1 << SHOOT_ID)


typedef enum entity_type
{
  EntityType_Player,
  EntityType_Enemy,
  EntityType_Bullet_Enemy,
  EntityType_Bullet_Player
} entity_type;

typedef struct type_component
{
  entity_type Type;
} type_component;

typedef struct health_component
{
  s32 Health;
  u8  DamageTaken;
} health_component;

typedef struct position_component
{
  f32   X;
  f32   Y;
  float Rotation;
} position_component;

typedef struct velocity_component
{
  f32 X;
  f32 Y;
} velocity_component;

typedef struct render_component
{
  texture* Texture;
  float    Alpha;
  bool     FlippedZ;
} render_component;

typedef struct collider_component
{
  vec2f Extents;
  u8    Collided;
} collider_component;

typedef struct shoot_component
{
  f32 TimeToShoot;
} shoot_component;

typedef struct query_result
{
  s32  Count;
  u32* Ids;
} query_result;

typedef u32 entity;

typedef struct entity_manager
{
  u32*         Masks;
  u32*         ComponentSize;
  u32*         ComponentOffset;
  u32*         EntityFreeList;
  void*        Data;
  u32          EntitySize;
  u32          EntityCount;
  u32          MaxEntityCount;

  query_result Result;

} entity_manager;

#endif
