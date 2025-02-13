#include "win32_renderer_d3d.h"
#include "pushbuffer.c"
#include "vector.c"
#include "common.h"

void Win32_Deallocate(void* Memory, u64 Size)
{
  VirtualFree(Memory, Size, MEM_DECOMMIT);
}

static void* Win32_Allocate(u64 size)
{
  // ToDo Align the memory?
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void Win32_MapU32ColorToF32(f32 * Output, u32 Color)
{
  Output[3]     = ((Color >> 24) & 255) / 255.0f;
  Output[0]     = ((Color >> 16) & 255) / 255.0f;
  Output[1]     = ((Color >> 8) & 255) / 255.0f;
  Output[2]     = ((Color >> 0) & 255) / 255.0f;
}

bool Win32_ReadFile(arena* Arena, const char* Filename, u8** FileBuffer, u32* Size)
{

  DWORD  dwBytesRead = 0;

  HANDLE hFile       = CreateFile(Filename,              //
                                  GENERIC_READ,          //
                                  FILE_SHARE_READ,       //
                                  NULL,                  //
                                  OPEN_EXISTING,         //
                                  FILE_ATTRIBUTE_NORMAL, //
                                  NULL                   //
        );

  if (hFile == INVALID_HANDLE_VALUE)
  {
    return false;
  }

  u32        FileSize          = GetFileSize(hFile, NULL);
  u8*        Buffer            = (u8*)Arena_Allocate(Arena, FileSize + 1);

  DWORD      NumberOfBytesRead = 0;
  OVERLAPPED ol                = {};

  s32        Result            = ReadFile(hFile, Buffer, FileSize, &NumberOfBytesRead, &ol);
  DWORD      Error             = GetLastError();
  if (Result == false || FileSize != NumberOfBytesRead)
  {
    Arena_Deallocate(Arena, FileSize + 1);
    return false;
  }
  Buffer[FileSize] = '\0';
  *FileBuffer      = Buffer;
  *Size            = FileSize;

  CloseHandle(hFile);

  return true;
}

d3d_texture * Win32_GetTexture(win32_renderer_d3d * Renderer, texture * Texture)
{
  for(u32 TextureIndex = 0; TextureIndex < Renderer->TextureCount; TextureIndex++)
  {
    d3d_texture * GLTexture = &Renderer->Textures[TextureIndex];
    if(String_Compare(&GLTexture->Name, &Texture->Name))
    {
      return GLTexture;
    }
  }
  Assert(Renderer->TextureCount < ArrayCount(Renderer->Textures));
  // ToDo Create texture
  d3d_texture * NewTexture = &Renderer->Textures[Renderer->TextureCount++];
  NewTexture->Name = Texture->Name;

  return NewTexture;
}

vec2f Win32_TransformSSToCS(win32_renderer_d3d * Renderer, vec2f v)
{
  v.X = (v.X / (f32)Renderer->ScreenWidth * 2.0f - 1.0f);
  v.Y = -(v.Y / (f32)Renderer->ScreenHeight * 2.0f - 1.0f);

  return v;
}

shader Win32_GetShaderByName(win32_renderer_d3d * Renderer, const char * Name)
{
  string NameString = {};
  NameString.Buffer = (u8*)Name;
  NameString.Length = String_Length((u8*)Name);
  for(u32 ShaderIndex = 0; ShaderIndex < Renderer->ShaderCount; ShaderIndex++)
  {
    shader Shader = Renderer->Shaders[ShaderIndex];
    if(String_Compare(&NameString, &Shader.Name))
    {
      return Shader;
    }
  }
  Assert(0 && "Didn't find the shader!");
  return (shader){};
}

void Win32_Render(win32_renderer_d3d * Renderer, pushbuffer* Pushbuffer)
{
  while(Pushbuffer->ReadOffset < Pushbuffer->AllocatedOffset)
  {
    pushbuffer_entry_type EntryType = Pushbuffer_ReadEntryType(Pushbuffer);
    switch(EntryType)
    {
      case Pushbuffer_Entry_Clear:
      {
        pushbuffer_entry_clear Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_clear);
        u32 Color = Entry.Color;
        f32 A     = ((Color >> 24) & 255) / 255.0f;
        f32 R     = ((Color >> 16) & 255) / 255.0f;
        f32 G     = ((Color >> 8) & 255) / 255.0f;
        f32 B     = ((Color >> 0) & 255) / 255.0f;
        vec4f ColorV4 = V4f(R, G, B, A); 
        Renderer->Context->lpVtbl->ClearRenderTargetView(
          Renderer->Context,
          Renderer->RenderTargetView,
          (float*)&ColorV4);
        break;
      }
      case Pushbuffer_Entry_Text:
      {

        Pushbuffer_Read(Pushbuffer, pushbuffer_entry_text);
          #if false
        pushbuffer_entry_text Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_text);
        shader Shader = Win32_GetShaderByName(Renderer, "text");
        glUseProgram(Shader.ID);
        glBindVertexArray(Renderer->VAOText);

        msdf_font * Font    = Entry.Font;
        u32 GlyphCount      = Entry.Text->Length;
        u32 GlyphBufferSize = sizeof(f32) * 16 * GlyphCount;
        f32 * VBOData       = Arena_Allocate(&Renderer->Arena, GlyphBufferSize);

        gl_texture * Texture = Win32_GetTexture(Renderer, &Font->Texture);
        // Set Texture  in shader

        u32 Location = glGetUniformLocation(Shader.ID, "texture1");
        glUniform1i(Location, GL_TEXTURE0);
        f32 Color[4] = {};
        Win32_MapU32ColorToF32(Color, Entry.Color);
        glUniform4fv(glGetUniformLocation(Shader.ID, "color"), 1, Color);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture->ID);
        // Figure out size of a glyph
        vec2f LetterSize = V2f(
            (f32)(Entry.Size * (Font->WidthPerCell / (f32)Font->HeightPerCell)),
            (f32)Entry.Size);

        // Figure out where the text is placed
        Assert(Entry.Alignment == UI_TextAlignment_Centered);
        vec2f Offset = Win32_TransformSSToCS(Renderer, V2f(
            Entry.Position.X - 0.5f * LetterSize.X * Entry.Text->Length,
            Entry.Position.Y - 0.5f * Entry.Size
        ));

        LetterSize.X = LetterSize.X / (f32)Renderer->ScreenWidth   * 2.0f;
        LetterSize.Y = LetterSize.Y / (f32)Renderer->ScreenHeight  * 2.0f;

        u32 NumberOfIndices = 6 * GlyphCount;
        u32 * Indices = Arena_Allocate(&Renderer->Arena, sizeof(u32) * NumberOfIndices);

        f32 MaxU = Font->WidthPerCell  /(f32)Font->Texture.Width;
        f32 MaxV = Font->HeightPerCell  /(f32)Font->Texture.Height;
        for(u32 GlyphIndex = 0; GlyphIndex < GlyphCount; GlyphIndex++)
        {
          u8 Glyph = Entry.Text->Buffer[GlyphIndex];
          Assert(Glyph >= 32 && Glyph <= 126);
          if(Glyph > 32)
          {
            u32 CellIndex = Glyph - 33;
            u32 CellY     = CellIndex / Font->Columns;
            u32 CellX     = CellIndex % Font->Columns;

            Assert(CellX < Font->Columns && CellY < Font->Rows);


            f32 UOffset = CellX * Font->WidthPerCell / (f32)Font->Texture.Width;
            f32 VOffset = CellY * Font->HeightPerCell / (f32)Font->Texture.Height;

            u32 BufferOffset = 16 * GlyphIndex;
            // Top Right
            VBOData[BufferOffset + 0] = Offset.X + LetterSize.X;
            VBOData[BufferOffset + 1] = Offset.Y - LetterSize.Y;
            VBOData[BufferOffset + 2] = UOffset + MaxU;
            VBOData[BufferOffset + 3] = VOffset + MaxV;
            // Bot Right
            VBOData[BufferOffset + 4] = Offset.X + LetterSize.X;
            VBOData[BufferOffset + 5] = Offset.Y;
            VBOData[BufferOffset + 6] = UOffset + MaxU;
            VBOData[BufferOffset + 7] = VOffset;
            // Bot Left
            VBOData[BufferOffset + 8] = Offset.X;
            VBOData[BufferOffset + 9] = Offset.Y;
            VBOData[BufferOffset + 10] = UOffset;
            VBOData[BufferOffset + 11] = VOffset;
            // Top Left
            VBOData[BufferOffset + 12] = Offset.X;
            VBOData[BufferOffset + 13] = Offset.Y - LetterSize.Y;
            VBOData[BufferOffset + 14] = UOffset;
            VBOData[BufferOffset + 15] = VOffset + MaxV;
          }
          Indices[6 * GlyphIndex + 0] = 4 * GlyphIndex + 0;
          Indices[6 * GlyphIndex + 1] = 4 * GlyphIndex + 1;
          Indices[6 * GlyphIndex + 2] = 4 * GlyphIndex + 2;
          Indices[6 * GlyphIndex + 3] = 4 * GlyphIndex + 0;
          Indices[6 * GlyphIndex + 4] = 4 * GlyphIndex + 2;
          Indices[6 * GlyphIndex + 5] = 4 * GlyphIndex + 3;
          Offset.X += LetterSize.X * 0.75f;
        }

        // Send it
        glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBOText);
        glBufferData(GL_ARRAY_BUFFER, GlyphBufferSize, VBOData, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBOText);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * NumberOfIndices, Indices, GL_DYNAMIC_DRAW);
        glDrawElements(GL_TRIANGLES, NumberOfIndices, GL_UNSIGNED_INT, 0);

        Arena_Deallocate(&Renderer->Arena, GlyphBufferSize + sizeof(u32) * NumberOfIndices);
          #endif
        break;
      }
      case Pushbuffer_Entry_Rect_Color:
      {
         Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_color);
          #if false
        pushbuffer_entry_rect_color Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_color);
        shader Shader = Win32_GetShaderByName(Renderer, "quad");

        glBindVertexArray(Renderer->VAOQ);

        // Use the shader
        glUseProgram(Shader.ID);

        // Set Color in shader
        f32 Color[4] = {};
        Win32_MapU32ColorToF32(Color, Entry.Color);
        glUniform4fv(glGetUniformLocation(Shader.ID, "color"), 1, Color);

        // Create the vertex data for the quad
        // The 4 corners mapped form [-1,1]

        // Calculate the corners in ScreenSpace
        vec2f                       v0         = Entry.Origin;
        vec2f                       v1         = Vec2f_Add(Entry.Origin, Entry.XAxis);
        vec2f                       v2         = Vec2f_Add(Entry.Origin, Entry.YAxis);
        vec2f                       v3         = Vec2f_Add(v2, Entry.XAxis);


        v0 = Win32_TransformSSToCS(Renderer, v0);
        v1 = Win32_TransformSSToCS(Renderer, v1);
        v2 = Win32_TransformSSToCS(Renderer, v2);
        v3 = Win32_TransformSSToCS(Renderer, v3);

        // Map to to Clip Space
        f32 VertexLocations[8] =
        {
          v3.X, v3.Y, // Top Right
          v1.X, v1.Y, // Bot Right
          v0.X, v0.Y, // Bot Left
          v2.X, v2.Y  // Top Left
        };

        // Send it
        glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBOQ);
        glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 8, VertexLocations, GL_DYNAMIC_DRAW);

        // Draw it baby
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBOQ);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        #endif
        break;
      }
      case Pushbuffer_Entry_Rect_Texture:
      {
        Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_texture);
          #if false
        pushbuffer_entry_rect_texture Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_texture);
        shader Shader = Win32_GetShaderByName(Renderer, "texture");

        glBindVertexArray(Renderer->VAOT);

        // Use the shader
        glUseProgram(Shader.ID);

        // Check if texture already exists somewhere
        // If so use it, otherwise create it
        gl_texture * Texture = Win32_GetTexture(Renderer, Entry.Texture);
        // Set Texture  in shader

        u32 Location = glGetUniformLocation(Shader.ID, "texture1");
        glUniform1i(Location, GL_TEXTURE0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture->ID);


        vec2f                       v0         = Entry.Origin;
        vec2f                       v1         = Vec2f_Add(Entry.Origin, Entry.XAxis);
        vec2f                       v2         = Vec2f_Add(Entry.Origin, Entry.YAxis);
        vec2f                       v3         = Vec2f_Add(v2, Entry.XAxis);


        v0 = Win32_TransformSSToCS(Renderer, v0);
        v1 = Win32_TransformSSToCS(Renderer, v1);
        v2 = Win32_TransformSSToCS(Renderer, v2);
        v3 = Win32_TransformSSToCS(Renderer, v3);

        // Map to to Clip Space
        f32 VModifier = !Entry.FlippedZ ? 1.0f : 0.0f;
        f32 VertexLocations[16] =
        {
          v3.X, v3.Y,  // Top Right
          1.0f,  1.0f - VModifier,
          v1.X, v1.Y,  // Bot Right
          1.0f, 0.0f + VModifier,
          v0.X, v0.Y,  // Bot Left
          0.0f, 0.0f + VModifier,
          v2.X, v2.Y,  // Top Left
          0.0f, 1.0f - VModifier
        };

        // Send it
        glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBOT);
        glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 16, VertexLocations, GL_DYNAMIC_DRAW);

        // Draw it baby
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBOT);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        #endif
        break;
      }
    }
  }
}

void BeginFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{
  win32_renderer_d3d * Renderer = (win32_renderer_d3d*)PlatformRenderer;

  RECT Rect;
  GetClientRect(Renderer->hwnd, &Rect);
  D3D11_VIEWPORT Viewport = {
    0, 0,
    (float) (Rect.right - Rect.left),
    (float)(Rect.bottom - Rect.top),
    0,
    1.0f
  };
  Renderer->Context->lpVtbl->RSSetViewports(Renderer->Context, 1, &Viewport);
  Renderer->SwapChain->lpVtbl->Present(Renderer->SwapChain, 1, 0);

}

void EndFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{

  win32_renderer_d3d * Renderer = (win32_renderer_d3d*)PlatformRenderer;
  Win32_Render(Renderer, Pushbuffer);
}

ID3DBlob * Win32_CompileShader(string Location, const char * Main, const char * Profile)
{

  ID3DBlob* Blob = 0;
  ID3DBlob *Error = 0;

  HRESULT Result = D3DCompileFromFile(
    (LPCWSTR)Location.Buffer, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
    Main,
    Profile,
    0,
    0,
    &Blob,
    &Error 
  );
  if(FAILED(Result))
  {
    if(Error)
    {
      OutputDebugStringA((char*)Error->lpVtbl->GetBufferPointer(Error));
      Error->lpVtbl->Release(Error);
    }
    Assert(false);
  }

  return Blob;
}


