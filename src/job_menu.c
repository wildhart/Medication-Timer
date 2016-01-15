#include "main.h"

uint8_t job_index;
bool job_changed;

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static MenuLayer *s_menulayer;

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, false);
  #endif
  
  // s_menulayer
  s_menulayer = menu_layer_create(GRect(0, 0, 144, 152));
  menu_layer_set_click_config_onto_window(s_menulayer, s_window);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_menulayer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  menu_layer_destroy(s_menulayer);
}
// END AUTO-GENERATED UI CODE

// *****************************************************************************************************
// MENU CALLBACKS
// *****************************************************************************************************

enum { // main menu structure
  MENU_JOB,
  MENU_ADD10,
  MENU_SUB10,
  MENU_RESET,
  MENU_RENAME,
  MENU_ADJUST,
  MENU_DELETE,
  
  NUM_MENU_ROWS
};

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  return NUM_MENU_ROWS;
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  return (cell_index->row == MENU_JOB) ? MENU_HEIGHT_DOUBLE : MENU_HEIGHT_SINGLE;
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case MENU_JOB: menu_cell_draw_job(ctx, cell_layer, job_index); break;
    case MENU_RESET: menu_cell_draw_other(ctx, cell_layer, "Reset Timer", NULL, bitmap_reset); break;
    case MENU_ADD10: menu_cell_draw_other(ctx, cell_layer, "Add 10 Minutes", NULL, bitmap_add); break;
    case MENU_SUB10: menu_cell_draw_other(ctx, cell_layer, "Sub 10 Minutes", NULL, bitmap_minus); break;
    case MENU_RENAME: menu_cell_draw_other(ctx, cell_layer, "Rename", NULL, bitmap_edit); break;
    case MENU_ADJUST: menu_cell_draw_other(ctx, cell_layer, "Set Repeat", NULL, bitmap_adjust); break;
    case MENU_DELETE: menu_cell_draw_other(ctx, cell_layer, "Delete", NULL, bitmap_delete); break;
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case MENU_RESET: 
      jobs_reset_and_save(&job_index);
      job_menu_hide();
      break;
    case MENU_ADD10:
      jobs_add_minutes(&job_index, (settings.Mode==MODE_NEXT_TIME) ? 10:-10); // this updates job_index incase it gets sorted
      job_changed=true;
      menu_layer_reload_data(s_menulayer);
      break;
    case MENU_SUB10:
      jobs_add_minutes(&job_index, (settings.Mode==MODE_NEXT_TIME) ? -10:10); // this updates job_index incase it gets sorted
      job_changed=true;
      menu_layer_reload_data(s_menulayer);
      break;
    case MENU_RENAME: jobs_rename_job(job_index); break;
    case MENU_ADJUST: job_adjust_show(); break;
    case MENU_DELETE: 
      jobs_delete_job_and_save(job_index);
      job_menu_hide();
      break;
  }
}

static void menu_select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->row) {
    case MENU_ADD10:
      jobs_add_minutes(&job_index, (settings.Mode==MODE_NEXT_TIME) ? 1:-1); // this updates job_index incase it gets sorted
      job_changed=true;
      menu_layer_reload_data(s_menulayer);
      break;
    case MENU_SUB10:
      jobs_add_minutes(&job_index, (settings.Mode==MODE_NEXT_TIME) ? -1:1); // this updates job_index incase it gets sorted
      job_changed=true;
      menu_layer_reload_data(s_menulayer);
      break;
  }
}

// *****************************************************************************************************
// MAIN
// *****************************************************************************************************

static void handle_window_unload(Window* window) {
  destroy_ui();
  s_window=NULL;
  if (job_changed) {
    main_save_data();
  }
}

void job_menu_show(uint8_t index) {
  job_index=index;
  job_changed=false;
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(s_menulayer, NULL, (MenuLayerCallbacks){
    .get_num_sections = NULL, //menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = NULL, //menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = NULL, //menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
    .select_long_click = menu_select_long_callback
  });
  window_stack_push(s_window, true);
}

void job_menu_hide(void) {
  window_stack_remove(s_window, true);
}

void job_menu_update(void) {
  if (s_window) menu_layer_reload_data(s_menulayer);
}
