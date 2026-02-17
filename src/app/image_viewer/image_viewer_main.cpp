#include "config.h"
#include "image_viewer_main.h"
#include "image_viewer_app.h"

#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/widget_factory.h"
#include "gui/widget_styles.h"

#include <M5EPD.h>

static lv_obj_t *image_viewer_main_tile = NULL;
static lv_obj_t *image_list = NULL;
static lv_obj_t *status_label = NULL;

static String current_file = "";
static int img_index = 0;
static int total_images = 0;

static void load_image(const char* filename) {
    String fullpath = String("/") + filename;
    current_file = filename;
    lv_label_set_text(status_label, filename);
    
    // TODO: Implement image loading with JPEGDEC
    M5.EPD.Clear(true);
    M5.EPD.UpdateFull(UPDATE_MODE_GC16);
}

static void file_select_cb(lv_obj_t *btn, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        const char* filename = lv_list_get_btn_text(btn);
        load_image(filename);
        lv_obj_set_hidden(image_list, true);
    }
}

static void scan_sd_card() {
    File root = SD.open("/");
    if (!root) {
        lv_list_add_btn(image_list, NULL, "SD Card Error");
        return;
    }
    
    total_images = 0;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String filename = file.name();
            filename.toLowerCase();
            if (filename.endsWith(".jpg") || filename.endsWith(".jpeg") || filename.endsWith(".png")) {
                lv_obj_t *btn = lv_list_add_btn(image_list, NULL, file.name());
                lv_obj_set_event_cb(btn, file_select_cb);
                total_images++;
            }
        }
        file = root.openNextFile();
    }
    root.close();
    
    if (total_images == 0) {
        lv_list_add_btn(image_list, NULL, "No image files found");
    }
}

static void exit_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        mainbar_jump_to_maintile(LV_ANIM_OFF);
        statusbar_hide(false);
        lv_obj_set_hidden(image_list, false);
    }
}

static void list_btn_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        lv_obj_set_hidden(image_list, false);
        M5.EPD.UpdateFull(UPDATE_MODE_DU);
    }
}

void image_viewer_main_setup(uint32_t tile_num) {
    image_viewer_main_tile = mainbar_get_tile_obj(tile_num);
    
    // Create file list
    image_list = lv_list_create(image_viewer_main_tile, NULL);
    lv_obj_set_size(image_list, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL) - 80);
    lv_obj_align(image_list, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);
    
    // Title label
    lv_obj_t *title = lv_label_create(image_viewer_main_tile, NULL);
    lv_label_set_text(title, "Image Viewer - Select Image");
    lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    
    // Status label
    status_label = lv_label_create(image_viewer_main_tile, NULL);
    lv_label_set_text(status_label, "");
    lv_obj_align(status_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -40);
    
    // Exit button
    lv_obj_t *exit_btn = lv_imgbtn_create(image_viewer_main_tile, NULL);
    lv_imgbtn_set_src(exit_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_obj_align(exit_btn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10);
    lv_obj_set_event_cb(exit_btn, exit_cb);
    
    // List button
    lv_obj_t *list_btn = lv_btn_create(image_viewer_main_tile, NULL);
    lv_obj_set_size(list_btn, 80, 40);
    lv_obj_align(list_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10);
    lv_obj_t *list_label = lv_label_create(list_btn, NULL);
    lv_label_set_text(list_label, "LIST");
    lv_obj_set_event_cb(list_btn, list_btn_cb);
    
    // Scan SD card
    scan_sd_card();
}
