#include "invaders.h"
#include "image.c"
#include "platform.h"
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

    GameState->PlayerPosition = V2f(100, 100);
    Memory->IsInitialized     = true;
  }
  // Arena_Clear(&GameState->TemporaryArena);

  f32 Velocity = 10.0f;
  if (Input->Up)
  {
    GameState->PlayerPosition.Y -= Velocity / 60.0f;
  }
  if (Input->Down)
  {
    GameState->PlayerPosition.Y += Velocity / 60.0f;
  }
  if (Input->Left)
  {
    GameState->PlayerPosition.X -= Velocity / 60.0f;
  }
  if (Input->Right)
  {
    GameState->PlayerPosition.X += Velocity / 60.0f;
  }

  const char * PlayerTextureName = "spaceShips1";
  texture * PlayerTexture = GetTextureByName(GameState, (u8*)PlayerTextureName);
  vec2i Min = V2i((u32)(GameState->PlayerPosition.X - PlayerTexture->Width  / 2), (u32)(GameState->PlayerPosition.Y - PlayerTexture->Height / 2));
  vec2i Max = V2i((u32)(GameState->PlayerPosition.X + PlayerTexture->Width  / 2), (u32)(GameState->PlayerPosition.Y + PlayerTexture->Height / 2));
  Pushbuffer_Push_Rect_Texture(Pushbuffer, PlayerTexture->Memory, Min, Max);
}
