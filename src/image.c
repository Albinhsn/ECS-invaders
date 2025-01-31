#include "image.h"

#define COLOR_MAPPED_IMAGE     1
#define UNMAPPED_RGB_IMAGE     2
#define RLE_COLOR_MAPPED_IMAGE 9
#define RLE_RGB_MAPPED_IMAGE   10

#include <stdio.h>

void Image_SavePPM(image* Image, const char* Filename)
{
  FILE* ptr = fopen(Filename, "w");
  Assert(ptr);

  fprintf(ptr, "P3\n%d %d\n255\n", Image->Width, Image->Height);
  for (u32 y = 0; y < Image->Height; y++)
  {
    for (u32 x = 0; x < Image->Width; x++)
    {

      u32 Color = Image->Buffer[y * Image->Width + x];
      u32 Red   = (Color >> 16) & 0xFF;
      u32 Green = (Color >> 8) & 0xFF;
      u32 Blue  = Color & 0xFF;
      fprintf(ptr, "%d %d %d\n", Red, Green, Blue);
    }
  }

  fclose(ptr);
}

void Targa_ParseColorMappedImage(arena* Arena, image* Image, u8* Buffer, u32 Size, targa_header* Header)
{
  // Length is determined by color map specification
}

u32 Targa_GetIndex(u32 BufferIndex, u8 ScreenOriginBit, u32 Width)
{
  u32 Y = BufferIndex / Width;

  u32 X = BufferIndex % Width;
  X     = ScreenOriginBit ? Width - 1 - X : X;

  return Y * Width + X;
}

void Targa_ParseUnmappedRGBImage(arena* Arena, image* Image, u8* Buffer, u32 Size, targa_header* Header)
{

  u32 Offset = sizeof(targa_header) + Header->CharactersInIdentificationField;

  if (Header->ColorMapType != 0)
  {
    // Parse color map
    Assert(0 && "Unimplemented parsing of color map");
  }

  Image->Width           = *(u16*)(&Header->ImageSpecification[4]);
  Image->Height          = *(u16*)(&Header->ImageSpecification[6]);

  u32 ImageSize          = Image->Width * Image->Height;
  Image->Buffer          = (u32*)Arena_Allocate(Arena, ImageSize * sizeof(u32));

  u8 ImagePixelSize      = Header->ImageSpecification[8];
  u8 ImageDescriptorByte = Header->ImageSpecification[9];
  u8 AttributeBits       = ImageDescriptorByte & 0b1111;
  u8 ScreenOriginBit     = (ImageDescriptorByte & 0b100000) >> 5;
  if (ImagePixelSize == 32 && AttributeBits == 8)
  {
    AttributeBits = 0;
  }
  else if (ImagePixelSize == 32)
  {
    Assert(0 && "Expected attributebits to be 8 for targa 32?");
  }

  u32 EntrySize = ImagePixelSize + AttributeBits;
  for (u32 BufferIndex = 0; BufferIndex < ImageSize; BufferIndex++)
  {
    u32 Index = Targa_GetIndex(BufferIndex, ScreenOriginBit, Image->Width);
    switch (EntrySize)
    {
    case 32:
    {
      u8  red              = *(Buffer + Offset + 2);
      u8  green            = *(Buffer + Offset + 1);
      u8  blue             = *(Buffer + Offset + 0);
      u8  attribute        = *(Buffer + Offset + 3);

      u32 Entry            = ((u32)attribute << 24) | ((u32)red << 16) | ((u32)green << 8) | blue;

      Image->Buffer[Index] = Entry;
      Offset += 4;
      break;
    }
    case 24:
    {
      break;
    }
    default:
    {
      Assert(0 && "Unimplemented entry size!");
    }
    }
  }

  int a = 5;
}

void Image_LoadTarga(arena* Arena, image* Image, u8* Buffer, u32 Size)
{
  Assert(Size > sizeof(targa_header) && "The buffer isn't even the size of the header!");

  targa_header Header = *(targa_header*)Buffer;

  switch (Header.ImageTypeCode)
  {
  case COLOR_MAPPED_IMAGE:
  {
    Targa_ParseColorMappedImage(Arena, Image, Buffer, Size, &Header);
    break;
  }
  case UNMAPPED_RGB_IMAGE:
  {
    Targa_ParseUnmappedRGBImage(Arena, Image, Buffer, Size, &Header);
    break;
  }
  case RLE_RGB_MAPPED_IMAGE:
  {

    break;
  }
  case RLE_COLOR_MAPPED_IMAGE:
  {

    break;
  }
  default:
  {
    Assert(0 && "Unimplemented targa image type!");
  }
  }
}

#undef COLOR_MAPPED_IMAGE
#undef UNMAPPED_RGB_IMAGE
#undef RLE_RGB_MAPPED_IMAGE
#undef RLE_RGB_MAPPED_IMAGE

void Image_LoadBMP(arena* Arena, image* Image, u8* Buf, u32 Size)
{
  file_buffer Buffer          = {};
  Buffer.Buffer               = Buf;
  Buffer.Index                = 0;
  Buffer.Length               = Size;

  bmp_header*      Header     = FileBuffer_ParseStruct(&Buffer, bmp_header);
  bmp_info_header* InfoHeader = FileBuffer_ParseStruct(&Buffer, bmp_info_header);

  Assert(InfoHeader->Compression == 0 && "Unimplemented!");
  Assert(InfoHeader->Width * InfoHeader->Height * 3 == InfoHeader->ImageSize);

  Image->Width  = InfoHeader->Width;
  Image->Height = InfoHeader->Height;

  Image->Buffer = (u32*)Arena_Allocate(Arena, sizeof(u32) * Image->Width * Image->Height);

  u32 ImageSize = InfoHeader->ImageSize / 3;

  for (u32 Y = 0; Y < Image->Height; Y++)
  {
    for (u32 X = 0; X < Image->Width; X++)
    {
      u8  Blue                  = FileBuffer_ParseU8(&Buffer);
      u8  Green                 = FileBuffer_ParseU8(&Buffer);
      u8  Red                   = FileBuffer_ParseU8(&Buffer);

      u32 PixelIndex            = (Image->Height - 1 - Y) * Image->Width + X;

      Image->Buffer[PixelIndex] = (((u32)Red << 16) |  //
                                   ((u32)Green << 8) | //
                                   ((u32)Blue << 0)    //
      );
    }
  }
}
