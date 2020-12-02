@echo off

set VC_VERSION=2019
set APP_NAME=Flash
set PROJECT_PATH=%~dp0
set SRC_PATH=\src
set /A DEBUGGER_FLAG=0
set /A CLEAN_UP_TMP=0
setlocal EnableDelayedExpansion

if "%1" == "d" (
    if exist "%APP_NAME%.pdb" (
        del "%APP_NAME%.pdb" /q
    )

    if not exist "build" (
        mkdir build
    )

    pushd build

    if not exist "%APP_NAME%" (
        mkdir %APP_NAME%
    )
        
    pushd %APP_NAME%

    if not defined DevEnvDir (
        call "C:\Program Files (x86)\Microsoft Visual Studio\%VC_VERSION%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    )

    cl %PROJECT_PATH%\%SRC_PATH%\%APP_NAME%\*.cpp /ZI /Od /W4 /MDd /EHsc /D "_DEBUG" /D "_UNICODE" /D "UNICODE" /Fe%APP_NAME% kernel32.lib user32.lib gdi32.lib opengl32.lib /link /LIBPATH:".." /DEBUG:FASTLINK /INCREMENTAL /SUBSYSTEM:WINDOWS
    
    if !errorlevel! equ 0 (
        if "%2" == "d" (
            set /A DEBUGGER_FLAG = 1
        )

        set /A CLEAN_UP_TMP=1

        goto cleanup
    )

    goto end
) else (
    if "%1"=="r" (
        if exist "%APP_NAME%.pdb" (
            del "%APP_NAME%.pdb" /q
        )

        if not exist "build" (
            mkdir build
        )

        pushd build

        if not exist "%APP_NAME%" (
            mkdir %APP_NAME%
        )
        
        pushd %APP_NAME%

        if not defined DevEnvDir (
            call "C:\Program Files (x86)\Microsoft Visual Studio\%VC_VERSION%\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
        )

        cl %PROJECT_PATH%\%SRC_PATH%\%APP_NAME%\*.cpp /Zi /O2 /Oi /GL /MD /W3 /EHsc /D "_NDEBUG" /D "_UNICODE" /D "UNICODE" /Fe%APP_NAME% kernel32.lib user32.lib gdi32.lib opengl32.lib /link /LIBPATH:".." /DEBUG:FULL /INCREMENTAL:NO /OPT:REF /OPT:ICF /SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup
        
        if !errorlevel! equ 0 (
            goto cleanup
        )

        goto end
    ) else (
        if "%1"=="D" (
            goto debugger
        ) else (
            echo "./build.bat d    - Build for debug"
            echo "./build.bat d D  - Build for debug and launch debugger"
            echo "./build.bat r    - Build for release"
            echo "./build.bat r D  - Build for release and launch debugger"
            echo "./build.bat D    - Launch debugger"
            goto end
        )
    )
)

:cleanup
copy /y .\%APP_NAME%.exe %PROJECT_PATH%
copy /y .\%APP_NAME%.pdb %PROJECT_PATH%

popd
popd

:debugger
if %DEBUGGER_FLAG%==1 (
    if exist %APP_NAME%.exe (
        devenv /DebugExe %APP_NAME%.exe
    )
)

:end
exit /b ERRORLEVEL

