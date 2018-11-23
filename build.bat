@echo off

set APP_NAME=Flash
set SDL2_INCLUDE="..\external\SDL2-2.0.9\include"
set SDL2_TTF_INCLUDE="..\external\SDL2_ttf-2.0.14\include"
set SDL2_LIB="..\external\SDL2-2.0.9\lib\x64"
set SDL2_TTF_LIB="..\external\SDL2_ttf-2.0.14\lib\x64"
set SRC_PATH=..\src
set OUTPUT_PATH=..

mkdir build
pushd build
if not defined DevEnvDir (
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)
cl %SRC_PATH%\*.cpp /O2 /EHsc /Fe%APP_NAME%.exe /I %SDL2_INCLUDE% /I %SDL2_TTF_INCLUDE% /link /LIBPATH:%SDL2_LIB% SDL2.lib SDL2main.lib /LIBPATH:%SDL2_TTF_LIB% SDL2_ttf.lib /SUBSYSTEM:CONSOLE
move /Y %APP_NAME%.exe %OUTPUT_PATH%
popd
if "%1"=="-r" (
    pause
    start "" "%APP_NAME%.exe"
    exit
)
