#include "invaders.h"
#include "entity.c"
#include "image.c"
#include "sound.c"
#include "platform.h"
#include "pushbuffer.c"
#include "vector.c"

sound * GetSoundByName(game_state * GameState, const char * SoundName)
{
  string Name = {};
  Name.Buffer = (u8*)SoundName;
  Name.Length = String_Length(Name.Buffer);
  for(u32 SoundIndex = 0; SoundIndex < GameState->SoundCount; SoundIndex++)
  {
    sound * Sound = GameState->Sounds + SoundIndex;

    if(String_Compare(&Name, &Sound->Name))
    {
      return Sound;
    }
  }
  Assert(0 && "Couldn't find the sound!");
  return 0;
}

void Inv_PlaySound(game_state * GameState, const char * SoundName)
{
  sound * Sound = GetSoundByName(GameState, SoundName);
  Assert(Sound && "Didn't find this sound!");
  Assert(GameState->PlayingSoundCount + 1 < ArrayCount(GameState->PlayingSounds));

  playing_sound * PlayingSound = &GameState->PlayingSounds[GameState->PlayingSoundCount++];
  PlayingSound->Sound = Sound;
  PlayingSound->Looping = false;
  PlayingSound->Volume = 0.1f;
  PlayingSound->SampleFramesPlayed = 0;

}

texture* GetTextureByName(game_state* GameState, const char * TextureName)
{

  string Name = {};
  Name.Buffer = (u8*)TextureName;
  Name.Length = String_Length((u8*)TextureName);
  for (u32 TextureIndex = 0; TextureIndex < GameState->TextureCount; TextureIndex++)
  {
    string n = GameState->Textures[TextureIndex].Name;
    if (String_Compare(&Name, &n))
    {
      return &GameState->Textures[TextureIndex];
    }
  }
  Assert(0 && "Couldn't find the texture!");
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

void LoadSounds(game_state * GameState, game_memory * Memory)
{
  const char* SoundLocation = "../assets/sounds.txt";

  u8*         SoundLocationBuffer;
  u32         SoundLocationSize;
  Memory->ReadFile(&GameState->PermanentArena, SoundLocation, &SoundLocationBuffer, &SoundLocationSize);

  u32 NumberOfSounds = 0;
  for (u32 BufferIndex = 0; BufferIndex < SoundLocationSize; BufferIndex++)
  {
    if (SoundLocationBuffer[BufferIndex] == '\n')
    {
      NumberOfSounds++;
    }
  }

  Assert(NumberOfSounds <= ArrayCount(GameState->Sounds) && "Too many sounds!");

  GameState->SoundCount = NumberOfSounds;


  u32 Offset              = 0;
  u32 SoundIndex          = 0;
  while (Offset < SoundLocationSize)
  {
    u32 StartOfSoundLocation = Offset;
    while (Offset < SoundLocationSize && SoundLocationBuffer[Offset] != ' ')
    {
      Offset++;
    }
    u8 c                          = SoundLocationBuffer[Offset];
    SoundLocationBuffer[Offset] = '\0';
    u8*  SoundBuffer            = 0;
    u32  SoundBufferSize        = 0;
    bool Result                   = Memory->ReadFile(&GameState->PermanentArena, (const char*)&SoundLocationBuffer[StartOfSoundLocation], &SoundBuffer, &SoundBufferSize);
    if (Result == false)
    {
      Assert(0 && "Failed to read sound!")
    }

    Result = Sound_ParseWave(&GameState->PermanentArena, &GameState->Sounds[SoundIndex], SoundBuffer, SoundBufferSize);
    Assert(Result == true && "Failed to parse wav file?");

    SoundLocationBuffer[Offset]            = c;
    while (Offset < SoundLocationSize && SoundLocationBuffer[Offset] == ' ')
    {
      Offset++;
    }

    u32 StartOfSoundNameLocation = Offset;
    while (Offset < SoundLocationSize && IsAlphaOrDigit(SoundLocationBuffer[Offset]))
    {
      Offset++;
    }
    GameState->Sounds[SoundIndex].Name.Buffer = &SoundLocationBuffer[StartOfSoundNameLocation];
    GameState->Sounds[SoundIndex].Name.Length = Offset - StartOfSoundNameLocation;
    Offset++;
    SoundIndex++;

    // Skip until next letter
    while (Offset < SoundLocationSize && (SoundLocationBuffer[Offset] == ' ' || SoundLocationBuffer[Offset] == '\n'))
    {
      Offset++;
    }
  }
}

void RenderObjects(game_state* GameState, pushbuffer* Pushbuffer)
{

  query_result Query = EntityManager_Query(&GameState->EntityManager, POSITION_MASK | RENDER_MASK);
  for (s32 QueryIndex = 0; QueryIndex < Query.Count; QueryIndex++)
  {

    entity              Entity   = Query.Ids[QueryIndex];

    position_component* Position = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);
    render_component*   Render   = (render_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, RENDER_ID);

    // Is the bottom left corner!

    vec2f XAxis = {};
    XAxis.X = cosf(Position->Rotation);
    XAxis.Y = sinf(Position->Rotation);
    XAxis = Vec2f_Scale(XAxis, (f32)Render->Texture->Width);

    vec2f YAxis = {};
    YAxis.X = cosf(Position->Rotation + PI / 2);
    YAxis.Y = sinf(Position->Rotation + PI / 2);
    YAxis = Vec2f_Scale(YAxis, -(f32)Render->Texture->Height);



    vec2f Origin = Vec2f_Sub(Vec2f_Sub(V2f(Position->X, Position->Y), Vec2f_Scale(XAxis, 0.5f)), Vec2f_Scale(YAxis, 0.5f));
    Pushbuffer_PushRectTexture(Pushbuffer, Render->Texture, Origin,  XAxis, YAxis, Render->FlippedZ);
    // Pushbuffer_PushRectColor(Pushbuffer, Origin, XAxis, YAxis, 0x0000FF00);

  }
}

