#ifndef IMAGE_H
#define IMAGE_H

#include "common.h"

typedef enum image_format
{
  ImageFormat_RGBA,
  ImageFormat_ARGB,
  ImageFormat_Greyscale
} image_format;

typedef struct targa_header
{
  u8 CharactersInIdentificationField;
  u8 ColorMapType;
  u8 ImageTypeCode;
  u8 ColorMapSpec[5];
  u8 ImageSpecification[10];
} targa_header;

typedef struct targa_image
{
  u32*         Buffer;
  image_format Format;
  u32          Width;
  u32          Height;
} targa_image;

#endif
