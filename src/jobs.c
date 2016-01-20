#include "main.h"

#define JOB_NAME_LENGTH 24
typedef struct {
  char Name[JOB_NAME_LENGTH];
  time_t Seconds;
  uint8_t Repeat_hrs;
  bool Fixed;
} Job;

typedef struct Job_ptr {
  Job* Job;
  struct Job_ptr* Next_ptr;
} Job_ptr ;

static Job_ptr* first_job_ptr=NULL;
uint8_t jobs_count=0;

// *****************************************************************************************************
// JOB LIST FUNCTIONS
// *****************************************************************************************************

static void jobs_list_append_job(const char* name, time_t seconds, uint8_t repeat, int fixed) {
  Job* new_job = malloc(sizeof(Job));
  Job_ptr* new_job_ptr = malloc(sizeof(Job_ptr));
  
  new_job_ptr->Job = new_job;
  new_job_ptr->Next_ptr = NULL;
  strncpy(new_job->Name, name, JOB_NAME_LENGTH);
  new_job->Seconds = seconds;
  new_job->Repeat_hrs = repeat;
  new_job->Fixed = fixed ? true : false;
    
  if (first_job_ptr) {
    Job_ptr* last_job_ptr = first_job_ptr;
    while (last_job_ptr->Next_ptr) last_job_ptr=last_job_ptr->Next_ptr;
    last_job_ptr->Next_ptr = new_job_ptr;
  } else {
    first_job_ptr = new_job_ptr;
  }
  LOG("appended job: %s, seconds=%ld, repeat=%d, fixed=%d", new_job->Name, new_job->Seconds, new_job->Repeat_hrs, new_job->Fixed);
  jobs_count++;
}

void jobs_list_sort(void) {
  time_t end_time;
  
  Job_ptr* job_ptr_before = first_job_ptr;
  while (job_ptr_before && job_ptr_before->Next_ptr) {
    end_time = END_TIME(job_ptr_before->Job);
    Job_ptr* job_ptr_min = job_ptr_before;
    
    Job_ptr* job_ptr_loop = job_ptr_before->Next_ptr;
    while (job_ptr_loop) {
      if (END_TIME(job_ptr_loop->Job) < end_time) {
        end_time = END_TIME(job_ptr_loop->Job);
        job_ptr_min = job_ptr_loop;
      }
      job_ptr_loop = job_ptr_loop->Next_ptr;
    }
    
    if (job_ptr_min != job_ptr_before) {
      // swap med
      Job* temp_job = job_ptr_before->Job;
      job_ptr_before->Job = job_ptr_min->Job;
      job_ptr_min->Job = temp_job;
    }
    
    job_ptr_before = job_ptr_before->Next_ptr;
  }
}

void jobs_list_save(uint8_t first_key) {
  Job_ptr* job_ptr = first_job_ptr;
  while (job_ptr) {
    persist_write_data(first_key++, job_ptr->Job, sizeof(Job));
    job_ptr=job_ptr->Next_ptr;
  }
  // if we've delete a job then need to delete the saved version or it will come back!
  persist_delete(first_key);
}

void jobs_list_write_dict(DictionaryIterator *iter, uint8_t first_key) {
  Job_ptr* job_ptr = first_job_ptr;
  Job * job;
  char buffer[JOB_NAME_LENGTH+30];
  while (job_ptr) {
    job=job_ptr->Job;
    snprintf(buffer,JOB_NAME_LENGTH+30,"%s|%ld|%u|%d",job->Name, job->Seconds, job->Repeat_hrs, job->Fixed ? 1:0);
    dict_write_cstring(iter, first_key++, buffer);
    job_ptr=job_ptr->Next_ptr;
  }
}

