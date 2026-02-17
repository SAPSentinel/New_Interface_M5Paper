Import("env", "projenv")
import os
import shutil

for e in [ env, projenv ]:
    # Ensure MSVC linker mode
    e['IMPLIBPREFIX'] = ''
    e['IMPLIBSUFFIX'] = '.lib'
    
    # Add explicit vcpkg SDL2 library paths - MSVC style
    sdl2_lib = "C:/vcpkg/installed/x64-windows/lib/SDL2.lib"
    sdl2main_lib = "C:/vcpkg/installed/x64-windows/lib/manual-link/SDL2main.lib"
    
    # Append as direct library objects for MSVC linker
    e.Append(LIBS = [sdl2_lib, sdl2main_lib])
    
    print(f"✓ SDL2 (MSVC): Linking libraries directly")
    print(f"  - SDL2.lib: {sdl2_lib}")
    print(f"  - SDL2main.lib: {sdl2main_lib}")

exec_name = "${BUILD_DIR}/${PROGNAME}${PROGSUFFIX}"

# Override unused "upload" to execute compiled binary
from SCons.Script import AlwaysBuild
AlwaysBuild(env.Alias("upload", exec_name, exec_name))

# Post-build action: copy SDL2.dll to build directory
def copy_sdl2_dll(source, target, env):
    """Copy SDL2.dll from vcpkg to build directory after build succeeds"""
    src_dll = "C:/vcpkg/installed/x64-windows/bin/SDL2.dll"
    build_dir = env.Dir("${BUILD_DIR}").path.replace("\\", "/")
    dst_dll = f"{build_dir}/SDL2.dll"
    
    if os.path.exists(src_dll):
        try:
            shutil.copy2(src_dll, dst_dll.replace("/", "\\"))
            print(f"✓ SDL2.dll copied to {dst_dll}")
        except Exception as e:
            print(f"✗ Failed to copy SDL2.dll: {e}")
    else:
        print(f"✗ SDL2.dll not found at {src_dll}")

# Add the post-build action
if "clean" not in COMMAND_LINE_TARGETS:
    env.AddPostAction(exec_name, copy_sdl2_dll)

# Add custom target to explorer
env.AddTarget(
    name = "execute",
    dependencies = exec_name,
    actions = exec_name,
    title = "Execute",
    description = "Build and execute",
    group="General"
)




#print('=====================================')
#print(env.Dump())
