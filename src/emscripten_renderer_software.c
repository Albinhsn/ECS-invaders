#include "emscripten_renderer_software.h"
#include "renderer_software.c"

void                  EMCC_BeginFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{
}

void EMCC_EndFrame(platform_renderer* PlatformRenderer, pushbuffer* Pushbuffer)
{

  emcc_renderer_software * Renderer = (emcc_renderer_software*) PlatformRenderer;

  software_renderer SoftwareRenderer = {};
  SoftwareRenderer.Width  = Renderer->Width;
  SoftwareRenderer.Height = Renderer->Height;
  SoftwareRenderer.Buffer = Renderer->Buffer;
  Software_Renderer_Render(&SoftwareRenderer, Pushbuffer);

  EM_ASM({
      var ctx = document.getElementById('canvas').getContext('2d');
      var imageData = ctx.getImageData(0, 0, $0, $1);
      var buffer = new Uint8Array(Module.HEAPU8.buffer, $2, $0 * $1 * 4);
      imageData.data.set(Renderer->Buffer);
      ctx.putImageData(imageData, 0, 0);
  }, Renderer->Width, Renderer->Height, Renderer->Buffer);

}

platform_renderer * EMCC_CreateRenderer(u32 ScreenWidth, u32 ScreenHeight, void * Window)
{

  u64 RendererMemorySize = sizeof(u32) * ScreenWidth * ScreenHeight + sizeof(emcc_renderer_software);
  void * Memory = malloc(RendererMemorySize);
  emcc_renderer_software * Renderer = (emcc_renderer_software*)Memory;

  Memory = (void*)((u8*)Memory + sizeof(emcc_renderer_software));

  Renderer->Width = ScreenWidth;
  Renderer->Height = ScreenHeight;
  Renderer->Buffer = (u32*)Memory;

  return (platform_renderer*)Renderer;
}

void EMCC_ReleaseRenderer(platform_renderer* PlatformRenderer)
{
}

