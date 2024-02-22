#include <stdio.h>
#include "JobLL.h"

int main() {
    // Create a new empty job list
    JobLL *job_list = new_list();

    // Add some jobs to the list
    add_job(job_list, new_job(1001, "ls -l"));
    add_job(job_list, new_job(1002, "gcc program.c -o program"));
    add_job(job_list, new_job(1003, "sleep 10"));

    // Print the list of jobs
    printf("Initial job list:\n");
    print_jobs(job_list);

    // Find a job by process ID
    pid_t pid_to_find = 1002;
    int job_index = find_job(job_list, pid_to_find);
    if (job_index != -1) {
        printf("Job with PID %d found at index %d in the list.\n", pid_to_find, job_index);
    } else {
        printf("Job with PID %d not found in the list.\n", pid_to_find);
    }

    // Remove a job by index
    int index_to_remove = 1; // Remove the second job
    Job *removed_job = remove_nth_job(job_list, index_to_remove);
    if (removed_job != NULL) {
        printf("Removed job: %s\n", job_to_string(removed_job));
        free(removed_job); // Free the removed job to prevent memory leaks
    } else {
        printf("Failed to remove job at index %d.\n", index_to_remove);
    }

    // Print the updated list of jobs
    printf("Updated job list:\n");
    print_jobs(job_list);

    // Remove the first job
    Job *first_job = remove_first_job(job_list);
    if (first_job != NULL) {
        printf("Removed first job: %s\n", job_to_string(first_job));
        free(first_job); // Free the removed job to prevent memory leaks
    } else {
        printf("Failed to remove the first job.\n");
    }

    // Print the final list of jobs
    printf("Final job list:\n");
    print_jobs(job_list);

    // Free memory allocated for the job list
    free_all_jobs(job_list);

    return 0;
}
