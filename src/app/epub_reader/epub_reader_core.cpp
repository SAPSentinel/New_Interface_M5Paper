#include "config.h"
#include "epub_reader_core.h"
#include <SD.h>

// Book state - global variables
std::vector<String> current_book_lines;
int current_page = 0;
int total_pages = 0;
int book_font_size = 18;
String current_book_path = "";
String epub_base_path = "";

// Configuration
const int MAX_BOOK_LINES = 2000;

void epub_open_book(String filename) {
    current_page = 0;
    total_pages = 0;
    current_book_lines.clear();
    book_font_size = 18;
    current_book_path = filename;
    
    if (filename.endsWith(".epub")) {
        if (!epub_extract_epub(filename.c_str())) {
            log_e("EPUB extraction failed");
            return;
        }
    } else {
        // Plain text file
        File file = SD.open(filename);
        if (!file) {
            log_e("Cannot open file: %s", filename.c_str());
            return;
        }
        
        while (file.available() && current_book_lines.size() < MAX_BOOK_LINES) {
            String line = file.readStringUntil('\n');
            line.trim();
            if (line.length() > 0) {
                current_book_lines.push_back(line);
            }
        }
        file.close();
    }
    
    current_page = 0;
    total_pages = epub_get_total_pages(1);
    log_i("Book opened: %d lines, %d pages", current_book_lines.size(), total_pages);
}

bool epub_extract_epub(const char* epub_path) {
    File epub_file = SD.open(epub_path);
    if (!epub_file) {
        log_e("Cannot open EPUB file");
        return false;
    }
    
    // Create extraction directory
    epub_base_path = String(epub_path);
    int last_dot = epub_base_path.lastIndexOf('.');
    if (last_dot > 0) {
        epub_base_path = epub_base_path.substring(0, last_dot) + "_extracted";
    } else {
        epub_base_path = "/books/_epub_temp";
    }
    
    // Check file size
    int file_size = epub_file.size();
    if (file_size > 5000000) {  // 5MB limit
        log_e("EPUB file too large: %d bytes", file_size);
        epub_file.close();
        return false;
    }
    
    log_i("Extracting EPUB (%d bytes) to %s", file_size, epub_base_path.c_str());
    
    // Read ZIP data into PSRAM
    uint8_t* zip_data = (uint8_t*)ps_malloc(file_size);
    if (!zip_data) {
        log_e("Not enough PSRAM for ZIP extraction");
        epub_file.close();
        return false;
    }
    
    epub_file.read(zip_data, file_size);
    epub_file.close();
    
    // Extract HTML/text content only (simplified extraction)
    // This is a basic approach - for full EPUB support, use a proper ZIP library
    String content = "";
    
    // Find .opf or .ncx files to get content order
    // For now, search for any HTML files containing text
    // This is a limitation - proper EPUB parsing requires more work
    
    // Basic fallback: treat as text file
    content = String((char*)zip_data);
    epub_strip_html_tags(content);
    
    // Split into lines
    int start = 0;
    while (start < content.length() && current_book_lines.size() < MAX_BOOK_LINES) {
        int end = content.indexOf('\n', start);
        if (end == -1) end = content.length();
        
        String line = content.substring(start, end);
        line.trim();
        if (line.length() > 0) {
            current_book_lines.push_back(line);
        }
        start = end + 1;
    }
    
    free(zip_data);
    
    log_i("EPUB extracted: %d lines", current_book_lines.size());
    return current_book_lines.size() > 0;
}

void epub_strip_html_tags(String &html) {
    // Remove <img> tags but replace with reference
    int pos = 0;
    while ((pos = html.indexOf("<img", pos)) >= 0) {
        int src_pos = html.indexOf("src=", pos);
        if (src_pos > 0) {
            char quote = html[src_pos + 4];
            int start = src_pos + 5;
            int end = html.indexOf(quote, start);
            if (end > start) {
                String img_path = html.substring(start, end);
                int tag_end = html.indexOf(">", pos);
                if (tag_end > pos) {
                    html.replace(html.substring(pos, tag_end + 1), "[IMAGE: " + img_path + "]");
                }
            }
        }
        pos++;
    }
    
    // Remove all other HTML tags
    while ((pos = html.indexOf("<")) >= 0) {
        int end = html.indexOf(">", pos);
        if (end < 0) break;
        html.remove(pos, end - pos + 1);
    }
    
    // Decode HTML entities
    html.replace("&nbsp;", " ");
    html.replace("&amp;", "&");
    html.replace("&lt;", "<");
    html.replace("&gt;", ">");
    html.replace("&quot;", "\"");
}

void epub_change_font_size(int delta) {
    int old_size = book_font_size;
    book_font_size += delta;
    
    if (book_font_size < 12) book_font_size = 12;
    if (book_font_size > 40) book_font_size = 40;
    
    if (book_font_size != old_size) {
        // Recalculate pages with new font size
        int old_page = current_page;
        total_pages = epub_get_total_pages(1);
        
        // Try to maintain approximate position
        if (total_pages > 0) {
            current_page = (old_page * total_pages) / total_pages;
            if (current_page >= total_pages) current_page = total_pages - 1;
        }
    }
}

void epub_get_page_content(int page_num, std::vector<String> &out_lines) {
    out_lines.clear();
    
    int line_height = 25;  // Approximate line height in pixels
    int visible_height = 760;  // Display height minus header/footer
    int lines_per_page = visible_height / line_height;
    
    // Adjust for font size
    if (book_font_size < 14) lines_per_page = 32;
    else if (book_font_size < 18) lines_per_page = 24;
    else if (book_font_size < 24) lines_per_page = 18;
    else lines_per_page = 12;
    
    int start_line = page_num * lines_per_page;
    int end_line = min(start_line + lines_per_page, (int)current_book_lines.size());
    
    for (int i = start_line; i < end_line; i++) {
        out_lines.push_back(current_book_lines[i]);
    }
}

int epub_get_total_pages(int font_multiplier) {
    int line_height = 25;  // Approximate line height
    int visible_height = 760;  // Display height minus header/footer
    int lines_per_page = visible_height / line_height;
    
    // Adjust for font size
    if (book_font_size < 14) lines_per_page = 32;
    else if (book_font_size < 18) lines_per_page = 24;
    else if (book_font_size < 24) lines_per_page = 18;
    else lines_per_page = 12;
    
    int total = (current_book_lines.size() + lines_per_page - 1) / lines_per_page;
    return max(1, total);
}
