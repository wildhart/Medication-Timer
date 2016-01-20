
#include "main.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_gothic_28_bold;
static GFont s_res_gothic_14;
static TextLayer *s_textlayer_heading;
static TextLayer *s_textlayer_features;

static void initialise_ui(void) {
  s_window = window_create();
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, false);
  #endif
  
  s_res_gothic_28_bold = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  s_res_gothic_14 = fonts_get_system_font(FONT_KEY_GOTHIC_18);
  // s_textlayer_heading
  s_textlayer_heading = text_layer_create(GRect(2, -1, 140, 28));
  text_layer_set_text(s_textlayer_heading, "New Feature:");
  text_layer_set_font(s_textlayer_heading, s_res_gothic_28_bold);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_heading);
  
  // s_textlayer_features
  s_textlayer_features = text_layer_create(GRect(2, 27, 140, 125));
  text_layer_set_text(s_textlayer_features, "* Configuration page for easier medication entry.\n\n * DONATIONS can be accepted from the config page!");
  text_layer_set_font(s_textlayer_features, s_res_gothic_14);
  layer_add_child(window_get_root_layer(s_window), (Layer *)s_textlayer_features);
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
