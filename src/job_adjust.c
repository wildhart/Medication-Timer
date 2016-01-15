#include "main.h"

#define N_LAYERS 1
#define BUFFER_LENGTH 6
static uint8_t active_layer;
static uint16_t limits[N_LAYERS]={25};
static uint16_t values[N_LAYERS];
static const char* formats[N_LAYERS]={"%d"};
static char buffers[N_LAYERS][BUFFER_LENGTH];

static Window *s_window;
static GFont s_res_gothic_24_bold;
static GFont s_res_gothic_18;
static GFont s_res_gothic_14;
static ActionBarLayer *s_actionbarlayer;
static TextLayer *s_textlayer_name;
static TextLayer *s_textlayer_hrs;
static TextLayer *layers[N_LAYERS];

static void action_bar_up_click_handler() {
  values[active_layer]=(values[active_layer]+1) % limits[active_layer];
  snprintf(buffers[active_layer],BUFFER_LENGTH,formats[active_layer],values[active_layer]);
  text_layer_set_text(layers[active_layer], buffers[active_layer]);
}

static void action_bar_down_click_handler() {
  values[active_layer]=(values[active_layer]+limits[active_layer]-1) % limits[active_layer];
  snprintf(buffers[active_layer],BUFFER_LENGTH,formats[active_layer],values[active_layer]);
  text_layer_set_text(layers[active_layer], buffers[active_layer]);
}

static void action_bar_select_click_handler() {
  if (active_layer==N_LAYERS-1) {
    jobs_set_job_repeat(&job_index,values[0]); // this updates job_index if it gets moved during sort
    job_adjust_hide();
    return;
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
  
  s_res_gothic_24_bold = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  s_res_gothic_18 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_14);
  // s_actionbarlayer
  s_actionbarlayer = action_bar_layer_create();
  action_bar_layer_add_to_window(s_actionbarlayer, s_window);
  action_bar_layer_set_background_color(s_actionbarlayer, GColorBlack);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_UP, bitmap_add);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_SELECT, bitmap_tick);
  action_bar_layer_set_icon(s_actionbarlayer, BUTTON_ID_DOWN, bitmap_minus);
  action_bar_layer_set_click_config_provider(s_actionbarlayer, action_bar_click_config_provider);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_actionbarlayer);
  
  // s_textlayer_name
  s_textlayer_name = text_layer_create(GRect(5, 7, 100, 24));
  text_layer_set_font(s_textlayer_name, s_res_gothic_24_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_name);
  
  // s_textlayer_hrs
  s_textlayer_hrs = text_layer_create(GRect(30+20+4, 60, 30, 24));
  text_layer_set_font(s_textlayer_hrs, s_res_gothic_18);
  text_layer_set_text(s_textlayer_hrs, "hrs");
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_hrs);
  
  for (uint8_t l=0; l<N_LAYERS; l++) {
    layers[l] = text_layer_create(GRect(30+20*l, 60, 20, 24));
    text_layer_set_text_alignment(layers[l], GTextAlignmentCenter);
    text_layer_set_font(layers[l], s_res_gothic_18);
    layer_add_child(window_get_root_layer(s_window), (Layer *)layers[l]);
  }
}

static void destroy_ui(void) {
  window_destroy(s_window);
  action_bar_layer_destroy(s_actionbarlayer);
  text_layer_destroy(s_textlayer_name);
  
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
  text_layer_set_text(s_textlayer_name, jobs_get_job_name(job_index));
  active_layer=0;
  values[0]=jobs_get_job_repeat(job_index);
  for (uint8_t l=0; l<N_LAYERS; l++) {
    text_layer_set_background_color(layers[l], (l==active_layer) ? GColorBlack : GColorWhite);
    text_layer_set_text_color(layers[l], (l==active_layer) ? GColorWhite : GColorBlack);
    snprintf(buffers[l],BUFFER_LENGTH,formats[l],values[l]);
    text_layer_set_text(layers[l], buffers[l]);
  }
  
  window_stack_push(s_window, true);
}

void job_adjust_hide(void) {
  window_stack_remove(s_window, true);
}
