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
  for (s32 i = 0; i < BufferSize; i++)
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
    case Pushbuffer_Entry_Rect_Color:
    {
      s32                         Width  = Renderer->Width;
      s32                         Height = Renderer->Height;
      pushbuffer_entry_rect_color Entry  = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_color);

      u32                         MinX   = Entry.Min.X < 0 ? 0 : Entry.Min.X;
      u32                         MinY   = Entry.Min.Y < 0 ? 0 : Entry.Min.Y;

      u32                         MaxX   = Entry.Max.X > Width ? Width : Entry.Max.X;
      u32                         MaxY   = Entry.Max.Y > Height ? Height : Entry.Max.Y;

      u32*                        Buffer = Renderer->Buffer;

      u32                         Color  = Entry.Color;
      for (u32 Y = MinY; Y < MaxY; Y++)
      {
        for (u32 X = MinX; X < MaxX; X++)
        {
          *(Buffer + Y * Width + X) = Color;
        }
      }

      break;
    }
    case Pushbuffer_Entry_Rect_Texture:
    {
      s32                           Width       = Renderer->Width;
      s32                           Height      = Renderer->Height;
      pushbuffer_entry_rect_texture Entry       = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_texture);

      u32                           MinX        = Entry.Min.X < 0 ? 0 : Entry.Min.X;
      u32                           MinY        = Entry.Min.Y < 0 ? 0 : Entry.Min.Y;

      u32                           MaxX        = Entry.Max.X > Width ? Width : Entry.Max.X;
      u32                           MaxY        = Entry.Max.Y > Height ? Height : Entry.Max.Y;

      u32*                          Buffer      = Renderer->Buffer;
      u32*                          ImageBuffer = (u32*)Entry.Memory;

      u32                           ImageWidth  = Entry.Max.X - Entry.Min.X;

      for (u32 Y = MinY; Y < MaxY; Y++)
      {
        for (u32 X = MinX; X < MaxX; X++)
        {
          u32 Color                 = *(ImageBuffer + (Y - MinY) * ImageWidth + (X - MinX));
          *(Buffer + Y * Width + X) = Color;

          if (Color != 0)
          {
            int a = 5;
          }
        }
      }

      break;
    }
    default:
    {
      Assert(0 && "Unknown pushbuffer entry!");
    }
    }
  }
}
