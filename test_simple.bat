@echo off
call "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
cd /d "%~dp0"
if not exist build_test mkdir build_test
cd build_test
cl.exe /nologo /O2 /W3 /I".." /I"..\lib" /I"..\src" /DNATIVE_64BIT /c "..\src\main.cpp"
echo Compilation exit code: %errorlevel%
