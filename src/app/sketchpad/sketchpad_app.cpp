#include "config.h"
#include "sketchpad_app.h"
#include "sketchpad_main.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"

uint32_t sketchpad_main_tile_num;
icon_t *sketchpad_app = NULL;

LV_IMG_DECLARE(sketchpad_64px);

static void enter_sketchpad_event_cb(lv_obj_t *obj, lv_event_t event);
static int registed = app_autocall_function( &sketchpad_app_setup, 0 );           /** @brief app autocall function */

void sketchpad_app_setup(void) {
    sketchpad_main_tile_num = mainbar_add_app_tile(1, 1, "sketchpad");
    sketchpad_app = app_register("sketch\npad", &sketchpad_64px, enter_sketchpad_event_cb);
    sketchpad_main_setup(sketchpad_main_tile_num);
}

static void enter_sketchpad_event_cb(lv_obj_t *obj, lv_event_t event) {
    switch(event) {
        case LV_EVENT_CLICKED:
            statusbar_hide( true );
            mainbar_jump_to_tilenumber( sketchpad_main_tile_num, LV_ANIM_OFF, true );
            break;
    }
}
