#include "main.h"

#define JOB_NAME_LENGTH 24
typedef struct {
  char Name[JOB_NAME_LENGTH];
  uint32_t Seconds;
  uint8_t Repeat_hrs;
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

static void jobs_list_append_job(const char* name) {
  Job* new_job = malloc(sizeof(Job));
  Job_ptr* new_job_ptr = malloc(sizeof(Job_ptr));
  
  new_job_ptr->Job = new_job;
  new_job_ptr->Next_ptr = NULL;
  strncpy(new_job->Name, name, JOB_NAME_LENGTH);
  new_job->Seconds = time(NULL);
  new_job->Repeat_hrs = 0;
    
  if (first_job_ptr) {
    Job_ptr* last_job_ptr = first_job_ptr;
    while (last_job_ptr->Next_ptr) last_job_ptr=last_job_ptr->Next_ptr;
    last_job_ptr->Next_ptr = new_job_ptr;
  } else {
    first_job_ptr = new_job_ptr;
  }
  jobs_count++;
  main_save_data();
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

void jobs_list_load2(uint8_t first_key, const uint8_t version) {
  jobs_list_append_job("Paracetamol");
  jobs_list_append_job("Ibuprofin");
}

void jobs_list_load(uint8_t first_key, const uint8_t version) {
  Job* new_job;
  Job_ptr* new_job_ptr;
  Job_ptr* prev_job_ptr=NULL;
  while (persist_exists(first_key)) {
    new_job = malloc(sizeof(Job));
    persist_read_data(first_key, new_job, sizeof(Job));
    
    new_job_ptr = malloc(sizeof(Job_ptr));
    new_job_ptr->Job = new_job;
    new_job_ptr->Next_ptr = NULL;
    if (prev_job_ptr) prev_job_ptr->Next_ptr = new_job_ptr;
    prev_job_ptr = new_job_ptr;
    if (NULL==first_job_ptr) first_job_ptr = new_job_ptr;
    jobs_count++;
    first_key++;
  }
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
  LOG("%d",index);
  if (index==-1) {
    jobs_list_append_job(result);  
  } else {
    snprintf(jobs_list_get_index(index)->Name,JOB_NAME_LENGTH, result);
    main_save_data();
  }
  main_menu_update();
}

void jobs_add_job() {
  tertiary_text_prompt("New medication name?", callback, (void*) -1);
}

void jobs_rename_job(uint8_t index) {
  tertiary_text_prompt(jobs_get_job_name(index), callback, (void*) (int) index);
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

void jobs_set_job_repeat(uint8_t index, uint8_t repeat) {
  Job* job=jobs_list_get_index(index);
  if (job) job->Repeat_hrs=repeat;
  main_save_data();
}

#define MAX_CLOCK_LENGTH 24
char clock_buffer[MAX_CLOCK_LENGTH];
char repeat_buffer[MAX_CLOCK_LENGTH];

char* jobs_get_job_clock_as_text(uint8_t index) {
  int seconds = jobs_get_job_seconds(index);
  
  snprintf(clock_buffer,MAX_CLOCK_LENGTH,"%d:%02d:%02d",(seconds/3600) /*hours*/,(seconds / 60) % 60 /*mins*/,seconds % 60 /*secs*/);
  return clock_buffer;
}

char* jobs_get_job_repeat_as_text(uint8_t index) {
  int repeat = jobs_get_job_repeat(index);
  
  snprintf(repeat_buffer,MAX_CLOCK_LENGTH,"every %d hrs", repeat);
  return repeat_buffer;
}

void jobs_reset_and_save(uint8_t index) {
  jobs_list_get_index(index)->Seconds=time(NULL);
  main_save_data();
}

void jobs_add_minutes(uint8_t index, int minutes) {
  Job* job=jobs_list_get_index(index);
  int seconds = (int) job->Seconds;
  if (seconds + 60*minutes < time(NULL)) {
    seconds += 60*minutes;
  } else {
    seconds = time(NULL);
  }
  job->Seconds = seconds;
  main_save_data();
}