void CreatePlayer(game_state* GameState)
{

  u32              Mask                = POSITION_MASK | HEALTH_MASK | RENDER_MASK | COLLIDER_MASK | VELOCITY_MASK |TYPE_MASK |SHOOT_MASK;
  entity           Entity              = EntityManager_Create_Entity(&GameState->EntityManager, Mask);

  health_component Health              = {};
  Health.Health                        = 3;
  position_component Position          = {};
  Position.X                           = 200;
  Position.Y                           = 550;
  Position.Rotation                    = 0;
  velocity_component Velocity          = {};
  render_component   Render            = {};

  Render.Texture                       = GetTextureByName(GameState, "spaceShips1");
  Render.Alpha                         = 1;
  Render.FlippedZ                      = true;
  collider_component Collider          = {};
  Collider.Extents                     = V2f(Render.Texture->Width * 0.5f, Render.Texture->Height * 0.5f);

  shoot_component Shoot = {};


  type_component Type = {};
  Type.Type = EntityType_Player;
  EntityManager_AddComponents(&GameState->EntityManager, Entity, Mask, 7, &Health, &Position, &Velocity, &Render, &Collider, &Type, &Shoot);
  GameState->PlayerEntity = Entity;
}

void UseInput(game_state* GameState, game_input* Input)
{

  entity              Entity         = GameState->PlayerEntity;
  velocity_component* Velocity       = (velocity_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, VELOCITY_ID);
  position_component* PlayerPosition = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);
  shoot_component * Shoot            = (shoot_component*) EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, SHOOT_ID);
  Velocity->X                        = 0;
  Velocity->Y                        = 0;

  float PlayerVelocity               = 300.0f;
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
  Velocity->X              = NormalizedVelocity.X * PlayerVelocity;
  Velocity->Y              = NormalizedVelocity.Y * PlayerVelocity;


  if (Input->Shoot && Shoot->TimeToShoot <= GameState->CommandBuffer.Time)
  {
    u32                BulletMask = RENDER_MASK | POSITION_MASK | VELOCITY_MASK | COLLIDER_MASK | TYPE_MASK | HEALTH_MASK;
    entity             Bullet     = EntityManager_Create_Entity(&GameState->EntityManager, BulletMask);
    position_component Position   = *PlayerPosition;
    Position.Y -= 40.0f;
    render_component Render     = {};
    Render.Alpha                = 1.0f;

    Render.Texture              = GetTextureByName(GameState, "spaceMissiles2");
    Render.FlippedZ             = false;
    velocity_component Velocity = {};
    Velocity.Y                  = -200.0f;
    collider_component Collider = {};
    Collider.Extents.X = Render.Texture->Width * 0.5f;
    Collider.Extents.Y = Render.Texture->Height  * 0.5f;

    health_component Health = {};
    Health.Health = 1;

    type_component Type = {};
    Type.Type = EntityType_Bullet_Player;
    EntityManager_AddComponents(&GameState->EntityManager, Bullet, BulletMask, 6, &Health, &Position, &Velocity, &Render, &Collider, &Type);

    f32 ShootCooldown = 1.0f;
    Shoot->TimeToShoot = GameState->CommandBuffer.Time + ShootCooldown;
    Inv_PlaySound(GameState, "shoot");
  }
}

