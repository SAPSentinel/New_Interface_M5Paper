@echo off
REM M5Paper Emulator Direct MSVC Build Script
REM Bypasses PlatformIO/SCons, uses MSVC cl.exe directly

setlocal enabledelayedexpansion

REM ========== CONFIGURATION ==========
set "MSVC_BIN=D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64"
set "SDL2_LIB=C:\vcpkg\installed\x64-windows\lib"
set "SDL2_BIN=C:\vcpkg\installed\x64-windows\bin"
set "SDL2_INCLUDE=C:\vcpkg\installed\x64-windows\include"
set "BUILD_DIR=build_msvc"
set "OUTPUT=program.exe"

REM ========== INITIALIZE MSVC ENVIRONMENT ==========
echo.
echo ============================================================
echo M5Paper Emulator Direct MSVC Build
echo ============================================================
echo.

call "D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 (
    echo ERROR: Failed to initialize MSVC environment
    exit /b 1
)

echo [1/3] Environment initialized
echo.

REM ========== CLEAN BUILD DIR ==========
if exist "%BUILD_DIR%" rd /s /q "%BUILD_DIR%"
mkdir "%BUILD_DIR%"

echo [2/3] Compiling source files...
echo.

REM ========== SOURCE FILES ==========
set "SOURCES="
REM LVGL core files
for /r "lib\LVGL\lvgl\lv_core" %%F in (*.c) do set "SOURCES=!SOURCES! "%%F""
for /r "lib\LVGL\lvgl\lv_draw" %%F in (*.c) do set "SOURCES=!SOURCES! "%%F""
for /r "lib\LVGL\lvgl\lv_font" %%F in (*.c) do set "SOURCES=!SOURCES! "%%F""
for /r "lib\LVGL\lvgl\lv_misc" %%F in (*.c) do set "SOURCES=!SOURCES! "%%F""

REM lv_drivers files
set "SOURCES=!SOURCES! "lib\lv_drivers\lv_drivers.c""
set "SOURCES=!SOURCES! "lib\lv_drivers\display\monitor.c""
set "SOURCES=!SOURCES! "lib\lv_drivers\indev\mouse.c""

REM Main application
set "SOURCES=!SOURCES! "src\main.cpp""

REM ========== COMPILE ==========
"%MSVC_BIN%\cl.exe" /nologo /O2 /W3 /WX- ^
    /I"lib" ^
    /I"%SDL2_INCLUDE%" ^
    /I"src" ^
    /DUSE_MONITOR=1 /DUSE_MOUSE=1 /DSDL_MAIN_HANDLED ^
    /LDd /Fe"%BUILD_DIR%\%OUTPUT%" ^
    %SOURCES% ^
    /link "/LIBPATH:%SDL2_LIB%" SDL2.lib SDL2main.lib

if errorlevel 1 (
    echo.
    echo ============================================================
    echo COMPILATION FAILED
    echo ============================================================
    exit /b 1
)

echo.
echo [3/3] Copying runtime dependencies...

REM ========== COPY SDL2.DLL ==========
if exist "%SDL2_BIN%\SDL2.dll" (
    copy /y "%SDL2_BIN%\SDL2.dll" "%BUILD_DIR%\SDL2.dll" >nul
    echo ✓ SDL2.dll copied
) else (
    echo ⚠ Warning: SDL2.dll not found
)

echo.
echo ============================================================
echo ✓ BUILD SUCCESSFUL
echo ============================================================
echo.
echo Output: %BUILD_DIR%\%OUTPUT%
echo To run: %BUILD_DIR%\%OUTPUT%
echo.

exit /b 0
