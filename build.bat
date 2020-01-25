@echo off

set "VSCMD_START_DIR=%cd%"
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

set release_flags= -MT -nologo -fp:fast -fp:except- -Gm- -GR -EHsa- -Oi -WX -W1 -I ../src
set debug_flags= -Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR -EHsa- -Zo -Oi -WX -W1 -Z7 -I ../src
set linker_flags= /LIBPATH:"..\lib" OpenGL32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Glu32.lib

if not exist build mkdir build

pushd build

rem cl %release_flags% -Fegame.exe ..\src\win32_main.cpp /link %linker_flags% 
cl %debug_flags% -Fegamed.exe ..\src\win32_main.cpp /link %linker_flags%

xcopy "..\assets" "assets" /d /k /y /e /s /i

popd
