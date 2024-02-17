//Job.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "Job.h"
#define TO_STRING_CHARS 36 //pid_t = 10 chars, command = 24 chars, status = 1 char

//given process ID and command, create Job (init as running by default)
Job *new_job(pid_t pid, char *command) {
    Job *J = malloc(sizeof(Job));
    J->pid = pid;
    J->command = command;
    J->status = 1; //status is running (1) by default
    return J;
} //new_job()

pid_t get_pid(const Job *j) {
    return j->pid;
} //get_pid()

char *get_command(const Job *j) {
    return j->command;
}//get_command()

int get_status(const Job *j) {
    return j->status;
}//get_status()

//print <pid: command | status>
char *job_to_string(const Job *j) {
    char* print = malloc(TO_STRING_CHARS); 
    sprintf(print, "<%d: %s | %d>", get_pid(j), get_command(j), get_status(j));
    return print;
} //job_to_string()

void set_job_status(Job *j, int status) {
    j->status = status; //suspended = 0, running = 1
} //set_job_status()

