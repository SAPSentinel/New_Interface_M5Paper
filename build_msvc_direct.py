#!/usr/bin/env python3
"""
Direct MSVC Compiler for M5Paper Emulator
Bypasses PlatformIO entirely to avoid SCons toolchain issues
"""

import subprocess
import sys
import os
from pathlib import Path

MSVC_CL = r"D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe"
MSVC_LINK = r"D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\link.exe"
VCVARS = r"D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat"
SDL2_LIB = r"C:\vcpkg\installed\x64-windows\lib"
SDL2_INCLUDE = r"C:\vcpkg\installed\x64-windows\include"
SDL2_DLL = r"C:\vcpkg\installed\x64-windows\bin\SDL2.dll"
BUILD_DIR = Path("build_msvc")

# Get absolute paths for all important directories
PROJECT_ROOT = Path.cwd()
LIBDEPS = PROJECT_ROOT / ".pio/libdeps/emulator_m5paper"

print("Initializing MSVC environment...")
print("Building M5Paper Emulator with MSVC + SDL2\n")

# Initialize MSVC environment via vcvarsall first
init_env = os.environ.copy()
try:
    vcvars_output = subprocess.run(
        f'cmd /c "{VCVARS}" x64 && set',
        capture_output=True,
        text=True,
        shell=True
    )
    
    # Parse environment variables from vcvarsall output
    for line in vcvars_output.stdout.split('\n'):
        if '=' in line:
            key, value = line.split('=', 1)
            init_env[key] = value
            
except Exception as e:
    print(f"ERROR: Failed to initialize MSVC environment: {e}")
    sys.exit(1)

# Key source files
SOURCES = [
    "src/main.cpp",
    # LVGL core
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_core/lv_disp.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_core/lv_group.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_core/lv_indev.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_core/lv_obj.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_core/lv_refr.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_core/lv_style.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_draw/lv_draw.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_draw/lv_draw_line.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_draw/lv_draw_polygon.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_draw/lv_draw_rect.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_draw/lv_draw_triangle.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_font/lv_font.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_font/lv_font_builtin.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_anim.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_area.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_async.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_color.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_fs.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_gc.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_ll.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_log.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_math.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_mem.c",
    ".pio/libdeps/emulator_m5paper/lvgl/src/lv_misc/lv_task.c",
    # lv_drivers
    ".pio/libdeps/emulator_m5paper/lv_drivers/lv_drivers.c",
    ".pio/libdeps/emulator_m5paper/lv_drivers/display/monitor.c",
    ".pio/libdeps/emulator_m5paper/lv_drivers/indev/mouse.c",
]

print(f"Compiling {len(SOURCES)} source files...\n")

# Create build directory
BUILD_DIR.mkdir(exist_ok=True)

# Compile each source file individually
compile_failed = False
for i, src in enumerate(SOURCES, 1):
    obj_file = BUILD_DIR / (Path(src).stem + ".obj")
    
    compile_args = [
        MSVC_CL,
        "/nologo",
        "/O2",
        "/W3",
        "/WX-",
        f'/I"{LIBDEPS / "lvgl/src"}"',
        f'/I"{LIBDEPS / "lv_drivers"}"',
        f'/I"{PROJECT_ROOT / "lib"}"',
        f'/I"{SDL2_INCLUDE}"',
        f'/I"{PROJECT_ROOT / "src"}"',
        "/DUSE_MONITOR=1",
        "/DUSE_MOUSE=1",
        "/DSDL_MAIN_HANDLED",
        "/c",  # Compile only, don't link
        f'/Fo"{obj_file}"',
        src,
    ]
    
    print(f"[{i}/{len(SOURCES)}] Compiling {Path(src).name}...", end=" ")
    # DEBUG: Print the command being run for first file
    if i == 1:
        print(f"\n  DEBUG: {' '.join(compile_args[:15])}")
    result = subprocess.run(compile_args, env=init_env, capture_output=True, text=True)
    
    if result.returncode == 0:
        print("[OK]")
    else:
        print("[FAIL]")
        print(result.stdout)
        print(result.stderr)
        compile_failed = True
        break

if compile_failed:
    print("\n" + "="*60)
    print("BUILD FAILED - Compilation error")
    print("="*60 + "\n")
    sys.exit(1)

print("\n[OK] Compilation complete, linking...")

# Get list of obj files that were created
obj_files = [str(BUILD_DIR / (Path(s).stem + ".obj")) for s in SOURCES]

# Link via cmd to maximize compatibility
link_cmd_parts = [
    f'"{MSVC_LINK}"',
    "/nologo",
    f'/OUT:"{BUILD_DIR / "program.exe"}"',
    f'/LIBPATH:"{SDL2_LIB}"',
    "SDL2.lib",
    "SDL2main.lib",
    "/SUBSYSTEM:CONSOLE",
] + [f'"{obj}"' for obj in obj_files]

link_cmd = " ".join(link_cmd_parts)
full_link_cmd = f'"{VCVARS}" x64 && {link_cmd}'

print("Linking...")
result = subprocess.run(full_link_cmd, shell=True, cwd=str(PROJECT_ROOT))

if result.returncode == 0:
    # Copy SDL2.dll
    if Path(SDL2_DLL).exists():
        import shutil
        try:
            shutil.copy2(SDL2_DLL, BUILD_DIR / "SDL2.dll")
            print("[OK] SDL2.dll copied")
        except:
            pass
    
    print("\n" + "="*60)
    print("[SUCCESS] BUILD SUCCESSFUL")
    print("="*60)
    print(f"\nOutput: {(BUILD_DIR / 'program.exe').absolute()}")
    print(f"To run: {BUILD_DIR}\\program.exe\n")
    sys.exit(0)
else:
    print("\n" + "="*60)
    print("BUILD FAILED - Linking error")
    print("="*60 + "\n")
    sys.exit(1)