void UpdatePhysics(game_state* GameState)
{
  query_result Query = EntityManager_Query(&GameState->EntityManager, VELOCITY_MASK | POSITION_MASK);
  for (s32 QueryIndex = 0; QueryIndex < Query.Count; QueryIndex++)
  {
    entity              Entity   = Query.Ids[QueryIndex];

    position_component* Position = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);
    velocity_component* Velocity = (velocity_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, VELOCITY_ID);

    // Calculate the X and Y axis given this rotation
    vec2f XAxis = {};
    XAxis.X = cosf(Position->Rotation);
    XAxis.Y = sinf(Position->Rotation);
    vec2f YAxis = {};
    YAxis.X = cosf(Position->Rotation + PI / 2);
    YAxis.Y = sinf(Position->Rotation + PI / 2);

    vec2f Pos = V2f(Position->X, Position->Y);
    vec2f dx  = Vec2f_Scale(XAxis, Velocity->X * GameState->DeltaTime);
    Pos       = Vec2f_Add(dx, Pos);
    vec2f dy  = Vec2f_Scale(YAxis, Velocity->Y * GameState->DeltaTime);
    Pos       = Vec2f_Add(dy, Pos);

    Position->X  = Pos.X;
    Position->Y = Pos.Y;
    // Position->Rotation += GameState->DeltaTime;
  }
}

bool Collision_Rect_Rect(vec2f e0, vec2f p0, vec2f e1, vec2f p1)
{
  return Abs(p0.X - p1.X) < e0.X + e1.X && //
         Abs(p0.Y - p1.Y) < e0.Y + e1.Y;   //
}
bool Collision_OBB_OBB(f32 r0, vec2f e0, vec2f p0, f32 r1, vec2f e1, vec2f p1)
{
  return false;
}

bool IsColliding(type_component * T0, collider_component* C0, position_component* P0, type_component * T1, collider_component* C1, position_component* P1)
{

  bool CanCollide = true;
  if(T0->Type == T1->Type){
    CanCollide = false;
  }
  else if(
    (T0->Type == EntityType_Player && T1->Type == EntityType_Bullet_Player) ||
    (T0->Type == EntityType_Bullet_Player && T1->Type == EntityType_Player)
    ){
    CanCollide = false;
  }
  else if(
    (T0->Type == EntityType_Enemy && T1->Type == EntityType_Bullet_Enemy) ||
    (T0->Type == EntityType_Bullet_Enemy && T1->Type == EntityType_Enemy)
    ){
    CanCollide = false;
  }

  // ToDo This should be OBB!!!
  return CanCollide && Collision_Rect_Rect(C0->Extents, V2f(P0->X, P0->Y), C1->Extents, V2f(P1->X, P1->Y));
}

