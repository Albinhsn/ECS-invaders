@echo off

set CommonCompilerFlags=-MT -nologo -Gm- -GR- -EHa- -O0 -Oi -WX -W4 -wd4505 -wd4456 -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7
set CommonLinkerFlags= -opt:ref user32.lib gdi32.lib winmm.lib

IF NOT EXIST .\build mkdir .\build
pushd .\build

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

del *.pdb > NUL 2> NUL
del *.rdi > NUL 2> NUL
REM 64-bit build
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags%  ..\src\invaders.c -Fminvaders.map -LD /link -incremental:no -opt:ref -PDB:invaders_%random%.pdb -EXPORT:GameUpdate
del lock.tmp
cl %CommonCompilerFlags% ..\src\win32_platform.c  /link %CommonLinkerFlags%
popd
