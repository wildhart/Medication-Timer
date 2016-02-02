
#include "main.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static TextLayer *s_textlayer_heading;
static TextLayer *s_textlayer_features;

#define MARGIN PBL_IF_ROUND_ELSE(12,0)
#define HEIGHT PBL_IF_ROUND_ELSE(56,32)

void select_button_handler(ClickRecognizerRef recognizer, void *context) {
  update_hide();
}

void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_button_handler);
}

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, false);
  #endif
  
  GRect bounds = layer_get_bounds(window_get_root_layer(s_window));
  
  // s_textlayer_heading
  s_textlayer_heading = text_layer_create(GRect(0, -1, bounds.size.w, HEIGHT+MARGIN));
  text_layer_set_text(s_textlayer_heading, PBL_IF_ROUND_ELSE("New\nFeatures:","New Features:"));
  text_layer_set_background_color(s_textlayer_heading,GColorBlack);
  text_layer_set_text_alignment(s_textlayer_heading, GTextAlignmentCenter);
  text_layer_set_text_color(s_textlayer_heading,GColorWhite);
  text_layer_set_font(s_textlayer_heading, FONT_GOTHIC_28_BOLD);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_heading);
  
  // s_textlayer_features
  s_textlayer_features = text_layer_create(GRect(2+MARGIN, HEIGHT+MARGIN, bounds.size.w-2*(2+MARGIN), bounds.size.h-HEIGHT-MARGIN));
  text_layer_set_text(s_textlayer_features, "* Medications now appear on the timeline\n\n* Optimised for Pebble Time Round");
  text_layer_set_font(s_textlayer_features, FONT_GOTHIC_18);
  #ifdef PBL_ROUND
  text_layer_set_text_alignment(s_textlayer_features, GTextAlignmentCenter);
  #endif
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_features);
  window_set_click_config_provider(s_window,(void*)click_config_provider);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(s_textlayer_heading);
  text_layer_destroy(s_textlayer_features);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void update_show(const uint8_t old_version) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, ANIMATED);
}

void update_hide(void) {
  window_stack_remove(s_window, ANIMATED);
}
