#include <gl/gl.h>
#include <gl/glext.h>

PFNGLCREATESHADERPROC             glCreateShader             = NULL;
PFNGLCOMPILESHADERPROC            glCompileShader            = NULL;
PFNGLGETSHADERIVPROC              glGetShaderiv              = NULL;
PFNGLGETSHADERINFOLOGPROC         glGetShaderInfoLog         = NULL;
PFNGLCREATEPROGRAMPROC            glCreateProgram            = NULL;
PFNGLBINDATTRIBLOCATIONPROC       glBindAttribLocation       = NULL;
PFNGLLINKPROGRAMPROC              glLinkProgram              = NULL;
PFNGLSHADERSOURCEPROC             glShaderSource             = NULL;
PFNGLGETPROGRAMIVPROC             glGetProgramiv             = NULL;
PFNGLGETPROGRAMINFOLOGPROC        glGetProgramInfoLog        = NULL;
PFNGLDETACHSHADERPROC             glDetachShader             = NULL;
PFNGLDELETESHADERPROC             glDeleteShader             = NULL;
PFNGLDELETEPROGRAMPROC            glDeleteProgram            = NULL;
PFNGLUSEPROGRAMPROC               glUseProgram               = NULL;
PFNGLATTACHSHADERPROC             glAttachShader             = NULL;
PFNGLGETUNIFORMLOCATIONPROC       glGetUniformLocation       = NULL;
PFNGLUNIFORMMATRIX4FVPROC         glUniformMatrix4fv         = NULL;
PFNGLUNIFORMMATRIX3FVPROC         glUniformMatrix3fv         = NULL;
PFNGLGENVERTEXARRAYSPROC          glGenVertexArrays          = NULL;
PFNGLGENBUFFERSPROC               glGenBuffers               = NULL;
PFNGLBINDBUFFERPROC               glBindBuffer               = NULL;
PFNGLBUFFERDATAPROC               glBufferData               = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC  glEnableVertexAttribArray  = NULL;
PFNGLVERTEXATTRIBPOINTERPROC      glVertexAttribPointer      = NULL;
PFNGLVERTEXATTRIBIPOINTERPROC     glVertexAttribIPointer     = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = NULL;
PFNGLDELETEBUFFERSPROC            glDeleteBuffers            = NULL;
PFNGLDELETEVERTEXARRAYSPROC       glDeleteVertexArrays       = NULL;
PFNGLUNIFORM1IPROC                glUniform1i                = NULL;
PFNGLGENERATEMIPMAPPROC           glGenerateMipmap           = NULL;
PFNGLUNIFORM2FVPROC               glUniform2fv               = NULL;
PFNGLUNIFORM3FVPROC               glUniform3fv               = NULL;
PFNGLUNIFORM4FVPROC               glUniform4fv               = NULL;
PFNGLMAPNAMEDBUFFERPROC           glMapNamedBuffer           = NULL;
PFNGLMAPBUFFERPROC                glMapBuffer                = NULL;
PFNGLUNMAPBUFFERPROC              glUnmapBuffer              = NULL;
PFNGLUNIFORM1FPROC                glUniform1f                = NULL;
PFNGLGENFRAMEBUFFERSPROC          glGenFramebuffers          = NULL;
PFNGLDELETEFRAMEBUFFERSPROC       glDeleteFramebuffers       = NULL;
PFNGLBINDFRAMEBUFFERPROC          glBindFramebuffer          = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC     glFramebufferTexture2D     = NULL;
PFNGLGENRENDERBUFFERSPROC         glGenRenderbuffers         = NULL;
PFNGLBINDRENDERBUFFERPROC         glBindRenderbuffer         = NULL;
PFNGLRENDERBUFFERSTORAGEPROC      glRenderbufferStorage      = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC  glFramebufferRenderbuffer  = NULL;
PFNGLDRAWBUFFERSPROC              glDrawBuffers              = NULL;
PFNGLDELETERENDERBUFFERSPROC      glDeleteRenderbuffers      = NULL;
PFNGLBLENDFUNCSEPARATEPROC        glBlendFuncSeparate        = NULL;
PFNGLBINDVERTEXARRAYPROC          glBindVertexArray          = NULL;
PFNGLNAMEDBUFFERSTORAGEPROC       glNamedBufferStorage       = NULL;
PFNGLBUFFERSTORAGEPROC            glBufferStorage            = NULL;
PFNGLBUFFERSUBDATAPROC            glBufferSubData            = NULL;
PFNGLNAMEDBUFFERSUBDATAPROC       glNamedBufferSubData       = NULL;
PFNGLCREATEVERTEXARRAYSPROC       glCreateVertexArrays       = NULL;
PFNGLCREATEBUFFERSPROC            glCreateBuffers            = NULL;
PFNGLVERTEXARRAYVERTEXBUFFERPROC  glVertexArrayVertexBuffer  = NULL;
PFNGLENABLEVERTEXARRAYATTRIBPROC  glEnableVertexArrayAttrib  = NULL;
PFNGLVERTEXARRAYATTRIBFORMATPROC  glVertexArrayAttribFormat  = NULL;
PFNGLVERTEXARRAYATTRIBBINDINGPROC glVertexArrayAttribBinding = NULL;
PFNGLGETSTRINGIPROC               glGetStringi               = NULL;
PFNGLBINDSAMPLERPROC              glBindSampler              = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC    glBlendEquationSeparate    = NULL;
PFNGLGETATTRIBLOCATIONPROC        glGetAttribLocation        = NULL;
PFNGLISPROGRAMPROC                glIsProgram                = NULL;
PFNGLDRAWELEMENTSBASEVERTEXPROC   glDrawElementsBaseVertex   = NULL;
PFNGLFRAMEBUFFERTEXTUREPROC       glFramebufferTexture       = NULL;


