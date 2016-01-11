#include "main.h"

GBitmap *bitmap_matrix;
GBitmap *bitmap_pause;
GBitmap *bitmap_play;
GBitmap *bitmap_add;
GBitmap *bitmap_settings;
GBitmap *bitmap_delete;
GBitmap *bitmap_edit;
GBitmap *bitmap_adjust;
GBitmap *bitmap_reset;
GBitmap *bitmap_minus;
GBitmap *bitmap_tick;

// *****************************************************************************************************
// DATA STORAGE
// *****************************************************************************************************

void main_save_data(void) {
  persist_write_int(STORAGE_KEY_VERSION, CURRENT_STORAGE_VERSION);
  //persist_write_data(STORAGE_KEY_SETTINGS, &settings, sizeof(Settings));
  jobs_list_save(STORAGE_KEY_FIRST_JOB);
}

static void main_load_data(void) {
  uint8_t stored_version = persist_read_int(STORAGE_KEY_VERSION); // defaults to 0 if key is missing
  /* 
  if (persist_exists(STORAGE_KEY_SETTINGS)) {
    persist_read_data(STORAGE_KEY_SETTINGS, &settings, sizeof(Settings));
  } else {
    settings.Show_clock=true;
  }
  */
  jobs_list_load(STORAGE_KEY_FIRST_JOB, stored_version);
  
  if (stored_version < STORAGE_KEY_VERSION) main_save_data();
}

// *****************************************************************************************************
// MAIN
// *****************************************************************************************************

void init(void) {
  main_load_data();
  bitmap_matrix=gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_MATRIX);
  bitmap_pause=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_PAUSE);
  bitmap_play=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_PLAY);
  bitmap_add=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_ADD);
  //bitmap_settings=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_SETTINGS);
  bitmap_delete=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_DELETE);
  bitmap_edit=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_EDIT);
  bitmap_adjust=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_ADJUST);
  bitmap_reset=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_RESET);
  bitmap_minus=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_MINUS);
  bitmap_tick=gbitmap_create_as_sub_bitmap(bitmap_matrix, ICON_RECT_TICK);
  main_menu_show();
}

void deinit(void) {
  main_menu_hide();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
