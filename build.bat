@echo off
setlocal

set TARGET_PLATFORM=WINDOWS

IF NOT EXIST .\build mkdir .\build
pushd .\build

set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4127 -wd4505 -wd4456 -wd4201 -wd4100 -wd4189 -FC -Z7 -DRENDERER_D3D=1 -DPLATFORM_WINDOWS=1
set CommonLinkerFlags= -opt:ref user32.lib gdi32.lib winmm.lib ole32.lib winmm.lib Ws2_32.lib opengl32.lib d3d11.lib dxgi.lib d3dcompiler.lib  dxguid.lib
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
del *.pdb > NUL 2> NUL
del *.rdi > NUL 2> NUL
cl %CommonCompilerFlags%  ..\src\invaders.c -Fminvaders.map -LD /link %CommonLinkerFlags% -incremental:no -opt:ref -PDB:invaders_%random%.pdb -EXPORT:GameUpdate -EXPORT:GameGetSoundSamples

cl %CommonCompilerFlags%  ..\src\win32_renderer_software.c -LD /link %CommonLinkerFlags% -incremental:no -opt:ref -PDB:win32_software_renderer_%random%.pdb -EXPORT:ReleaseRenderer -EXPORT:BeginFrame -EXPORT:CreateRenderer -EXPORT:EndFrame
cl %CommonCompilerFlags%  ..\src\win32_renderer_gl.c -LD /link %CommonLinkerFlags% -incremental:no -opt:ref -PDB:win32_software_renderer_%random%.pdb -EXPORT:ReleaseRenderer -EXPORT:BeginFrame -EXPORT:CreateRenderer -EXPORT:EndFrame
cl %CommonCompilerFlags%  ..\src\win32_renderer_d3d.c -LD /link %CommonLinkerFlags% d3d11.lib -incremental:no  -opt:ref -PDB:win32_software_renderer_%random%.pdb -EXPORT:ReleaseRenderer -EXPORT:BeginFrame -EXPORT:CreateRenderer -EXPORT:EndFrame
cl %CommonCompilerFlags% ..\src\win32_platform.c  /link %CommonLinkerFlags%
popd
