
#ifndef PUSHBUFFER_H
#define PUSHBUFFER_H

#include "common.h"
#include "vector.h"

typedef struct pushbuffer
{
  void* Memory;
  u64   Size;
  u64   AllocatedOffset;
  u64   ReadOffset;
} pushbuffer;

typedef enum pushbuffer_entry_type
{
  Pushbuffer_Entry_Clear,
  Pushbuffer_Entry_Rect_Color,
  Pushbuffer_Entry_Rect_Texture
} pushbuffer_entry_type;

typedef struct pushbuffer_entry_clear
{
  u32 Color;
} pushbuffer_entry_clear;

typedef struct pushbuffer_entry_rect_color
{
  vec2i Min;
  vec2i Max;
  u32   Color;
} pushbuffer_entry_rect_color;

typedef struct pushbuffer_entry_rect_texture
{
  // It's implied that the width / height are min/max values
  vec2i Min;
  vec2i Max;
  void * Memory;
} pushbuffer_entry_rect_texture;

#endif
