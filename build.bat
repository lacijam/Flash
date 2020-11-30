@echo off

set VC_VERSION=2019
set APP_NAME=Flash
set SRC_PATH=..\src
set OUTPUT_PATH=..
set /A DEBUGGER_FLAG=0
set /A CLEAN_UP_TMP=0

if "%1" == "d" (
    if not exist "build" (
        mkdir build
    )

    if exist "*.pdb" (
        del "*.pdb" /q
    )

    pushd build
    if not defined DevEnvDir (
        call "C:\Program Files (x86)\Microsoft Visual Studio\%VC_VERSION%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    )

    cl %SRC_PATH%\*.cpp /ZI /Od /W4 /MDd /EHsc /D "_DEBUG" /D "_UNICODE" /D "UNICODE" /Fe%APP_NAME% kernel32.lib user32.lib gdi32.lib opengl32.lib /link /LIBPATH:".." /DEBUG:FASTLINK /INCREMENTAL /SUBSYSTEM:WINDOWS
    
    if ERRORLEVEL == 0 (
        if "%2" == "d" (
            set /A DEBUGGER_FLAG = 1
        )

        set /A CLEAN_UP_TMP=1

        goto cleanup
    )

    goto end
) else (
    if "%1"=="r" (
        if not exist "build" (
            mkdir build
        )

        if exist "%APP_NAME%.pdb" (
            del "%APP_NAME%.pdb" /q
        )

        pushd build
        if not defined DevEnvDir (
            call "C:\Program Files (x86)\Microsoft Visual Studio\%VC_VERSION%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
        )

        cl %SRC_PATH%\*.cpp /Zi /O2 /Oi /GL /MD /W3 /EHsc /D "_NDEBUG" /D "_UNICODE" /D "UNICODE" /Fe%APP_NAME% kernel32.lib user32.lib gdi32.lib opengl32.lib /link /LIBPATH:".." /DEBUG:FULL /INCREMENTAL:NO /OPT:REF /OPT:ICF /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup
        
        if ERRORLEVEL == 0 (
            goto cleanup
        )

        goto end
    ) else (
        echo "./build.bat d    - Build for debug"
        echo "./build.bat d d  - Build for debug and launch debugger"
        echo "./build.bat r    - Build for release"
        goto end
    )
)

:cleanup
copy /y .\%APP_NAME%.exe %OUTPUT_PATH%
copy /y .\%APP_NAME%.pdb %OUTPUT_PATH%

popd
if %DEBUGGER_FLAG%==1 (
    devenv /DebugExe %APP_NAME%.exe
)

:end
exit /b ERRORLEVEL