void CollisionDetection(game_state* GameState, pushbuffer * Pushbuffer)
{
  query_result Query = EntityManager_Query(&GameState->EntityManager, COLLIDER_MASK | POSITION_MASK | TYPE_MASK);
  for (s32 First = 0; First < Query.Count - 1; First++)
  {
    entity              FirstEntity   = Query.Ids[First];
    collider_component* FirstCollider = (collider_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, FirstEntity, COLLIDER_ID);
    position_component* FirstPosition = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, FirstEntity, POSITION_ID);
    type_component *    FirstType     = (type_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, FirstEntity, TYPE_ID);
    for (s32 Second = First + 1; Second < Query.Count; Second++)
    {
      if (First != Second)
      {
        entity              SecondEntity   = Query.Ids[Second];
        collider_component* SecondCollider = (collider_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, SecondEntity, COLLIDER_ID);
        position_component* SecondPosition = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, SecondEntity, POSITION_ID);
        type_component *    SecondType     = (type_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, SecondEntity, TYPE_ID);
        if (IsColliding(FirstType, FirstCollider, FirstPosition, SecondType, SecondCollider, SecondPosition))
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

  for (s32 QueryIndex = 0; QueryIndex < Query.Count; QueryIndex++)
  {
    entity              Entity   = Query.Ids[QueryIndex];
    collider_component* Collider = (collider_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, COLLIDER_ID);
    position_component* Position = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);
    type_component *    Type     = (type_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, TYPE_ID);
    if(Type->Type == EntityType_Player)
    {
      // Check if we're hitting the wall
      // Clamp the position
      f32 MinX = Position->X - Collider->Extents.X;
      f32 MaxX = Position->X + Collider->Extents.X;

      f32 MinY = Position->Y - Collider->Extents.Y;
      f32 MaxY = Position->Y + Collider->Extents.Y;

      if(MinX < 0)
      {
        Position->X = Collider->Extents.X;
      }
      else if(MaxX >= GameState->ScreenWidth - 1)
      {
        Position->X = GameState->ScreenWidth - Collider->Extents.X - 1;
      }

      if(MinY < 0)
      {
        Position->Y = Collider->Extents.Y;
      }
      else if(MaxY >= GameState->ScreenHeight - 1)
      {
        Position->Y = GameState->ScreenHeight - Collider->Extents.Y - 1;
      }

    }
    else if(Type->Type == EntityType_Enemy)
    {
      // Get the BB of the enemy
      vec2f XAxis = {};
      XAxis.X = cosf(Position->Rotation);
      XAxis.Y = sinf(Position->Rotation);
      vec2f YAxis = {};
      YAxis.X = cosf(Position->Rotation + PI / 2);
      YAxis.Y = sinf(Position->Rotation + PI / 2);

      XAxis = Vec2f_Scale(XAxis, Collider->Extents.X * 2);
      YAxis = Vec2f_Scale(YAxis, -Collider->Extents.Y * 2);

      vec2f Origin = Vec2f_Sub(Vec2f_Sub(V2f(Position->X, Position->Y), Vec2f_Scale(XAxis, 0.5f)), Vec2f_Scale(YAxis, 0.5f));
      vec2f v0 = Origin;
      vec2f v1 = Vec2f_Add(v0, XAxis);
      vec2f v2 = Vec2f_Add(v0, YAxis);
      vec2f v3 = Vec2f_Add(v2, XAxis);




      f32 MinX =  10000;
      f32 MaxX = -10000;
      vec2f Corners[4] = {v0, v1, v2, v3};
      for(u32 CornerIndex = 0; CornerIndex < ArrayCount(Corners); CornerIndex++)
      {
        vec2f Corner = Corners[CornerIndex];
        MinX = Min(Corner.X, MinX);
        MaxX = Max(Corner.X, MaxX);
      }


      // Check if we're hitting the wall
      if(MinX < 0)
      {
        vec2f Reflected = Vec2f_Reflect(YAxis, V2f(1,0));
        f32 NewRotation = atan2f(Reflected.Y, Reflected.X) + PI / 2;
        Position->Rotation = NewRotation;
        Position->X  -= MinX - 0.0001f;
      }
      else if(MaxX >= GameState->ScreenWidth - 1)
      {
        vec2f Reflected = Vec2f_Reflect(YAxis, V2f(-1,0));
        f32 NewRotation = atan2f(Reflected.Y, Reflected.X) + PI / 2;
        Position->Rotation = NewRotation;
        Position->X  -= MaxX - GameState->ScreenWidth + 1;
      }
      else{
      }
      Assert(Position->X >= 0);
      Assert(Position->X < GameState->ScreenWidth - 1);


    }
  }
}

