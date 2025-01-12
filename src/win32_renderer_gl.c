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

vec2f Win32_TransformSSToCS(vec2f v, win32_renderer_gl * Renderer)
{
  v.X = v.X / (f32)Renderer->ScreenWidth * 2.0f - 1.0f;
  v.Y = v.Y / (f32)Renderer->ScreenHeight * 2.0f - 1.0f;

  return v;
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
        Entry.Size = 5;
        break;
      }
      case Pushbuffer_Entry_Rect_Color:
      {
        pushbuffer_entry_rect_color Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_color);
        shader Shader = Win32_GetShaderByName(Renderer, "quad");

        glBindVertexArray(Renderer->VAO);

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


        v0 = Win32_TransformSSToCS(v0, Renderer);
        v1 = Win32_TransformSSToCS(v1, Renderer);
        v2 = Win32_TransformSSToCS(v2, Renderer);
        v3 = Win32_TransformSSToCS(v3, Renderer);

        // Map to to Clip Space
        f32 VertexLocations[8] =
        {
          v3.X, v3.Y, // Top Right
          v1.X, v1.Y, // Bot Right
          v0.X, v0.Y, // Bot Left
          v2.X, v2.Y  // Top Left
        };

        // Send it
        glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 8, VertexLocations, GL_DYNAMIC_DRAW);

        // Draw it baby
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        break;
      }
      case Pushbuffer_Entry_Rect_Texture:
      {
        pushbuffer_entry_rect_texture Entry = Pushbuffer_Read(Pushbuffer, pushbuffer_entry_rect_texture);
        Entry.Texture  = 0;
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

  // Load shaders!

  Win32_LoadShaders(Renderer);



  glGenVertexArrays(1, &Renderer->VAO);
  glGenBuffers(1, &Renderer->VBO);
  glGenBuffers(1, &Renderer->EBO);

  glBindVertexArray(Renderer->VAO);
  // Don't remember why you have to do this..
  // It's like the object isn't initialized until something
  // so you have to just dump data here?
  glBindBuffer(GL_ARRAY_BUFFER, Renderer->VBO);
  f32 TokenVertices[8] =
  {
    1.0f, 1.0f,
    1.0f, -1.0f,
    -1.0f, -1.0f,
    -1.0f, 1.0f
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * 8, TokenVertices, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderer->EBO);
  u32 Indices[6] = {0,1,2,0,2,3};
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * ArrayCount(Indices), Indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(f32) * 2, (void*)(0));
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

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