void Win32_LoadShaders(win32_renderer_d3d * Renderer)
{
  const char * ShaderLocation = "../assets/shadersd3d.txt";


  file_buffer Buffer = {};
  bool Result = Win32_ReadFile(&Renderer->Arena, ShaderLocation, &Buffer.Buffer, &Buffer.Length);
  Assert(Result);

  while(Buffer.Index < Buffer.Length)
  {
    // Parse name
    string Name = {};
    FileBuffer_ParseString(&Buffer, &Name);

    FileBuffer_SkipWhitespace(&Buffer);
    // Parse vertex
    string VertexLocation = {};
    FileBuffer_ParseString(&Buffer, &VertexLocation);


    shader Shader = {};
    ID3DBlob* Blob = Win32_CompileShader(VertexLocation, "vs_main", "vs_5_0");
    Result = Renderer->Device->lpVtbl->CreateVertexShader(
      Renderer->Device, Blob->lpVtbl->GetBufferPointer(Blob),
      Blob->lpVtbl->GetBufferSize(Blob), NULL,
      &Shader.Vertex
    );

    Blob = Win32_CompileShader(VertexLocation, "ps_main", "ps_5_0");
    Result = Renderer->Device->lpVtbl->CreatePixelShader(
      Renderer->Device, Blob->lpVtbl->GetBufferPointer(Blob),
      Blob->lpVtbl->GetBufferSize(Blob), NULL,
      &Shader.Pixel
    );

    Shader.Name   = Name;
    Assert(Renderer->ShaderCount < ArrayCount(Renderer->Shaders));
    Renderer->Shaders[Renderer->ShaderCount++] = Shader;
    FileBuffer_SkipWhitespace(&Buffer);
  }

}


