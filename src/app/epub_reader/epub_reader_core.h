#ifndef _EPUB_READER_CORE_H
#define _EPUB_READER_CORE_H

#include "config.h"
#include <Arduino.h>
#include <vector>

// Book state - declared in epub_reader_core.cpp
extern std::vector<String> current_book_lines;
extern int current_page;
extern int total_pages;
extern int book_font_size;
extern String current_book_path;
extern String epub_base_path;

// Core functions
void epub_open_book(String filename);
bool epub_extract_epub(const char* epub_path);
void epub_strip_html_tags(String &html);
void epub_change_font_size(int delta);
void epub_get_page_content(int page_num, std::vector<String> &out_lines);
int epub_get_total_pages(int font_multiplier);

#endif // _EPUB_READER_CORE_H
