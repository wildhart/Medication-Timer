#include "main.h"

#include "pebble_process_info.h"
extern const PebbleProcessInfo __pbl_app_info;
#define APP_VERSION_LENGTH 10
char app_version[APP_VERSION_LENGTH];

GBitmap *bitmaps[N_BITMAPS][PBL_IF_SDK_3_ELSE(2,1)];

Settings settings={MODE_COUNT_UP, false /*alarm*/, true /*sort*/};
static bool JS_ready = false;
static bool data_loaded_from_watch = false;
static uint32_t data_timestamp = 0;
uint8_t stored_version=0;
bool export_after_save=false;
#ifdef PBL_SDK_3
  uint8_t timeline_settings=TIMELINE_FLAG_ON | TIMELINE_FLAG_NOTIFICATIONS;
  uint8_t quit_after_secs=0;
#endif

// *****************************************************************************************************
// MESSAGES
// *****************************************************************************************************

#define KEY_MEDICATIONS   100
#define KEY_CONFIG_DATA   0
#define KEY_MODE          1
#define KEY_ALARM         2
#define KEY_SORT          3
#define KEY_VERSION       4
#define KEY_APP_VERSION   5
#define KEY_EXPORT        6
#define KEY_TIMESTAMP     7
#define KEY_TIMELINE      8

static void send_settings_to_phone() {
  if (!JS_ready) return;
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  int dummy_int;
  
  dict_write_cstring(iter, KEY_APP_VERSION, app_version);
  dummy_int=CURRENT_STORAGE_VERSION;   dict_write_int(iter, KEY_VERSION, &dummy_int, sizeof(int), true);
  dummy_int=data_timestamp;          dict_write_int(iter, KEY_TIMESTAMP, &dummy_int, sizeof(int), true);
  dummy_int=settings.Mode;     dict_write_int(iter, KEY_MODE, &dummy_int, sizeof(int), true);
  dummy_int=settings.Alarm;    dict_write_int(iter, KEY_ALARM, &dummy_int, sizeof(int), true);
  dummy_int=settings.Sort;     dict_write_int(iter, KEY_SORT, &dummy_int, sizeof(int), true);
  #ifdef PBL_SDK_3
  dummy_int=timeline_settings; dict_write_int(iter, KEY_TIMELINE, &dummy_int, sizeof(int), true);
  #endif
  jobs_list_write_dict(iter, KEY_MEDICATIONS);

  if (export_after_save) {
    dummy_int=true;
    dict_write_int(iter, KEY_EXPORT, &dummy_int, sizeof(int), true);
    export_after_save=false;
  }

  dict_write_end(iter);
  
  //LOG("ended, dict_size=%d", (int) dict_size(iter));
  app_message_outbox_send();
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  LOG("Inbox received...");
  JS_ready = true;
  Tuple *tuple_t;
  
  bool new_data_from_config_page = dict_find(iter, KEY_CONFIG_DATA);
  tuple_t= dict_find(iter, KEY_TIMESTAMP);
  uint32_t inbox_timestamp = tuple_t ? tuple_t->value->int32 : 0;

  if (new_data_from_config_page || inbox_timestamp > data_timestamp)  {
    LOG("Loading settings from phone...");
    data_timestamp=inbox_timestamp;
    tuple_t=dict_find(iter, KEY_VERSION);  stored_version = (tuple_t) ? tuple_t->value->int32 : 1;
    tuple_t=dict_find(iter, KEY_MODE);   if (tuple_t) settings.Mode = tuple_t->value->int32;
    tuple_t=dict_find(iter, KEY_ALARM); if (tuple_t) settings.Alarm = tuple_t->value->int8 > 0; // convert int to boolean
    tuple_t=dict_find(iter, KEY_SORT); if (tuple_t) settings.Sort = tuple_t->value->int8 > 0; // convert int to boolean
    #ifdef PBL_SDK_3
    tuple_t=dict_find(iter, KEY_TIMELINE);   if (tuple_t) timeline_settings = tuple_t->value->int32;
    #endif
    jobs_delete_all_jobs();
    jobs_list_read_dict(iter, KEY_MEDICATIONS, stored_version);

    if (stored_version < CURRENT_STORAGE_VERSION) {
      update_show(stored_version);  
      stored_version = CURRENT_STORAGE_VERSION;
    }
    main_save_data();
    main_menu_highlight_top();
  } else if (inbox_timestamp < data_timestamp) {
    send_settings_to_phone();
  }
  
  LOG("Inbox processed.");
}

// *****************************************************************************************************
// WAKEUP
// *****************************************************************************************************

#define WAKEUP_REASON 0
#define WAKEUP_ID_KEY 0
static WakeupId wakeup_id;

void main_wakeup_set() {
  wakeup_cancel_all();
  if (!settings.Alarm || jobs_count==0) return;
  time_t next_alarm = jobs_get_next_wakeup_time();
  if (next_alarm) wakeup_id = wakeup_schedule(next_alarm, WAKEUP_REASON, true);
  //LOG("now=%ld, wakeup=%ld, diff=%ld", time(NULL), jobs_get_next_wakeup_time(), jobs_get_next_wakeup_time()-time(NULL));
}

// *****************************************************************************************************
// DATA STORAGE
// *****************************************************************************************************

