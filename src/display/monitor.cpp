#include "monitor.h"
#include <SDL2/SDL.h>
#include <stdio.h>

static SDL_Window *g_window = NULL;
static SDL_Renderer *g_renderer = NULL;
static SDL_Texture *g_texture = NULL;
static int g_width = 540;
static int g_height = 960;

void monitor_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return;
    }

    g_window = SDL_CreateWindow("M5Paper Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                g_width, g_height, SDL_WINDOW_SHOWN);
    if (!g_window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return;
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!g_renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return;
    }

    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, g_width, g_height);
    if (!g_texture) {
        printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
        return;
    }
}

void monitor_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    (void)disp_drv;
    if (!g_texture || !g_renderer)
        return;

    int x = area->x1;
    int y = area->y1;
    int w = area->x2 - area->x1 + 1;
    int h = area->y2 - area->y1 + 1;

    /* Update texture region using RGB565 raw buffer (lv_color_t expected 16-bit) */
    void *pixels = NULL;
    int pitch = 0;
    if (SDL_LockTexture(g_texture, NULL, &pixels, &pitch) == 0) {
        /* pitch is bytes per row */
        uint8_t *dst = (uint8_t *)pixels + y * pitch + x * 2; /* 2 bytes per pixel RGB565 */
        uint8_t *src = (uint8_t *)color_p;
        for (int row = 0; row < h; ++row) {
            memcpy(dst + row * pitch, src + row * w * 2, w * 2);
        }
        SDL_UnlockTexture(g_texture);

        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    }

    lv_disp_flush_ready(disp_drv);
}

void monitor_write_area(int x, int y, int w, int h, const void *pixels_src)
{
    if (!g_texture || !g_renderer)
        return;

    void *pixels = NULL;
    int pitch = 0;
    if (SDL_LockTexture(g_texture, NULL, &pixels, &pitch) == 0) {
        uint8_t *dst = (uint8_t *)pixels + y * pitch + x * 2;
        const uint8_t *src = (const uint8_t *)pixels_src;
        for (int row = 0; row < h; ++row) {
            memcpy(dst + row * pitch, src + row * w * 2, w * 2);
        }
        SDL_UnlockTexture(g_texture);

        SDL_RenderClear(g_renderer);
        SDL_RenderCopy(g_renderer, g_texture, NULL, NULL);
        SDL_RenderPresent(g_renderer);
    }
}
