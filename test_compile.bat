@echo off
cd /d "D:\platform-io\New_Interface_M5Paper"
call "D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul
"D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe" /nologo /O2 /W3 ^
  /I"." /I"lib" /I".pio\libdeps\emulator_m5paper\lvgl\src" ^
  /I".pio\libdeps\emulator_m5paper\lv_drivers" /I"C:\vcpkg\installed\x64-windows\include" ^
  /I"src" /DUSE_MONITOR=1 /DUSE_MOUSE=1 /DSDL_MAIN_HANDLED /DNATIVE_64BIT ^
  /c "src\main.cpp" /Fo"test_main.obj"
echo Exit code: %ERRORLEVEL%