void RemoveDeadUnits(game_state* GameState)
{
  query_result Query = EntityManager_Query(&GameState->EntityManager, HEALTH_MASK | TYPE_MASK);
  for (s32 EntityIndex = 0; EntityIndex < Query.Count; EntityIndex++)
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
        type_component* Type     = EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, TYPE_ID);
        if(Type->Type == EntityType_Enemy)
        {
          Inv_PlaySound(GameState, "explosion");
        }
        EntityManager_Remove_Entity(&GameState->EntityManager, Entity, "Dead");
      }
    }
  }
}

f32 GetNextShootTime(f32 CurrentTime)
{
  f32 Cooldown = 3.0f;
  return rand() / (f32)RAND_MAX * Cooldown + CurrentTime;
}

void SpawnEnemy(game_state* GameState, position_component Position)
{
  u32                Mask      = COLLIDER_MASK | RENDER_MASK | POSITION_MASK | HEALTH_MASK | VELOCITY_MASK | TYPE_MASK |SHOOT_MASK;
  entity             Entity    = EntityManager_Create_Entity(&GameState->EntityManager, Mask);

  health_component Health      = {};
  Health.Health                = 1;
  render_component Render      = {};

  Render.Texture               = GetTextureByName(GameState, "spaceShips2");
  Render.FlippedZ              = false;
  collider_component Collider  = {};
  Collider.Extents             = V2f(Render.Texture->Width * 0.5f, Render.Texture->Height * 0.5f);
  velocity_component Velocity = {};
  Velocity.Y = 200.0f;

  type_component Type = {};
  Type.Type = EntityType_Enemy;

  shoot_component Shoot = {};
  Shoot.TimeToShoot = GetNextShootTime(GameState->CommandBuffer.Time);
  EntityManager_AddComponents(&GameState->EntityManager, Entity, Mask, 7, &Health, &Position, &Velocity, &Render, &Collider, &Type, &Shoot);
}

void RemoveOutOfBoundsUnits(game_state* GameState)
{
  query_result Query         = EntityManager_Query(&GameState->EntityManager, POSITION_MASK | COLLIDER_MASK | TYPE_MASK);

  vec2f        ScreenExtents = V2f(GameState->ScreenWidth * 0.5f, GameState->ScreenHeight * 0.5f);
  vec2f        ScreenCenter  = V2f(GameState->ScreenWidth * 0.5f, GameState->ScreenHeight * 0.5f);
  for (s32 QueryIndex = 0; QueryIndex < Query.Count; QueryIndex++)
  {
    entity              Entity   = Query.Ids[QueryIndex];
    position_component* Position = EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);
    collider_component* Collider = EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, COLLIDER_ID);
    type_component    * Type     = EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, TYPE_ID);
    if (Type->Type != EntityType_Enemy && !Collision_Rect_Rect(ScreenExtents, ScreenCenter, Collider->Extents, V2f(Position->X, Position->Y)))
    {
      EntityManager_Remove_Entity(&GameState->EntityManager, Entity, "Out of bounds!");
    }
  }
}

position_component GetRandomEnemySpawnPosition(game_state* GameState)
{
  position_component Result = {};
  Result.X                  = GameState->ScreenWidth * 0.5f;
  Result.Y                  = rand() / (f32)RAND_MAX * GameState->ScreenHeight * 0.25f;

  Result.Rotation           = 0; // (rand() / (f32)RAND_MAX * PI) - PI / 2;

  return Result;
}

void RespawnOutOfBoundsEnemies(game_state* GameState)
{

  query_result Query = EntityManager_Query(&GameState->EntityManager, COLLIDER_MASK | TYPE_MASK);
  u32          Count = Query.Count;
  for (u32 QueryIndex = 0; QueryIndex < Count; QueryIndex++)
  {
    entity              Entity   = Query.Ids[QueryIndex];
    collider_component* Collider = (collider_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, COLLIDER_ID);
    type_component * Type        = (type_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, TYPE_ID);
    if (Type->Type == EntityType_Enemy)
    {
      position_component* Position = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);

      if (Position->Y - Collider->Extents.Y > GameState->ScreenHeight)
      {
        position_component SpawnPosition = GetRandomEnemySpawnPosition(GameState);
        *Position                        = SpawnPosition;
      }
    }
  }
}

