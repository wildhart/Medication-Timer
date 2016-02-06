#pragma once

#define MAX_JOBS 20
#define JOB_NAME_SIZE 24

extern uint8_t jobs_count;
struct Job {
  char Name[JOB_NAME_SIZE];
  time_t Seconds;
  uint8_t Repeat_hrs;
  bool Fixed;
};
extern struct Job jobs[MAX_JOBS];
extern uint8_t jobs_count;

void jobs_list_sort(void);
void jobs_list_save(uint8_t first_key);
void jobs_list_load(uint8_t first_key, const uint8_t version);
void jobs_list_write_dict(DictionaryIterator *iter, uint8_t first_key);
void jobs_list_read_dict(DictionaryIterator *iter, uint8_t first_key, const uint8_t version);

void jobs_delete_all_jobs(void);
void jobs_delete_job_and_save(uint8_t index);
void jobs_add_job();
void jobs_rename_job(uint8_t index);
void jobs_set_job_repeat(uint8_t *index, uint8_t repeat, uint8_t fixed);
char* jobs_get_job_clock_as_text(uint8_t index);
char* jobs_get_job_repeat_as_text(uint8_t index);
void jobs_reset_and_save(uint8_t *index);
void jobs_add_minutes(uint8_t *index, int minutes);
time_t jobs_get_next_wakeup_time(void);
void jobs_check_alarms(void);