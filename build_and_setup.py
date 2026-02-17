#!/usr/bin/env python3
"""
M5Paper Emulator - Setup and Run with SDL2
Uses existing compiled emulator and ensures SDL2 graphics support
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

SDL2_DLL_SRC = r"C:\vcpkg\installed\x64-windows\bin\SDL2.dll"
EMULATOR_BUILD_DIR = ".pio/build/emulator_m5paper"
OUTPUT_EXE = f"{EMULATOR_BUILD_DIR}/program.exe"

def main():
    os.chdir(os.path.dirname(os.path.abspath(__file__)) or ".")
    
    print("\n" + "="*60)
    print("M5Paper Emulator - SDL2 Setup")  
    print("="*60)
    
    # Check if emulator exists
    if not os.path.exists(OUTPUT_EXE):
        print(f"\n✗ Emulator not found at {OUTPUT_EXE}")
        print("  Building with PlatformIO...")
        result = subprocess.run("python -m platformio run -e emulator_m5paper", shell=True)
        if result.returncode != 0:
            print("\n✗ PlatformIO build failed (GCC/toolchain issue)")
            print("  Using pre-built emulator instead...")
            if not os.path.exists(OUTPUT_EXE):
                print("\n✗ ERROR: Cannot find emulator executable")
                return 1
    
    print(f"✓ Emulator found: {OUTPUT_EXE}")
    
    # Copy SDL2.dll for graphics support
    print("\nSetting up SDL2 graphics support...")
    if os.path.exists(SDL2_DLL_SRC):
        dll_dst = os.path.join(EMULATOR_BUILD_DIR, "SDL2.dll")
        try:
            shutil.copy2(SDL2_DLL_SRC, dll_dst)
            print(f"✓ Copied SDL2.dll to {EMULATOR_BUILD_DIR}")
        except Exception as e:
            print(f"⚠ Could not copy SDL2.dll: {e}")
            print("  (Emulator may run without graphics)")
    else:
        print(f"⚠ SDL2.dll not found at {SDL2_DLL_SRC}")
        print("  (Emulator may run without graphics)")
    
    # Final status
    print("\n" + "="*60)
    print("✓ SETUP COMPLETE!")
    print("="*60)
    print(f"\nExecutable ready: {OUTPUT_EXE}")
    print("Run with: .pio/build/emulator_m5paper/program.exe\n")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
