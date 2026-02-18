"""
Post-build extra_script: runs AFTER PlatformIO native platform main.py.

PlatformIO's native main.py calls env.Tool("gcc") / env.Tool("g++") which on
Windows sets CC=gcc / CXX=g++ but leaves LINK = "link" (MSVC link.exe) and
AR = "lib" or "ar" that may resolve to wrong tools.

Here we load the gnulink SCons tool and override all toolchain vars to point
explicitly at MSYS2 MinGW64 binaries, so the final link and archive steps use
MinGW rather than MSVC.
"""

Import("env")
import os

MINGW_BIN  = "C:/msys64/mingw64/bin"
MINGW_BASE = "C:/msys64/mingw64"

def mingw(tool):
    return os.path.join(MINGW_BIN, tool).replace("\\", "/")

# ─── Load gnulink SCons tool (sets LINK=$CXX, LINKFLAGS=[], LINKCOM GNU-style)
env.Tool("gnulink")

# ─── Force all toolchain vars to MinGW64 ─────────────────────────────────────
env.Replace(
    CC      = mingw("gcc.exe"),
    CXX     = mingw("g++.exe"),
    LINK    = mingw("g++.exe"),
    AR      = mingw("ar.exe"),
    RANLIB  = mingw("ranlib.exe"),
    AS      = mingw("as.exe"),
    ARFLAGS = ["rcs"],
    # GNU-style archive command (NOT MSVC link /lib)
    ARCOM   = '"' + mingw("ar.exe") + '" $ARFLAGS $TARGET $SOURCES',
    RANLIBCOM = '"' + mingw("ranlib.exe") + '" $TARGET',
    # GNU-style link command
    LINKCOM   = '"' + mingw("g++.exe") + '" -o $TARGET $LINKFLAGS $__RPATH $SOURCES $_LIBDIRFLAGS $_LIBFLAGS',
    SHLINKCOM = '"' + mingw("g++.exe") + '" -shared -o $TARGET $LINKFLAGS $__RPATH $SOURCES $_LIBDIRFLAGS $_LIBFLAGS',
)

# ─── SDL2 and MinGW include/lib paths ────────────────────────────────────────
env.Append(
    CPPPATH = [os.path.join(MINGW_BASE, "include")],
    LIBPATH = [os.path.join(MINGW_BASE, "lib")],
    LIBS    = ["SDL2", "mingw32"],
)

# ─── Remove any MSVC-style flags that would confuse MinGW ─────────────────────
for msvc_flag in ["/W4", "/WX", "/MD", "/MDd", "/MP", "/EHsc"]:
    for var in ("CFLAGS", "CXXFLAGS", "CCFLAGS"):
        flags = env.get(var, [])
        if msvc_flag in flags:
            flags.remove(msvc_flag)

# ─── Window sub-system entry (WinMain) — needed for SDL_MAIN_HANDLED + gcc ───
env.Append(LINKFLAGS = ["-mwindows"])
