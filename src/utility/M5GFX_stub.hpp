#ifndef M5GFX_STUB_HPP
#define M5GFX_STUB_HPP

#include <cstdint>
#include "display/monitor.h"

namespace lgfx {
    using rgb565_t = uint16_t;
}

class M5GFX {
public:
    M5GFX() : _width(540), _height(960), _win_x(0), _win_y(0) {}
    void init() {}
    int width() const { return _width; }
    int height() const { return _height; }
    void startWrite() {}
    void endWrite() {}
    void setAddrWindow(int x, int y, int w, int h) { _win_x = x; _win_y = y; _win_w = w; _win_h = h; }
    void writePixels(const lgfx::rgb565_t *src, size_t len) {
        /* src is RGB565 contiguous pixels for _win_w * _win_h */
        monitor_write_area(_win_x, _win_y, _win_w, _win_h, src);
    }
    bool getTouch(uint16_t *x, uint16_t *y) {
        (void)x; (void)y; return false; /* use LVGL pointer/mouse input instead */
    }

private:
    int _width;
    int _height;
    int _win_x, _win_y, _win_w, _win_h;
};

extern M5GFX gfx; /* define in cpp */

#endif // M5GFX_STUB_HPP
