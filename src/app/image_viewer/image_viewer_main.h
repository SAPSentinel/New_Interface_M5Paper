#ifndef _IMAGE_VIEWER_MAIN_H
#define _IMAGE_VIEWER_MAIN_H

#include "config.h"
#include <lvgl.h>
#include "gui/app.h"

LV_IMG_DECLARE(exit_32px);

extern icon_t *image_viewer_app;

void image_viewer_main_setup(uint32_t tile_num);

#endif // _IMAGE_VIEWER_MAIN_H
