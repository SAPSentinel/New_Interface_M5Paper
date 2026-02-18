"""
Pre-build extra_script: runs BEFORE PlatformIO native platform main.py.

Puts MinGW64 bin dir first in PATH so that env.Tool("gcc") / env.Tool("g++")
in the native platform builder detect MinGW gcc rather than MSVC/cl.exe.
All toolchain overrides are done in the companion post: script (sdl2_mingw_post.py)
which runs after the platform builder has finished its setup.
"""

Import("env")
import os

MINGW_BIN = "C:/msys64/mingw64/bin"

# Put MinGW FIRST in PATH — platform main.py calls env.Tool("gcc") which
# does env.WhereIs("gcc"); it must find MinGW gcc, not anything else.
os.environ["PATH"] = MINGW_BIN.replace("/", "\\") + os.pathsep + os.environ.get("PATH", "")

