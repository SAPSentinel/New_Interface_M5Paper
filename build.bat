@echo off
REM Simple wrapper to build with PlatformIO from the x64 Command Prompt environment

setlocal

REM Ensure we're in the project directory
cd /d "%~dp0"

echo.
echo ============================================================
echo Building M5Paper Emulator with PlatformIO
echo ============================================================
echo.

REM Try to initialize MSVC (may already be initialized)
if exist "D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat" (
    call "D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
)

REM Clean build directory if --clean flag
if "%1"=="--clean" (
    echo Cleaning...
    rmdir /s /q ".pio\build\emulator_m5paper" 2>nul
)

REM Run PlatformIO build
echo Running PlatformIO build...
python -m platformio run -e emulator_m5paper

if errorlevel 1 (
    echo.
    echo ============================================================
    echo BUILD FAILED
    echo ============================================================
    exit /b 1
)

REM Copy SDL2.dll
echo.
echo Copying SDL2.dll...
if exist "C:\vcpkg\installed\x64-windows\bin\SDL2.dll" (
    copy /y "C:\vcpkg\installed\x64-windows\bin\SDL2.dll" ".pio\build\emulator_m5paper\" >nul
)

echo.
echo ============================================================
echo BUILD SUCCESSFUL
echo ============================================================
echo.
echo Output: .pio\build\emulator_m5paper\program.exe
echo To run: .pio\build\emulator_m5paper\program.exe
echo.

exit /b 0
