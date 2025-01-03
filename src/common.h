#ifndef COMMON_H
#define COMMON_H

#include "windows.h"
#include <stdio.h>
#include <stdint.h>
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef float    f32;
typedef double   f64;

#define Kilobyte(size) (size * 1024LL)
#define Megabyte(size) (Kilobyte(size) * 1024LL)
#define Gigabyte(size) (Megabyte(size) * 1024LL)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Assert(Expr)                                                                                                                                                                                   \
  if (!Expr)                                                                                                                                                                                           \
    int a = *(int*)0;

typedef struct arena
{
  void* Memory;
  u64   Size;
  u64   Offset;
} arena;

void Arena_Create(arena* Arena, void* Memory, u64 Size)
{
  Arena->Memory = Memory;
  Arena->Size   = Size;
  Arena->Offset = 0;
}

void* Arena_Allocate(arena* Arena, u64 Size)
{

  if (Arena->Offset + Size > Arena->Size)
  {
    Assert(0 && "Allocated outside of the arena!");
  }

  void* Pointer = (void*)((u8*)Arena->Memory + Arena->Offset);
  Arena->Offset += Size;
  char Buffer[1024] = {};
  sprintf_s(Buffer, ArrayCount(Buffer),"Allocated %lld out of %lld (%.2f)\n", Arena->Offset, Arena->Size, Arena->Offset / (f32)Arena->Size);
  OutputDebugStringA(Buffer);

  return Pointer;
}

void Arena_Reset(arena* Arena)
{
  Arena->Offset = 0;
}
void Arena_Clear(arena* Arena)
{
  Arena_Reset(Arena);
  u8* Memory = (u8*)Arena->Memory;
  for (int i = 0; i < Arena->Size; i++)
  {
    Memory[i] = 0;
  }
}


#endif
