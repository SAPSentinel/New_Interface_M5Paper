#include <stdio.h>
#include <SDL2/SDL.h>

int main() {
    printf("Testing SDL2 linking...\n");
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    
    printf("✓ SDL2 initialized successfully!\n");
    printf("SDL Version: %d.%d.%d\n", 
           SDL_MAJOR_VERSION, 
           SDL_MINOR_VERSION, 
           SDL_PATCHLEVEL);
    
    SDL_Quit();
    return 0;
}
