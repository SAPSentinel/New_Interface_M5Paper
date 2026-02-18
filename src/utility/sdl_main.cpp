#if defined(NATIVE_64BIT)
// Minimal main for native emulator
#include "../utility/M5GFX_stub.hpp"
#include <SDL2/SDL.h>
#include <windows.h>

void setup(void);
void loop(void);

// Internal main function
int main_impl(void)
{
    setup();
    while (true) {
        loop();
        SDL_Delay(10);
    }
    return 0;
}

// Windows entry point for MinGW
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    return main_impl();
}

#endif /* NATIVE_64BIT */
