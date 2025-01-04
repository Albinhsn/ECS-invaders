#include "image.h"
#include "platform.h"

#define COLOR_MAPPED_IMAGE     1
#define UNMAPPED_RGB_IMAGE     2
#define RLE_COLOR_MAPPED_IMAGE 9
#define RLE_RGB_MAPPED_IMAGE   10

void ParseColorMappedImage(arena * Arena, targa_image * Image, u8 * Buffer, u32 Size, u32 Offset)
{
  // Length is determined by color map specification 
}

void Image_LoadTarga(arena * Arena, targa_image* Image, u8* Buffer, u32 Size)
{
  Assert(Size > sizeof(targa_header) && "The buffer isn't even the size of the header!");

  targa_header Header = *(targa_header*)Buffer;
  u32          Offset = sizeof(targa_header) + Header.CharactersInIdentificationField;

  switch (Header.ColorMapType)
  {
  case COLOR_MAPPED_IMAGE:
  {
    ParseColorMappedImage(Arena, Image, Buffer, Size, Offset);
    break;
  }
  case UNMAPPED_RGB_IMAGE:
  {
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
  default:{
      Assert(0 && "Unimplemented targa image type!");
    }
  }
}

#undef COLOR_MAPPED_IMAGE
#undef UNMAPPED_RGB_IMAGE
#undef RLE_RGB_MAPPED_IMAGE
#undef RLE_RGB_MAPPED_IMAGE
