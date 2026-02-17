#include "config.h"
#include "epub_reader_main.h"
#include "epub_reader_app.h"
#include "epub_reader_core.h"
#include <SD.h>
#include <vector>
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"

static lv_obj_t *epub_reader_main_tile = NULL;
static lv_obj_t *book_list = NULL;
static lv_obj_t *text_area = NULL;
static lv_obj_t *page_label = NULL;
static lv_obj_t *status_label = NULL;
static lv_obj_t *reader_panel = NULL;

static bool book_open = false;
static int display_mode = 0;  // 0=file list, 1=reading

// Forward declarations
static void book_select_cb(lv_obj_t *btn, lv_event_t event);

static void scan_sd_for_books(void) {
    lv_list_clean(book_list);
    
    File root = SD.open("/books");
    if (!root) {
        SD.mkdir("/books");
        lv_list_add_btn(book_list, NULL, "Create /books folder");
        return;
    }
    
    int book_count = 0;
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            String filename = file.name();
            filename.toLowerCase();
            
            if (filename.endsWith(".epub") || filename.endsWith(".txt")) {
                lv_obj_t *btn = lv_list_add_btn(book_list, NULL, file.name());
                lv_obj_set_event_cb(btn, book_select_cb);
                book_count++;
            }
        }
        file = root.openNextFile();
    }
    root.close();
    
    lv_label_set_text_fmt(status_label, "Found %d books", book_count);
}

static void update_display_page(void) {
    if (!book_open) return;
    
    epub_get_page_content(current_page, current_book_lines);
    
    // Clear and update text area
    lv_textarea_set_text(text_area, "");
    
    for (size_t i = 0; i < current_book_lines.size(); i++) {
        lv_textarea_add_char(text_area, ' ');
        lv_textarea_add_text(text_area, current_book_lines[i].c_str());
        lv_textarea_add_text(text_area, "\n");
    }
    
    // Update page counter
    lv_label_set_text_fmt(page_label, "Page %d/%d", current_page + 1, total_pages);
}

static void show_reader_view(void) {
    display_mode = 1;
    lv_obj_set_hidden(book_list, true);
    lv_obj_set_hidden(reader_panel, false);
    statusbar_hide(true);
    
    update_display_page();
}

static void show_book_list(void) {
    display_mode = 0;
    lv_obj_set_hidden(book_list, false);
    lv_obj_set_hidden(reader_panel, true);
    statusbar_hide(false);
    
    scan_sd_for_books();
}

static void book_select_cb(lv_obj_t *btn, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        const char* filename = lv_list_get_btn_text(btn);
        String book_path = String("/books/") + filename;
        
        lv_label_set_text(status_label, "Loading...");
        epub_open_book(book_path);
        book_open = true;
        show_reader_view();
    }
}

static void prev_page_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        if (current_page > 0) {
            current_page--;
            update_display_page();
        }
    }
}

static void next_page_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        if (current_page < total_pages - 1) {
            current_page++;
            update_display_page();
        }
    }
}

static void font_decrease_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        epub_change_font_size(-2);
        update_display_page();
    }
}

static void font_increase_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        epub_change_font_size(2);
        update_display_page();
    }
}

static void back_to_list_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        book_open = false;
        current_book_lines.clear();
        show_book_list();
    }
}

static void exit_cb(lv_obj_t *obj, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
        mainbar_jump_to_maintile(LV_ANIM_OFF);
        statusbar_hide(false);
        book_open = false;
        current_book_lines.clear();
    }
}

void epub_reader_main_setup(uint32_t tile_num) {
    epub_reader_main_tile = mainbar_get_tile_obj(tile_num);
    
    // Create hidden reader panel
    reader_panel = lv_obj_create(epub_reader_main_tile, NULL);
    lv_obj_set_size(reader_panel, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_set_hidden(reader_panel, true);
    
    // Header for reader
    lv_obj_t *reader_header = lv_label_create(reader_panel, NULL);
    lv_label_set_text(reader_header, "EPUB Reader");
    lv_obj_align(reader_header, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    
    // Text area for book content
    text_area = lv_textarea_create(reader_panel, NULL);
    lv_obj_set_size(text_area, lv_disp_get_hor_res(NULL) - 20, 700);
    lv_obj_align(text_area, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);
    
    // Page counter
    page_label = lv_label_create(reader_panel, NULL);
    lv_label_set_text(page_label, "Page 1/1");
    lv_obj_align(page_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -50);
    
    // Control buttons for reader
    lv_obj_t *prev_btn = lv_btn_create(reader_panel, NULL);
    lv_obj_set_size(prev_btn, 50, 40);
    lv_obj_align(prev_btn, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 10, -10);
    lv_obj_t *prev_label = lv_label_create(prev_btn, NULL);
    lv_label_set_text(prev_label, "<");
    lv_obj_set_event_cb(prev_btn, prev_page_cb);
    
    lv_obj_t *font_dec = lv_btn_create(reader_panel, NULL);
    lv_obj_set_size(font_dec, 50, 40);
    lv_obj_align(font_dec, NULL, LV_ALIGN_IN_BOTTOM_MID, -60, -10);
    lv_obj_t *font_dec_label = lv_label_create(font_dec, NULL);
    lv_label_set_text(font_dec_label, "A-");
    lv_obj_set_event_cb(font_dec, font_decrease_cb);
    
    lv_obj_t *font_inc = lv_btn_create(reader_panel, NULL);
    lv_obj_set_size(font_inc, 50, 40);
    lv_obj_align(font_inc, NULL, LV_ALIGN_IN_BOTTOM_MID, 60, -10);
    lv_obj_t *font_inc_label = lv_label_create(font_inc, NULL);
    lv_label_set_text(font_inc_label, "A+");
    lv_obj_set_event_cb(font_inc, font_increase_cb);
    
    lv_obj_t *next_btn = lv_btn_create(reader_panel, NULL);
    lv_obj_set_size(next_btn, 50, 40);
    lv_obj_align(next_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10);
    lv_obj_t *next_label = lv_label_create(next_btn, NULL);
    lv_label_set_text(next_label, ">");
    lv_obj_set_event_cb(next_btn, next_page_cb);
    
    lv_obj_t *back_btn = lv_imgbtn_create(reader_panel, NULL);
    lv_imgbtn_set_src(back_btn, LV_BTN_STATE_RELEASED, &exit_32px);
    lv_obj_align(back_btn, NULL, LV_ALIGN_IN_TOP_RIGHT, -10, 10);
    lv_obj_set_event_cb(back_btn, back_to_list_cb);
    
    // Book list panel
    lv_obj_t *list_header = lv_label_create(epub_reader_main_tile, NULL);
    lv_label_set_text(list_header, "Select Book");
    lv_obj_align(list_header, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    
    book_list = lv_list_create(epub_reader_main_tile, NULL);
    lv_obj_set_size(book_list, lv_disp_get_hor_res(NULL), 840);
    lv_obj_align(book_list, NULL, LV_ALIGN_IN_TOP_MID, 0, 50);
    
    status_label = lv_label_create(epub_reader_main_tile, NULL);
    lv_label_set_text(status_label, "Scanning...");
    lv_obj_align(status_label, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -10);
    
    lv_obj_t *exit_btn = lv_btn_create(epub_reader_main_tile, NULL);
    lv_obj_set_size(exit_btn, 60, 40);
    lv_obj_align(exit_btn, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, -10, -10);
    lv_obj_t *exit_label = lv_label_create(exit_btn, NULL);
    lv_label_set_text(exit_label, "EXIT");
    lv_obj_set_event_cb(exit_btn, exit_cb);
    
    scan_sd_for_books();
}
