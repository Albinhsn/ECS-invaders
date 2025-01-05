#include "pushbuffer.h"
#include "common.h"

void Pushbuffer_CheckSpace(pushbuffer* Pushbuffer, u64 Size)
{
  if (Pushbuffer->AllocatedOffset + Size > Pushbuffer->Size)
  {
    Assert(0 && "Writing outside of the pushbuffer!");
  }
}

void Pushbuffer_Create(pushbuffer* Pushbuffer, void* Memory, u64 Size)
{
  Pushbuffer->AllocatedOffset = 0;
  Pushbuffer->Memory          = Memory;
  Pushbuffer->Size            = Size;
}

void Pushbuffer_Reset(pushbuffer* Pushbuffer)
{
  Pushbuffer->AllocatedOffset = 0;
  Pushbuffer->ReadOffset      = 0;
  u8* Memory                  = (u8*)Pushbuffer->Memory;
  for (int i = 0; i < Pushbuffer->Size; i++)
  {
    Memory[i] = 0;
  }
}

#define Pushbuffer_Write(Pushbuffer, Entry, EntryType)                                                                                                                                                 \
  *(EntryType*)((u8*)Pushbuffer->Memory + Pushbuffer->AllocatedOffset) = Entry;                                                                                                                        \
  Pushbuffer->AllocatedOffset += sizeof(EntryType);

#define Pushbuffer_Read(Pushbuffer, Entry)                                                                                                                                                             \
  *(Entry*)((u8*)Pushbuffer->Memory + Pushbuffer->ReadOffset);                                                                                                                                         \
  Pushbuffer->ReadOffset += sizeof(Entry);

pushbuffer_entry_type Pushbuffer_ReadEntryType(pushbuffer* Pushbuffer)
{
  pushbuffer_entry_type Type = *(pushbuffer_entry_type*)((u8*)Pushbuffer->Memory + Pushbuffer->ReadOffset);
  Pushbuffer->ReadOffset += sizeof(pushbuffer_entry_type);

  return Type;
}
void Pushbuffer_Push_Rect_Color(pushbuffer* Pushbuffer, vec2i Min, vec2i Max, u32 Color)
{

  Pushbuffer_CheckSpace(Pushbuffer, sizeof(pushbuffer_entry_rect_color));

  pushbuffer_entry_rect_color Entry = {};
  Entry.Color                       = Color;
  Entry.Min                         = Min;
  Entry.Max                         = Max;
  Pushbuffer_Write(Pushbuffer, Pushbuffer_Entry_Rect_Color, pushbuffer_entry_type);
  Pushbuffer_Write(Pushbuffer, Entry, pushbuffer_entry_rect_color);
}
void Pushbuffer_Push_Rect_Texture(pushbuffer* Pushbuffer, u32 *Memory, vec2i Min, vec2i Max, bool FlippedZ)
{

  Pushbuffer_CheckSpace(Pushbuffer, sizeof(pushbuffer_entry_rect_texture));
  pushbuffer_entry_rect_texture Entry = {};
  Entry.Memory = Memory;
  Entry.Min = Min;
  Entry.Max = Max;
  Entry.FlippedZ = FlippedZ;
  Pushbuffer_Write(Pushbuffer, Pushbuffer_Entry_Rect_Texture, pushbuffer_entry_type);
  Pushbuffer_Write(Pushbuffer, Entry, pushbuffer_entry_rect_texture);

  u32 Width = Max.X - Min.X;
  u32 Height = Max.Y - Min.Y;
}

void Pushbuffer_PushClear(pushbuffer* Pushbuffer, u32 Color)
{

  Pushbuffer_CheckSpace(Pushbuffer, sizeof(pushbuffer_entry_clear));

  pushbuffer_entry_clear Entry = {};
  Entry.Color                  = Color;
  Pushbuffer_Write(Pushbuffer, Pushbuffer_Entry_Clear, pushbuffer_entry_type);
  Pushbuffer_Write(Pushbuffer, Entry, pushbuffer_entry_clear);
}
