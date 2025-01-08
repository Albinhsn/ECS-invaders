#include "sound.h"
#include "winsock.h"

typedef struct file_buffer{
  u8 * Buffer;
  u32 Length;
  u32 Index;
}file_buffer;

u16 FileBuffer_ParseU16BE(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u16) <= Buffer->Length); // Assume that the string i null terminated as well, so file content == Length

  u16 Result = *(u16*)&Buffer->Buffer[Buffer->Index];
  Buffer->Index += sizeof(u16);

  return htons(Result);
}

u32 FileBuffer_ParseU32BE(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u16) <= Buffer->Length);

  u32 Result = *(u32*)&Buffer->Buffer[Buffer->Index];
  Buffer->Index += sizeof(u32);

  return htonl(Result);
}
u32 FileBuffer_ParseU32(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u16) <= Buffer->Length);

  u32 Result = *(u32*)&Buffer->Buffer[Buffer->Index];
  Buffer->Index += sizeof(u32);

  return Result;
}

u16 FileBuffer_ParseU16(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u16) <= Buffer->Length);

  u16 Result = *(u16*)&Buffer->Buffer[Buffer->Index];
  Buffer->Index += sizeof(u16);

  return Result;
}
u8 FileBuffer_ParseU8(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u8) <= Buffer->Length);

  u8 Result = *(u8*)&Buffer->Buffer[Buffer->Index];
  Buffer->Index += sizeof(u8);

  return Result;
}
s32 FileBuffer_ParseS24(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u16) + sizeof(u8) <= Buffer->Length);

  u8 B0 = FileBuffer_ParseU8(Buffer);
  u8 B1 = FileBuffer_ParseU8(Buffer);
  u8 B2 = FileBuffer_ParseU8(Buffer);
  s32 Result = (B0 << 8) | (B1 << 16) | (B2 << 24);
  Result >>= 8;

  return Result;
}

void FileBuffer_Advance(file_buffer * Buffer, u32 Count)
{
  Buffer->Index += Count;
}

bool Sound_ParseWave(arena * Arena, sound * Sound, u8 * Buffer, u32 Length)
{
  Assert(Length > 36 && "not right size?");
  file_buffer FileBuffer = {};
  FileBuffer.Buffer = Buffer;
  FileBuffer.Length = Length;
  FileBuffer.Index  = 0;

  // Skip the RIFF Header
  FileBuffer_Advance(&FileBuffer, 4);

  // Size of the file - 8 bytes
  u32 ChunkSize = FileBuffer_ParseU32(&FileBuffer);
  Assert(Length - 8 == ChunkSize);

  // Skip WAVE
  FileBuffer_Advance(&FileBuffer, 4);

  // Skip Subchunk1 ID
  FileBuffer_Advance(&FileBuffer, 4);
  u32 Subchunk1Size     = FileBuffer_ParseU32(&FileBuffer);
  u16 AudioFormat       = FileBuffer_ParseU16(&FileBuffer);
  Assert(AudioFormat == 1); // PCM!
  Sound->Channels          = FileBuffer_ParseU16(&FileBuffer);
  Sound->SampleRate        = FileBuffer_ParseU32(&FileBuffer);
  Sound->ByteRate          = FileBuffer_ParseU32(&FileBuffer);
  Sound->BlockAlign         = FileBuffer_ParseU16(&FileBuffer);
  Sound->BitsPerSample     = FileBuffer_ParseU16(&FileBuffer);

  u32 DATA = 0x64617461;

  bool NotFoundData = true;
  u32 RanFor =0;
  while(NotFoundData){
    u32 ID          = FileBuffer_ParseU32BE(&FileBuffer);
    u8 A = (u8)((ID >> 24) & 0xFF);
    u8 B = (u8)((ID >> 16) & 0xFF);
    u8 C = (u8)((ID >> 8) & 0xFF);
    u8 D = (u8)(ID & 0xFF);

    // u8 IDAscii[4] = {A,B,C,D};
    u32 ChunkSize   = FileBuffer_ParseU32(&FileBuffer);
    if(ID == DATA){
      NotFoundData = false;
      Sound->SampleFrameCount = ChunkSize / (Sound->BitsPerSample / 8  * Sound->Channels);
      Sound->Samples = (f32*)Arena_Allocate(Arena, Sound->SampleFrameCount * Sound->Channels * sizeof(f32));
      for(u32 SampleIndex = 0; SampleIndex < Sound->SampleFrameCount * Sound->Channels; SampleIndex++)
      {
        switch(Sound->BitsPerSample)
        {
          case 24:
          {
            s32 Sample = FileBuffer_ParseS24(&FileBuffer);
            f32 SampleF32 = Sample /(f32)(1 << 23);
            Sound->Samples[SampleIndex] =SampleF32;
            break;
          }
          default:{
            Assert(0 && "Unimplemented");
          }
        }
      }
    }else{
      FileBuffer_Advance(&FileBuffer, ChunkSize);
    }
  }


  return true;
}