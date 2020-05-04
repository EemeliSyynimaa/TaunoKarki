@echo off

set "VSCMD_START_DIR=%cd%"
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

set release_flags= -MT -nologo -fp:fast -fp:except- -Gm- -GR -EHsa- -Oi -WX -W1 -I ../src
set debug_flags= -Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR -EHsa- -Zo -Oi -WX -W1 -Z7 -I ../src /diagnostics:column -FC
set linker_flags= /LIBPATH:"..\lib" OpenGL32.lib kernel32.lib user32.lib gdi32.lib

pushd ..
if not exist build mkdir build
pushd build

del *.pdb > NUL 2> NUL

rem cl %release_flags% -Fegame.exe ..\src\win32_main.c /link %linker_flags% 
cl %debug_flags% -Fegame.dll ..\src\game.c -LDd /link -incremental:no -PDB:game_%random%.pdb -opt:ref -EXPORT:game_init -EXPORT:game_update
cl %debug_flags% -Fegamed.exe ..\src\win32_main.c /link %linker_flags% 

xcopy "..\assets" "assets" /d /k /y /e /s /i

popd
popd