#if defined(NATIVE_64BIT)
/**
 * @file sdl_stubs.c
 * Minimal SDL2 wrapper stubs to allow emulator to compile without SDL linking issues
 * These are weak symbols that will be overridden if proper SDL2 is linked
 */

#include <SDL2/SDL.h>

/* Weak stub implementations - will be overridden by real SDL if linked */

__attribute__((weak)) int SDL_Init(Uint32 flags) { return -1; }
__attribute__((weak)) void SDL_Quit(void) { }

__attribute__((weak)) SDL_Window* SDL_CreateWindow(
    const char *title, int x, int y, int w, int h, Uint32 flags) {
    return NULL;
}

__attribute__((weak)) void SDL_DestroyWindow(SDL_Window *window) { }

__attribute__((weak)) SDL_Renderer* SDL_CreateRenderer(
    SDL_Window *window, int index, Uint32 flags) {
    return NULL;
}

__attribute__((weak)) void SDL_DestroyRenderer(SDL_Renderer *renderer) { }

__attribute__((weak)) SDL_Texture* SDL_CreateTexture(
    SDL_Renderer *renderer, Uint32 format, int access, int w, int h) {
    return NULL;
}

__attribute__((weak)) void SDL_DestroyTexture(SDL_Texture *texture) { }

__attribute__((weak)) int SDL_SetTextureBlendMode(SDL_Texture *texture, SDL_BlendMode blendMode) {
    return -1;
}

__attribute__((weak)) int SDL_UpdateTexture(SDL_Texture *texture, const SDL_Rect* rect,
                                          const void *pixels, int pitch) {
    return -1;
}

__attribute__((weak)) int SDL_RenderClear(SDL_Renderer *renderer) { return -1; }
__attribute__((weak)) int SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture,
                                        const SDL_Rect *srcrect, const SDL_Rect *dstrect) {
    return -1;
}

__attribute__((weak)) void SDL_RenderPresent(SDL_Renderer *renderer) { }

__attribute__((weak)) int SDL_PollEvent(SDL_Event *event) { return 0; }

__attribute__((weak)) int SDL_SetEventFilter(SDL_EventFilter filter, void *userdata) {
    return 0;
}

__attribute__((weak)) const char* SDL_GetError(void) {
    return "SDL not initialized";
}

/* Version info */
__attribute__((weak)) const SDL_version* SDL_GetVersion(SDL_version *ver) {
    if (ver) {
        ver->major = 2;
        ver->minor = 32;
        ver->patch = 10;
    }
    return ver;
}

#endif /* NATIVE_64BIT */
