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

u32 Software_Renderer_SampleTextureBilinear(texture * Texture, f32 U, f32 V)
{
  u32 * Buffer = (u32*)Texture->Memory;
  u32 X = (u32)((Texture->Width - 2) * U);
  u32 Y = (u32)((Texture->Height - 2) * V);
  Assert(X <= Texture->Width - 2);
  Assert(Y <= Texture->Height - 2);
  u32 S0 = Buffer[Y * Texture->Width + X];
  u32 A0 = (S0 >> 24) & 0xFF;
  u32 R0 = (S0 >> 16) & 0xFF;
  u32 G0 = (S0 >> 8) & 0xFF;
  u32 B0 = S0 & 0xFF;

  u32 S1 = Buffer[Y * Texture->Width + X + 1];
  u32 A1 = (S1 >> 24) & 0xFF;
  u32 R1 = (S1 >> 16) & 0xFF;
  u32 G1 = (S1 >> 8) & 0xFF;
  u32 B1 = S1 & 0xFF;

  u32 S2 = Buffer[(Y + 1) * Texture->Width + X];
  u32 A2 = (S2 >> 24) & 0xFF;
  u32 R2 = (S2 >> 16) & 0xFF;
  u32 G2 = (S2 >> 8) & 0xFF;
  u32 B2 = S2 & 0xFF;

  u32 S3 = Buffer[(Y + 1) * Texture->Width + X + 1];
  u32 A3 = (S3 >> 24) & 0xFF;
  u32 R3 = (S3 >> 16) & 0xFF;
  u32 G3 = (S3 >> 8) & 0xFF;
  u32 B3 = S3 & 0xFF;

  u32 Result = 0;
  u32 ResultA = (A0 + A1 + A2 + A3) / 4;
  u32 ResultR = (R0 + R1 + R2 + R3) / 4;
  u32 ResultG = (G0 + G1 + G2 + G3) / 4;
  u32 ResultB = (B0 + B1 + B2 + B3) / 4;

  Result = (
    (ResultA << 24) |
    (ResultR << 16) |
    (ResultG << 8)  |
    (ResultB << 0)
  );

  return Result;
}

u32 Software_Renderer_LinearBlend(u32 Current, u32 New)
{
  u32 A = New;
  u32 B = Current;

  f32 Inv255f = 1.0f / 255.0f;
  f32 A0 = ((A >> 24) & 0xFF) * Inv255f;
  f32 R0 = ((A >> 16) & 0xFF) * Inv255f;
  f32 G0 = ((A >> 8) & 0xFF) * Inv255f;
  f32 B0 = (A & 0xFF) * Inv255f;

  f32 A1 = ((B >> 24) & 0xFF) * Inv255f;
  f32 R1 = ((B >> 16) & 0xFF) * Inv255f;
  f32 G1 = ((B >> 8) & 0xFF) * Inv255f;
  f32 B1 = (B & 0xFF) * Inv255f;


  f32 ResultA = A0 + A1 * (1- A0);
  f32 ResultR = (R0 * A0 + R1 * A1 * (1 - A0)) / ResultA;
  f32 ResultG = (G0 * A0 + G1 * A1 * (1 - A0)) / ResultA;
  f32 ResultB = (B0 * A0 + B1 * A1 * (1 - A0)) / ResultA;

  u32 ResultA255 = (u32)(ResultA * 255);
  u32 ResultR255 = (u32)(ResultR * 255);
  u32 ResultG255 = (u32)(ResultG * 255);
  u32 ResultB255 = (u32)(ResultB * 255);

  Assert(ResultA255 <= 255);
  Assert(ResultR255 <= 255);
  Assert(ResultG255 <= 255);
  Assert(ResultB255 <= 255);

  u32 Result = (
    (ResultA255 << 24) |
    (ResultR255 << 16) |
    (ResultG255 << 8)  |
    (ResultB255 << 0)
  );
  return Result;
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
        }
      }
      break;
    }
    case Pushbuffer_Entry_Rect_Texture:
    {
      s32                           Width       = Renderer->Width;
      s32                           Height      = Renderer->Height;
      pushbuffer_entry_rect_texture Entry       = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_texture);

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
      MaxX = MinI(MaxX, Width - 2); // - 2 Since we do bilinear sample
      MinY = MaxI(MinY, 0);
      MaxY = MinI(MaxY, Height - 2);


      u32*                        Buffer = Renderer->Buffer;

      u32                         Color  = 0x00FF0000;

      vec2f XAxis = Entry.XAxis;
      vec2f YAxis = Entry.YAxis;
      f32 XAxisLength = Vec2f_Length(XAxis);
      XAxisLength *= XAxisLength;
      f32 YAxisLength = Vec2f_Length(YAxis);
      YAxisLength *= YAxisLength;
      vec2f Origin = Entry.Origin;
      for (s32 Y = MinY; Y <= MaxY; Y++)
      {
        for (s32 X = MinX; X <= MaxX; X++)
        {
          vec2f Position = V2f((f32)X,(f32)Y);
          vec2f OriginToPoint = Vec2f_Sub(Position, Origin);
          f32 XEdge = Vec2f_Dot(OriginToPoint, XAxis);
          f32 YEdge = Vec2f_Dot(OriginToPoint, YAxis);

          bool WithinX = XEdge >= 0 && XEdge <= XAxisLength;
          bool WithinY = YEdge >= 0 && YEdge <= YAxisLength;
          if(WithinX && WithinY){

            // Figure out the uv coordinates
            f32 U = XEdge / XAxisLength;
            f32 V = 1 - YEdge / YAxisLength; // Since Y is down!
            Assert(U >= 0 && U <= 1);
            Assert(V >= 0 && V <= 1);
            V = Entry.FlippedZ ? 1 - V : V;


            u32 Sample = Software_Renderer_SampleTextureBilinear(Entry.Texture, U, V);
            u32 CurrentPixel = *(Buffer + Y * Width + X);

            *(Buffer + Y * Width + X) = Software_Renderer_LinearBlend(CurrentPixel, Sample);
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
