@echo off
REM M5Paper Emulator - Comprehensive Direct MSVC Compiler Build
REM Compiles ALL project source files + LVGL + SDL2

setlocal enabledelayedexpansion

set "MSVC_CL=D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe"
set "MSVC_LINK=D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\link.exe"
set "SDL2_LIB=C:\vcpkg\installed\x64-windows\lib"
set "SDL2_BIN=C:\vcpkg\installed\x64-windows\bin"
set "SDL2_INCLUDE=C:\vcpkg\installed\x64-windows\include"

cd /d "%~dp0"

echo.
echo ============================================================
echo M5Paper Emulator - Comprehensive MSVC Direct Build
echo Compiling all sources: LVGL + lv_drivers + project code
echo ============================================================
echo.

REM Initialize MSVC environment
echo Initializing MSVC compiler environment...
call "D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
if errorlevel 1 (
    echo ERROR: Failed to initialize MSVC environment
    exit /b 1
)

REM Clean if requested
if "%1"=="--clean" (
    echo Cleaning build directory...
    if exist "build_all" rmdir /s /q "build_all" >nul 2>&1
)

REM Create build directory
if not exist "build_all" mkdir "build_all"

REM Copy lv_drv_conf.h to libdeps location so drivers can find it via relative path
echo Copying lv_drv_conf.h...
copy /y "lib\lv_drv_conf.h" ".pio\libdeps\emulator_m5paper\lv_drv_conf.h" >nul 2>&1

echo.
echo Compiling all source files...
echo.

set "COMPILE_CMD=%MSVC_CL% /nologo /O2 /W3 /WX-"
set "COMPILE_CMD=!COMPILE_CMD! /I".""
set "COMPILE_CMD=!COMPILE_CMD! /I"lib""
set "COMPILE_CMD=!COMPILE_CMD! /I".pio\libdeps\emulator_m5paper\lvgl""
set "COMPILE_CMD=!COMPILE_CMD! /I".pio\libdeps\emulator_m5paper\lvgl\src""
set "COMPILE_CMD=!COMPILE_CMD! /I".pio\libdeps\emulator_m5paper\lv_drivers""
set "COMPILE_CMD=!COMPILE_CMD! /I"%SDL2_INCLUDE%""
set "COMPILE_CMD=!COMPILE_CMD! /I"src""
set "COMPILE_CMD=!COMPILE_CMD! /DUSE_MONITOR=1 /DUSE_MOUSE=1 /DSDL_MAIN_HANDLED /DNATIVE_64BIT /DLV_LVGL_H_INCLUDE_SIMPLE"
set "COMPILE_CMD=!COMPILE_CMD! /c /Fo"build_all\/""

REM Build source list - everything we need
set "SOURCES="

REM Main source
set "SOURCES=!SOURCES! "src\main.cpp""

REM All project source files from src tree
for /r "src" %%f in (*.cpp) do (
    set "SOURCES=!SOURCES! "%%f""
)

REM LVGL core modules (alphabetically)
for /r ".pio\libdeps\emulator_m5paper\lvgl\src\lv_core" %%f in (*.c) do (
    set "SOURCES=!SOURCES! "%%f""
)

REM LVGL draw modules
for /r ".pio\libdeps\emulator_m5paper\lvgl\src\lv_draw" %%f in (*.c) do (
    set "SOURCES=!SOURCES! "%%f""
)

REM LVGL font modules
for /r ".pio\libdeps\emulator_m5paper\lvgl\src\lv_font" %%f in (*.c) do (
    set "SOURCES=!SOURCES! "%%f""
)

REM LVGL  misc modules
for /r ".pio\libdeps\emulator_m5paper\lvgl\src\lv_misc" %%f in (*.c) do (
    set "SOURCES=!SOURCES! "%%f""
)

REM LVGL theme modules (if exist)
if exist ".pio\libdeps\emulator_m5paper\lvgl\src\lv_themes" (
    for /r ".pio\libdeps\emulator_m5paper\lvgl\src\lv_themes" %%f in (*.c) do (
        set "SOURCES=!SOURCES! "%%f""
    )
)

REM LVGL widget modules (if exist)
if exist ".pio\libdeps\emulator_m5paper\lvgl\src\lv_widgets" (
    for /r ".pio\libdeps\emulator_m5paper\lvgl\src\lv_widgets" %%f in (*.c) do (
        set "SOURCES=!SOURCES! "%%f""
    )
)

REM LVGL hal modules (if exist)
if exist ".pio\libdeps\emulator_m5paper\lvgl\src\lv_hal" (
    for /r ".pio\libdeps\emulator_m5paper\lvgl\src\lv_hal" %%f in (*.c) do (
        set "SOURCES=!SOURCES! "%%f""
    )
)

REM Only essential lv_drivers (monitor display + mouse input)
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
for /f "delims=" %%A in ('dir /s /b "build_all\*.obj" 2^>nul') do (
    set "OBJ_FILES=!OBJ_FILES! "%%A""
)

%MSVC_LINK% /nologo /OUT:"build_all\program.exe" ^
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
    copy /y "%SDL2_BIN%\SDL2.dll" "build_all\" >nul
)

echo.
echo ============================================================
echo [SUCCESS] BUILD COMPLETE
echo ============================================================
echo.
echo Executable: build_all\program.exe
echo To run: build_all\program.exe
echo.

