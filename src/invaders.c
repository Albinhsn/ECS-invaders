#include "invaders.h"
#include "entity.c"
#include "image.c"
#include "platform.h"
#include "pushbuffer.c"
#include "vector.c"
#include <debugapi.h>

texture* GetTextureByName(game_state* GameState, u8* TextureName)
{

  string Name = {};
  Name.Buffer = TextureName;
  Name.Length = String_Length(TextureName);
  for (u32 TextureIndex = 0; TextureIndex < GameState->TextureCount; TextureIndex++)
  {
    string n = GameState->Textures[TextureIndex].Name;
    if (String_Compare(&Name, &n))
    {
      return &GameState->Textures[TextureIndex];
    }
  }

  return 0;
}

void LoadTextures(game_state* GameState, game_memory* Memory)
{
  const char* TextureLocation = "../assets/textures.txt";

  u8*         TextureLocationBuffer;
  u32         TextureLocationSize;
  Memory->ReadFile(&GameState->PermanentArena, TextureLocation, &TextureLocationBuffer, &TextureLocationSize);

  u32 NumberOfTextures = 0;
  for (u32 BufferIndex = 0; BufferIndex < TextureLocationSize; BufferIndex++)
  {
    if (TextureLocationBuffer[BufferIndex] == '\n')
    {
      NumberOfTextures++;
    }
  }

  GameState->TextureCount = NumberOfTextures;
  GameState->Textures     = (texture*)Arena_Allocate(&GameState->PermanentArena, sizeof(texture) * NumberOfTextures);

  u32 Offset              = 0;
  u32 TextureIndex        = 0;
  while (Offset < TextureLocationSize)
  {
    u32 StartOfTextureLocation = Offset;
    while (Offset < TextureLocationSize && TextureLocationBuffer[Offset] != ' ')
    {
      Offset++;
    }
    u8 c                          = TextureLocationBuffer[Offset];
    TextureLocationBuffer[Offset] = '\0';
    u8*  TextureBuffer            = 0;
    u32  TextureBufferSize        = 0;
    bool Result                   = Memory->ReadFile(&GameState->PermanentArena, (const char*)&TextureLocationBuffer[StartOfTextureLocation], &TextureBuffer, &TextureBufferSize);
    if (Result == false)
    {
      Assert(0 && "Failed to read texture!")
    }

    targa_image Image = {};
    Image_LoadTarga(&GameState->PermanentArena, &Image, TextureBuffer, TextureBufferSize);
    GameState->Textures[TextureIndex].Memory = Image.Buffer;
    GameState->Textures[TextureIndex].Width  = Image.Width;
    GameState->Textures[TextureIndex].Height = Image.Height;

    TextureLocationBuffer[Offset]            = c;
    while (Offset < TextureLocationSize && TextureLocationBuffer[Offset] == ' ')
    {
      Offset++;
    }

    u32 StartOfTextureNameLocation = Offset;
    while (Offset < TextureLocationSize && IsAlphaOrDigit(TextureLocationBuffer[Offset]))
    {
      Offset++;
    }
    GameState->Textures[TextureIndex].Name.Buffer = &TextureLocationBuffer[StartOfTextureNameLocation];
    GameState->Textures[TextureIndex].Name.Length = Offset - StartOfTextureNameLocation;
    Offset++;
    TextureIndex++;

    // Skip until next letter
    while (Offset < TextureLocationSize && (TextureLocationBuffer[Offset] == ' ' || TextureLocationBuffer[Offset] == '\n'))
    {
      Offset++;
    }
  }
}

void RenderObjects(game_state* GameState, pushbuffer* Pushbuffer)
{
  query_result Query = EntityManager_Query(&GameState->EntityManager, POSITION_MASK | RENDER_MASK);
  for (u32 QueryIndex = 0; QueryIndex < Query.Count; QueryIndex++)
  {
    entity              Entity   = Query.Ids[QueryIndex];

    position_component* Position = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);
    render_component*   Render   = (render_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, RENDER_ID);

    vec2i               Min      = V2i((s32)(Position->X - Render->Texture->Width * 0.5f), (s32)(Position->Y - Render->Texture->Height * 0.5f));
    vec2i               Max      = V2i((s32)(Position->X + Render->Texture->Width * 0.5f), (s32)(Position->Y + Render->Texture->Height * 0.5f));
    Pushbuffer_Push_Rect_Texture(Pushbuffer, Render->Texture->Memory, Min, Max, Render->FlippedZ);
  }
}

