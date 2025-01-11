#include "sound.h"

#include <winsock.h>
#include <limits.h>


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
      NotFoundData            = false;
      Sound->SampleFrameCount = ChunkSize / (Sound->BitsPerSample / 8  * Sound->Channels);
      u32 SampleCount         = Sound->SampleFrameCount * Sound->Channels;
      Sound->Samples          = (f32*)Arena_Allocate(Arena,  SampleCount * sizeof(f32));
      for(u32 SampleIndex = 0; SampleIndex < SampleCount; SampleIndex++)
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
          case 16:
          {
            s16 Sample = FileBuffer_ParseU16(&FileBuffer);
            f32 SampleF32 = Sample /(f32)(SHRT_MAX);
            Sound->Samples[SampleIndex] = SampleF32;
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
