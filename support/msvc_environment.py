Import("env")
import os
import platform

# Force MSVC for Windows builds
if platform.system() == "Windows":
    msvc_path = "D:\\VisualStudio2022\\VC\\Tools\\MSVC\\14.44.35207\\bin\\Hostx64\\x64"
    cmake_path = "C:\\Program Files\\CMake\\bin"
    
    # Set compiler executable directly (without using env.Tool which is broken)
    env["CC"] = os.path.join(msvc_path, "cl.exe")
    env["CXX"] = os.path.join(msvc_path, "cl.exe")
    env["LINK"] = os.path.join(msvc_path, "link.exe")
    env["AR"] = os.path.join(msvc_path, "lib.exe")
    env["RANLIB"] = ""
    
    # Update PATH so tools can be found
    path_components = [msvc_path, cmake_path]
    if "PATH" in os.environ:
        path_components.append(os.environ["PATH"])
    env["PATH"] = os.pathsep.join(path_components)
    
    # MSVC-specific compiler flags (optimize for release build)
    env["CCFLAGS"] = ["/O2", "/W3"]
    env["CFLAGS"] = []
    env["CXXFLAGS"] = ["/EHsc"]
    
    # MSVC linker flags
    env["LINKFLAGS"] = ["/SUBSYSTEM:CONSOLE"]
    
    print("✓ MSVC Compiler Configured")
    print(f"  - CC: {env['CC']}")
    print(f"  - LINK: {env['LINK']}")
else:
    print("⚠ Not Windows - using platform default")