void CreatePlayer(game_state* GameState)
{

  u32              Mask                = POSITION_MASK | HEALTH_MASK | RENDER_MASK | COLLIDER_MASK | VELOCITY_MASK;
  entity           Entity              = EntityManager_Create_Entity(&GameState->EntityManager, Mask);

  health_component Health              = {};
  Health.Health                        = 3;
  position_component Position          = {};
  Position.X                           = 200;
  Position.Y                           = 550;
  velocity_component Velocity          = {};
  render_component   Render            = {};
  const char*        PlayerTextureName = "spaceShips1";
  Render.Texture                       = GetTextureByName(GameState, (u8*)PlayerTextureName);
  Render.Alpha                         = 1;
  Render.FlippedZ                      = true;
  collider_component Collider          = {};
  Collider.Extents                     = V2f(Render.Texture->Width * 0.5f, Render.Texture->Height * 0.5f);
  Collider.CanCollideWithMask          = ENEMY_MASK;
  Collider.ColliderIsMask              = PLAYER_MASK;
  EntityManager_AddComponents(&GameState->EntityManager, Entity, Mask, 5, &Health, &Position, &Velocity, &Render, &Collider);
  GameState->PlayerEntity = Entity;
}

void UseInput(game_state* GameState, game_input* Input)
{

  entity              Entity         = GameState->PlayerEntity;
  velocity_component* Velocity       = (velocity_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, VELOCITY_ID);
  position_component* PlayerPosition = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);

  Velocity->X                        = 0;
  Velocity->Y                        = 0;

  float PlayerVelocity               = 1.0f;
  float DeltaTime                    = GameState->DeltaTime;
  if (Input->Up)
  {
    Velocity->Y -= 1;
  }
  if (Input->Left)
  {
    Velocity->X -= 1;
  }
  if (Input->Right)
  {
    Velocity->X += 1;
  }
  if (Input->Down)
  {
    Velocity->Y += 1;
  }

  vec2f NormalizedVelocity = Vec2f_NormalizeSafe(V2f(Velocity->X, Velocity->Y));
  Velocity->X              = NormalizedVelocity.X * PlayerVelocity * DeltaTime;
  Velocity->Y              = NormalizedVelocity.Y * PlayerVelocity * DeltaTime;

  // u32 Shoot;
  if (Input->Shoot)
  {
    u32                BulletMask = RENDER_MASK | POSITION_MASK | VELOCITY_MASK | COLLIDER_MASK;
    entity             Bullet     = EntityManager_Create_Entity(&GameState->EntityManager, BulletMask);
    position_component Position   = *PlayerPosition;
    Position.Y += 40.0f;
    render_component Render     = {};
    Render.Alpha                = 1.0f;
    const char* TextureName     = "spaceMissiles2";
    Render.Texture              = GetTextureByName(GameState, (u8*)TextureName);
    Render.FlippedZ             = false;
    velocity_component Velocity = {};
    Velocity.Y                  = 5.0f;
    collider_component Collider = {};
    Collider.ColliderIsMask     = PLAYER_MASK;
    Collider.CanCollideWithMask = ENEMY_MASK;
    EntityManager_AddComponents(&GameState->EntityManager, Bullet, BulletMask, 4, &Position, &Velocity, &Render, &Collider);
  }
}

void UpdatePhysics(game_state* GameState)
{

  query_result Query = EntityManager_Query(&GameState->EntityManager, VELOCITY_MASK | POSITION_MASK);
  for (u32 QueryIndex = 0; QueryIndex < Query.Count; QueryIndex++)
  {
    entity              Entity   = Query.Ids[QueryIndex];

    position_component* Position = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);
    velocity_component* Velocity = (velocity_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, VELOCITY_ID);

    Position->X += Velocity->X * GameState->DeltaTime;
    Position->Y += Velocity->Y * GameState->DeltaTime;
  }
}

bool Collision_Rect_Rect(vec2f e0, vec2f p0, vec2f e1, vec2f p1)
{
  return Abs(p0.X - p1.X) < e0.X + e1.X && //
         Abs(p0.Y - p1.Y) < e0.Y + e1.Y;   //
}

bool IsColliding(collider_component* C0, position_component* P0, collider_component* C1, position_component* P1)
{
  bool CanCollide = (C0->CanCollideWithMask & C1->ColliderIsMask) > 0;

  return CanCollide && Collision_Rect_Rect(C0->Extents, V2f(P0->X, P0->Y), C1->Extents, V2f(P1->X, P1->Y));
}

void CollisionDetection(game_state* GameState)
{
  query_result Query = EntityManager_Query(&GameState->EntityManager, COLLIDER_MASK);
  for (u32 First = 0; First < Query.Count - 1; First++)
  {
    entity              FirstEntity   = Query.Ids[First];
    collider_component* FirstCollider = (collider_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, FirstEntity, COLLIDER_ID);
    position_component* FirstPosition = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, FirstEntity, POSITION_ID);
    for (u32 Second = First + 1; Second < Query.Count; Second++)
    {
      if (First != Second)
      {
        entity              SecondEntity   = Query.Ids[Second];
        collider_component* SecondCollider = (collider_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, SecondEntity, COLLIDER_ID);
        position_component* SecondPosition = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, SecondEntity, POSITION_ID);
        if (IsColliding(FirstCollider, FirstPosition, SecondCollider, SecondPosition))
        {
          health_component* FirstHealth = (health_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, FirstEntity, HEALTH_ID);
          if (FirstHealth)
          {
            FirstHealth->Health -= 1;
          }
          health_component* SecondHealth = (health_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, SecondEntity, HEALTH_ID);
          if (SecondHealth)
          {
            SecondHealth->Health -= 1;
          }
        }
      }
    }
  }
}

