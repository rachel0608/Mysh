//Job.h

#ifndef JOB_H
#define JOB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

typedef struct Job {
    int jid; //job id
    pid_t pid; //process ID
    char *command; //command being executed
    int status; // Running (0) or Suspended (1) or Terminated/Done (2)
    struct termios* setting; //terminal settings info
    //pointer to last job backgrounded/foregrounded
    volatile sig_atomic_t handle_flag; // flag for sig_chld_handler to update
}   Job;

Job *new_job(pid_t pid, char *command); //given process ID and command, create Job (init as running by default) and return pointer

//Accessors
pid_t get_pid(const Job *j);
char *get_command(const Job *j);
int get_status(const Job *j);
char *job_to_string(const Job *j);

void set_job_status(Job *j, int status); //suspended = 0, running = 1
void free_job(Job *job); //free memory allocated for Job struct
void handle_flag_true(Job *job, int boolean); //set handle flag to boolean (true or false)

#endif
