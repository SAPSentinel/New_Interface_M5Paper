#ifndef _EPUB_READER_MAIN_H
#define _EPUB_READER_MAIN_H

#include "config.h"
#include <lvgl.h>
#include "gui/app.h"

LV_IMG_DECLARE(exit_32px);

extern icon_t *epub_reader_app;

void epub_reader_main_setup(uint32_t tile_num);

#endif // _EPUB_READER_MAIN_H