void CommandBuffer_PushCommand(command_buffer* Buffer, command Command)
{
  Buffer->Commands[Buffer->CommandCount++] = Command;
}

void CommandBuffer_PushSpawnEnemy(command_buffer* Buffer, f32 Time)
{
  command Command = {};
  Command.Time    = Time;
  Command.Type    = Command_SpawnEnemy;
  CommandBuffer_PushCommand(Buffer, Command);
}

void CommandBuffer_PushDecideSpawn(command_buffer* Buffer, f32 Time, u32 EnemiesToSpawn)
{
  command Command = {};
  Command.Time    = Time;
  Command.Type    = Command_DecideSpawn;
  Command.EnemiesToSpawn = EnemiesToSpawn;
  CommandBuffer_PushCommand(Buffer, Command);
}
void ExecuteNewCommands(game_state* GameState)
{
  command_buffer* Buffer = &GameState->CommandBuffer;
  Buffer->Time += GameState->DeltaTime;
  Assert(Buffer->Time > 0);
  for (s32 CommandIndex = Buffer->CommandCount - 1; CommandIndex >= 0; CommandIndex--)
  {
    command* Command = &Buffer->Commands[CommandIndex];
    if (Command->Time < Buffer->Time)
    {
      switch (Command->Type)
      {
      case Command_SpawnEnemy:
      {
        position_component Position = GetRandomEnemySpawnPosition(GameState);
        SpawnEnemy(GameState, Position);
        break;
      }
      case Command_DecideSpawn:
      {
        for(u32 EnemyIndex = 0; EnemyIndex < Command->EnemiesToSpawn; EnemyIndex++){
            f32 AdditionalTime = rand() / (f32)RAND_MAX * 10.0f;
            CommandBuffer_PushSpawnEnemy(Buffer, Buffer->Time + AdditionalTime);
        }
        f32 TimeUntilNextDecision = 10.0f;
        u32 NextNumberOfEnemiesToSpawn = Command->EnemiesToSpawn >= 10 ? 10 : Command->EnemiesToSpawn + 2;
        CommandBuffer_PushDecideSpawn(Buffer, Buffer->Time + TimeUntilNextDecision, NextNumberOfEnemiesToSpawn);
        break;
      }
      default:
      {
        Assert(0 && "Unimplemented command type!");
      }
      }

      Buffer->CommandCount--;
      Buffer->Commands[CommandIndex] = Buffer->Commands[Buffer->CommandCount];

    }
  }
}

void CleanupEntities(game_state* GameState)
{
  RemoveDeadUnits(GameState);
  RespawnOutOfBoundsEnemies(GameState);
  RemoveOutOfBoundsUnits(GameState);
}

void RenderHealth(game_state * GameState, pushbuffer * Pushbuffer)
{
  health_component * Health = EntityManager_GetComponentFromEntity(&GameState->EntityManager, GameState->PlayerEntity, HEALTH_ID);
  f32 X = 25, Y = 25;
  f32 XOffset = 30;


  texture * FilledHeartTexture = GetTextureByName(GameState, "filledHeart");
  texture * UnfilledHeartTexture = GetTextureByName(GameState, "unfilledHeart");

  // ToDo find somewhere else?
  s32 MaxHealth = 3;
  for(s32 HealthIndex = 0; HealthIndex < Health->Health; HealthIndex++)
  {
    vec2f Origin = {};
    Origin.X = X - 0.5f * FilledHeartTexture->Width;
    Origin.Y = Y - 0.5f * FilledHeartTexture->Height;
    vec2f XAxis = V2f((f32)FilledHeartTexture->Width, 0);
    vec2f YAxis = V2f(0, (f32)FilledHeartTexture->Height);
    Pushbuffer_PushRectTexture(Pushbuffer, FilledHeartTexture, Origin, XAxis, YAxis, true);
    X += XOffset;

  }
  for(s32 HealthIndex = Health->Health; HealthIndex < MaxHealth; HealthIndex++)
  {
    vec2f Origin = {};
    Origin.X = X - 0.5f * UnfilledHeartTexture->Width;
    Origin.Y = Y - 0.5f * UnfilledHeartTexture->Height;
    vec2f XAxis = V2f((f32)UnfilledHeartTexture->Width, 0);
    vec2f YAxis = V2f(0, (f32)UnfilledHeartTexture->Height);
    Pushbuffer_PushRectTexture(Pushbuffer, UnfilledHeartTexture, Origin, XAxis, YAxis, true);
    X += XOffset;
  }
}

