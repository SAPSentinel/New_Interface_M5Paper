# My-TTGO-Watch M5Paper Project — Knowledge Base

> **Purpose:** Reference document for AI coding agents and developers working on the sharandac/My-TTGO-Watch firmware targeting the **M5Paper** e-ink device. Captures environment setup, architecture, past issues, resolutions, and hard constraints that must not be violated when making changes.

---

## Table of Contents

1. [Environment & Toolchain](#1-environment--toolchain)
2. [Project Architecture Overview](#2-project-architecture-overview)
3. [Build System Details](#3-build-system-details)
4. [M5Paper Hardware Constraints](#4-m5paper-hardware-constraints)
5. [LVGL UI Architecture](#5-lvgl-ui-architecture)
6. [Tile Layout System — M5Paper](#6-tile-layout-system--m5paper)
7. [Keyboard System — Critical Design](#7-keyboard-system--critical-design)
8. [WiFi / Password Entry Flow](#8-wifi--password-entry-flow)
9. [App Registration System](#9-app-registration-system)
10. [NTP Time Sync](#10-ntp-time-sync)
11. [Frozen Requirements — DO NOT CHANGE](#11-frozen-requirements--do-not-change)
12. [Common Pitfalls & Past Failures](#12-common-pitfalls--past-failures)
13. [Step-by-Step: Adding a New App](#13-step-by-step-adding-a-new-app)
14. [Step-by-Step: Build, Flash, Monitor](#14-step-by-step-build-flash-monitor)
15. [Files Modified From Upstream](#15-files-modified-from-upstream)

---

## 1. Environment & Toolchain

### Host Machine
| Item | Value |
|------|-------|
| OS | Windows 10/11 |
| IDE | VS Code with PlatformIO extension |
| Shell | PowerShell (pwsh) |
| Workspace | `D:\platform-io\New_Interface_M5Paper` |
| PlatformIO CLI | `C:\Users\<user>\.platformio\penv\Scripts\platformio.exe` |
| MSYS2 | Installed (not actively used for build; PlatformIO manages the toolchain) |

### Toolchain (Frozen)
| Component | Version | Notes |
|-----------|---------|-------|
| `platform` | `espressif32@3.3.0` | **MUST NOT upgrade** — see [Section 11](#11-frozen-requirements--do-not-change) |
| `framework` | `arduino` (custom fork) | `framework-arduinoespressif32 @ https://github.com/sharandac/arduino-esp32-hedge.git` |
| `toolchain-xtensa32` | `2.50200.97` (GCC 5.2.0) | Ships with espressif32@3.3.0 |
| Arduino ESP32 core | **1.x** (from above custom fork) | Uses old-style WiFi events: `WiFiEvent_t::SYSTEM_EVENT_STA_*` |
| LVGL | `7.11.0` (git tag `#v7.11.0`) | Not 8.x — API is completely different |
| Board | `m5stack-fire` | Used for M5Paper (16MB flash, PSRAM) |
| Flash | 80MHz QIO, `default_16MB.csv` partition table | |
| Serial | COM port (auto-detect or `--upload-port COMxx`) at 115200 baud | |

### Key Paths
```
D:\platform-io\New_Interface_M5Paper\
├── platformio.ini              # Only m5paper + emulator_m5paper envs
├── support\build_fixes.py      # Windows linker workaround (ESSENTIAL)
├── src\config.h                # Device-specific defines (RES_X_MAX, etc.)
├── src\gui\                    # LVGL UI code
├── src\hardware\               # Hardware abstraction (WiFi, BLE, PMU, etc.)
├── src\app\                    # Application modules (autocall registration)
├── lib\lv_conf.h               # LVGL v7.11 config (THE authoritative LVGL config)
└── .pio\build\m5paper\         # Build output (firmware.bin)
```

> **NOTE:** The correct LVGL config is `lib/lv_conf.h`. The file `include/lv_conf.h` must **NOT** exist — if it appears it is likely an unwanted LVGL v8 config that will conflict with LVGL v7.11.

---

## 2. Project Architecture Overview

This is a smartwatch/e-ink device firmware built on:
- **Arduino ESP32 1.x** framework (custom fork with power management)
- **LVGL 7.11** for the GUI
- **FreeRTOS** (bundled with ESP-IDF under Arduino)

The firmware supports multiple hardware targets via `#if defined(...)` preprocessor guards:
- `M5PAPER` — 540×960 e-ink (our target)
- `LILYGO_WATCH_2020_V1/V2/V3` — 240×240 TFT
- `M5CORE2` — 320×240 TFT
- `LILYGO_WATCH_2021` — 240×240
- `WT32_SC01` — 480×320

**Every hardware-specific change must be guarded with `#if defined( M5PAPER )`** to avoid breaking other targets.

---

## 3. Build System Details

### The Windows 32KB Linker Problem

The project compiles ~536 object files. On Windows, the linker command exceeds the 32,768 character `CreateProcess` limit. This happens at TWO levels:
1. The `g++` invocation itself
2. The internal `g++ → collect2` subprocess

### Solution: `support/build_fixes.py`

This file is **critical and must not be deleted**. It:
1. Archives all 536 `.o` files into `project_objs.a` using `ar -M` (MRI script mode — no CLI length limit)
2. Links with `--whole-archive project_objs.a --no-whole-archive` instead of listing individual `.o` files
3. Keeps the final link command under ~3000 characters

Referenced in `platformio.ini` as:
```ini
extra_scripts = support/build_fixes.py
```

### Build Commands
```powershell
# Build only
C:\Users\<user>\.platformio\penv\Scripts\platformio.exe run --environment m5paper

# Build + Upload
C:\Users\<user>\.platformio\penv\Scripts\platformio.exe run --target upload --environment m5paper --upload-port COM22

# Serial Monitor
C:\Users\<user>\.platformio\penv\Scripts\platformio.exe device monitor --baud 115200 --port COM22
```

### Build Output
- Firmware: `.pio\build\m5paper\firmware.bin` (~4.97 MB, 75.8% of 16MB flash)
- RAM usage: ~1.2% of 6.25MB
- PSRAM: ~1.85 MB free after boot

---

## 4. M5Paper Hardware Constraints

| Property | Value |
|----------|-------|
| Display | 540×960 e-ink (IT8951 controller) |
| Refresh | Partial update via `M5.EPD.UpdateArea()`, full via `M5.EPD.UpdateFull()` |
| Framebuffer | **Two full-screen buffers** in PSRAM (~1MB each) |
| Refresh delay | 100ms debounce (`FRAMEBUFFER_REFRESH_DELAY`) — batches LVGL flush calls |
| Touch | Capacitive (GT911) |
| CPU | ESP32-D0WDQ6-V3 @ 240MHz, dual-core |
| Flash | 16MB, QIO @ 80MHz |
| PSRAM | 8MB (SPIRAM) |
| RTC | BM8563 |
| Battery | Reports via PMU (4.2V–3.3V) |

### E-ink Display Critical Rules

1. **No rapid full-screen redraws.** The `framebuffer_flush_cb` for M5Paper does pixel-by-pixel `canvas.drawPixel()` for each dirty area. Full-screen overlays (540×960 = 518,400 pixels) cause multi-second freezes or LVGL watchdog triggers.

2. **No animations.** Use `LV_ANIM_OFF` where possible for M5Paper tile transitions. E-ink cannot display smooth animations — they just cause multiple slow refreshes.

3. **LVGL layer rendering order:** `lv_layer_top()` > tileview (`lv_scr_act()` children) > `lv_layer_sys()`. The keyboard must render on `lv_layer_top()` to appear above the tileview.

---

## 5. LVGL UI Architecture

### GUI Initialization Order (`gui.cpp → gui_setup()`)
```
1. mainbar_setup()        — creates the lv_tileview
2. main_tile_setup()      — home screen (clock, battery, etc.)
3. app_tile_setup()       — app icon grid pages
4. note_tile_setup()      — note/info page
5. setup_tile_setup()     — settings icon grid pages
6. statusbar_setup()      — top status bar
7. quickbar_setup()       — quick action bar
8. keyboard_setup()       — text keyboard overlay
9. num_keyboard_setup()   — numeric keyboard overlay
```

### LVGL Concurrency
- A FreeRTOS mutex (`xGUI_SemaphoreMutex`) guards LVGL operations
- `gui_take()` / `gui_give()` must bracket LVGL calls from non-GUI contexts
- The main `powermgm_loop` drives `lv_task_handler()` — LVGL is **single-threaded**

### Object Hierarchy
```
lv_scr_act()
  └─ mainbar (lv_tileview)
       ├─ main_tile [0,0]
       ├─ app_tiles [0,1] [0,2] ...
       ├─ setup_tiles [0, 1+MAX_APPS_TILES] ...
       ├─ note_tile [0, 1+MAX_APPS_TILES+MAX_SETUP_TILES]
       └─ (app-specific tiles via mainbar_add_app_tile / mainbar_add_setup_tile)

lv_layer_top()
  └─ kb_screen (keyboard overlay — M5Paper only)
       ├─ kb_textarea
       ├─ kb (lv_keyboard)
       └─ nkb (lv_keyboard, numeric)
```

---

## 6. Tile Layout System — M5Paper

The M5Paper uses a **vertical** tile layout (column 0, rows 0..N). Other devices use horizontal layouts.

### Main Navigation Tiles (Column 0)
| Row | Tile | Code Location |
|-----|------|---------------|
| 0 | Main (clock) | `main_tile.cpp: mainbar_add_tile(0, 0, ...)` |
| 1 | App Page 1 | `app_tile.cpp: mainbar_add_tile(0, 1 + tiles, ...)` |
| 2 | App Page 2 | (tiles=1 in loop) |
| 3 | Setup Page | `setup_tile.cpp: mainbar_add_tile(0, 1 + MAX_APPS_TILES + tiles, ...)` |
| 4 | Note Page | `note_tile.cpp: mainbar_add_tile(0, 1 + MAX_APPS_TILES + MAX_SETUP_TILES, ...)` |

### Dynamic Position Calculation
Setup and note tile positions are now calculated dynamically using `MAX_APPS_TILES` and `MAX_SETUP_TILES` constants. **If you change `MAX_APPS_TILES`, setup and note tiles automatically adjust** — no hardcoded row numbers.

### App-specific tiles
Each app gets its own tile(s) via `mainbar_add_app_tile(x, y, id)` or `mainbar_add_setup_tile(x, y, id)`. These are placed at `MAINBAR_APP_TILE_Y_START = 8` (row 8+) in a separate coordinate space, preventing overlap with navigation tiles.

### M5Paper App Grid
- 4 columns × 5 rows per page = 20 icons per page
- `MAX_APPS_TILES = 2` → 40 total app icon slots
- `MAX_SETUP_TILES = 1` → 20 setup icon slots
- Icon size: 70×70px with 48px horizontal / 72px vertical clearance

---

## 7. Keyboard System — Critical Design

### The Problem (M5Paper)
The original keyboard uses `lv_scr_act()` as its parent. On the watch devices (240×240 TFT), this works because the keyboard container is a child of the active screen and renders on top. But on M5Paper with its tileview-based navigation:

1. **`lv_scr_act()` approach:** The keyboard renders behind/at the same level as the tileview, making it invisible or causing the tileview to steal touch events.

2. **Tile-based approach (FAILED):** Creating the keyboard on a dedicated tile and using `mainbar_jump_to_tilenumber()` to navigate there causes **nested tile jumps** — the WiFi password flow does `jump_to(password_tile)` and then the text field's click event immediately calls `keyboard_set_textarea()` which would do another `jump_to(keyboard_tile)`. Two simultaneous LVGL tileview animations deadlock the e-ink refresh pipeline.

### The Solution: `lv_layer_top()`
The keyboard is created on `lv_layer_top()` — LVGL's topmost rendering layer:
- Renders above everything including the tileview
- No tile navigation needed (just show/hide)
- No nested jump conflicts
- Touch events are captured by `lv_layer_top()` children first

```cpp
// keyboard.cpp — M5Paper branch
kb_screen = lv_cont_create( lv_layer_top(), NULL );
```

### Current Status (as of latest build)
The `lv_layer_top()` approach is deployed and under testing. If it still freezes, the remaining suspect is the full-screen dirty region that the keyboard causes on the 540×960 e-ink. A potential next fix would be to make the keyboard smaller (not full-screen) or to use a dedicated LVGL screen (`lv_scr_load()`) instead.

---

## 8. WiFi / Password Entry Flow

### Event Chain
```
1. User enables WiFi toggle → wifictl_on()
2. WiFi scans → WIFICTL_SCAN_ENTRY events fire
3. For each network: creates list button with wifi_settings_enter_pass_event_cb
4. User taps network name:
   → wifi_settings_enter_pass_event_cb(LV_EVENT_CLICKED)
   → sets network name label
   → clears password text field
   → mainbar_jump_to_tilenumber(wifi_password_tile_num, LV_ANIM_ON)
5. Password tile appears with empty text field
6. User taps the text field:
   → wlan_password_event_cb(LV_EVENT_CLICKED)
   → keyboard_set_textarea(wifi_password_pass_textfield)
   → keyboard overlay appears
7. User types password, presses ✓ (Apply):
   → copies text to wifi_password_pass_textfield
   → hides keyboard
8. User taps checkmark button:
   → apply_wifi_password_event_cb
   → wifictl_insert_network(name, password)
   → hides keyboard, jumps back to WiFi list
```

### Critical Rule
**NEVER call `keyboard_set_textarea()` in `wifi_settings_enter_pass_event_cb`.** The keyboard must only open when the user taps the password text field on the password tile. Adding keyboard calls to the network-tap handler causes nested tile jumps or immediate LVGL deadlocks on e-ink.

---

## 9. App Registration System

### Autocall Mechanism
Apps register themselves using a static initializer trick:
```cpp
// In <app_name>_app.cpp:
static int registed = app_autocall_function( &my_app_setup, priority );
```

This runs before `setup()` and registers the app's setup function. During `hardware_post_setup()`, all registered functions are called.

### App Setup Function Requirements
Each app setup must:
1. **Get app tiles:** `mainbar_add_app_tile(1, 1, "my app")` — returns the first tile number
2. **Register an icon:** `app_tile_register_app("App Name")` — returns an `lv_obj_t*` icon container
3. **Set icon image:** Create an `lv_imgbtn` inside the icon container
4. **Set click handler:** The click handler MUST call:
   ```cpp
   statusbar_hide(true);
   mainbar_jump_to_tilenumber(my_tile_num, LV_ANIM_OFF, true);
   ```

### App Icon Slot Limits
- M5Paper: `MAX_APPS_ICON = 4 × 5 × 2 = 40` slots
- If all slots are used, `app_tile_register_app()` returns `NULL` and logs `[E] no space for an app icon`
- **Check the log** on boot — if you see this error, increase `MAX_APPS_TILES` in `app_tile.h`

### Adding an App — DO NOT:
- Call `M5.begin()` in app setup — it's already called in `main.cpp`
- Call `M5.EPD.SetRotation()` or `M5.EPD.Clear()` in setup — framebuffer.cpp handles this
- Leave `app_autocall_function()` commented out — the app won't load
- Forget to add a click handler that jumps to the app tile

---

## 10. NTP Time Sync

### How It Works
1. WiFi connects → `WIFICTL_CONNECT` event fires
2. `timesync_wifictl_event_cb` receives event
3. Creates FreeRTOS task `timesync_Task` with **5000 word stack** (was 2000, increased to prevent stack overflow)
4. Task calls `configTzTime(timezone_rule, "pool.ntp.org", ...)` then `getLocalTime()`
5. On success, sets RTC and fires `TIME_SYNC` callback

### Known Issues
- Stack was originally 2000 words — `configTzTime` + TLS negotiation + `getLocalTime` could overflow silently, causing the task to crash without any log output
- Time sync only triggers when WiFi connects — if device boots with saved WiFi, it connects and syncs automatically
- Timezone config is stored in `/timesync.json` on SPIFFS

---

## 11. Frozen Requirements — DO NOT CHANGE

These are hard constraints learned from failures. Violating any of these will break the build or the device.

### Build System
| Constraint | Reason |
|------------|--------|
| `espressif32@3.3.0` | Higher versions (e.g., 6.5.0) bring Arduino ESP32 3.x which uses completely different WiFi API (`WiFiEvent_t` → `arduino_event_id_t`), drops `SYSTEM_EVENT_STA_*` constants, breaks compilation of 50+ files |
| `framework-arduinoespressif32 @ sharandac/arduino-esp32-hedge.git` | Custom power management, DFS, light sleep support. Standard Arduino ESP32 1.x lacks these |
| `support/build_fixes.py` must exist and be referenced in `platformio.ini` | Without it, Windows builds fail with "command line too long" errors at the link stage |
| `toolchain-xtensa32@2.50200.97` (GCC 5.2.0) | Bundled with espressif32@3.3.0. Do not mix toolchains |
| `lib_archive = true` in `[env:m5paper]` | Required for the archive-based linker workaround |
| **Every emulator-only file in `src/` MUST be wrapped in `#if defined(NATIVE_64BIT)`** | `build_src_filter = +<*>` compiles ALL files under `src/` for BOTH environments, including the ESP32 m5paper target. Files with SDL2, `windows.h`, or M5GFX headers will fail to compile on ESP32 if not guarded. See Failure 8. |
| `include/lv_conf.h` must NOT exist | If present, it overrides `lib/lv_conf.h`. Any `include/lv_conf.h` is likely an accidental LVGL v8 config that silently breaks the LVGL v7.11 build |

### Framework API
| Constraint | Reason |
|------------|--------|
| WiFi events use `SYSTEM_EVENT_STA_*` | Arduino ESP32 1.x API. Do NOT use `ARDUINO_EVENT_WIFI_*` (that's 2.x/3.x) |
| LVGL 7.11 API only | `lv_keyboard_create(parent, copy)`, `lv_textarea_create(parent, copy)`, etc. LVGL 8.x removed the `copy` parameter and renamed many functions |
| `#include <M5EPD.h>` for M5Paper hardware access | Not `M5Unified.h` or `M5.h` (those are for newer M5Stack libraries) |

### UI Architecture
| Constraint | Reason |
|------------|--------|
| Keyboard must use `lv_layer_top()` on M5Paper | `lv_scr_act()` → invisible behind tileview. Tile-based → nested jump deadlock |
| Never do two `mainbar_jump_to_tilenumber()` calls in the same event chain | LVGL tileview animation + e-ink refresh = freeze |
| Use `LV_ANIM_OFF` for M5Paper tile jumps where possible | E-ink can't animate |
| All `#ifdef` hardware-specific code must use `M5PAPER` guard | Never change code in `#else` or other device branches |
| Setup/note tile positions must use `MAX_APPS_TILES` constant, not hardcoded numbers | Changing `MAX_APPS_TILES` must automatically adjust downstream tile positions |

### Hardware
| Constraint | Reason |
|------------|--------|
| Do NOT call `M5.begin()` outside of `main.cpp` | Double init corrupts I2C/SPI buses |
| Do NOT call `M5.EPD.Clear(true)` in app setup functions | `framebuffer_setup()` handles initial clear. Extra clears cause visible screen flash and slow startup |
| FreeRTOS task stacks should be ≥5000 words for network tasks | TLS/NTP operations need significant stack space. 2000 words causes silent overflow crashes |

---

## 12. Common Pitfalls & Past Failures

### Failure 1: espressif32 Upgrade (3.3.0 → 6.5.0)
**Symptom:** 50+ compilation errors — unknown types, missing constants.
**Root Cause:** espressif32 6.5.0 brings Arduino ESP32 3.x which fundamentally changes WiFi event API, removes `SYSTEM_EVENT_STA_*`, changes BLE API.
**Fix:** Full revert to espressif32@3.3.0 with the custom framework.
**Lesson:** NEVER upgrade the platform version without a full API audit.

### Failure 2: Windows Linker 32KB Limit
**Symptom:** Build fails at link stage with cryptic `CreateProcess` or `spawn` errors.
**Root Cause:** 536 `.o` files + flags exceed Windows' 32,768-char command line limit.
**Fix:** `support/build_fixes.py` — archives objects into `.a`, links with short command.
**Lesson:** Any build system change must verify the link command stays under 32KB.

### Failure 3: Keyboard Overlay Freezes Device
**Symptom:** Device freezes immediately after `keyboard_set_textarea called, showing keyboard` log message.
**Root Cause (Attempt 1):** The keyboard was created on `lv_scr_act()` which on M5Paper placed it behind the tileview, and the full-screen redraw (540×960 pixels, pixel-by-pixel through e-ink canvas) stalled the system.
**Root Cause (Attempt 2):** Moving keyboard to a dedicated tile required `mainbar_jump_to_tilenumber()` to show it, but the WiFi password flow already does a tile jump (network tap → password tile). The text field's touch event then triggered the keyboard tile jump = two nested jumps = LVGL deadlock.
**Fix:** Create keyboard on `lv_layer_top()` — no tile navigation, renders above everything.
**Lesson:** On e-ink, avoid any approach that triggers large overlapping redraws or nested tile transitions.

### Failure 4: App Icons Overlapping with Settings
**Symptom:** Last app icon overlaps with the first settings icon on the same tile.
**Root Cause:** Setup tile position was hardcoded as `(0, 2)` which conflicted with app page 2 at `(0, 2)` when `MAX_APPS_TILES` was increased from 1 to 2.
**Fix:** Use `mainbar_add_tile(0, 1 + MAX_APPS_TILES + tiles, ...)` for setup, and `mainbar_add_tile(0, 1 + MAX_APPS_TILES + MAX_SETUP_TILES, ...)` for note.
**Lesson:** All tile positions must be calculated relative to preceding sections, never hardcoded.

### Failure 5: Custom Apps Not Appearing
**Symptom:** epub_reader, image_viewer, sketchpad not in app grid.
**Root Cause:** `app_autocall_function()` lines were commented out with "TEMPORARILY DISABLED FOR DEBUGGING."
**Fix:** Uncommented the registration lines.
**Lesson:** Always grep for `app_autocall_function` to verify all apps are registered.

### Failure 6: App Click Does Nothing
**Symptom:** Tapping app icon shows no response.
**Root Cause:** Click handler was empty `{}` — no `mainbar_jump_to_tilenumber()` call.
**Fix:** Added proper navigation: `statusbar_hide(true); mainbar_jump_to_tilenumber(tile_num, LV_ANIM_OFF, true);`
**Lesson:** Every app icon must have a click handler that jumps to its tile.

### Failure 7: "No space for an app icon" Error
**Symptom:** `[E][app_tile.cpp:164] no space for an app icon` in serial log.
**Root Cause:** `MAX_APPS_TILES=1` only had 20 slots. With 3 new apps, total exceeded 20.
**Fix:** Increased `MAX_APPS_TILES` to 2 (40 slots).
**Lesson:** When adding apps, count total registered apps and ensure `MAX_APPS_ICON` covers them all.

### Failure 8: Emulator-Only Files Break the m5paper ESP32 Build
**Symptom:** Build fails with errors like `fatal error: SDL2/SDL.h: No such file or directory`, `'M5GFX' was not declared`, `windows.h: No such file or directory` for the `m5paper` environment.
**Root Cause:** New files were added to `src/` for the Windows emulator (`emulator_m5paper` environment) without wrapping them in `#if defined(NATIVE_64BIT)`. Because `platformio.ini` has `build_src_filter = +<*>`, PlatformIO compiles **every file under `src/`** for both environments — including the ESP32 target. SDL2, `windows.h`, and `M5GFX.h` do not exist on ESP32.

A secondary bug was an **inverted guard** in `src/utility/lvgl_port_m5stack.hpp`:
```cpp
// WRONG — this includes M5GFX.h specifically FOR the m5paper target:
#if defined(M5PAPER) || !defined(NATIVE_64BIT)

// CORRECT — this includes M5GFX.h only for the emulator:
#if defined(NATIVE_64BIT) && !defined(M5PAPER)
```
A third bug was the presence of `include/lv_conf.h` (an LVGL v8 config file) which overrode the correct `lib/lv_conf.h` (LVGL v7.11), causing API mismatch errors throughout the codebase.

**Fix:** Wrapped all emulator-only files in `#if defined(NATIVE_64BIT)` ... `#endif`. Fixed the inverted guard. Deleted `include/lv_conf.h`.

**Files affected:** `src/sdl_main_stub.c`, `src/winmain.c`, `src/indev/mouse.cpp`, `src/utility/sdl_entry.cpp`, `src/utility/sdl_stubs.c`, `src/utility/sdl_main.cpp`, `src/utility/M5GFX_stub.cpp`, `src/utility/lvgl_port_m5stack.hpp`, `src/utility/lvgl_port_m5stack.cpp`.

**Lesson:** **Every file added to `src/` that uses Windows/SDL2/M5GFX APIs must be guarded with `#if defined(NATIVE_64BIT)`**. Double-check guard logic — an inverted condition (`||` vs `&&`, wrong operands) is as bad as a missing guard. Never create `include/lv_conf.h`.

---

## 13. Step-by-Step: Adding a New App

### File Structure
```
src/app/my_app/
├── my_app_app.cpp        # Registration + icon setup
├── my_app_app.h          # Header with setup declaration
├── my_app_main.cpp       # Main functionality
└── my_app_main.h         # Main header
```

### my_app_app.cpp Template
```cpp
#include "config.h"
#include "my_app_app.h"
#include "my_app_main.h"

#include "gui/mainbar/app_tile/app_tile.h"
#include "gui/mainbar/mainbar.h"
#include "gui/statusbar.h"
#include "gui/widget_factory.h"
#include "gui/app.h"

// Icon (64x64 C array, generated from PNG via LVGL converter)
LV_IMG_DECLARE(my_app_64px);

uint32_t my_app_main_tile_num;
icon_t *my_app_icon = NULL;

static void enter_my_app_event_cb(lv_obj_t *obj, lv_event_t event);

static void my_app_app_setup(void) {
    my_app_main_tile_num = mainbar_add_app_tile(1, 1, "my app");
    
    my_app_icon = app_tile_get_free_app_icon();
    if (my_app_icon == NULL) {
        log_e("no free app icon for my_app");
        return;
    }
    
    my_app_icon->active = true;
    
    // Set icon image
    lv_obj_t *img = lv_imgbtn_create(my_app_icon->icon_cont, NULL);
    lv_imgbtn_set_src(img, LV_BTN_STATE_RELEASED, &my_app_64px);
    lv_imgbtn_set_src(img, LV_BTN_STATE_PRESSED, &my_app_64px);
    lv_imgbtn_set_src(img, LV_BTN_STATE_CHECKED_RELEASED, &my_app_64px);
    lv_imgbtn_set_src(img, LV_BTN_STATE_CHECKED_PRESSED, &my_app_64px);
    lv_obj_reset_style_list(img, LV_OBJ_PART_MAIN);
    lv_obj_set_event_cb(img, enter_my_app_event_cb);
    
    // Set label
    lv_label_set_text(my_app_icon->label, "My App");
    lv_obj_align(my_app_icon->label, my_app_icon->icon_cont, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_hidden(my_app_icon->icon_cont, false);
    lv_obj_set_hidden(my_app_icon->label, false);
    
    // Initialize main UI
    my_app_main_setup(my_app_main_tile_num);
}

static void enter_my_app_event_cb(lv_obj_t *obj, lv_event_t event) {
    switch (event) {
        case LV_EVENT_CLICKED:
            statusbar_hide(true);
            mainbar_jump_to_tilenumber(my_app_main_tile_num, LV_ANIM_OFF, true);
            break;
    }
}

// THIS LINE MUST NOT BE COMMENTED OUT
static int registed = app_autocall_function(&my_app_app_setup, 0);
```

### Checklist After Adding
- [ ] `app_autocall_function()` line is NOT commented out
- [ ] Click handler calls `mainbar_jump_to_tilenumber()`
- [ ] No `M5.begin()` in setup
- [ ] No `M5.EPD.Clear()` in setup
- [ ] Total app count doesn't exceed `MAX_APPS_ICON` (check serial log for errors)
- [ ] If adding many apps: consider increasing `MAX_APPS_TILES` in `app_tile.h`
- [ ] If using SD card: include `<SD.h>` and check `SD.begin()` isn't called redundantly

---

## 14. Step-by-Step: Build, Flash, Monitor

### Prerequisites
1. M5Paper connected via USB (check Device Manager for COM port)
2. PlatformIO installed in VS Code
3. No other program using the COM port (close serial monitors before flashing)

### Build
```powershell
C:\Users\<user>\.platformio\penv\Scripts\platformio.exe run --environment m5paper
```
Expected: `SUCCESS` with firmware.bin ~4.97MB, Flash usage ~75.8%.

### Flash
```powershell
# Close any serial monitors first!
C:\Users\<user>\.platformio\penv\Scripts\platformio.exe run --target upload --environment m5paper --upload-port COM22
```
Expected: Writing progress 0–100%, `Hash of data verified`, `Hard resetting via RTS pin`, `SUCCESS`.

### Monitor
```powershell
C:\Users\<user>\.platformio\penv\Scripts\platformio.exe device monitor --baud 115200 --port COM22
```
Exit with Ctrl+C.

### What to Look For in Serial Output

**Good (normal boot):**
```
M5EPD initializing...OK
[I] framebuffer_setup(): framebuffer 1: 0x... (1036800 bytes, 540x960px)
[I] gui_setup(): mainbar setup
[I] gui_setup(): app tile setup
[I] gui_setup(): setup tile setup
[I] gui_setup(): keyboard setup
[I] hardware_post_setup(): Free heap: ~150KB
[I] hardware_post_setup(): Free PSRAM heap: ~1.85MB
```

**Bad (errors to watch for):**
```
[E] no space for an app icon       → Increase MAX_APPS_TILES
[E] keyboard not initialized       → keyboard_setup() not called before use
[E] apptile not initialized        → app_tile_setup() order wrong in gui.cpp
Guru Meditation Error              → Stack overflow or null pointer
```

### Corrupt Serial Output
Some garbled characters in serial output (like `␆`, `~f`, `␀`) are **normal** — they occur when the ESP32 transitions between CPU frequencies (240MHz ↔ 80MHz) during light sleep, which briefly corrupts UART timing. These are not errors.

---

## 15. Files Modified From Upstream

| File | Change | Why |
|------|--------|-----|
| `platformio.ini` | Pruned to m5paper + emulator_m5paper only; added `extra_scripts = support/build_fixes.py` | Cleanup + Windows linker fix |
| `support/build_fixes.py` | **New file** — archive-based linker workaround | Windows 32KB command line limit |
| `src/gui/mainbar/app_tile/app_tile.h` | `MAX_APPS_TILES`: 1→2 for M5Paper | 3 new apps need >20 icon slots |
| `src/gui/keyboard.cpp` | M5Paper: create on `lv_layer_top()` instead of `lv_scr_act()` | Prevent freeze on e-ink display |
| `src/gui/mainbar/setup_tile/setup_tile.cpp` | Setup tile row: `0, 2+tiles` → `0, 1+MAX_APPS_TILES+tiles` | Dynamic positioning after app pages |
| `src/gui/mainbar/note_tile/note_tile.cpp` | Note tile row: `0, 3` → `0, 1+MAX_APPS_TILES+MAX_SETUP_TILES` | Dynamic positioning after setup pages |
| `src/hardware/timesync.cpp` | NTP task stack: 2000→5000 words | Prevent silent stack overflow during TLS/NTP |
| `src/gui/mainbar/setup_tile/wlan_settings/wlan_settings.cpp` | Verified upstream state (no keyboard call in network tap handler) | Prevent nested tile jump freeze |
| `src/app/epub_reader/epub_reader_app.cpp` | Uncommented `app_autocall_function`; added proper click handler | App was disabled |
| `src/app/image_viewer/image_viewer_app.cpp` | Uncommented `app_autocall_function`; fixed click handler; removed redundant `M5.begin()` | App was disabled + bad init |
| `src/app/sketchpad/sketchpad_app.cpp` | Uncommented `app_autocall_function`; fixed click handler; removed redundant `M5.begin()` | App was disabled + bad init |
| `src/sdl_main_stub.c` | Wrapped entire file in `#if defined(NATIVE_64BIT)` | Emulator-only; was failing ESP32 compile |
| `src/winmain.c` | Wrapped entire file in `#if defined(NATIVE_64BIT)` | Emulator-only; was failing ESP32 compile |
| `src/indev/mouse.cpp` | Wrapped entire file in `#if defined(NATIVE_64BIT)` | Emulator-only SDL2 input; was failing ESP32 compile |
| `src/display/monitor.cpp` | Guarded with `#if defined(NATIVE_64BIT) \|\| !defined(M5PAPER)` | Emulator SDL2 display driver |
| `src/utility/sdl_entry.cpp` | Wrapped entire file in `#if defined(NATIVE_64BIT)` | Emulator-only; was failing ESP32 compile |
| `src/utility/sdl_stubs.c` | Wrapped entire file in `#if defined(NATIVE_64BIT)` | Emulator-only; was failing ESP32 compile |
| `src/utility/sdl_main.cpp` | Wrapped entire file in `#if defined(NATIVE_64BIT)` | Emulator-only; was failing ESP32 compile |
| `src/utility/M5GFX_stub.cpp` | Wrapped entire file in `#if defined(NATIVE_64BIT) && !defined(M5PAPER)` | Emulator-only M5GFX shim |
| `src/utility/lvgl_port_m5stack.hpp` | Fixed **inverted** guard from `defined(M5PAPER) \|\| !defined(NATIVE_64BIT)` → `defined(NATIVE_64BIT) && !defined(M5PAPER)` | Guard was backwards — was including M5GFX on ESP32 |
| `src/utility/lvgl_port_m5stack.cpp` | Wrapped entire file in `#if defined(NATIVE_64BIT) && !defined(M5PAPER)` | Emulator-only LVGL/M5GFX port |
| `include/lv_conf.h` | **DELETED** | Was an LVGL v8 config accidentally added; overrode correct `lib/lv_conf.h` (v7.11) |

---

## Appendix: Quick Reference for AI Agents

### When Asked to Modify This Project
1. Read this document first
2. Check [Section 11](#11-frozen-requirements--do-not-change) before making any change
3. Always build and verify firmware size stays under 6.5MB
4. Test with serial monitor — check for `[E]` log messages
5. Never upgrade `espressif32` platform version
6. Never delete `support/build_fixes.py`
7. Every `#ifdef M5PAPER` change should be isolated — don't touch other device branches

### When Asked to Add an App
1. Follow [Section 13](#13-step-by-step-adding-a-new-app) template
2. Count existing registered apps (grep for `app_autocall_function` across `src/app/`)
3. If total approaches `MAX_APPS_ICON`, increase `MAX_APPS_TILES`
4. Verify no `M5.begin()`, no `M5.EPD.Clear()`, no redundant hardware init in app code

### When Asked to Fix a Freeze
1. Check serial monitor for the last log message before freeze
2. Look for nested `mainbar_jump_to_tilenumber()` calls in the event chain
3. Look for full-screen LVGL redraws (overlay show/hide on 540×960)
4. Check FreeRTOS task stack sizes (minimum 5000 words for network tasks)
5. Check for LVGL mutex deadlocks (`gui_take()` without `gui_give()`)

### When Asked to Change the Build
1. Verify `support/build_fixes.py` is still referenced
2. After any `platformio.ini` change, do a full clean build
3. Check link command length in build output (should be ~3000 chars, not 30000+)
4. If adding new libraries, verify they're compatible with Arduino ESP32 1.x and ESP-IDF 3.x

### When Adding New Files to `src/` for the Emulator
1. **ALWAYS** wrap the entire file contents in `#if defined(NATIVE_64BIT)` ... `#endif /* NATIVE_64BIT */`
2. If the file is ALSO excluded from M5Paper specifically, use `#if defined(NATIVE_64BIT) && !defined(M5PAPER)`
3. Double-check guard logic — an inverted condition silently compiles for the wrong target
4. **NEVER** create `include/lv_conf.h` — this path is reserved as an accidentally-placed LVGL v8 config and will break the build. The correct LVGL config is `lib/lv_conf.h` only.
5. After adding, do a full clean build of the `m5paper` environment to confirm no ESP32 compile errors
