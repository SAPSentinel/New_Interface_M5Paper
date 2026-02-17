#include "config.h"
#include "image_viewer_app.h"
#include "image_viewer_main.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/app.h"

uint32_t image_viewer_main_tile_num;
icon_t *image_viewer_app = NULL;

LV_IMG_DECLARE(image_viewer_64px);

static void enter_image_viewer_event_cb(lv_obj_t *obj, lv_event_t event);
static int registed = app_autocall_function( &image_viewer_app_setup, 0 );           /** @brief app autocall function */

void image_viewer_app_setup(void) {
    image_viewer_main_tile_num = mainbar_add_app_tile(1, 1, "image viewer");
    image_viewer_app = app_register("image\nviewer", &image_viewer_64px, enter_image_viewer_event_cb);
    image_viewer_main_setup(image_viewer_main_tile_num);
}

static void enter_image_viewer_event_cb(lv_obj_t *obj, lv_event_t event) {
    switch(event) {
        case LV_EVENT_CLICKED:
            statusbar_hide( true );
            mainbar_jump_to_tilenumber( image_viewer_main_tile_num, LV_ANIM_OFF, true );
            break;
    }
}
