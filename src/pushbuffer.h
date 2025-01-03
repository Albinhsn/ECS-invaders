
#ifndef PUSHBUFFER_H
#define PUSHBUFFER_H

#include "common.h"

typedef struct pushbuffer
{
  void * Memory;
  u64 Size;
  u64 AllocatedOffset;
  u64 ReadOffset;
} pushbuffer;

typedef enum pushbuffer_entry_type
{
  Pushbuffer_Entry_Clear
} pushbuffer_entry_type;

typedef struct pushbuffer_entry_clear{
  u32 Color;
} pushbuffer_entry_clear;

#endif
