#include "main.h"

struct Job jobs[MAX_JOBS];
uint8_t jobs_count=0;

// *****************************************************************************************************
// JOB LIST FUNCTIONS
// *****************************************************************************************************

static void jobs_list_append_job(const char* name, time_t seconds, uint8_t repeat, int fixed) {
  if (jobs_count==MAX_JOBS) return;
  struct Job* new_job=&jobs[jobs_count++];
  strncpy(new_job->Name, name, JOB_NAME_SIZE)[JOB_NAME_SIZE-1]=0;
  new_job->Seconds = seconds;
  new_job->Repeat_hrs = repeat;
  new_job->Fixed = fixed ? true : false;
  LOG("appended job: %s, seconds=%ld, repeat=%d, fixed=%d", new_job->Name, new_job->Seconds, new_job->Repeat_hrs, new_job->Fixed);
}

void jobs_list_sort(void) {
  for (uint8_t before=0; before<jobs_count-1; before++) {
    time_t end_time = END_TIME(jobs[before]);
    uint8_t job_min=before;
    
    for (uint8_t loop=before+1; loop<jobs_count; loop++) {
      if (END_TIME(jobs[loop]) < end_time) {
        end_time = END_TIME(jobs[loop]);
        job_min = loop;
      }
    }
    
    if (job_min!=before) {
      // swap med
      struct Job temp_job = jobs[before];
      jobs[before] = jobs[job_min];
      jobs[job_min] = temp_job;
    }
  }
}

void jobs_list_save(uint8_t first_key) {
  for (uint8_t a=0; a<jobs_count; a++) persist_write_data(first_key++, &jobs[a], sizeof(jobs[0]));
  // if we've delete a job then need to delete the saved version or it will come back!
  while (persist_exists(first_key))  persist_delete(first_key++);
}

void jobs_list_write_dict(DictionaryIterator *iter, uint8_t first_key) {
  char buffer[JOB_NAME_SIZE+30];
  for (uint8_t a=0; a<jobs_count; a++) {
    snprintf(buffer,JOB_NAME_SIZE+30,"%s|%ld|%u|%d",jobs[a].Name, jobs[a].Seconds, jobs[a].Repeat_hrs, jobs[a].Fixed ? 1:0);
    dict_write_cstring(iter, first_key++, buffer);
  }
}

void jobs_list_read_dict(DictionaryIterator *iter, uint8_t first_key, const uint8_t version) {
  if (jobs_count) return;
  Tuple *tuple_t;
  uint8_t fields=4;
  char buffer[fields][JOB_NAME_SIZE];
  while ((tuple_t=dict_find(iter, first_key++))) {
    char *source = tuple_t->value->cstring;
    for (int c=0; c<fields; c++) {
      uint d=0; // destination offset
      while (*source && *source!='|' && d<JOB_NAME_SIZE) {
        buffer[c][d++]=*source++;
      }
      while (*source && *source!='|') source++;
      buffer[c][d]=0;
      source++;
    }
    jobs_list_append_job(buffer[0], atoi(buffer[1]), atoi(buffer[2]), atoi(buffer[3]));
  }
}

void jobs_list_load2(uint8_t first_key, const uint8_t version) {
  jobs_list_append_job("Paracetamol",time(NULL),0,0);
  jobs_list_append_job("Ibuprofin",time(NULL),0,0);
}

void jobs_list_load(uint8_t first_key, const uint8_t version) {
  LOG("key=%d, exists=%d", first_key, persist_exists(first_key));
  while (persist_exists(first_key)) {
    struct Job* new_job=&jobs[jobs_count++];
    persist_read_data(first_key++, new_job, sizeof(jobs[0]));
    if (version<2) new_job->Fixed = false; // this flag was not included before storage version 2
    LOG("loaded job: %s, seconds=%ld, repeat=%d, fixed=%d, version=%d", new_job->Name, new_job->Seconds, new_job->Repeat_hrs, new_job->Fixed, version);
  }
  LOG("Loaded %d jobs.",jobs_count);
}

void jobs_list_move_to_top_not_used(uint8_t index) {
  if (index==0 || index>=jobs_count) return;
  
  struct Job removed_job=jobs[index];
  while (index) {
    jobs[index]=jobs[index-1];
    index--;
  }
  jobs[0]=removed_job;
}

// *****************************************************************************************************
// PUBLIC FUNCTIONS
// *****************************************************************************************************

static uint8_t jobs_find_job_index_from_name(const char *name) {
  // get new index of job after sort
  uint8_t index=0;
  while (strcmp(jobs[index].Name,name)) index++;
  return index;
}

static void callback(const char* result, size_t result_length, void* extra) {
	// Do something with result
  int index = (int) extra;
  if (index==-1) {
    jobs_list_append_job(result, time(NULL), 0, 0);
  } else {
    strncpy(jobs[index].Name, result, JOB_NAME_SIZE)[JOB_NAME_SIZE-1]=0;
  }
  main_save_data();
  main_menu_update();
  if (index==-1) {
    job_menu_show(jobs_find_job_index_from_name(result));
    job_adjust_show();
  }
}

