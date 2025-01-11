#include "pushbuffer.h"
#include "common.h"

void Pushbuffer_EntryCheckSpace(pushbuffer* Pushbuffer, u64 Size)
{
  if (Pushbuffer->AllocatedOffset + Size + sizeof(pushbuffer_entry_type) > Pushbuffer->Size)
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
void Pushbuffer_PushRectColor(pushbuffer* Pushbuffer, vec2f Origin, vec2f XAxis, vec2f YAxis, u32 Color)
{

  Pushbuffer_EntryCheckSpace(Pushbuffer, sizeof(pushbuffer_entry_rect_color));

  pushbuffer_entry_rect_color Entry = {};
  Entry.Color                       = Color;
  Entry.Origin                      = Origin;
  Entry.XAxis                       = XAxis;
  Entry.YAxis                       = YAxis;
  Pushbuffer_Write(Pushbuffer, Pushbuffer_Entry_Rect_Color, pushbuffer_entry_type);
  Pushbuffer_Write(Pushbuffer, Entry, pushbuffer_entry_rect_color);
}
void Pushbuffer_PushRectTexture(pushbuffer* Pushbuffer, texture* Texture, vec2f Origin, vec2f XAxis, vec2f YAxis, bool FlippedZ)
{

  Pushbuffer_EntryCheckSpace(Pushbuffer, sizeof(pushbuffer_entry_rect_texture));
  pushbuffer_entry_rect_texture Entry = {};

  Entry.Texture                       = Texture;
  Entry.Origin                        = Origin;
  Entry.XAxis                         = XAxis;
  Entry.YAxis                         = YAxis;
  Entry.FlippedZ                      = FlippedZ;
  Pushbuffer_Write(Pushbuffer, Pushbuffer_Entry_Rect_Texture, pushbuffer_entry_type);
  Pushbuffer_Write(Pushbuffer, Entry, pushbuffer_entry_rect_texture);
}

void Pushbuffer_PushClear(pushbuffer* Pushbuffer, u32 Color)
{

  Pushbuffer_EntryCheckSpace(Pushbuffer, sizeof(pushbuffer_entry_clear));

  pushbuffer_entry_clear Entry = {};
  Entry.Color                  = Color;
  Pushbuffer_Write(Pushbuffer, Pushbuffer_Entry_Clear, pushbuffer_entry_type);
  Pushbuffer_Write(Pushbuffer, Entry, pushbuffer_entry_clear);
}

void Pushbuffer_PushText(pushbuffer* Pushbuffer, string *Text, msdf_font* Font, ui_text_alignment Alignment, vec2f Position, u32 Size, u32 Color)
{
  Pushbuffer_EntryCheckSpace(Pushbuffer, sizeof(pushbuffer_entry_text));
  Pushbuffer_Write(Pushbuffer, Pushbuffer_Entry_Text, pushbuffer_entry_type);
  pushbuffer_entry_text Entry = {};
  Entry.Text                  = Text;
  Entry.Font                  = Font;
  Entry.Size                  = Size;
  Entry.Position              = Position;
  Entry.Alignment             = Alignment;
  Entry.Color                 = Color;

  Pushbuffer_Write(Pushbuffer, Entry, pushbuffer_entry_text);
}
