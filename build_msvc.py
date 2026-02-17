#!/usr/bin/env python3
"""
M5Paper Emulator - Comprehensive Python Build Script
Compiles all LVGL + lv_drivers + project code with MSVC and links with SDL2
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

# Configuration
MSVC_CL = r"D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\cl.exe"
MSVC_LINK = r"D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64\link.exe"
VCVARSALL = r"D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat"
SDL2_LIB = r"C:\vcpkg\installed\x64-windows\lib"
SDL2_BIN = r"C:\vcpkg\installed\x64-windows\bin"
SDL2_INCLUDE = r"C:\vcpkg\installed\x64-windows\include"
BUILD_DIR = "build_py"

def run_command(cmd, description, fail_msg):
    """Run a command and report status."""
    print(f"{description}...", end=" ", flush=True)
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd=os.getcwd())
        if result.returncode == 0:
            print("✓")
            return True
        else:
            print(f"✗\n{fail_msg}\n{result.stderr}")
            return False
    except Exception as e:
        print(f"✗\nException: {e}")
        return False

def find_sources(directory, extensions):
    """Recursively find all source files with given extensions."""
    sources = []
    for ext in extensions:
        for file in Path(directory).rglob(f"*{ext}"):
            sources.append(str(file))
    return sorted(sources)

def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)) or ".")
    
    print("\n" + "="*60)
    print("M5Paper Emulator - Comprehensive Python MSVC Build")
    print("="*60 + "\n")
    
    # Clean build directory
    if os.path.exists(BUILD_DIR):
        print(f"Cleaning {BUILD_DIR}...")
        shutil.rmtree(BUILD_DIR)
    os.makedirs(BUILD_DIR, exist_ok=True)
    
    # Copy lv_drv_conf.h
    src_conf = "lib/lv_drv_conf.h"
    dst_conf = ".pio/libdeps/emulator_m5paper/lv_drv_conf.h"
    if os.path.exists(src_conf):
        print(f"Copying lv_drv_conf.h to libdeps...")
        shutil.copy2(src_conf, dst_conf)
    
    # Initialize MSVC environment
    init_env = f'call "{VCVARSALL}" x64 >nul && set'
    result = subprocess.run(init_env, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
        print("ERROR: Failed to initialize MSVC environment")
        return 1
    
    # Build compiler command
    includes = " ".join([
        f'/I"."',
        f'/I"lib"',
        f'/I".pio\\libdeps\\emulator_m5paper\\lvgl"',
        f'/I".pio\\libdeps\\emulator_m5paper\\lvgl\\src"',
        f'/I".pio\\libdeps\\emulator_m5paper\\lv_drivers"',
        f'/I"{SDL2_INCLUDE}"',
        f'/I"src"',
    ])
    defines = "/DUSE_MONITOR=1 /DUSE_MOUSE=1 /DSDL_MAIN_HANDLED /DNATIVE_64BIT /DLV_LVGL_H_INCLUDE_SIMPLE"
    base_cmd = f'cd /d "{BUILD_DIR}" && "{MSVC_CL}" /nologo /O2 /W3 /WX- {includes} {defines} /c'
    
    # Collect all sources
    print("Collecting source files...")
    sources = []
    
    # Project sources
    sources.extend(find_sources(".\\src", [".cpp", ".c"]))
    
    # LVGL modules
    lvgl_dirs = [
        ".pio\\libdeps\\emulator_m5paper\\lvgl\\src\\lv_core",
        ".pio\\libdeps\\emulator_m5paper\\lvgl\\src\\lv_draw",
        ".pio\\libdeps\\emulator_m5paper\\lvgl\\src\\lv_font",
        ".pio\\libdeps\\emulator_m5paper\\lvgl\\src\\lv_misc",
        ".pio\\libdeps\\emulator_m5paper\\lvgl\\src\\lv_themes",
        ".pio\\libdeps\\emulator_m5paper\\lvgl\\src\\lv_widgets",
        ".pio\\libdeps\\emulator_m5paper\\lvgl\\src\\lv_hal",
    ]
    for d in lvgl_dirs:
        if os.path.exists(d):
            sources.extend(find_sources(d, [".c"]))
    
    # Essential lv_drivers only
    sources.extend([
        ".pio\\libdeps\\emulator_m5paper\\lv_drivers\\display\\monitor.c",
        ".pio\\libdeps\\emulator_m5paper\\lv_drivers\\indev\\mouse.c",
    ])
    
    sources = [s for s in sources if os.path.exists(s)]  # Filter out non-existent files
    print(f"Found {len(sources)} source files\n")
    
    # Compile in batches (to avoid command line length limits)
    print("Compiling sources...")
    batch_size = 20
    obj_files = []
    
    for i in range(0, len(sources), batch_size):
        batch = sources[i:i+batch_size]
        source_str = " ".join(f'"{s}"' for s in batch)
        cmd = f'{base_cmd} {source_str}'
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, cwd=".")
        if result.returncode != 0:
            print(f"\n✗ Compilation failed:\n{result.stderr}")
            return 1
        print(f"  Batch {i//batch_size + 1}/{(len(sources)-1)//batch_size + 1}: ✓")
    
    # Link
    print("\nLinking executable...")
    obj_pattern = os.path.join(BUILD_DIR, "*.obj")
    obj_files_str = " ".join(f'"{f}"' for f in Path(BUILD_DIR).glob("*.obj"))
    if not obj_files_str:
        print("✗ No object files found!")
        return 1
    
    link_cmd = f'"{MSVC_LINK}" /nologo /OUT:"{BUILD_DIR}\\program.exe" /LIBPATH:"{SDL2_LIB}" SDL2.lib /SUBSYSTEM:CONSOLE {obj_files_str}'
    result = subprocess.run(link_cmd, shell=True, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"✗ Linking failed:\n{result.stderr}")
        return 1
    print("✓ Linking complete")
    
    # Copy SDL2.dll
    dll_src = os.path.join(SDL2_BIN, "SDL2.dll")
    dll_dst = os.path.join(BUILD_DIR, "SDL2.dll")
    if os.path.exists(dll_src):
        shutil.copy2(dll_src, dll_dst)
        print("✓ Copied SDL2.dll")
    
    print("\n" + "="*60)
    print("✓ BUILD SUCCESSFUL!")
    print("="*60)
    print(f"\nExecutable: {BUILD_DIR}\\program.exe")
    print(f"To run: {BUILD_DIR}\\program.exe\n")
    return 0

if __name__ == "__main__":
    sys.exit(main())