void jobs_list_read_dict(DictionaryIterator *iter, uint8_t first_key, const uint8_t version) {
  if (first_job_ptr!=NULL) return;
  
  Tuple *tuple_t;
  uint8_t fields=4;
  char buffer[fields][JOB_NAME_LENGTH];
  
  while ((tuple_t=dict_find(iter, first_key++))) {
    char *source = tuple_t->value->cstring;
    for (int c=0; c<fields; c++) {
      uint d=0; // destination offset
      while (*source && *source!='|' && d<JOB_NAME_LENGTH) {
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
  Job* new_job;
  Job_ptr* new_job_ptr;
  Job_ptr* prev_job_ptr=NULL;
  LOG("key=%d, exists=%d", first_key, persist_exists(first_key));
  while (persist_exists(first_key)) {
    new_job = malloc(sizeof(Job));
    persist_read_data(first_key, new_job, sizeof(Job));
    if (version<2) new_job->Fixed = false; // this flag was not included before storage version 2
    
    LOG("loaded job: %s, seconds=%ld, repeat=%d, fixed=%d, version=%d", new_job->Name, new_job->Seconds, new_job->Repeat_hrs, new_job->Fixed, version);
    new_job_ptr = malloc(sizeof(Job_ptr));
    new_job_ptr->Job = new_job;
    new_job_ptr->Next_ptr = NULL;
    if (prev_job_ptr) prev_job_ptr->Next_ptr = new_job_ptr;
    prev_job_ptr = new_job_ptr;
    if (NULL==first_job_ptr) first_job_ptr = new_job_ptr;
    jobs_count++;
    first_key++;
  }
  LOG("Loaded %d jobs.",jobs_count);
}

Job* jobs_list_get_index(uint8_t index) {
  if (index>=jobs_count) return NULL;
  Job_ptr* job_ptr = first_job_ptr;
  while (index--) job_ptr=job_ptr->Next_ptr;
  return job_ptr->Job;
}

void jobs_list_move_to_top(uint8_t index) {
  if (index==0 || index>=jobs_count) return;
  Job_ptr* job_ptr = first_job_ptr;
  Job_ptr* prev_job_ptr=NULL;
  while (index--) {
    prev_job_ptr=job_ptr;
    job_ptr=job_ptr->Next_ptr;
  }
  // remove job_ptr from list
  prev_job_ptr->Next_ptr = job_ptr->Next_ptr;
  // Insert before first_job_ptr
  job_ptr->Next_ptr = first_job_ptr;
  first_job_ptr=job_ptr;
}

// *****************************************************************************************************
// PUBLIC FUNCTIONS
// *****************************************************************************************************

static void callback(const char* result, size_t result_length, void* extra) {
	// Do something with result
  int index = (int) extra;
  if (index==-1) {
    jobs_list_append_job(result, time(NULL), 0, 0);  
  } else {
    snprintf(jobs_list_get_index(index)->Name,JOB_NAME_LENGTH, result);
  }
  main_save_data();
  main_menu_update();
}

void jobs_add_job() {
  tertiary_text_prompt("New medication name?", callback, (void*) -1);
}

void jobs_rename_job(uint8_t index) {
  tertiary_text_prompt(jobs_get_job_name(index), callback, (void*) (int) index);
}

void jobs_delete_all_jobs(void) {
  Job_ptr* job_ptr = first_job_ptr;
  while (first_job_ptr) {
    Job_ptr * next_job=first_job_ptr->Next_ptr;
    free(first_job_ptr->Job);
    free(first_job_ptr);
    first_job_ptr=next_job;
  }
  jobs_count=0;
}

void jobs_delete_job_and_save(uint8_t index) {
  if (index>=jobs_count) return;
  
  Job_ptr* job_ptr = first_job_ptr;
  
  if (index) {
    Job_ptr* prev_job_ptr = NULL;
    while (index--) {
      prev_job_ptr=job_ptr;
      job_ptr=job_ptr->Next_ptr;
    }
    prev_job_ptr->Next_ptr = job_ptr->Next_ptr;
  } else {
    first_job_ptr = job_ptr->Next_ptr;
  }
  free(job_ptr->Job);
  free(job_ptr);
  
  jobs_count--;
  main_save_data();
  main_menu_update();
}

char* jobs_get_job_name(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  return (job) ? job->Name : NULL;
}

uint32_t jobs_get_job_seconds(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  return (job) ? time(NULL) - job->Seconds : 0;
}

uint8_t jobs_get_job_repeat(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  return (job) ? job->Repeat_hrs : 0;
}

bool jobs_get_job_fixed(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  return (job) ? job->Fixed : 0;
}
static void jobs_update_job_index(Job* job, uint8_t *index) {
  // check if job was moved during sort
  if (job==jobs_list_get_index(*index)) return;
  // get new index
  Job_ptr* job_ptr = first_job_ptr;
  *index=0;
  while (job_ptr->Job != job) {
    (*index)++;
    job_ptr=job_ptr->Next_ptr;
  }
}

void jobs_set_job_repeat(uint8_t *index, uint8_t repeat, uint8_t fixed) {
  Job* job=jobs_list_get_index(*index);
  if (job) {
    job->Repeat_hrs=repeat;
    job->Fixed = fixed ? true : false;
    main_save_data();
    jobs_update_job_index(job, index);
  }
}

#define MAX_CLOCK_LENGTH 24
char clock_buffer[MAX_CLOCK_LENGTH];
char repeat_buffer[MAX_CLOCK_LENGTH];

char* jobs_get_job_clock_as_text(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  time_t seconds;
  
  switch (settings.Mode) {
    case MODE_COUNT_DOWN:
      seconds = job->Seconds + job->Repeat_hrs*3600 - time(NULL);
      bool minus = seconds < 0;
      if (minus) seconds = -seconds;
      snprintf(clock_buffer,MAX_CLOCK_LENGTH,"%s%ld:%02ld:%02ld",minus?"+":"-",(seconds/3600) /*hours*/,(seconds / 60) % 60 /*mins*/,seconds % 60 /*secs*/);
      break;
    case MODE_COUNT_UP: 
      seconds = time(NULL) - job->Seconds;
      snprintf(clock_buffer,MAX_CLOCK_LENGTH,"%ld:%02ld:%02ld",(seconds/3600) /*hours*/,(seconds / 60) % 60 /*mins*/,seconds % 60 /*secs*/);
      break;
    case MODE_NEXT_TIME:
      ; time_t next = END_TIME(job);
      strftime(clock_buffer,MAX_CLOCK_LENGTH,clock_is_24h_style() ? "%H:%M" : "%I:%M %p",localtime(&next));
      break;
  }
  return clock_buffer;
}

char* jobs_get_job_repeat_as_text(uint8_t index) {
  Job* job=jobs_list_get_index(index);
  
  snprintf(repeat_buffer,MAX_CLOCK_LENGTH,"Every %d%s hrs", job->Repeat_hrs, job->Fixed ? "" : "+");
  return repeat_buffer;
}

void jobs_reset_and_save(uint8_t *index) {
  Job* job=jobs_list_get_index(*index);
  if (job->Fixed) {
    job->Seconds+=job->Repeat_hrs*3600; 
  } else {
    job->Seconds=time(NULL);
  }
  main_save_data();
  jobs_update_job_index(job, index);
}

void jobs_add_minutes(uint8_t *index, int minutes) {
  Job* job=jobs_list_get_index(*index);
  int seconds = (int) job->Seconds;
  if (seconds + 60*minutes < time(NULL)) {
    seconds += 60*minutes;
  } else {
    seconds = time(NULL);
  }
  job->Seconds = seconds;
}

time_t jobs_get_next_wakeup_time(void) {
  Job_ptr* job_ptr = first_job_ptr;
  time_t min_time = (END_TIME(job_ptr->Job) > time(NULL)) ? END_TIME(job_ptr->Job) : 0;
  
  while (job_ptr) {
    if (job_ptr->Job->Repeat_hrs && END_TIME(job_ptr->Job) > time(NULL) && (min_time==0 || END_TIME(job_ptr->Job)<min_time)) min_time = END_TIME(job_ptr->Job);
    job_ptr = job_ptr->Next_ptr;
  }
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
  Job_ptr* job_ptr = first_job_ptr;
  while (job_ptr) {
    if (job_ptr->Job->Repeat_hrs && END_TIME(job_ptr->Job) <= time(NULL)) new_alarm_count++;
    job_ptr=job_ptr->Next_ptr;
  }
  if (new_alarm_count>jobs_alarm_count) vibrate();
  jobs_alarm_count=new_alarm_count;
}