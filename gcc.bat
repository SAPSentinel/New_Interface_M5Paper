@echo off
REM GCC-to-MSVC Compiler Wrapper
REM When SCons calls "gcc", this batch file intercepts it and calls MSVC cl.exe instead

setlocal enabledelayedexpansion

set MSVC_BIN=D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64
set VCPKG_LIB=C:\vcpkg\installed\x64-windows\lib
set VCPKG_INCLUDE=C:\vcpkg\installed\x64-windows\include

REM Set up MSVC environment
call "%PROGRAMFILES(X86)%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if errorlevel 1 (
    call "D:\VisualStudio2022\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
)

REM Ensure MSVC bin is in PATH
set "PATH=%MSVC_BIN%;%PATH%"

REM Convert GCC flags to MSVC flags
setlocal enabledelayedexpansion
set "MSVC_FLAGS=/nologo /W3 /O2"
set "MSVC_INCLUDES=-I%VCPKG_INCLUDE%"
set "MSVC_LIBDIRS=/LIBPATH:%VCPKG_LIB%"

REM Process arguments
set "ARGS="
set "IS_COMPILE=0"
set "IS_LINK=0"
set "OUTPUT_FILE="

for %%A in (%*) do (
    set "ARG=%%A"
    if "!ARG:~0,2!"=="/c" (
        set "IS_COMPILE=1"
    ) else if "!ARG:~0,3!"=="/Fo" (
        set "OUTPUT_FILE=!ARG:~3!"
    ) else if "!ARG!"=="-c" (
        REM GCC compile flag, ignore
    ) else if "!ARG:~0,2!"=="-o" (
        REM GCC output flag, convert to MSVC
        set "IS_LINK=1"
        if "!ARG:~2!"=="" (
            REM Output file is next argument
        ) else (
            set "OUTPUT_FILE=!ARG:~2!"
        )
    ) else if "!ARG:~0,1!"=="-" (
        REM Skip GCC flags starting with -
    ) else (
        set "ARGS=!ARGS! !ARG!"
    )
)

REM Call appropriate MSVC tool
if %IS_COMPILE%==1 (
    "%MSVC_BIN%\cl.exe" %MSVC_FLAGS% %MSVC_INCLUDES% /Fo"!OUTPUT_FILE!" /c %ARGS%
) else if %IS_LINK%==1 (
    "%MSVC_BIN%\link.exe" /NOLOGO /SUBSYSTEM:CONSOLE /OUT:"!OUTPUT_FILE!" %MSVC_LIBDIRS% %ARGS%
) else (
    "%MSVC_BIN%\cl.exe" %MSVC_FLAGS% %MSVC_INCLUDES% %ARGS%
)

endlocal
