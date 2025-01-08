#ifndef SOUND_H

#include "common.h"

typedef struct sound
{
  u32 Channels;
  u32 SampleRate;
  u32 ByteRate;
  u32 BlockAlign;
  u32 BitsPerSample;
  u16 * Samples;
  u16 SampleCount;
} sound;

typedef struct playing_sound
{
  f32 Volume;
  u32 Id;
  u32 SamplesPlayed;
} playing_sound;

#endif
