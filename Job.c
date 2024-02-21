//Job.c

#include "Job.h"

int job_count = 1; //global var to keep track of how many jobs are in list

//given process ID and command, create Job (init as running by default)
Job *new_job(pid_t pid, char *command) {
    Job *J = malloc(sizeof(Job));
    J->pid = pid;
    J->command = command;
    J->status = 1; //status is running (1) by default
    J->jid = job_count;
    job_count++;
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
  if (j == NULL) 
        return NULL; // Invalid input

    // Calculate the size needed for the string
    // Format: "PID: <pid>, Command: <command>, Status: <status>"
    size_t size = snprintf(NULL, 0, "JID: %d, PID: %d, Command: %s, Status: %d\n", j->jid, j->pid, j->command, j->status) + 1;
    // Allocate memory for the string
    char *result = malloc(size);
    if (result != NULL) {
        // Format the string
        snprintf(result, size, "JID: %d, PID: %d, Command: %s, Status: %d\n", j->jid, j->pid, j->command, j->status);
    }
    return result;
} //job_to_string()

void set_job_status(Job *j, int status) {
    j->status = status; //suspended = 0, running = 1
} //set_job_status()

void reset_job_count() {
    job_count = 1;
} //reset_job_count()

void free_job(Job *job) {
    if (job != NULL) {
        free(job->command); // Free the dynamically allocated command string
        // Add additional cleanup for other dynamically allocated fields, if any
        free(job); // Free the Job structure itself
    }
} //free_job()
