#include "mouse.h"
#include <SDL2/SDL.h>
#include <atomic>

static std::atomic<int> g_mouse_x{0};
static std::atomic<int> g_mouse_y{0};
static std::atomic<int> g_mouse_pressed{0};

static int sdl_event_thread(void *arg)
{
    (void)arg;
    SDL_Event e;
    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                /* ignore for now */
            } else if (e.type == SDL_MOUSEMOTION) {
                g_mouse_x = e.motion.x;
                g_mouse_y = e.motion.y;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                g_mouse_pressed = 1;
                g_mouse_x = e.button.x;
                g_mouse_y = e.button.y;
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                g_mouse_pressed = 0;
                g_mouse_x = e.button.x;
                g_mouse_y = e.button.y;
            }
        }
        SDL_Delay(5);
    }
    return 0;
}

void mouse_init(void)
{
    /* Start a background thread to poll SDL events */
    SDL_CreateThread(sdl_event_thread, "sdl_event_thread", NULL);
}

bool mouse_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
    (void)drv;
    data->point.x = (lv_coord_t)g_mouse_x.load();
    data->point.y = (lv_coord_t)g_mouse_y.load();
    data->state = g_mouse_pressed.load() ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    return false; /* no more data */
}
