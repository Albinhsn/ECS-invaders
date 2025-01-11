#ifndef IMAGE_H
#define IMAGE_H

#include "common.h"

typedef struct image
{
  u32* Buffer;
  u32  Width;
  u32  Height;
} image;

typedef struct targa_header
{
  u8 CharactersInIdentificationField;
  u8 ColorMapType;
  u8 ImageTypeCode;
  u8 ColorMapSpec[5];
  u8 ImageSpecification[10];
} targa_header;

#pragma pack(push, 1)
typedef struct bmp_header
{
  u16 Signature;
  u32 FileSize;
  u32 reserved;
  u32 DataOffset;
} bmp_header;
typedef struct bmp_info_header
{
  u32 Size;
  u32 Width;
  u32 Height;
  u16 Planes;
  u16 BitsPerPixel;
  u32 Compression;
  u32 ImageSize;
  u32 XpixelsPerM;
  u32 YpixelsPerM;
  u32 ColorsUsed;
  u32 ImportantColors;
}bmp_info_header;

typedef struct bmp_color_table 
{
  u8 Red;
  u8 Green;
  u8 BLue;
  u8 Reserved;
} bmp_color_table;
#pragma pack(pop)


#endif
