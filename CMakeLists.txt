cmake_minimum_required(VERSION 3.15)
project(m5paper_emulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force MSVC
if(NOT MSVC)
    message(FATAL_ERROR "This project requires MSVC compiler")
endif()

# Add LVGL library
file(GLOB LVGL_SOURCES 
    "lib/LVGL/lvgl/lv_*.c"
    "lib/LVGL/lvgl/lv_core/*.c"
    "lib/LVGL/lvgl/lv_draw/*.c"
    "lib/LVGL/lvgl/lv_font/*.c"
    "lib/LVGL/lvgl/lv_gpu/*.c"
    "lib/LVGL/lvgl/lv_halext/*.c"
    "lib/LVGL/lvgl/lv_misc/*.c"
    "lib/LVGL/lvgl/lv_objx/*.c"
    "lib/LVGL/lvgl/lv_themes/*.c"
)

file(GLOB LV_DRIVERS_SOURCES
    "lib/lv_drivers/lv_drivers.c"
    "lib/lv_drivers/display/monitor.c"
    "lib/lv_drivers/indev/mouse.c"
)

file(GLOB APP_SOURCES
    "src/*.cpp"
    "src/app/*.cpp"
    "src/app/*/main.cpp"
)

# Create executable
add_executable(program
    ${LVGL_SOURCES}
    ${LV_DRIVERS_SOURCES}
    ${APP_SOURCES}
)

# SDL2 from vcpkg
find_package(SDL2 REQUIRED CONFIG)

# Include directories
target_include_directories(program PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/lv_conf.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "C:/vcpkg/installed/x64-windows/include"
)

# Link SDL2
target_link_libraries(program PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
)

# MSVC-specific settings
if(MSVC)
    target_compile_options(program PRIVATE /O2 /W3 /WX-)
    set_target_properties(program PROPERTIES
        WIN32_EXECUTABLE ON
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
endif()

# Copy SDL2.dll to output directory
add_custom_command(TARGET program POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "C:\\vcpkg\\installed\\x64-windows\\bin\\SDL2.dll"
        "$<TARGET_FILE_DIR:program>/SDL2.dll"
    COMMENT "Copying SDL2.dll"
)

message(STATUS "✓ M5Paper Emulator CMake configured")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  SDL2 found")
