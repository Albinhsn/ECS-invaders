#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef float    f32;
typedef double   f64;

#define Kilobyte(size) (size * 1024LL)
#define Megabyte(size) (Kilobyte(size) * 1024LL)
#define Gigabyte(size) (Megabyte(size) * 1024LL)

#endif
