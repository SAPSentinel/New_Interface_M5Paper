#ifndef _SKETCHPAD_MAIN_H
#define _SKETCHPAD_MAIN_H

#include "config.h"
#include <lvgl.h>
#include "gui/app.h"

LV_IMG_DECLARE(exit_32px);

extern icon_t *sketchpad_app;

void sketchpad_main_setup(uint32_t tile_num);

#endif // _SKETCHPAD_MAIN_H
