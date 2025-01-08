#ifndef SOUND_H

#include "common.h"

typedef struct sound
{
  f32 * Samples;
  u32 SampleRate;
  u32 ByteRate;
  u32 BlockAlign;
  u32 BitsPerSample;
  u32 SampleFrameCount;
  u16 Channels;

} sound;

typedef struct playing_sound
{
  f32 Volume;
  u32 Id;
  u32 SamplesPlayed;
} playing_sound;

#endif
