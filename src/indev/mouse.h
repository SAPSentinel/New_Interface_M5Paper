#ifndef INDEV_MOUSE_H
#define INDEV_MOUSE_H

#include "lvgl.h"

void mouse_init(void);
bool mouse_read(lv_indev_drv_t * drv, lv_indev_data_t*data);

#endif // INDEV_MOUSE_H