void main_save_data(void) {
  data_loaded_from_watch = true;
  persist_write_int(STORAGE_KEY_VERSION, CURRENT_STORAGE_VERSION);
  data_timestamp=time(NULL);
  persist_write_int(STORAGE_KEY_TIMESTAMP, data_timestamp);
  #ifdef PBL_SDK_3
  persist_write_int(STORAGE_KEY_TIMELINE, timeline_settings);
  #endif
  persist_write_data(STORAGE_KEY_SETTINGS, &settings, sizeof(Settings));
  if (settings.Sort) jobs_list_sort();
  jobs_list_save(STORAGE_KEY_FIRST_MED);
  send_settings_to_phone();
  main_wakeup_set();
}

static void main_load_data(void) {
  stored_version = persist_read_int(STORAGE_KEY_VERSION); // defaults to 0 if key is missing
  
  if (stored_version) {
    data_loaded_from_watch = true;
    if (persist_exists(STORAGE_KEY_TIMESTAMP)) data_timestamp=persist_read_int(STORAGE_KEY_TIMESTAMP);
    #ifdef PBL_SDK_3
    timeline_settings = persist_exists(STORAGE_KEY_TIMELINE) ? persist_read_int(STORAGE_KEY_TIMELINE) : (TIMELINE_FLAG_ON | TIMELINE_FLAG_NOTIFICATIONS);
    #endif
    persist_read_data(STORAGE_KEY_SETTINGS, &settings, sizeof(Settings));
    jobs_list_load(STORAGE_KEY_FIRST_MED, stored_version);
    if (stored_version < CURRENT_STORAGE_VERSION)  {
      LOG("Saving data in new version");
      main_save_data();
    }
  }
}

// *****************************************************************************************************
// MAIN
// *****************************************************************************************************

void init(void) {
  //WARN("Ignoring saved data!!");
  
  snprintf(app_version,APP_VERSION_LENGTH,"%d.%d",__pbl_app_info.process_version.major, __pbl_app_info.process_version.minor);
  
  main_load_data();
  
  bitmaps[BITMAP_MATRIX][0]  =gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_MATRIX);
  bitmaps[BITMAP_ADD][0]     =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][0], ICON_RECT_ADD);
  bitmaps[BITMAP_MINUS][0]   =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][0], ICON_RECT_MINUS);
  bitmaps[BITMAP_SETTINGS][0]=gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][0], ICON_RECT_SETTINGS);
  bitmaps[BITMAP_DELETE][0]  =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][0], ICON_RECT_DELETE);
  bitmaps[BITMAP_EDIT][0]    =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][0], ICON_RECT_EDIT);
  bitmaps[BITMAP_ADJUST][0]  =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][0], ICON_RECT_ADJUST);
  bitmaps[BITMAP_RESET][0]    =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][0], ICON_RECT_RESET);
  bitmaps[BITMAP_PLAY][0]  =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][0], ICON_RECT_PLAY);
  bitmaps[BITMAP_TICK][0]    =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][0], ICON_RECT_TICK);
  #ifdef PBL_SDK_3
  // SDK 3 doesn't invert highligted menu icon, so need to use pre-inverted image...
  bitmaps[BITMAP_MATRIX][1]  =gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ICON_MATRIX_INV);
  bitmaps[BITMAP_ADD][1]     =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][1], ICON_RECT_ADD);
  bitmaps[BITMAP_MINUS][1]   =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][1], ICON_RECT_MINUS);
  bitmaps[BITMAP_SETTINGS][1]=gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][1], ICON_RECT_SETTINGS);
  bitmaps[BITMAP_DELETE][1]  =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][1], ICON_RECT_DELETE);
  bitmaps[BITMAP_EDIT][1]    =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][1], ICON_RECT_EDIT);
  bitmaps[BITMAP_ADJUST][1]  =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][1], ICON_RECT_ADJUST);
  bitmaps[BITMAP_RESET][1]    =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][1], ICON_RECT_RESET);
  bitmaps[BITMAP_PLAY][1]  =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][1], ICON_RECT_PLAY);
  bitmaps[BITMAP_TICK][1]    =gbitmap_create_as_sub_bitmap(bitmaps[BITMAP_MATRIX][1], ICON_RECT_TICK);
  #endif
  
  if (data_loaded_from_watch && stored_version < CURRENT_STORAGE_VERSION) update_show(stored_version);
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(636, 636); // should be enough for 23 meds: 78bytes+23meds*24bytes = 630
  
  #ifdef PBL_SDK_3
    if (launch_reason() == APP_LAUNCH_TIMELINE_ACTION) {
      uint8_t reason=launch_get_args();
      LOG("launch code: %d", reason);
      if (reason>=10) {
        // let list of med names, to find alphabetical order
        uint8_t names[MAX_JOBS];
        uint8_t temp;
        for (uint8_t a=0; a<jobs_count; a++) names[a]=a;
        // sort the array
        LOG("before: %s, %s, %s, %s",jobs[names[0]].Name,jobs[names[1]].Name,jobs[names[2]].Name,jobs[names[3]].Name);
        for (uint8_t a=0; a<jobs_count; a++) {
          for (uint8_t b=0; b<jobs_count; b++) {
            if (strcmp(jobs[names[a]].Name,jobs[names[b]].Name)<0) {
              // swap
              temp=names[a];
              names[a]=names[b];
              names[b]=temp;
            }
          }
        }
        LOG("sorted: %s, %s, %s, %s",jobs[names[0]].Name,jobs[names[1]].Name,jobs[names[2]].Name,jobs[names[3]].Name);
        
        jobs_reset_and_save(&names[reason-10]); // get index of med taken (in alphabetical order)
        quit_after_secs=3;
      }
    }
  #endif
  main_menu_show();
}

void deinit(void) {
  main_menu_hide();
  main_wakeup_set();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
