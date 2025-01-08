#include "renderer_software.h"
#include "pushbuffer.c"
#include "vector.c"
#include "math.h"

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


      vec2f v0  = Entry.Origin;
      vec2f v1  = Vec2f_Add(Entry.Origin, Entry.XAxis);
      vec2f v2  = Vec2f_Add(Entry.Origin, Entry.YAxis);
      vec2f v3  = Vec2f_Add(v2, Entry.XAxis);

      s32 MinX =  10000;
      s32 MaxX = -10000;
      s32 MinY =  10000;
      s32 MaxY = -10000;

      vec2f Corners[4] = {v0, v1, v2, v3};
      for(u32 CornerIndex = 0; CornerIndex < ArrayCount(Corners); CornerIndex++)
      {
        vec2f Corner = Corners[CornerIndex];
        MinX = (s32)Min(Corner.X, (f32)MinX);
        MaxX = (s32)Max(Corner.X, (f32)MaxX);
        MinY = (s32)Min(Corner.Y, (f32)MinY);
        MaxY = (s32)Max(Corner.Y, (f32)MaxY);
      }

      MinX = MaxI(MinX, 0);
      MaxX = MinI(MaxX, Width - 1);
      MinY = MaxI(MinY, 0);
      MaxY = MinI(MaxY, Height - 1);


      u32*                        Buffer = Renderer->Buffer;

      u32                         Color  = Entry.Color;

      vec2f XAxis = Entry.XAxis;
      vec2f YAxis = Entry.YAxis;
      f32 XAxisLength = Vec2f_Length(XAxis);
      XAxisLength *= XAxisLength;
      f32 YAxisLength = Vec2f_Length(YAxis);
      YAxisLength *= YAxisLength;
      vec2f Origin = Entry.Origin;
      for (s32 Y = MinY; Y < MaxY; Y++)
      {
        for (s32 X = MinX; X < MaxX; X++)
        {
          f32 XEdge = Vec2f_Dot(V2f((f32)X - Origin.X,(f32)Y - Origin.Y), XAxis);
          f32 YEdge = Vec2f_Dot(V2f((f32)X - Origin.X,(f32)Y - Origin.Y), YAxis);

          bool WithinX = XEdge >= 0 && XEdge < XAxisLength;
          bool WithinY = YEdge >= 0 && YEdge < YAxisLength;
          if(WithinX && WithinY){
            *(Buffer + Y * Width + X) = Color;
          }
          else{
              *(Buffer + Y * Width + X) = 0x000000FF;
          }
          if(Abs(Y - v0.Y) < 5 && Abs(X - v0.X) < 5){
              *(Buffer + Y * Width + X) = 0x0000FF00;
          }
          if(Abs(Y - v1.Y) < 5 && Abs(X - v1.X) < 5){
            *(Buffer + Y * Width + X) = 0x0000FF00;
          }
          if(Abs(Y - v2.Y) < 5 && Abs(X - v2.X) < 5){
            *(Buffer + Y * Width + X) = 0x0000FF00;
          }
          if(Abs(Y - v3.Y) < 5 && Abs(X - v3.X) < 5){
            *(Buffer + Y * Width + X) = 0x0000FF00;
          }
        }
      }
      break;
    }
    case Pushbuffer_Entry_Rect_Texture:
    {
      s32                           Width       = Renderer->Width;
      s32                           Height      = Renderer->Height;
      pushbuffer_entry_rect_texture Entry       = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_texture);

      s32                           MinX        = Entry.Min.X < 0 ? 0 : Entry.Min.X;
      s32                           MinY        = Entry.Min.Y < 0 ? 0 : Entry.Min.Y;

      s32                           MaxX        = Entry.Max.X > Width ? Width : Entry.Max.X;
      s32                           MaxY        = Entry.Max.Y > Height ? Height : Entry.Max.Y;

      u32*                          Buffer      = Renderer->Buffer;
      u32*                          ImageBuffer = Entry.Memory;

      s32                           ImageWidth  = Entry.Max.X - Entry.Min.X;
      ImageWidth += Entry.Min.X < 0 ? 1 : 0;
      s32 XOffset = Entry.Min.X < 0 ? -Entry.Min.X : 0;

      for (s32 Y = MinY; Y < MaxY; Y++)
      {
        for (s32 X = MinX; X < MaxX; X++)
        {
          s32 YOffset     = Entry.FlippedZ ? MaxY - 1 - Y : (Y - MinY);
          s32 ImageOffset = YOffset * ImageWidth + (X - MinX + XOffset);
          u32 Color       = *(ImageBuffer + ImageOffset);
          u32 DestAlpha   = Color >> 24;
          if (DestAlpha != 0)
          {
            *(Buffer + Y * Width + X) = Color;
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