void HandleEnemyShooting(game_state * GameState)
{
  query_result Query = EntityManager_Query(&GameState->EntityManager, SHOOT_MASK | TYPE_MASK);
  for(s32 QueryIndex = 0; QueryIndex < Query.Count; QueryIndex++)
  {
    entity Entity         = Query.Ids[QueryIndex];
    shoot_component *Shoot = (shoot_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, SHOOT_ID);
    type_component * Type = (type_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, TYPE_ID);
    if(Shoot->TimeToShoot <= GameState->CommandBuffer.Time && Type->Type == EntityType_Enemy)
    {
      position_component * EnemyPosition = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);

      vec2f YAxis = {};
      YAxis.X = cosf(EnemyPosition->Rotation + PI / 2);
      YAxis.Y = sinf(EnemyPosition->Rotation + PI / 2);


      u32                BulletMask =  RENDER_MASK | POSITION_MASK | VELOCITY_MASK | COLLIDER_MASK | TYPE_MASK;
      entity             Bullet     =  EntityManager_Create_Entity(&GameState->EntityManager, BulletMask);
      position_component Position   =  *EnemyPosition;
      vec2f PositionOffset          =  Vec2f_Scale(YAxis, 40);
      Position.X                    += PositionOffset.X;
      Position.Y                    += PositionOffset.Y;


      render_component Render     = {};
      Render.Alpha                = 1.0f;

      Render.Texture              = GetTextureByName(GameState, "spaceMissiles2");
      Render.FlippedZ             = true;
      velocity_component Velocity = {};
      Velocity.Y                  = 400.0f;
      collider_component Collider = {};


      type_component Type = {};
      Type.Type = EntityType_Bullet_Enemy;
      EntityManager_AddComponents(&GameState->EntityManager, Bullet, BulletMask, 5, &Position, &Velocity, &Render, &Collider, &Type);
      Shoot->TimeToShoot  = GetNextShootTime(GameState->CommandBuffer.Time);
    }
  }
}



GAME_UPDATE(GameUpdate)
{

  game_state* GameState = (game_state*)Memory->PermanentStorage;
  Assert(GameState->CommandBuffer.Time >= 0);
  GameState->DeltaTime  = Memory->DeltaTime;
  GameState->Score += GameState->DeltaTime * 1000;
  Pushbuffer_PushClear(Pushbuffer, 0xFF00FFFF);
  if (!Memory->IsInitialized)
  {
    // Initialize GameArena

    GameState->ScreenWidth   = Memory->ScreenWidth;
    GameState->ScreenHeight  = Memory->ScreenHeight;
    u64 PermanentStorageSize = Memory->PermanentSize - (sizeof(game_state));
    Arena_Create(&GameState->PermanentArena, (u8*)Memory->PermanentStorage + sizeof(game_state), PermanentStorageSize);

    u64 TemporaryStorageSize = Memory->TemporaryStorageSize;
    Arena_Create(&GameState->TemporaryArena, Memory->TemporaryStorage, TemporaryStorageSize);

    GameState->CommandBuffer.MaxCommands  = 128;
    GameState->CommandBuffer.Commands     = Arena_Allocate(&GameState->PermanentArena, sizeof(command) * GameState->CommandBuffer.MaxCommands);
    GameState->CommandBuffer.CommandCount = 0;

    CommandBuffer_PushDecideSpawn(&GameState->CommandBuffer, 0, 2);

    LoadTextures(GameState, Memory);
    LoadSounds(GameState, Memory);
    EntityManager_Create(&GameState->PermanentArena, &GameState->EntityManager, 256, 7, sizeof(health_component), sizeof(position_component), sizeof(velocity_component), sizeof(render_component),
                         sizeof(collider_component), sizeof(type_component), sizeof(shoot_component));
    CreatePlayer(GameState);
    Memory->IsInitialized = true;
  }
  // Omega slow :)
  // Arena_Clear(&GameState->TemporaryArena);


  UseInput(GameState, Input);
  ExecuteNewCommands(GameState);
  UpdatePhysics(GameState);
  HandleEnemyShooting(GameState);
  CollisionDetection(GameState, Pushbuffer);
  CleanupEntities(GameState);


  RenderObjects(GameState, Pushbuffer);
  RenderHealth(GameState, Pushbuffer);

}

