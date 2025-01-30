@echo off
setlocal

set TARGET_PLATFORM=WEB

IF NOT EXIST .\build mkdir .\build
pushd .\build

if "%TARGET_PLATFORM%"=="WINDOWS" (
	set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4127 -wd4505 -wd4456 -wd4201 -wd4100 -wd4189 -FC -Z7 -DRENDERER_SOFTWARE=1 -DPLATFORM_WINDOWS=1
	set CommonLinkerFlags= -opt:ref user32.lib gdi32.lib winmm.lib ole32.lib winmm.lib Ws2_32.lib opengl32.lib


	call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
	cl %CommonCompilerFlags%  ..\src\invaders.c -Fminvaders.map -LD /link %CommonLinkerFlags% -incremental:no -opt:ref -PDB:invaders_%random%.pdb -EXPORT:GameUpdate -EXPORT:GameGetSoundSamples

	del *.pdb > NUL 2> NUL
	del *.rdi > NUL 2> NUL
	if %errorlevel% neq 0 exit /b 1
	cl %CommonCompilerFlags%  ..\src\win32_renderer_software.c -Fmwin32_renderer_software.map -LD /link %CommonLinkerFlags% -incremental:no -opt:ref -PDB:win32_software_renderer_%random%.pdb -EXPORT:ReleaseRenderer -EXPORT:BeginFrame -EXPORT:CreateRenderer -EXPORT:EndFrame
	if %errorlevel% neq 0 exit /b 1
	cl %CommonCompilerFlags%  ..\src\win32_renderer_gl.c -Fmwin32_renderer_software.map -LD /link %CommonLinkerFlags% -incremental:no -opt:ref -PDB:win32_software_renderer_%random%.pdb -EXPORT:ReleaseRenderer -EXPORT:BeginFrame -EXPORT:CreateRenderer -EXPORT:EndFrame
	if %errorlevel% neq 0 exit /b 1
	cl %CommonCompilerFlags% ..\src\win32_platform.c  /link %CommonLinkerFlags%
	if %errorlevel% neq 0 exit /b 1
) else (
	emcc ..\src\invaders.c -O3 -o .\invaders.wasm -sSIDE_MODULE=2
	REM emcc ..\src\emscripten_platform.c -sMAIN_MODULE=1 -O3 -s USE_SDL=2 -s FULL_ES3=1 -s WASM=1 -s GL_SUPPORT_AUTOMATIC_ENABLE_EXTENSIONS=1 -s GL_SUPPORT_SIMPLE_ENABLE_EXTENSIONS=1 -o .\hello.html
)
popd
