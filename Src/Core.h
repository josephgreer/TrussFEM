#pragma once
#pragma warning(push,0)

typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;
typedef double f64;

using ::s8;
using ::u16;
using ::s16;
using ::u32;
using ::s32;
using ::f32;
using ::f64;

#define PI 3.14159265359

#define ASSERT(x) if(!(x)) { int a = 0;  int b = 1/a; } 
#define MIN(x,y) ((x)<(y) ? (x) : (y))
#define MAX(x,y) ((x)>(y) ? (x) : (y))
#define ABS(x) ((x)>0?(x):(-(x)))
#define ROUND(x) ((s32)((x)+0.5))

namespace TFM
{
  using ::s8;
  using ::u16;
  using ::s16;
  using ::u32;
  using ::s32;
  using ::f32;
  using ::f64;
}