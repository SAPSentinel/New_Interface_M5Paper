#include "config.h"

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

const LV_ATTRIBUTE_MEM_ALIGN uint8_t sketchpad_64px_map[] = {
  0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const lv_img_dsc_t sketchpad_64px = {
  .header.always_zero = 0,
  .header.w = 64,
  .header.h = 64,
  .data_size = sizeof(sketchpad_64px_map),
  .header.cf = LV_IMG_CF_INDEXED_1BIT,
  .data = sketchpad_64px_map,
};