#include <math.h>
#define TAU (PI * 2)
void OutputSineWave(u32 SamplesPerSecond, u32 SampleCount, f32 * Samples, f32 ToneHz, f32 ToneVolume)
{
  static f64 TSine = 0;


  f32 WavePeriod = SamplesPerSecond / ToneHz;

  for(u32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++)
  {
    f32 Sine = (f32)sin(TSine);
    f32 Sample = (f32)(Sine * ToneVolume);
    *Samples++ = Sample;
    *Samples++ = Sample;

    TSine += TAU / WavePeriod;
    if(TSine >= TAU){
      TSine -= TAU;
    }

  }
}



GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
  game_state* GameState = (game_state*)Memory->PermanentStorage;



  f32 * Buffer = Audio->Buffer;
  u32 BufferSampleFrameIndex = 0;
  for(u32 SampleFrameIndex = 0; SampleFrameIndex < SampleFramesToWrite; SampleFrameIndex++)
  {
    Buffer[((Audio->SampleFrameIndexGameCode + SampleFrameIndex) % Audio->SampleFrameCount)* 2 + 0] = 0;
    Buffer[((Audio->SampleFrameIndexGameCode + SampleFrameIndex) % Audio->SampleFrameCount)* 2 + 1] = 0;
  }
  f32 Volume = 0.1f;

  for(s32 PlayingSoundIndex = GameState->PlayingSoundCount - 1; PlayingSoundIndex >= 0; PlayingSoundIndex--)
  {
    u32 BufferSampleFrameIndex = Audio->SampleFrameIndexGameCode;
    playing_sound * PlayingSound = &GameState->PlayingSounds[PlayingSoundIndex];
    sound * Sound = PlayingSound->Sound;

    // ToDo looping!
    u32 FramesRemaining = Sound->SampleFrameCount - PlayingSound->SampleFramesPlayed;
    u32 FramesToWrite = FramesRemaining < SampleFramesToWrite ? FramesRemaining : SampleFramesToWrite;

    for(u32 SampleFrameIndex = 0; SampleFrameIndex < FramesToWrite; SampleFrameIndex++)
    {
      switch(Sound->Channels)
      {
        case 1:
        {
          Buffer[BufferSampleFrameIndex * 2 + 0] += Sound->Samples[PlayingSound->SampleFramesPlayed] * Volume;
          Buffer[BufferSampleFrameIndex * 2 + 1] += Sound->Samples[PlayingSound->SampleFramesPlayed] * Volume;
          break;
        }
        case 2:
        {
          Buffer[BufferSampleFrameIndex * 2 + 0] += Sound->Samples[PlayingSound->SampleFramesPlayed * 2 + 0] * Volume;
          Buffer[BufferSampleFrameIndex * 2 + 1] += Sound->Samples[PlayingSound->SampleFramesPlayed * 2 + 1] * Volume;
          break;
        }
      }


      Assert(PlayingSound->SampleFramesPlayed < Sound->SampleFrameCount);
      PlayingSound->SampleFramesPlayed++;

      BufferSampleFrameIndex++;
      BufferSampleFrameIndex = BufferSampleFrameIndex % Audio->SampleFrameCount;
    }

    Assert(PlayingSound->SampleFramesPlayed <= Sound->SampleFrameCount);

    if(PlayingSound->SampleFramesPlayed >= Sound->SampleFrameCount)
    {
      // ToDo 0 fill?
      --GameState->PlayingSoundCount;
      GameState->PlayingSounds[PlayingSoundIndex] = GameState->PlayingSounds[GameState->PlayingSoundCount];
    }
  }


  Audio->SampleFrameIndexGameCode = (Audio->SampleFrameIndexGameCode + SampleFramesToWrite) % Audio->SampleFrameCount;
}