void RemoveDeadUnits(game_state* GameState)
{
  query_result Query = EntityManager_Query(&GameState->EntityManager, HEALTH_MASK);
  for (u32 EntityIndex = 0; EntityIndex < Query.Count; EntityIndex++)
  {
    entity            Entity = Query.Ids[EntityIndex];
    health_component* Health = EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, HEALTH_ID);
    if (Health->Health <= 0)
    {
      if (Entity == GameState->PlayerEntity)
      {
        // You Died!
        Assert(0 && "Why you die noob!");
      }
      else
      {
        EntityManager_Remove_Entity(&GameState->EntityManager, Entity);
      }
    }
  }
}

void SpawnEnemy(game_state* GameState, vec2f Pos)
{
  u32                Mask      = COLLIDER_MASK | RENDER_MASK | POSITION_MASK | HEALTH_MASK;
  entity             Entity    = EntityManager_Create_Entity(&GameState->EntityManager, Mask);

  position_component Position  = {};
  Position.X                   = Pos.X;
  Position.Y                   = Pos.Y;
  health_component Health      = {};
  Health.Health                = 1;
  render_component Render      = {};
  const char*      TextureName = "spaceShips2";
  Render.Texture               = GetTextureByName(GameState, (u8*)TextureName);
  Render.FlippedZ              = false;
  collider_component Collider  = {};
  Collider.Extents             = V2f(Render.Texture->Width * 0.5f, Render.Texture->Height * 0.5f);
  Collider.ColliderIsMask      = ENEMY_MASK;
  Collider.CanCollideWithMask  = PLAYER_MASK;
  EntityManager_AddComponents(&GameState->EntityManager, Entity, Mask, 4, &Health, &Position,  &Render, &Collider);
}

void RemoveOutOfBoundsUnits(game_state* GameState)
{
  query_result Query         = EntityManager_Query(&GameState->EntityManager, POSITION_MASK | COLLIDER_MASK);

  vec2f        ScreenExtents = V2f(GameState->ScreenWidth * 0.5f, GameState->ScreenHeight * 0.5f);
  vec2f        ScreenCenter  = V2f(GameState->ScreenWidth * 0.5f, GameState->ScreenHeight * 0.5f);
  for (u32 QueryIndex = 0; QueryIndex < Query.Count; QueryIndex++)
  {
    entity              Entity   = Query.Ids[QueryIndex];
    position_component* Position = EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);
    collider_component* Collider = EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, COLLIDER_ID);

    if (!Collision_Rect_Rect(ScreenExtents, ScreenCenter, Collider->Extents, V2f(Position->X, Position->Y)))
    {
      EntityManager_Remove_Entity(&GameState->EntityManager, Entity);
    }
  }
}

void RespawnOutOfBoundsEnemies(game_state* GameState)
{
}

void CleanupEntities(game_state* GameState)
{
  RemoveDeadUnits(GameState);
  RespawnOutOfBoundsEnemies(GameState);
  RemoveOutOfBoundsUnits(GameState);
}

GAME_UPDATE(GameUpdate)
{
  Pushbuffer_PushClear(Pushbuffer, 0xFF00FFFF);
  game_state* GameState = (game_state*)Memory->PermanentStorage;
  GameState->DeltaTime  = Memory->DeltaTime;
  if (!Memory->IsInitialized)
  {
    // Initialize GameArena

    GameState->ScreenWidth   = Memory->ScreenWidth;
    GameState->ScreenHeight  = Memory->ScreenHeight;
    u64 PermanentStorageSize = Memory->PermanentSize - (sizeof(game_state));
    Arena_Create(&GameState->PermanentArena, (u8*)Memory->PermanentStorage + sizeof(game_state), PermanentStorageSize);

    u64 TemporaryStorageSize = Memory->TemporaryStorageSize;
    Arena_Create(&GameState->TemporaryArena, Memory->TemporaryStorage, TemporaryStorageSize);

    LoadTextures(GameState, Memory);

    EntityManager_Create(&GameState->PermanentArena, &GameState->EntityManager, 256, 5, sizeof(health_component), sizeof(position_component), sizeof(velocity_component), sizeof(render_component),
                         sizeof(collider_component));
    CreatePlayer(GameState);
    SpawnEnemy(GameState, V2f(200, 200));
    Memory->IsInitialized = true;
  }
  // Omega slow :)
  // Arena_Clear(&GameState->TemporaryArena);

  UseInput(GameState, Input);
  UpdatePhysics(GameState);
  // Collision detection and collision response

  CollisionDetection(GameState);
  CleanupEntities(GameState);

  RenderObjects(GameState, Pushbuffer);
}
