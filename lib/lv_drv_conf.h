/**
 * @file lv_drv_conf.h
 * Configuration file for lv_drivers library
 */

#ifndef LV_DRV_CONF_H
#define LV_DRV_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * MONITOR - Display using SDL window  
 */
#define USE_MONITOR         1

/**
 * MOUSE - Touch input simulation via system mouse
 */
#define USE_MOUSE           1

/**
 * Other drivers
 */
#define USE_MOUSEWHEEL      0
#define USE_KEYBOARD        0
#define USE_EVDEV           0
#define USE_LIBINPUT        0
#define USE_XPT2046         0
#define USE_WINDOWS         0
#define USE_FBDEV           0

#ifdef __cplusplus
}
#endif

#endif /* LV_DRV_CONF_H */
