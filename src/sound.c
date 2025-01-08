#include "sound.h"
#include "winsock.h"

typedef struct file_buffer{
  u8 * Buffer;
  u32 Length;
  u32 Index;
}file_buffer;

u16 FileBuffer_ParseU16LE(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u16) < Buffer->Length);

  u16 Result = *(u16*)&Buffer->Buffer[Buffer->Index];
  Buffer->Index += sizeof(u16);

  return htons(Result);
}

u32 FileBuffer_ParseU32LE(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u16) < Buffer->Length);

  u32 Result = *(u32*)&Buffer->Buffer[Buffer->Index];
  Buffer->Index += sizeof(u32);

  return htonl(Result);
}
u32 FileBuffer_ParseU32(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u16) < Buffer->Length);

  u32 Result = *(u32*)&Buffer->Buffer[Buffer->Index];
  Buffer->Index += sizeof(u32);

  return Result;
}
u16 FileBuffer_ParseU16(file_buffer * Buffer)
{
  Assert(Buffer->Index + sizeof(u16) < Buffer->Length);

  u16 Result = *(u16*)&Buffer->Buffer[Buffer->Index];
  Buffer->Index += sizeof(u16);

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
  u16 Channels          = FileBuffer_ParseU16(&FileBuffer);
  u32 SampleRate        = FileBuffer_ParseU32(&FileBuffer);
  u32 ByteRate          = FileBuffer_ParseU32(&FileBuffer);
  u16 ByteAlign         = FileBuffer_ParseU16(&FileBuffer);
  u16 BitsPerSample     = FileBuffer_ParseU16(&FileBuffer);

  FileBuffer_Advance(&FileBuffer, 4);
  u32 Subchunk2Size     = FileBuffer_ParseU32(&FileBuffer);

  // Data here :)
  int a =5;
  return 0;
}