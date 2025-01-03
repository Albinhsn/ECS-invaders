#ifndef IMAGE_H
#define IMAGE_H

#include "common.h"

typedef enum image_format
{
  ImageFormat_RGBA,
  ImageFormat_ARGB,
  ImageFormat_Greyscale
} image_format;

typedef struct targa_image
{
  u8*          Buffer;
  image_format Format;
  u32          Width;
  u32          Height;

} targa_image;

#endif
