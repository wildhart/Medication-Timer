#include "main.h"

static Window *s_window;
static MenuLayer *s_menulayer;

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, false);
  #endif
  
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  
  // s_menulayer
  s_menulayer = menu_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  menu_layer_set_click_config_onto_window(s_menulayer, s_window);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_menulayer);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  menu_layer_destroy(s_menulayer);
}

void handle_ticktimer_tick(struct tm *tick_time, TimeUnits units_changed) {
  //LOG("tick timer, units changed=%d",(int) units_changed);
  if (units_changed & SECOND_UNIT)  {
    main_menu_update();
    job_menu_update();
    jobs_check_alarms();
  }
}

// *****************************************************************************************************
// MENU CALLBACKS
// *****************************************************************************************************

enum { // main menu structure
  MENU_SECTION_JOBS,
  MENU_SECTION_OTHER,
  MENU_SECTION_SETTINGS,
  
  NUM_MENU_SECTIONS,
  
  MENU_OTHER_ADD=MENU_SECTION_OTHER*100,
  NUM_MENU_ITEMS_OTHER=1,
  
  MENU_SETTINGS_MODE=MENU_SECTION_SETTINGS*100,
  MENU_SETTINGS_ALARM,
  MENU_SETTINGS_SORT,
  MENU_SETTINGS_CONFIG,
  NUM_MENU_ITEMS_SETTINGS=4
};

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return NUM_MENU_SECTIONS;
}

static uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  switch (section_index) {
    case MENU_SECTION_JOBS: return jobs_count;
    case MENU_SECTION_OTHER: return NUM_MENU_ITEMS_OTHER;
    case MENU_SECTION_SETTINGS: return NUM_MENU_ITEMS_SETTINGS;
    default:
      return 0;
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
  return (section_index==MENU_SECTION_SETTINGS) ? MENU_CELL_BASIC_HEADER_HEIGHT : 0;
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {
  // Determine which section we're working with
  switch (section_index) {
    case MENU_SECTION_JOBS:
    case MENU_SECTION_OTHER:
      break;
    case MENU_SECTION_SETTINGS:  menu_cell_basic_header_draw(ctx, cell_layer, "Options"); break;
  }
}

static int16_t menu_get_cell_height_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == MENU_SECTION_JOBS) {
    return MENU_HEIGHT_DOUBLE;
  }
  return MENU_HEIGHT_SINGLE;
}


void menu_cell_draw_job(GContext* ctx, const Layer *cell_layer, const uint8_t index) {
  GRect bounds = layer_get_frame(cell_layer);
    
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, jobs_get_job_name(index), FONT_GOTHIC_24_BOLD, GRect(4, -4, bounds.size.w-8, 4+18), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, jobs_get_job_clock_as_text(index), FONT_GOTHIC_18, GRect(4, 20, bounds.size.w-8, 14), GTextOverflowModeFill, GTextAlignmentRight, NULL);
  graphics_draw_text(ctx, jobs_get_job_repeat_as_text(index), FONT_GOTHIC_14, GRect(4, 20+4, bounds.size.w-8, 14), GTextOverflowModeFill, GTextAlignmentLeft, NULL);

  //graphics_draw_bitmap_in_rect(ctx, timer.Active && timer.Job==index ? bitmap_play : bitmap_pause, GRect(6, (bounds.size.h-16)/2, 16, 16));
}

void menu_cell_draw_other(GContext* ctx, const Layer *cell_layer, const char *title, const char *sub_title, GBitmap * icon) {
  GRect bounds = layer_get_frame(cell_layer);
    
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, title, FONT_GOTHIC_24_BOLD, GRect(28, -4, bounds.size.w-28, 4+18), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  if (sub_title) graphics_draw_text(ctx, sub_title, FONT_GOTHIC_18, GRect(28, 20, bounds.size.w-28-4, 14), GTextOverflowModeFill, GTextAlignmentLeft, NULL);

  if (icon) graphics_draw_bitmap_in_rect(ctx, icon, GRect(6,(bounds.size.h-16)/2, 16, 16));
}

