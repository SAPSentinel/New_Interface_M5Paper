#include "config.h"
#include "epub_reader_app.h"
#include "epub_reader_main.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"

uint32_t epub_reader_main_tile_num;
icon_t *epub_reader_app = NULL;

LV_IMG_DECLARE(epub_reader_64px);

static void enter_epub_reader_event_cb(lv_obj_t *obj, lv_event_t event);
static int registed = app_autocall_function( &epub_reader_app_setup, 0 );           /** @brief app autocall function */

void epub_reader_app_setup(void) {
    epub_reader_main_tile_num = mainbar_add_app_tile(1, 1, "epub reader");
    epub_reader_app = app_register("epub\nreader", &epub_reader_64px, enter_epub_reader_event_cb);
    epub_reader_main_setup(epub_reader_main_tile_num);
}

static void enter_epub_reader_event_cb(lv_obj_t *obj, lv_event_t event) {
    switch(event) {
        case LV_EVENT_CLICKED:
            statusbar_hide( true );
            mainbar_jump_to_tilenumber( epub_reader_main_tile_num, LV_ANIM_OFF, true );
            break;
    }
}
