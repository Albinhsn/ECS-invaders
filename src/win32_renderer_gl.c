#include "win32_renderer_gl.h"
#include "pushbuffer.c"
#include "vector.c"
#include "common.h"
#include "wingdi.h"
#include "opengl.c"
#include <gl/gl.h>
#include <gl/glext.h>


void* Win32_GLGetProcAddress(const char* ProcName)
{
  return (void*)wglGetProcAddress((LPCSTR)ProcName);
}

shader Win32_GetShaderByName(win32_renderer_gl * Renderer, const char * Name)
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
void Win32_MapU32ColorToF32(f32 * Output, u32 Color)
{
  Output[3]     = ((Color >> 24) & 255) / 255.0f;
  Output[0]     = ((Color >> 16) & 255) / 255.0f;
  Output[1]     = ((Color >> 8) & 255) / 255.0f;
  Output[2]     = ((Color >> 0) & 255) / 255.0f;
}

vec2f Win32_TransformSSToCS(win32_renderer_gl * Renderer, vec2f v)
{
  v.X = (v.X / (f32)Renderer->ScreenWidth * 2.0f - 1.0f);
  v.Y = -(v.Y / (f32)Renderer->ScreenHeight * 2.0f - 1.0f);

  return v;
}

gl_texture * Win32_GetTexture(win32_renderer_gl * Renderer, texture * Texture)
{
  for(u32 TextureIndex = 0; TextureIndex < Renderer->TextureCount; TextureIndex++)
  {
    gl_texture * GLTexture = &Renderer->Textures[TextureIndex];
    if(String_Compare(&GLTexture->Name, &Texture->Name))
    {
      return GLTexture;
    }
  }
  Assert(Renderer->TextureCount < ArrayCount(Renderer->Textures));
  gl_texture * NewTexture = &Renderer->Textures[Renderer->TextureCount++];
  NewTexture->Name = Texture->Name;
  glGenTextures(1, &NewTexture->ID);
  glBindTexture(GL_TEXTURE_2D, NewTexture->ID);

  for(u32 SampleIndex = 0; SampleIndex < Texture->Width * Texture->Height; SampleIndex++)
  {
    u32 Color = ((u32*)Texture->Memory)[SampleIndex];
    u8 A     = ((Color >> 24) & 255);
    u8 R     = ((Color >> 16) & 255);
    u8 G     = ((Color >> 8) & 255);
    u8 B     = ((Color >> 0) & 255);
    ((u32*)Texture->Memory)[SampleIndex] = (
      ((u32)A << 24) |
      ((u32)B << 16) |
      ((u32)G << 8)  |
      ((u32)R << 0)
    );
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Texture->Width, Texture->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Texture->Memory);
  glGenerateMipmap(GL_TEXTURE_2D);

  return NewTexture;
}

void Win32_Render(win32_renderer_gl * Renderer, pushbuffer* Pushbuffer)
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
        glClearColor(R, G, B, A);
        break;
      }
      case Pushbuffer_Entry_Text:
      {
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
        Assert(Entry.Alignment == UI_Text_Alignment_Centered);
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
        break;
      }
      case Pushbuffer_Entry_Rect_Color:
      {
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

        break;
      }
      case Pushbuffer_Entry_Rect_Texture:
      {
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
        break;
      }
    }
  }
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


void Win32_Deallocate(void* Memory, u64 Size)
{
  VirtualFree(Memory, Size, MEM_DECOMMIT);
}

static void* Win32_Allocate(u64 size)
{
  // ToDo Align the memory?
  return VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}


void BeginFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{
  Pushbuffer_Reset(Pushbuffer);
  glClear(GL_COLOR_BUFFER_BIT);
}

void EndFrame(platform_renderer * PlatformRenderer, pushbuffer * Pushbuffer)
{
  win32_renderer_gl * Renderer = (win32_renderer_gl*)PlatformRenderer;
  Win32_Render(Renderer, Pushbuffer);

  HDC hdc = GetDC(Renderer->hwnd);
  SwapBuffers(hdc);
  ReleaseDC(Renderer->hwnd, hdc);
}

