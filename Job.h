//Job.h

#ifndef JOB_H
#define JOB_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
    int chld_flag; //0 default, 1 for attention needed
    int status_of_child; //status of child (to update to)
    int exit_status; //exit status if child exited
} SigchldInfo; //stores info on a stopped/terminated child

typedef struct Job {
    int jid; //job id
    pid_t pid; //process ID
    char **command; //command being executed
    int status; // Running (0) or Suspended (1) or Terminated/Done (2)
    struct termios* setting; //terminal settings info
    int tstp_flag; //flag for sig_tstp handler to update
    SigchldInfo exit_info; //package of state_change info for child, for use of shell, includes flag
        //pointer to last job backgrounded/foregrounded?
}   Job;

Job *new_job(pid_t pid, char **command); //given process ID and command, create Job (init as running by default) and return pointer

//Accessors
pid_t get_pid(const Job *j);
char **get_command(const Job *j);
int get_status(const Job *j);
char *job_to_string(const Job *j);
void reset_job_count();
void set_job_status(Job *j, int status); //suspended = 0, running = 1
void free_job(Job *job); //free memory allocated for Job struct
void handle_flag_true(Job *job, char *flag, int boolean); //set handle flag to boolean (true or false)

#endif
