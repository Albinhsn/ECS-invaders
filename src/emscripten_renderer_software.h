#ifndef EMSCRIPTEN_RENDERER_SOFTWARE
#define EMSCRIPTEN_RENDERER_SOFTWARE

#include "emscripten_platform.h"

typedef struct emcc_renderer_software
{
  platform_renderer Header;
  u32 Width, Height;
  u32 * Buffer;
}emcc_renderer_software;

void EMCC_InitRenderCode(emcc_render_code* RenderCode);

#endif
