#include "main.h"

#define N_LAYERS 2
static uint8_t active_layer;
static uint16_t limits[N_LAYERS]={25,2};
static uint16_t values[N_LAYERS];
static const char* values_fixed[2]={"Flexible","Fixed"};
static const char* help_fixed[2]={"Timer runs from when drug WAS actually taken","Timer runs from when drug SHOULD have been taken"};
#define NUM_BUFFER_LENGTH 3
static char num_buffer[NUM_BUFFER_LENGTH];

static Window *s_window;
static ActionBarLayer *s_actionbarlayer;
static TextLayer *s_textlayer_name;
static TextLayer *s_textlayer_repeat;
static TextLayer *s_textlayer_hrs;
static TextLayer *s_textlayer_note;
static TextLayer *layers[N_LAYERS];

static void set_layer_text() {
  for (uint8_t l=0; l<N_LAYERS; l++) {
    text_layer_set_background_color(layers[l], (l==active_layer) ? GColorBlack : GColorWhite);
    text_layer_set_text_color(layers[l], (l==active_layer) ? GColorWhite : GColorBlack);
    if (l==0) {
      snprintf(num_buffer,NUM_BUFFER_LENGTH,"%d",values[0]);
      text_layer_set_text(layers[l], num_buffer);
    } else {
      text_layer_set_text(layers[l], values_fixed[values[1]]);
      text_layer_set_text(s_textlayer_note, help_fixed[values[1]]);
    }
  }
}

static void action_bar_up_click_handler() {
  values[active_layer]=(values[active_layer]+1) % limits[active_layer];
  set_layer_text(active_layer);
}

static void action_bar_down_click_handler() {
  values[active_layer]=(values[active_layer]+limits[active_layer]-1) % limits[active_layer];
  set_layer_text(active_layer);
}

static void action_bar_select_click_handler() {
  if (active_layer==N_LAYERS-1) {
    jobs_set_job_repeat(&job_index,values[0], values[1]); // this updates job_index if it gets moved during sort
    job_adjust_hide();
    return;
  } else {
    active_layer++;
    set_layer_text();
    action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_SELECT, bitmaps[BITMAP_TICK][PBL_IF_SDK_3_ELSE(1,0)]);
  }
}

static void action_bar_click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, (ClickHandler) action_bar_up_click_handler);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, (ClickHandler) action_bar_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) action_bar_select_click_handler);
}

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, false);
  #endif
  
  // s_textlayer_name
  s_textlayer_name = text_layer_create(GRect(5, 7, 100, 30));
  text_layer_set_font(s_textlayer_name, FONT_GOTHIC_24_BOLD);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_name);
  
  // s_textlayer_repeat
  s_textlayer_repeat = text_layer_create(GRect(5+PBL_IF_ROUND_ELSE(20,0), 36+PBL_IF_ROUND_ELSE(16,0), 100, 30));
  text_layer_set_font(s_textlayer_repeat, FONT_GOTHIC_18);
  text_layer_set_text(s_textlayer_repeat, "Repeat Every");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_repeat);
  
  // hours number
  layers[0] = text_layer_create(GRect(20-14+PBL_IF_ROUND_ELSE(20,0), 60+PBL_IF_ROUND_ELSE(16,0), 20, 24));
  text_layer_set_text_alignment(layers[0], GTextAlignmentCenter);
  text_layer_set_font(layers[0], FONT_GOTHIC_18);
  layer_add_child(window_get_root_layer(s_window), (Layer *)layers[0]);
  
  // s_textlayer_hrs
  s_textlayer_hrs = text_layer_create(GRect(20+20+2-14+PBL_IF_ROUND_ELSE(20,0), 60+PBL_IF_ROUND_ELSE(16,0), 30, 24));
  text_layer_set_font(s_textlayer_hrs, FONT_GOTHIC_18);
  text_layer_set_text(s_textlayer_hrs, "hrs");
  text_layer_set_text_alignment(s_textlayer_hrs, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_hrs);
  
  // s_textlayer_fixed
  layers[1] = text_layer_create(GRect(66-14+PBL_IF_ROUND_ELSE(20,0), 60+PBL_IF_ROUND_ELSE(16,0), 50, 24));
  text_layer_set_text_alignment(layers[1], GTextAlignmentCenter);
  text_layer_set_font(layers[1], FONT_GOTHIC_18);
  layer_add_child(window_get_root_layer(s_window), (Layer *)layers[1]);
  
  // s_textlayer_note
  s_textlayer_note = text_layer_create(GRect(15-2+PBL_IF_ROUND_ELSE(20,0), 94+PBL_IF_ROUND_ELSE(16,0), 100, 45));
  text_layer_set_font(s_textlayer_note, FONT_GOTHIC_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_note);
  
  // s_actionbarlayer
  s_actionbarlayer = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actionbarlayer, s_window);
  action_bar_layer_set_background_color(s_actionbarlayer, GColorBlack);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_UP, bitmaps[BITMAP_ADD][PBL_IF_SDK_3_ELSE(1,0)]);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_DOWN, bitmaps[BITMAP_MINUS][PBL_IF_SDK_3_ELSE(1,0)]);
  action_bar_layer_set_click_config_provider(s_actionbarlayer, action_bar_click_config_provider);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_actionbarlayer);
  
  #ifdef PBL_ROUND
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  layer_set_frame((Layer *)s_textlayer_name, GRect(0,20,bounds.size.w,30));
  text_layer_set_text_alignment(s_textlayer_name, GTextAlignmentCenter);
  #endif
}

static void destroy_ui(void) {
  window_destroy(s_window);
  action_bar_layer_destroy(s_actionbarlayer);
  text_layer_destroy(s_textlayer_name);
  text_layer_destroy(s_textlayer_repeat);
  text_layer_destroy(s_textlayer_hrs);
  text_layer_destroy(s_textlayer_note);
  
  for (uint8_t l=0; l<N_LAYERS; l++) text_layer_destroy(layers[l]);
}

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void job_adjust_show() {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  text_layer_set_text(s_textlayer_name, jobs[job_index].Name);
  active_layer=0;
  values[0]=jobs[job_index].Repeat_hrs;
  values[1]=jobs[job_index].Fixed;
  set_layer_text(active_layer);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_SELECT, bitmaps[BITMAP_PLAY][PBL_IF_SDK_3_ELSE(1,0)]);
  
  window_stack_push(s_window, true);
}

void job_adjust_hide(void) {
  window_stack_remove(s_window, true);
}