void jobs_add_job() {
  tertiary_text_prompt("New medication name?", callback, (void*) -1);
}

void jobs_rename_job(uint8_t index) {
  tertiary_text_prompt(jobs[index].Name, callback, (void*) (int) index);
}

void jobs_delete_all_jobs(void) {
  jobs_count=0;
}

void jobs_delete_job_and_save(uint8_t index) {
  jobs_count--;
  while (index<jobs_count) {
    jobs[index]=jobs[index+1];
    index++;
  }
  
  main_save_data();
  main_menu_update();
}

void jobs_set_job_repeat(uint8_t *index, uint8_t repeat, uint8_t fixed) {
  char name[JOB_NAME_SIZE];
  strncpy(name,jobs[*index].Name,JOB_NAME_SIZE);
  jobs[*index].Repeat_hrs=repeat;
  jobs[*index].Fixed = fixed ? true : false;
  main_save_data(); // this might resort the jobs.
  *index=jobs_find_job_index_from_name(name);
}

#define MAX_CLOCK_LENGTH 24
char clock_buffer[MAX_CLOCK_LENGTH];
char repeat_buffer[MAX_CLOCK_LENGTH];

char* jobs_get_job_clock_as_text(uint8_t index) {
  time_t seconds;
  
  switch (settings.Mode) {
    case MODE_COUNT_DOWN:
      seconds = END_TIME(jobs[index]) - time(NULL);
      bool minus = seconds < 0;
      if (minus) seconds = -seconds;
      snprintf(clock_buffer,MAX_CLOCK_LENGTH,"%s%ld:%02ld:%02ld",minus?"+":"-",(seconds/3600) /*hours*/,(seconds / 60) % 60 /*mins*/,seconds % 60 /*secs*/);
      break;
    case MODE_COUNT_UP: 
      seconds = time(NULL) - jobs[index].Seconds;
      snprintf(clock_buffer,MAX_CLOCK_LENGTH,"%ld:%02ld:%02ld",(seconds/3600) /*hours*/,(seconds / 60) % 60 /*mins*/,seconds % 60 /*secs*/);
      break;
    case MODE_NEXT_TIME:
      ; time_t next = END_TIME(jobs[index]);
      strftime(clock_buffer,MAX_CLOCK_LENGTH,clock_is_24h_style() ? "%H:%M" : "%I:%M %p",localtime(&next));
      break;
  }
  return clock_buffer;
}

char* jobs_get_job_repeat_as_text(uint8_t index) {
  snprintf(repeat_buffer,MAX_CLOCK_LENGTH,"Every %d%s hrs", jobs[index].Repeat_hrs, jobs[index].Fixed ? "" : "+");
  return repeat_buffer;
}

void jobs_reset_and_save(uint8_t *index) {
  char name[JOB_NAME_SIZE];
  strncpy(name,jobs[*index].Name,JOB_NAME_SIZE);
  
  if (jobs[*index].Fixed) {
    jobs[*index].Seconds+=jobs[*index].Repeat_hrs*3600; 
  } else {
    jobs[*index].Seconds=time(NULL);
  }
  main_save_data(); // this might resort the jobs.
  *index=jobs_find_job_index_from_name(name);
}

void jobs_add_minutes(uint8_t *index, int minutes) {
  jobs[*index].Seconds += 60*minutes;
}

time_t jobs_get_next_wakeup_time(void) {
  time_t min_time = 0;
  for (uint8_t a=0; a<jobs_count; a++) {
    if (jobs[a].Repeat_hrs && END_TIME(jobs[a]) > time(NULL) && (min_time==0 || END_TIME(jobs[a])<min_time)) min_time = END_TIME(jobs[a]);
  }
  //LOG("next wakeup: %ld,  now: %ld",min_time,time(NULL));
  return min_time;
}

static void vibrate(void) {
  // Vibe pattern: ON for 200ms, OFF for 100ms, ON for 400ms:
  LOG("VIBRATE!!!");
  static const uint32_t segments[] = { 400,200, 400,200, 400,200, 400,200, 400 };
  VibePattern pat = {
    .durations = segments,
    .num_segments = ARRAY_LENGTH(segments),
  };
  vibes_enqueue_custom_pattern(pat);
}

static uint8_t jobs_alarm_count = 0;

void jobs_check_alarms(void) {
  if (!jobs_count || !settings.Alarm) return;
  uint8_t new_alarm_count=0;
  for (uint8_t a=0; a<jobs_count; a++) {
    if (jobs[a].Repeat_hrs && END_TIME(jobs[a]) <= time(NULL)) new_alarm_count++;
  }
  if (new_alarm_count>jobs_alarm_count) vibrate();
  jobs_alarm_count=new_alarm_count;
}