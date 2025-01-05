#include "invaders.h"
#include "image.c"
#include "platform.h"
#include "entity.c"
#include "pushbuffer.c"
#include <debugapi.h>

texture * GetTextureByName(game_state * GameState, u8 * TextureName)
{

  string Name = {};
  Name.Buffer = TextureName;
  Name.Length = String_Length(TextureName);
  for(u32 TextureIndex = 0; TextureIndex < GameState->TextureCount; TextureIndex++)
  {
    string n = GameState->Textures[TextureIndex].Name;
    if(String_Compare(&Name, &n)){
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
    if(Result == false){
      Assert(0 && "Failed to read texture!")
    } 

    targa_image Image = {};
    Image_LoadTarga(&GameState->PermanentArena, &Image, TextureBuffer, TextureBufferSize);
    GameState->Textures[TextureIndex].Memory = Image.Buffer;
    GameState->Textures[TextureIndex].Width = Image.Width;
    GameState->Textures[TextureIndex].Height = Image.Height;

    TextureLocationBuffer[Offset] = c;
    while(Offset < TextureLocationSize && TextureLocationBuffer[Offset] == ' '){
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
    while(Offset < TextureLocationSize && (TextureLocationBuffer[Offset] == ' ' ||TextureLocationBuffer[Offset] == '\n'))
    {
      Offset++;
    }
  }
}

void Render(game_state * GameState, pushbuffer * Pushbuffer)
{
  query_result Query = EntityManager_Query(&GameState->EntityManager, POSITION_MASK | RENDER_MASK);
  for(u32 QueryIndex = 0; QueryIndex < Query.Count; QueryIndex++){
    entity Entity = Query.Ids[QueryIndex];

    position_component * Position = (position_component*)EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, POSITION_ID);
    render_component * Render     = (render_component*) EntityManager_GetComponentFromEntity(&GameState->EntityManager, Entity, RENDER_ID);

    vec2i Min = V2i((s32)(Position->Position.X - Render->Texture->Width * 0.5f),(s32)(Position->Position.Y - Render->Texture->Height * 0.5f));
    vec2i Max = V2i((s32)(Position->Position.X + Render->Texture->Width * 0.5f),(s32)(Position->Position.Y + Render->Texture->Height * 0.5f));
    Pushbuffer_Push_Rect_Texture(Pushbuffer, Render->Texture->Memory, Min, Max);

  }

}

void CreatePlayer(game_state * GameState){

  u32 Mask = POSITION_MASK | HEALTH_MASK | RENDER_MASK | COLLIDER_MASK | VELOCITY_MASK;
  entity Entity = EntityManager_Create_Entity(&GameState->EntityManager, Mask);

  health_component Health = {};
  Health.Health = 3;
  position_component Position = {};
  Position.Position = V2f(200, 550);
  velocity_component Velocity= {};
  render_component Render= {};
  const char * PlayerTextureName = "spaceShips1";
  Render.Texture = GetTextureByName(GameState, (u8*)PlayerTextureName);
  Render.Alpha = 1;
  collider_component Collider= {};
  Collider.Extents = V2f(Render.Texture->Width * 0.5f, Render.Texture->Height  * 0.5f);
  EntityManager_AddComponents(&GameState->EntityManager, Entity, Mask, 5, &Health, &Position, &Velocity, &Render, &Collider);

}

GAME_UPDATE(GameUpdate)
{
  Pushbuffer_PushClear(Pushbuffer, 0xFF00FFFF);
  game_state* GameState = (game_state*)Memory->PermanentStorage;
  if (!Memory->IsInitialized)
  {
    // Initialize GameArena

    u64 PermanentStorageSize = Memory->PermanentSize - (sizeof(game_state));
    Arena_Create(&GameState->PermanentArena, (u8*)Memory->PermanentStorage + sizeof(game_state), PermanentStorageSize);

    u64 TemporaryStorageSize = Memory->TemporaryStorageSize;
    Arena_Create(&GameState->TemporaryArena, Memory->TemporaryStorage, TemporaryStorageSize);

    LoadTextures(GameState, Memory);


    EntityManager_Create(&GameState->PermanentArena, &GameState->EntityManager, 256, 5, sizeof(health_component), sizeof(position_component), sizeof(velocity_component), sizeof(render_component), sizeof(collider_component));
    CreatePlayer(GameState);
    Memory->IsInitialized     = true;
  }
  // Omega slow :)
  // Arena_Clear(&GameState->TemporaryArena);


  // Use input
  //  i.e set velocity and figure out if we shoot
  // Update physics
  // Collision detection and collision response

  Render(GameState, Pushbuffer);

}
