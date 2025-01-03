#ifndef COMMON_H
#define COMMON_H

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

typedef struct arena
{
  void* Memory;
  u64   Size;
  u64   Offset;
} arena;

void Arena_Create(arena * Arena, void * Memory,u64 Size)
{
  Arena->Memory = Memory;
  Arena->Size = Size;
  Arena->Offset = 0;
}

void * Arena_Allocate(arena * Arena, u64 Size)
{

  // ToDo Check this?
  void * Pointer = (void*)((u8*)Arena->Memory + Arena->Offset);
  Arena->Offset += Size;

  return Pointer;

}

void Arena_Reset(arena * Arena){
  Arena->Offset = 0;
}
void Arena_Clear(arena * Arena)
{
  Arena_Reset(Arena);
  u8 * Memory=  (u8*)Arena->Memory;
  for(int i = 0; i < Arena->Size; i++){
    Memory[i] = 0;
  }
}



#endif
