#ifndef COMMON_H
#define COMMON_H

#include "windows.h"
#include <stdint.h>
#include <stdio.h>
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

#define true              1
#define false             0

#define bool              u32
#define PI 3.141592653589793f

#define Kilobyte(size)    (size * 1024LL)
#define Megabyte(size)    (Kilobyte(size) * 1024LL)
#define Gigabyte(size)    (Megabyte(size) * 1024LL)
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define Assert(Expr)                                                                                                                                                                                   \
  {                                                                                                                                                                                                    \
    if (!(Expr))                                                                                                                                                                                       \
      int a = *(int*)0;                                                                                                                                                                                \
  }

typedef struct arena
{
  void* Memory;
  u64   Size;
  u64   Offset;
} arena;

typedef struct string
{
  u8* Buffer;
  u32 Length;
} string;

void String_Create(arena* Arena, string* String, u8* Buffer, u32 Length)
{
}




typedef struct texture texture;
struct texture
{
  void*  Memory;
  u32    Width;
  u32    Height;
  string Name;
};

typedef struct pool_free_node pool_free_node;
struct pool_free_node
{
  pool_free_node* Next;
};
typedef struct pool_allocator
{
  void*           Memory;
  u64             Size;
  u64             ChunkSize;
  pool_free_node* Head;
} pool_allocator;

void Memcpy(u8* Dest, u8* Src, u32 Count)
{

  for (u32 BufferIndex = 0; BufferIndex < Count; BufferIndex++)
  {
    Dest[BufferIndex] = Src[BufferIndex];
  }
}

void* Pool_Alloc(pool_allocator* Pool)
{
  pool_free_node* Node = Pool->Head;
  if (Node == 0)
  {
    Assert(0 && "Pool allocator has no memory");
    return 0;
  }

  Pool->Head = Node->Next;

  return memset(Node, 0, Pool->ChunkSize);
}

bool IsAlphaOrDigit(u8 Char)
{
  bool IsAlpha = (Char >= 'a' && Char <= 'z') || (Char >= 'A' && Char <= 'Z');
  bool IsDigit = (Char >= '0' && Char <= '9');
  return IsAlpha | IsDigit;
}

u32 String_Length(u8* Buffer)
{
  u32 Length = 0;
  while (Buffer[Length] != '\0')
  {
    Length++;
  }
  return Length;
}

bool String_Compare(string* s0, string* s1)
{
  if (s0->Length != s1->Length)
  {
    return false;
  }
  for (u32 CharIndex = 0; CharIndex < s0->Length; CharIndex++)
  {
    if (s0->Buffer[CharIndex] != s1->Buffer[CharIndex])
    {
      return false;
    }
  }
  return true;
}

void Pool_Free(pool_allocator* Pool, u64 Ptr)
{
  if (Ptr == 0)
  {
    return;
  }

  u64 Start = (u64)Pool->Memory;
  u64 End   = (u64)Pool->Memory + Pool->Size;

  if (!(Start <= Ptr && Ptr < End))
  {
    Assert(0 && "Memory is out of bounds of the buffer in this pool");
    return;
  }

  pool_free_node* Node = (pool_free_node*)Ptr;
  Node->Next           = Pool->Head;
  Pool->Head           = Node;
}
void Pool_Free_All(pool_allocator* Pool)
{
  u64 ChunkCount = Pool->Size / Pool->ChunkSize;

  for (u64 i = 0; i < ChunkCount; i++)
  {
    u8*             Ptr  = (u8*)Pool->Memory + Pool->ChunkSize * i;
    pool_free_node* Node = (pool_free_node*)Ptr;
    Node->Next           = Pool->Head;
    Pool->Head           = Node;
  }
}
u64 AlignOffset(u64 offset, u64 alignment)
{
  u64 modulo = offset & (alignment - 1);
  if (modulo != 0)
  {
    offset += alignment - modulo;
  }
  return offset;
}

#define DEFAULT_ALIGNMENT 16
void Pool_Create(pool_allocator* Pool, void* Memory, u64 ChunkSize)
{
  Pool->Memory    = Memory;
  Pool->ChunkSize = AlignOffset(ChunkSize, DEFAULT_ALIGNMENT);
  Pool->Head      = 0;

  Pool_Free_All(Pool);
}
#undef DEFAULT_ALIGNMENT

void Arena_Create(arena* Arena, void* Memory, u64 Size)
{
  Arena->Memory = Memory;
  Arena->Size   = Size;
  Arena->Offset = 0;
}
void Arena_Deallocate(arena* Arena, u64 Size)
{

  Arena->Offset -= Size;

  if (Arena->Offset >= Size)
  {
    Assert(0 && "Trying to deallocate past the offset?");
  }
  char Buffer[1024] = {};
  sprintf_s(Buffer, ArrayCount(Buffer), "Deallocated, Allocated %lld out of %lld (%.2f)\n", Arena->Offset, Arena->Size, Arena->Offset / (f32)Arena->Size);
  OutputDebugStringA(Buffer);
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
  sprintf_s(Buffer, ArrayCount(Buffer), "Allocated %lld out of %lld (%.2f)\n", Arena->Offset, Arena->Size, Arena->Offset / (f32)Arena->Size);
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

#define FileBuffer_ParseStruct(Buf, Struct) (Struct*)((Buf)->Buffer + (Buf)->Index);FileBuffer_Advance((Buf), sizeof(Struct))

#endif