typedef struct win32_shader_program_input
{
  string *ShaderSource;
  GLenum ShaderType;
} win32_shader_program_input;

bool Win32_TestProgramLinking(u32 ID)
{
  s32 Result;
  glGetProgramiv(ID, GL_LINK_STATUS, &Result);

  return Result;
}
bool Win32_TestShaderCompilation(u32 ID)
{
  s32 Result;
  glGetShaderiv(ID, GL_COMPILE_STATUS, &Result);
  if(Result == false)
  {
    char Log[512] = {};
    glGetShaderInfoLog(ID,ArrayCount(Log), NULL, Log);
    OutputDebugStringA(Log);
  }
  return Result;
}

u32 Win32_CreateShader(win32_shader_program_input Input)
{
  u32 ID = glCreateShader(Input.ShaderType);
  glShaderSource(ID, 1, (const GLchar**)&Input.ShaderSource->Buffer, (GLint*)&Input.ShaderSource->Length);
  glCompileShader(ID);

  Assert(Win32_TestShaderCompilation(ID));

  return ID;
}


u32 Win32_CreateShaderProgram(u32 ShaderCount, ...)
{
  va_list ShaderInputs;
  va_start(ShaderInputs, ShaderCount);
  u32 ID = glCreateProgram();
  for(u32 ShaderIndex = 0; ShaderIndex < ShaderCount; ShaderIndex++)
  {
    win32_shader_program_input Input = va_arg(ShaderInputs, win32_shader_program_input);
    u32 Shader                       = Win32_CreateShader(Input);
    glAttachShader(ID, Shader);
  }

  va_end(ShaderInputs);
  glLinkProgram(ID);

  Assert(Win32_TestProgramLinking(ID));
  return ID;
}

void Win32_LoadShaders(win32_renderer_gl * Renderer)
{
  const char * ShaderLocation = "../assets/shaders.txt";


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

    string VertexSource = {};
    u8 C  = Buffer.Buffer[Buffer.Index];
    Buffer.Buffer[Buffer.Index] = '\0';
    Result = Win32_ReadFile(&Renderer->Arena, (const char*)VertexLocation.Buffer, &VertexSource.Buffer, &VertexSource.Length);
    Assert(Result);
    Buffer.Buffer[Buffer.Index] = C;

    // Parse frag
    FileBuffer_SkipWhitespace(&Buffer);
    string FragLocation = {};
    FileBuffer_ParseString(&Buffer, &FragLocation);

    string FragSource = {};
    C  = Buffer.Buffer[Buffer.Index];
    Buffer.Buffer[Buffer.Index] = '\0';
    Result = Win32_ReadFile(&Renderer->Arena, (const char*)FragLocation.Buffer, &FragSource.Buffer, &FragSource.Length);
    Assert(Result);
    Buffer.Buffer[Buffer.Index] = C;

    win32_shader_program_input Input[2] = {};
    Input[0].ShaderSource               = &VertexSource;
    Input[0].ShaderType                 = GL_VERTEX_SHADER;
    Input[1].ShaderSource               = &FragSource;
    Input[1].ShaderType                 = GL_FRAGMENT_SHADER;
    u32 ID = Win32_CreateShaderProgram(2, &Input[0], &Input[1]);

    shader Shader = {};
    Shader.ID     = ID;
    Shader.Name   = Name;
    Assert(Renderer->ShaderCount < ArrayCount(Renderer->Shaders));
    Renderer->Shaders[Renderer->ShaderCount++] = Shader;
    FileBuffer_SkipWhitespace(&Buffer);
  }

}
void InitTextVertexArray(win32_renderer_gl * Renderer)
{
  glGenVertexArrays(1, &Renderer->VAOText);
  glGenBuffers(1, &Renderer->VBOText);
  glGenBuffers(1, &Renderer->EBOText);

  glBindVertexArray(Renderer->VAOText);
  // Don't remember why you have to do this..
  // It's like the object isn't initialized until something
  // so you have to just dump data here?
  glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBOText);
  f32 TokenVertices[8] =
  {
    1.0f, 1.0f,
    1.0f, -1.0f,
    -1.0f, -1.0f,
    -1.0f, 1.0f
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 8, TokenVertices, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBOText);
  u32 Indices[6] = {0,1,2,0,2,3};
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * ArrayCount(Indices), Indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(0));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(sizeof(f32) * 2));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glBindVertexArray(0);
}

