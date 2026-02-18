#if defined(NATIVE_64BIT)
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

extern void setup(void);
extern void loop(void);

int SDL_main(int argc, char **argv)
{
    (void)argc; (void)argv;
    setup();
    for (;;) {
        loop();
        SDL_Delay(10);
    }
    return 0;
}

#ifdef _WIN32
#include <windows.h>

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    (void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nShowCmd;
#if defined(__argc) && defined(__argv)
    return SDL_main(__argc, __argv);
#else
    return SDL_main(0, NULL);
#endif
}
#endif

#endif /* NATIVE_64BIT */
