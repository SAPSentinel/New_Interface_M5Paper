#include "config.h"
#include "sketchpad_main.h"
#include "sketchpad_app.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"

#include <M5EPD.h>

static lv_obj_t *sketchpad_main_tile = NULL;
static lv_obj_t *canvas_obj = NULL;
static lv_obj_t *status_label = NULL;

static int brush_size = 3;
static bool drawing = false;

static void clear_canvas() {
    M5.EPD.Clear(true);
    M5.EPD.UpdateFull(UPDATE_MODE_DU);
    lv_label_set_text(status_label, "Canvas cleared");
}

static void save_sketch() {
    // Create sketches directory if needed
    if (!SD.exists("/sketches")) {
        SD.mkdir("/sketches");
    }
    
    int sketch_num = 0;
    String filename;
    do {
        filename = "/sketches/sketch_" + String(sketch_num++) + ".bmp";
    } while (SD.exists(filename));
    
    lv_label_set_text_fmt(status_label, "Saved: %s", filename.c_str());
}

static void clear_btn_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        clear_canvas();
    }
}

static void save_btn_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        save_sketch();
    }
}

static void exit_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        mainbar_jump_to_maintile(LV_ANIM_OFF);
        statusbar_hide(false);
    }
}

static void brush_size_cb(lv_obj_t *slider, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
        brush_size = lv_slider_get_value(slider);
        lv_label_set_text_fmt(status_label, "Brush: %d px", brush_size);
    }
}

void sketchpad_main_setup(uint32_t tile_num) {
    sketchpad_main_tile = mainbar_get_tile_obj(tile_num);
    
    // Title
    lv_obj_t *title = lv_label_create(sketchpad_main_tile, NULL);
    lv_label_set_text(title, "Sketchpad");
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    
    // Canvas placeholder
    canvas_obj = lv_canvas_create(sketchpad_main_tile, NULL);
    lv_obj_set_size(canvas_obj, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL) - 130);
    lv_obj_align(canvas_obj, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);
    lv_obj_set_style_local_bg_color(canvas_obj, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    
    // Brush size label
    lv_obj_t *size_label = lv_label_create(sketchpad_main_tile, NULL);
    lv_label_set_text(size_label, "Brush:");
    lv_obj_align(size_label, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -40);
    
    // Brush size slider
    lv_obj_t *slider = lv_slider_create(sketchpad_main_tile, NULL);
    lv_slider_set_range(slider, 1, 10);
    lv_slider_set_value(slider, 3, LV_ANIM_ON);
    lv_obj_set_size(slider, 100, 20);
    lv_obj_align(slider, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 70, -40);
    lv_obj_set_event_cb(slider, brush_size_cb);
    
    // Status label
    status_label = lv_label_create(sketchpad_main_tile, NULL);
    lv_label_set_text(status_label, "Ready to draw");
    lv_obj_align(status_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -50);
    
    // Clear button
    lv_obj_t *clear_btn = lv_btn_create(sketchpad_main_tile, NULL);
    lv_obj_set_size(clear_btn, 60, 40);
    lv_obj_align(clear_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, -40, -10);
    lv_obj_t *clear_label = lv_label_create(clear_btn, NULL);
    lv_label_set_text(clear_label, "CLR");
    lv_obj_set_event_cb(clear_btn, clear_btn_cb);
    
    // Save button
    lv_obj_t *save_btn = lv_btn_create(sketchpad_main_tile, NULL);
    lv_obj_set_size(save_btn, 60, 40);
    lv_obj_align(save_btn, NULL, LV_ALIGN_IN_BOTTOM_MID, 40, -10);
    lv_obj_t *save_label = lv_label_create(save_btn, NULL);
    lv_label_set_text(save_label, "SAVE");
    lv_obj_set_event_cb(save_btn, save_btn_cb);
    
    // Exit button
    lv_obj_t *exit_btn = lv_imgbtn_create(sketchpad_main_tile, NULL);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_obj_align(exit_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10);
    lv_obj_set_event_cb(exit_btn, exit_cb);
    
}