void InitTextureVertexArray(win32_renderer_d3d *Renderer)
{
  shader Shader = Win32_GetShaderByName("texture");
}
void InitQuadVertexArray(win32_renderer_d3d *Renderer)
{
  shader Shader = Win32_GetShaderByName("quad");
}
void InitTextVertexArray(win32_renderer_d3d *Renderer)
{
  shader Shader = Win32_GetShaderByName("text");
}

platform_renderer * CreateRenderer(u32 ScreenWidth, u32 ScreenHeight, void * Window)
{
  HWND hwnd = (HWND)Window;
  HDC hdc = GetDC(hwnd);

  ID3D11Device * Device = 0;
  ID3D11DeviceContext * Context = 0;
  IDXGISwapChain * SwapChain = 0;
  ID3D11RenderTargetView* RenderTargetView = 0;

  DXGI_SWAP_CHAIN_DESC SwapChainDescription = {0};
  SwapChainDescription.BufferDesc.RefreshRate.Numerator = 0;
  SwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
  SwapChainDescription.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
  SwapChainDescription.SampleDesc.Count   = 1;
  SwapChainDescription.SampleDesc.Quality = 0;
  SwapChainDescription.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  SwapChainDescription.BufferCount  = 1;
  SwapChainDescription.OutputWindow = hwnd;
  SwapChainDescription.Windowed       = true;


  D3D_FEATURE_LEVEL FeatureLevel;
  u32 Flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG;

  HRESULT Result = D3D11CreateDeviceAndSwapChain(
    0,
    D3D_DRIVER_TYPE_HARDWARE,
    0,
    Flags,
    0,
    0,
    D3D11_SDK_VERSION,
    &SwapChainDescription,
    &SwapChain,
    &Device,
    &FeatureLevel,
    &Context);

  Assert(SUCCEEDED(Result) && SwapChain && Device && Context);


  ID3D11Texture2D * Framebuffer;
  Result = SwapChain->lpVtbl->GetBuffer(SwapChain, 0, &IID_ID3D11Texture2D, (void**)&Framebuffer);
  Assert(SUCCEEDED(Result));
  Result = Device->lpVtbl->CreateRenderTargetView(Device, (ID3D11Resource*)Framebuffer, 0, &RenderTargetView);
  Assert(SUCCEEDED(Result));
  Framebuffer->lpVtbl->Release(Framebuffer);

  u64 ArenaSize = Megabyte(1);
  win32_renderer_d3d *Renderer = Win32_Allocate(sizeof(win32_renderer_d3d) + ArenaSize);
  Renderer->Arena.Size    = ArenaSize;
  Renderer->Arena.Memory  = (u8*)Renderer + sizeof(win32_renderer_d3d);
  Renderer->Device = Device;
  Renderer->Context = Context;
  Renderer->SwapChain = SwapChain;
  Renderer->RenderTargetView = RenderTargetView;
  Renderer->hwnd = hwnd;
  Renderer->ScreenWidth = ScreenWidth;
  Renderer->ScreenHeight = ScreenHeight;

  Win32_LoadShaders(Renderer);

  InitTextureVertexArray(Renderer);
  InitQuadVertexArray(Renderer);
  InitTextVertexArray(Renderer);


  return (platform_renderer*)Renderer;
}

void ReleaseRenderer(platform_renderer * PlatformRenderer)
{
}