void GL_LoadExtensions(void*(LoadProcedure(const char *)))
{

  glFramebufferTexture       = (PFNGLFRAMEBUFFERTEXTUREPROC)LoadProcedure("glFramebufferTexture");
  glDrawElementsBaseVertex   = (PFNGLDRAWELEMENTSBASEVERTEXPROC)LoadProcedure("glDrawElementsBaseVertex");
  glIsProgram                = (PFNGLISPROGRAMPROC)LoadProcedure("glIsProgram");
  glGetAttribLocation        = (PFNGLGETATTRIBLOCATIONPROC)LoadProcedure("glGetAttribLocation");
  glBlendEquationSeparate    = (PFNGLBLENDEQUATIONSEPARATEPROC)LoadProcedure("glBlendEquationSeparate");
  glBindSampler              = (PFNGLBINDSAMPLERPROC)LoadProcedure("glBindSampler");
  glGetStringi               = (PFNGLGETSTRINGIPROC)LoadProcedure("glGetStringi");
  glBufferSubData            = (PFNGLBUFFERSUBDATAPROC)LoadProcedure("glBufferSubData");
  glNamedBufferSubData       = (PFNGLNAMEDBUFFERSUBDATAPROC)LoadProcedure("glNamedBufferSubData");
  glCreateShader             = (PFNGLCREATESHADERPROC)LoadProcedure("glCreateShader");
  glCompileShader            = (PFNGLCOMPILESHADERPROC)LoadProcedure("glCompileShader");
  glGetShaderiv              = (PFNGLGETSHADERIVPROC)LoadProcedure("glGetShaderiv");
  glGetShaderInfoLog         = (PFNGLGETSHADERINFOLOGPROC)LoadProcedure("glGetShaderInfoLog");
  glCreateProgram            = (PFNGLCREATEPROGRAMPROC)LoadProcedure("glCreateProgram");
  glBindAttribLocation       = (PFNGLBINDATTRIBLOCATIONPROC)LoadProcedure("glBindAttribLocation");
  glLinkProgram              = (PFNGLLINKPROGRAMPROC)LoadProcedure("glLinkProgram");
  glShaderSource             = (PFNGLSHADERSOURCEPROC)LoadProcedure("glShaderSource");
  glLinkProgram              = (PFNGLLINKPROGRAMPROC)LoadProcedure("glLinkProgram");
  glGetProgramiv             = (PFNGLGETPROGRAMIVPROC)LoadProcedure("glGetProgramiv");
  glGetProgramInfoLog        = (PFNGLGETPROGRAMINFOLOGPROC)LoadProcedure("glGetProgramInfoLog");
  glDetachShader             = (PFNGLDETACHSHADERPROC)LoadProcedure("glDetachShader");
  glDeleteShader             = (PFNGLDELETESHADERPROC)LoadProcedure("glDeleteShader");
  glDeleteProgram            = (PFNGLDELETEPROGRAMPROC)LoadProcedure("glDeleteProgram");
  glUseProgram               = (PFNGLUSEPROGRAMPROC)LoadProcedure("glUseProgram");
  glAttachShader             = (PFNGLATTACHSHADERPROC)LoadProcedure("glAttachShader");
  glGetUniformLocation       = (PFNGLGETUNIFORMLOCATIONPROC)LoadProcedure("glGetUniformLocation");
  glUniformMatrix4fv         = (PFNGLUNIFORMMATRIX4FVPROC)LoadProcedure("glUniformMatrix4fv");
  glUniformMatrix3fv         = (PFNGLUNIFORMMATRIX3FVPROC)LoadProcedure("glUniformMatrix3fv");
  glGenVertexArrays          = (PFNGLGENVERTEXARRAYSPROC)LoadProcedure("glGenVertexArrays");
  glBindVertexArray          = (PFNGLBINDVERTEXARRAYPROC)LoadProcedure("glBindVertexArray");
  glGenBuffers               = (PFNGLGENBUFFERSPROC)LoadProcedure("glGenBuffers");
  glBindBuffer               = (PFNGLBINDBUFFERPROC)LoadProcedure("glBindBuffer");
  glBufferData               = (PFNGLBUFFERDATAPROC)LoadProcedure("glBufferData");
  glEnableVertexAttribArray  = (PFNGLENABLEVERTEXATTRIBARRAYPROC)LoadProcedure("glEnableVertexAttribArray");
  glVertexAttribPointer      = (PFNGLVERTEXATTRIBPOINTERPROC)LoadProcedure("glVertexAttribPointer");
  glVertexAttribIPointer     = (PFNGLVERTEXATTRIBIPOINTERPROC)LoadProcedure("glVertexAttribIPointer");
  glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)LoadProcedure("glDisableVertexAttribArray");
  glDeleteBuffers            = (PFNGLDELETEBUFFERSPROC)LoadProcedure("glDeleteBuffers");
  glDeleteVertexArrays       = (PFNGLDELETEVERTEXARRAYSPROC)LoadProcedure("glDeleteVertexArrays");
  glUniform1i                = (PFNGLUNIFORM1IPROC)LoadProcedure("glUniform1i");
  glGenerateMipmap           = (PFNGLGENERATEMIPMAPPROC)LoadProcedure("glGenerateMipmap");
  glUniform2fv               = (PFNGLUNIFORM2FVPROC)LoadProcedure("glUniform2fv");
  glUniform3fv               = (PFNGLUNIFORM3FVPROC)LoadProcedure("glUniform3fv");
  glUniform4fv               = (PFNGLUNIFORM4FVPROC)LoadProcedure("glUniform4fv");
  glMapNamedBuffer           = (PFNGLMAPNAMEDBUFFERPROC)LoadProcedure("glMapNamedBuffer");
  glMapBuffer                = (PFNGLMAPBUFFERPROC)LoadProcedure("glMapBuffer");
  glUnmapBuffer              = (PFNGLUNMAPBUFFERPROC)LoadProcedure("glUnmapBuffer");
  glUniform1f                = (PFNGLUNIFORM1FPROC)LoadProcedure("glUniform1f");
  glGenFramebuffers          = (PFNGLGENFRAMEBUFFERSPROC)LoadProcedure("glGenFramebuffers");
  glDeleteFramebuffers       = (PFNGLDELETEFRAMEBUFFERSPROC)LoadProcedure("glDeleteFramebuffers");
  glBindFramebuffer          = (PFNGLBINDFRAMEBUFFERPROC)LoadProcedure("glBindFramebuffer");
  glFramebufferTexture2D     = (PFNGLFRAMEBUFFERTEXTURE2DPROC)LoadProcedure("glFramebufferTexture2D");
  glGenRenderbuffers         = (PFNGLGENRENDERBUFFERSPROC)LoadProcedure("glGenRenderbuffers");
  glBindRenderbuffer         = (PFNGLBINDRENDERBUFFERPROC)LoadProcedure("glBindRenderbuffer");
  glRenderbufferStorage      = (PFNGLRENDERBUFFERSTORAGEPROC)LoadProcedure("glRenderbufferStorage");
  glFramebufferRenderbuffer  = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)LoadProcedure("glFramebufferRenderbuffer");
  glDrawBuffers              = (PFNGLDRAWBUFFERSPROC)LoadProcedure("glDrawBuffers");
  glDeleteRenderbuffers      = (PFNGLDELETERENDERBUFFERSPROC)LoadProcedure("glDeleteRenderbuffers");
  glBlendFuncSeparate        = (PFNGLBLENDFUNCSEPARATEPROC)LoadProcedure("glBlendFuncSeparate");
  glCreateVertexArrays       = (PFNGLCREATEVERTEXARRAYSPROC)LoadProcedure("glCreateVertexArrays");
  glCreateBuffers            = (PFNGLCREATEBUFFERSPROC)LoadProcedure("glCreateBuffers");
  glNamedBufferStorage       = (PFNGLNAMEDBUFFERSTORAGEPROC)LoadProcedure("glNamedBufferStorage");
  glVertexArrayVertexBuffer  = (PFNGLVERTEXARRAYVERTEXBUFFERPROC)LoadProcedure("glVertexArrayVertexBuffer");
  glEnableVertexArrayAttrib  = (PFNGLENABLEVERTEXARRAYATTRIBPROC)LoadProcedure("glEnableVertexArrayAttrib");
  glVertexArrayAttribFormat  = (PFNGLVERTEXARRAYATTRIBFORMATPROC)LoadProcedure("glVertexArrayAttribFormat");
  glVertexArrayAttribBinding = (PFNGLVERTEXARRAYATTRIBBINDINGPROC)LoadProcedure("glVertexArrayAttribBinding");
}