// Ensure the SDL_main symbol exists for Windows SDL2 main shim
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

extern void setup(void);
extern void loop(void);

extern "C" int SDL_main(int argc, char **argv)
{
    (void)argc; (void)argv;
    setup();
    for (;;) {
        loop();
        SDL_Delay(10);
    }
    return 0;
}
