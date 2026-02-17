#!/usr/bin/env python3
"""
Build wrapper for M5Paper Emulator - Initializes MSVC environment and builds
Run from VSCode tasks without needing x64 Native Tools Command Prompt
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path


def init_msvc_environment():
    """Initialize MSVC environment by parsing vcvarsall.bat output"""
    vcvars_path = r"D:\VisualStudio2022\VC\Auxiliary\Build\vcvarsall.bat"
    
    if not Path(vcvars_path).exists():
        print(f"ERROR: vcvarsall.bat not found at {vcvars_path}")
        return False
    
    try:
        # Call vcvarsall.bat and capture environment
        result = subprocess.run(
            f'cmd /c "{vcvars_path}" x64 && set',
            capture_output=True,
            text=True,
            shell=True
        )
        
        if result.returncode != 0:
            print(f"ERROR: vcvarsall.bat failed with code {result.returncode}")
            return False
        
        # Parse output and set environment variables
        for line in result.stdout.split('\n'):
            if '=' in line:
                key, value = line.split('=', 1)
                os.environ[key] = value
        
        print("✓ MSVC Environment initialized")
        return True
        
    except Exception as e:
        print(f"ERROR: Failed to initialize MSVC environment: {e}")
        return False


def copy_sdl2_dll():
    """Copy SDL2.dll to build directory"""
    src = r"C:\vcpkg\installed\x64-windows\bin\SDL2.dll"
    dst = r".pio\build\emulator_m5paper\SDL2.dll"
    
    if Path(src).exists():
        try:
            Path(dst).parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)
            print(f"✓ SDL2.dll copied")
            return True
        except Exception as e:
            print(f"⚠ Failed to copy SDL2.dll: {e}")
            return False
    return False


def run_platformio_build(clean=False):
    """Run PlatformIO build with MSVC environment"""
    print("\n" + "="*60)
    print("Building M5Paper Emulator with MSVC + SDL2")
    print("="*60 + "\n")
    
    try:
        if clean:
            print("Cleaning build directory...")
            result = subprocess.run(
                [sys.executable, "-m", "platformio", "run", "-e", "emulator_m5paper", "--target", "clean"],
                env=os.environ,
                cwd=os.getcwd()
            )
            if result.returncode != 0:
                print("ERROR: Clean failed")
                return False
        
        print("Building...")
        result = subprocess.run(
            [sys.executable, "-m", "platformio", "run", "-e", "emulator_m5paper"],
            env=os.environ,
            cwd=os.getcwd()
        )
        
        if result.returncode == 0:
            print("\n" + "="*60)
            print("✓ BUILD SUCCESSFUL")
            print("="*60)
            # Copy SDL2.dll to build directory
            copy_sdl2_dll()
            print("\nTo run the emulator:")
            print("  .pio\\build\\emulator_m5paper\\program.exe")
            return True
        else:
            print("\n" + "="*60)
            print("✗ BUILD FAILED")
            print("="*60)
            return False
            
    except Exception as e:
        print(f"ERROR: Build failed with exception: {e}")
        return False


def main():
    """Main build entry point"""
    clean_build = "--clean" in sys.argv or "clean" in sys.argv
    
    # Change to project directory (where this script is located)
    project_dir = Path(__file__).parent
    os.chdir(project_dir)
    
    # Initialize MSVC environment
    if not init_msvc_environment():
        return 1
    
    # Run build
    if not run_platformio_build(clean=clean_build):
        return 1
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
