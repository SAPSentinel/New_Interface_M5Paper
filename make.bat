@echo off
REM M5Paper Emulator - Direct MSVC Compiler Build
REM This script compiles the project directly with MSVC cl.exe
REM No PlatformIO or SCons - maximum control and speed

setlocal enabledelayedexpansion

set "MSVC_CL=D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe"
set "MSVC_LINK=D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\link.exe"
set "SDL2_LIB=C:\vcpkg\installed\x64-windows\lib"
set "SDL2_BIN=C:\vcpkg\installed\x64-windows\bin"
set "SDL2_INCLUDE=C:\vcpkg\installed\x64-windows\include"

cd /d "%~dp0"

echo.
echo ============================================================
echo M5Paper Emulator - MSVC Direct Build
echo ============================================================
echo.

REM Initialize MSVC environment
echo Initializing MSVC compiler environment...
call "D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 (
    echo ERROR: Failed to initialize MSVC environment
    exit /b 1
)

REM Clean if requested
if "%1"=="--clean" (
    echo Cleaning build directory...
    if exist "build_msvc" rmdir /s /q "build_msvc" >nul 2>&1
)

REM Create build directory
if not exist "build_msvc" mkdir "build_msvc"

REM Copy lv_drv_conf.h to libdeps location so drivers can find it via relative path
echo Copying lv_drv_conf.h...
copy /y "lib\lv_drv_conf.h" ".pio\libdeps\emulator_m5paper\lv_drv_conf.h" >nul 2>&1

echo.
echo Compiling LVGL emulator...
echo.

REM Compile all LVGL source files
set "COMPILE_CMD=%MSVC_CL% /nologo /O2 /W3 /WX-"
set "COMPILE_CMD=!COMPILE_CMD! /I".""
set "COMPILE_CMD=!COMPILE_CMD! /I"lib""
set "COMPILE_CMD=!COMPILE_CMD! /I".pio\libdeps\emulator_m5paper\lvgl""
set "COMPILE_CMD=!COMPILE_CMD! /I".pio\libdeps\emulator_m5paper\lvgl\src""
set "COMPILE_CMD=!COMPILE_CMD! /I".pio\libdeps\emulator_m5paper\lv_drivers""
set "COMPILE_CMD=!COMPILE_CMD! /I"%SDL2_INCLUDE%""
set "COMPILE_CMD=!COMPILE_CMD! /I"src""
set "COMPILE_CMD=!COMPILE_CMD! /DUSE_MONITOR=1 /DUSE_MOUSE=1 /DSDL_MAIN_HANDLED /DNATIVE_64BIT /DLV_LVGL_H_INCLUDE_SIMPLE"
set "COMPILE_CMD=!COMPILE_CMD! /c /Fo"build_msvc\/""

REM List of source files (only essentials - skip unneeded display/input drivers)
set "SOURCES="
set "SOURCES=!SOURCES! "src\main.cpp""
REM LVGL core
set "SOURCES=!SOURCES! ".pio\libdeps\emulator_m5paper\lvgl\src\lv_core\*.c""
REM LVGL draw  
set "SOURCES=!SOURCES! ".pio\libdeps\emulator_m5paper\lvgl\src\lv_draw\*.c""
REM LVGL font
set "SOURCES=!SOURCES! ".pio\libdeps\emulator_m5paper\lvgl\src\lv_font\*.c""
REM LVGL misc
set "SOURCES=!SOURCES! ".pio\libdeps\emulator_m5paper\lvgl\src\lv_misc\*.c""
REM LVGL widgets
if exist ".pio\libdeps\emulator_m5paper\lvgl\src\lv_widgets" (
    set "SOURCES=!SOURCES! ".pio\libdeps\emulator_m5paper\lvgl\src\lv_widgets\*.c""
)
REM Only monitor and mouse drivers (lv_drivers.c doesn't exist)
set "SOURCES=!SOURCES! ".pio\libdeps\emulator_m5paper\lv_drivers\display\monitor.c""
set "SOURCES=!SOURCES! ".pio\libdeps\emulator_m5paper\lv_drivers\indev\mouse.c""

%COMPILE_CMD% %SOURCES%

if errorlevel 1 (
    echo.
    echo ============================================================
    echo COMPILATION FAILED
    echo ============================================================
    exit /b 1
)

echo.
echo Linking executable...

REM Link all object files
for /f "delims=" %%A in ('dir /s /b "build_msvc\*.obj" 2^>nul') do (
    set "OBJ_FILES=!OBJ_FILES! "%%A""
)

%MSVC_LINK% /nologo /OUT:"build_msvc\program.exe" ^
    /LIBPATH:"%SDL2_LIB%" SDL2.lib ^
    /SUBSYSTEM:CONSOLE ^
    %OBJ_FILES%

if errorlevel 1 (
    echo.
    echo ============================================================
    echo LINKING FAILED
    echo ============================================================
    exit /b 1
)

REM Copy SDL2.dll
if exist "%SDL2_BIN%\SDL2.dll" (
    copy /y "%SDL2_BIN%\SDL2.dll" "build_msvc\" >nul
)

echo.
echo ============================================================
echo [SUCCESS] BUILD COMPLETE
echo ============================================================
echo.
echo Executable: build_msvc\program.exe
echo To run: build_msvc\program.exe
echo.

exit /b 0
