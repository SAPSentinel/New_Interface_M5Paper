"""
PlatformIO Native Platform - MSVC + SDL2
"""

Import("env", "projenv")
import os
import platform
import subprocess

if platform.system() != "Windows":
    Exit(0)

# MSVC configuration
MSVC_BIN = r"D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\bin\Hostx64\x64"
cl_exe = os.path.join(MSVC_BIN, "cl.exe")
link_exe = os.path.join(MSVC_BIN, "link.exe")
lib_exe = os.path.join(MSVC_BIN, "lib.exe")

if not os.path.exists(cl_exe):
    Exit(1)

# Setup environment
os.environ["INCLUDE"] = (
    r"D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\include;" +
    r"C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\ucrt"
)
os.environ["LIB"] = (
    r"D:\VisualStudio2022\VC\Tools\MSVC\14.44.35207\lib\x64;" +
    r"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\ucrt\x64;" +
    r"C:\Program Files (x86)\Windows Kits\10\Lib\10.0.26100.0\um\x64"
)

# Clean PATH
path_list = os.environ.get("PATH", "").split(os.pathsep)
clean_path = [p for p in path_list if not any(x in p.lower() for x in ["mingw", "msys", "cygwin", "gcc", "clang"])]
os.environ["PATH"] = os.pathsep.join([MSVC_BIN] + clean_path)

# Force MSVC
env.Replace(
    CC=cl_exe,
    CXX=cl_exe,
    LD=link_exe,
    LINK=link_exe,
    AR=lib_exe,
    OBJSUFFIX=".obj",
    LIBSUFFIX=".lib",
    LIBPREFIX="",
    EXESUFFIX=".exe",
    CCFLAGS=["/O2", "/W3", "/nologo"],
    CXXFLAGS=["/EHsc"],
    LINKFLAGS=["/SUBSYSTEM:CONSOLE"],
    CCCOM='$CC /nologo /Fo$TARGET /c $SOURCES',
    CXXCOM='$CXX /nologo /Fo$TARGET /c $SOURCES',
    ARCOM='$AR /NOLOGO /OUT:$TARGET $SOURCES',
)

# Create a custom link action that uses response files to handle long command lines
def msvc_link_action(target, source, env):
    """Custom link action using response file for long command lines"""
    target_str = str(target[0])
    
    # Collect all objects and libraries
    objects = [str(s) for s in source]
    libs = env.get('LIBS', [])
    
    # Create response file with all objects
    rsp_file = target_str + ".rsp"
    with open(rsp_file, 'w') as f:
        for obj in objects:
            f.write(obj + "\n")
        for lib in libs:
            if not lib.endswith('.lib'):
                lib = lib + '.lib'
            f.write(lib + "\n")
    
    # Build link command
    link_cmd = [
        link_exe,
        '/NOLOGO',
        f'/OUT:{target_str}',
        f'@{rsp_file}'
    ]
    
    linkflags = env.get('LINKFLAGS', [])
    if isinstance(linkflags, list):
        link_cmd.extend(linkflags)
    
    # Execute link
    result = subprocess.call(link_cmd)
    if result != 0:
        raise Exception(f"Link failed: {result}")
    
    # Cleanup response file
    try:
        os.remove(rsp_file)
    except:
        pass

# Override program link if possible
if 'LINK' in env:
    env['LINKCOM'] = msvc_link_action
    env['LINKCOMSTR'] = "Linking $TARGET"

# SDL2
vcpkg_root = r"C:\vcpkg\installed\x64-windows"
env.Prepend(CPPPATH=[os.path.join(vcpkg_root, "include")])
env.Append(LIBPATH=[os.path.join(vcpkg_root, "lib")])
env.Append(LIBS=["SDL2", "SDL2main"])

print("[OK] MSVC + SDL2 configured")

# Copy SDL2.dll
def copy_dll(source, target, env):
    import shutil
    src = r"C:\vcpkg\installed\x64-windows\bin\SDL2.dll"
    if os.path.exists(src):
        dst = os.path.join(os.path.dirname(str(target[0])), "SDL2.dll")
        shutil.copy2(src, dst)

env.AddPostAction("${BUILD_DIR}/${PROGNAME}${PROGSUFFIX}", copy_dll)

