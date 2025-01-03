#include "renderer_software.h"
#include "pushbuffer.c"

void Software_Renderer_Create(software_renderer* Renderer, void* Buffer, u16 ScreenWidth, u16 ScreenHeight)
{
  Renderer->Width  = ScreenWidth;
  Renderer->Height = ScreenHeight;
  Renderer->Buffer = Buffer;
}

void Software_Renderer_Clear(software_renderer* Renderer, u32 Color)
{

  u64 BufferSize = Renderer->Width * Renderer->Height;
  for (int i = 0; i < BufferSize; i++)
  {
    Renderer->Buffer[i] = Color;
  }
}

void Software_Renderer_Render(software_renderer* Renderer, pushbuffer* Pushbuffer)
{
  while (Pushbuffer->ReadOffset < Pushbuffer->AllocatedOffset)
  {
    pushbuffer_entry_type EntryType = Pushbuffer_ReadEntryType(Pushbuffer);
    switch (EntryType)
    {
    case Pushbuffer_Entry_Clear:
    {
      pushbuffer_entry_clear Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_clear);
      Software_Renderer_Clear(Renderer, Entry.Color);
      break;
    }
    default:
    {
      Assert(0 && "Unknown pushbuffer entry!");
    }
    }
  }
}
