#pragma once

#include <pebble.h>
#include "jobs.h"
#include "main_menu.h"
#include "job_menu.h"
#include "job_adjust.h"
#include "update.h"
#include "tertiary_text.h"

#define DISABLE_LOGGING true

#if DISABLE_LOGGING
#define LOG(...)
#define DEBUG(...)
#define INFO(...)
#define WARN(...)
#define ERROR(...)
#else
#define LOG(...) app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define DEBUG(...) app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define INFO(...) app_log(APP_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define WARN(...) app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define ERROR(...) app_log(APP_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#endif

#define ANIMATED true
#define HIDDEN true

#define MENU_SECTION_CELL  (cell_index->section * 100 + cell_index->row)
#define MENU_HEIGHT_SINGLE 28
#define MENU_HEIGHT_DOUBLE 42

#define END_TIME(JOB) ((time_t) (JOB)->Seconds + (time_t) (JOB)->Repeat_hrs*3600)

#define FONT_GOTHIC_24_BOLD           fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD)
#define FONT_GOTHIC_18_BOLD           fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD)
#define FONT_GOTHIC_14_BOLD           fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD)
#define FONT_GOTHIC_18                fonts_get_system_font(FONT_KEY_GOTHIC_18)
#define FONT_GOTHIC_14                fonts_get_system_font(FONT_KEY_GOTHIC_14)
#define FONT_BITHAM_30_BLACK          fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK)
#define FONT_BITHAM_34_MEDIUM_NUMBERS fonts_get_system_font(FONT_KEY_BITHAM_34_MEDIUM_NUMBERS)

#define ICON_RECT_PLAY        (GRect) { {  0,  0 }, { 16, 16 } }
#define ICON_RECT_PAUSE       (GRect) { { 16,  0 }, { 16, 16 } }
#define ICON_RECT_STOP        (GRect) { { 32,  0 }, { 16, 16 } }
#define ICON_RECT_TICK        (GRect) { { 16, 16 }, { 16, 16 } }
#define ICON_RECT_TIMER       (GRect) { {  0, 16 }, {  8, 16 } }
#define ICON_RECT_STOPWATCH   (GRect) { {  8, 16 }, {  8, 16 } }
#define ICON_RECT_ADJUST      (GRect) { {  0, 16 }, { 16, 16 } }
#define ICON_RECT_ADD         (GRect) { { 48, 16 }, { 16, 16 } }
#define ICON_RECT_RESET       (GRect) { { 48,  0 }, { 16, 16 } }
#define ICON_RECT_DELETE      (GRect) { {  0, 32 }, { 16, 16 } }
#define ICON_RECT_EDIT        (GRect) { { 16, 32 }, { 16, 16 } }
#define ICON_RECT_CONTROLS    (GRect) { {  0, 48 }, { 16, 16 } }
#define ICON_RECT_ABOUT       (GRect) { { 48, 32 }, { 16, 16 } }
#define ICON_RECT_SETTINGS    (GRect) { { 32, 32 }, { 16, 16 } }
#define ICON_RECT_MINUS       (GRect) { { 16, 48 }, { 16, 16 } }
#define ICON_RECT_CLOCK       (GRect) { { 32, 16 }, { 16, 16 } }

extern GBitmap *bitmap_matrix;
//extern GBitmap *bitmap_pause;
extern GBitmap *bitmap_play;
extern GBitmap *bitmap_add;
extern GBitmap *bitmap_settings;
extern GBitmap *bitmap_delete;
extern GBitmap *bitmap_edit;
extern GBitmap *bitmap_adjust;
extern GBitmap *bitmap_reset;
extern GBitmap *bitmap_minus;
extern GBitmap *bitmap_tick;

// Persistent Storage Keys
#define STORAGE_KEY_VERSION    1
#define STORAGE_KEY_SETTINGS   2
#define STORAGE_KEY_FIRST_MED  100

#define CURRENT_STORAGE_VERSION 3
//changes in storage version: 2 added bool Fixed to end of every Job struct
//changes in storage version: 3 configuration

typedef struct {
  uint8_t Mode;
  bool Alarm;
  bool Sort;
} Settings;

extern Settings settings;
extern bool export_after_save;

enum {
  MODE_COUNT_UP,
  MODE_COUNT_DOWN,
  MODE_NEXT_TIME
};

void main_save_data(void);