void InitQuadVertexArray(win32_renderer_gl * Renderer)
{
  glGenVertexArrays(1, &Renderer->VAOQ);
  glGenBuffers(1, &Renderer->VBOQ);
  glGenBuffers(1, &Renderer->EBOQ);

  glBindVertexArray(Renderer->VAOQ);
  // Don't remember why you have to do this..
  // It's like the object isn't initialized until something
  // so you have to just dump data here?
  glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBOQ);
  f32 TokenVertices[8] =
  {
    1.0f, 1.0f,
    1.0f, -1.0f,
    -1.0f, -1.0f,
    -1.0f, 1.0f
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 8, TokenVertices, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBOQ);
  u32 Indices[6] = {0,1,2,0,2,3};
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * ArrayCount(Indices), Indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 2, (void*)(0));
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

void InitTextureVertexArray(win32_renderer_gl * Renderer)
{
  glGenVertexArrays(1, &Renderer->VAOT);
  glGenBuffers(1, &Renderer->VBOT);
  glGenBuffers(1, &Renderer->EBOT);

  glBindVertexArray(Renderer->VAOT);
   // Don't remember why you have to do this..
  // It's like the object isn't initialized until something
  // so you have to just dump data here?
  glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBOT);
  f32 TokenVertices[16] =
  {
    1.0f, 1.0f,
    1.0f, 1.0f,
    1.0f, -1.0f,
    1.0f, 0.0f,
    -1.0f, -1.0f,
    0.0f, 0.0f,
    -1.0f, 1.0f,
    0.0f, 1.0f
  };

  glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 16, TokenVertices, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBOT);
  u32 Indices[6] = {0,1,2,0,2,3};
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * ArrayCount(Indices), Indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(0));
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 4, (void*)(2 * sizeof(f32)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

platform_renderer * CreateRenderer(u32 ScreenWidth, u32 ScreenHeight, HWND hwnd)
{
  HDC hdc = GetDC(hwnd);

  PIXELFORMATDESCRIPTOR Format  ={};
  Format.nSize = sizeof(Format);
  Format.nVersion = 1;
  Format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  Format.iPixelType = PFD_TYPE_RGBA;
  Format.cColorBits = 24;
  Format.cDepthBits = 32;
  Format.iLayerType = PFD_MAIN_PLANE;

  int PixelFormat = ChoosePixelFormat(hdc, &Format);
  SetPixelFormat(hdc, PixelFormat, &Format);

  u64 ArenaSize = Megabyte(1);
  win32_renderer_gl *Renderer = Win32_Allocate(sizeof(win32_renderer_gl) + ArenaSize);
  Renderer->hwnd    = hwnd;
  Renderer->Context = wglCreateContext(hdc);
  Renderer->ScreenWidth   = ScreenWidth;
  Renderer->ScreenHeight  = ScreenHeight;
  Renderer->Arena.Size    = ArenaSize;
  Renderer->Arena.Memory  = Win32_Allocate(ArenaSize);

  wglMakeCurrent(hdc, Renderer->Context);

  #if 0
  s32 MajorVersion, MinorVersion;
  glGetIntegerv(GL_MAJOR_VERSION, &MajorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &MinorVersion);
  #endif

  GL_LoadExtensions(Win32_GLGetProcAddress);

  Win32_LoadShaders(Renderer);


  InitQuadVertexArray(Renderer);
  InitTextureVertexArray(Renderer);
  InitTextVertexArray(Renderer);

  ReleaseDC(hwnd, hdc);

  return (platform_renderer*)Renderer;
}

void ReleaseRenderer(platform_renderer * PlatformRenderer)
{
  win32_renderer_gl * Renderer = (win32_renderer_gl*)PlatformRenderer;
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(Renderer->Context);
  DestroyWindow(Renderer->hwnd);

  Win32_Deallocate(Renderer, Renderer->Arena.Size + sizeof(win32_renderer_gl));
}