static void menu_cell_draw_setting(GContext* ctx, const Layer *cell_layer, const char *title, const char *setting, const char *hint) {
  GRect bounds = layer_get_frame(cell_layer);
    
  graphics_context_set_text_color(ctx, GColorBlack);
  graphics_draw_text(ctx, title, FONT_GOTHIC_24_BOLD, GRect(4, -4, bounds.size.w-8, 4+18), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
  graphics_draw_text(ctx, setting, FONT_GOTHIC_18_BOLD, GRect(4, 2, bounds.size.w-8, 18), GTextOverflowModeFill, GTextAlignmentRight, NULL);
  graphics_draw_text(ctx, hint, FONT_GOTHIC_18, GRect(4, 20, bounds.size.w-8, 14), GTextOverflowModeFill, GTextAlignmentLeft, NULL);
}

static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
  char *mode[3]={"COUNT UP", "COUNT DOWN", "NEXT TIME"};
  switch (cell_index->section) {
    case MENU_SECTION_JOBS:
      menu_cell_draw_job(ctx, cell_layer, cell_index->row);
      break;

    default:
      switch (MENU_SECTION_CELL) {
        case MENU_OTHER_ADD: menu_cell_draw_other(ctx, cell_layer, "Add Medication", NULL, bitmap_add); break;
        case MENU_SETTINGS_MODE:
          menu_cell_draw_setting(ctx, cell_layer, "Mode", mode[settings.Mode],NULL);
          break;
        case MENU_SETTINGS_ALARM: menu_cell_draw_setting(ctx, cell_layer, "Alarm", settings.Alarm ? "YES" : "NO",NULL); break;
        case MENU_SETTINGS_SORT: menu_cell_draw_setting(ctx, cell_layer, "Sort", settings.Sort ? "YES" : "NO",NULL); break;
        case MENU_SETTINGS_CONFIG: menu_cell_draw_other(ctx, cell_layer, "Config/Donate", NULL , bitmap_settings); break;
      }
  }
}

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  switch (cell_index->section) {
    case MENU_SECTION_JOBS:
      job_menu_show(cell_index->row);
      break;
    default:
      switch (MENU_SECTION_CELL) {
        case MENU_OTHER_ADD: jobs_add_job(); break;
        case MENU_SETTINGS_MODE:
          settings.Mode = (settings.Mode + 1) % 3;
          menu_layer_reload_data(s_menulayer);
          main_save_data();
          break;
        case MENU_SETTINGS_ALARM:
          settings.Alarm = !settings.Alarm;
          menu_layer_reload_data(s_menulayer);
          main_save_data();
          break;
        case MENU_SETTINGS_SORT: 
          settings.Sort = !settings.Sort;
          main_save_data();
          menu_layer_reload_data(s_menulayer);
          break;
        case MENU_SETTINGS_CONFIG:
          export_after_save=true;
          main_save_data();
          break;
      }
  }
}

static void menu_select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if (cell_index->section == MENU_SECTION_JOBS) {
    uint8_t row=cell_index->row; 
    jobs_reset_and_save(&row); // pass a copy of the variable otherwise the selected menu option is updated when the medication moves
  }
}

// *****************************************************************************************************
// MAIN
// *****************************************************************************************************

static void handle_window_unload(Window* window) {
  destroy_ui();
  s_window=NULL;
}

void main_menu_update(void) {
  if (s_window) menu_layer_reload_data(s_menulayer);
}

void main_menu_show(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  // Set all the callbacks for the menu layer
  menu_layer_set_callbacks(s_menulayer, NULL, (MenuLayerCallbacks){
    .get_num_sections = menu_get_num_sections_callback,
    .get_num_rows = menu_get_num_rows_callback,
    .get_header_height = menu_get_header_height_callback,
    .get_cell_height = menu_get_cell_height_callback,
    .draw_header = menu_draw_header_callback,
    .draw_row = menu_draw_row_callback,
    .select_click = menu_select_callback,
    .select_long_click = menu_select_long_callback
  });
  window_stack_push(s_window, ANIMATED);
  tick_timer_service_subscribe(SECOND_UNIT, handle_ticktimer_tick);
}

void main_menu_hide(void) {
  window_stack_remove(s_window, ANIMATED);
}

void main_menu_highlight_top(void) {
  menu_layer_set_selected_index(s_menulayer, MenuIndex(0,0), MenuRowAlignTop, ANIMATED);
}