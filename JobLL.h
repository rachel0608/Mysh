//JobLL.h

#ifndef JOB_LL_H
#define JOB_LL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "node.h"
#include "Job.h"

#define EMPTY 0
#define ONE 1

// Define data structures and prototypes
typedef struct {
    int size; //size of LL
    struct Node *head; //head of LL
    struct Node *tail; //tail of LL
} JobLL;

JobLL *new_list(); //Creates a new empty list
int size(const JobLL *l); //returns size of LL
int empty(const JobLL *l); //returns if LL is empty (1=empty, 0=notempty)
int find_job(const JobLL *l, pid_t pid); //returns the index of the job with a certain pid (counting from end of LL)
Job *remove_nth_job(JobLL *l, int n); //removes nth node and returns Job in node
Job *get_nth_job(JobLL *l, int n); //get nth node and returns Job in node
Job *remove_first_job(JobLL *l); //removes head and returns Job
void add_job(JobLL *l, Job *j); //add Job at end of LL
void print_jobs(const JobLL *l); //prints all jobs in LL
void free_all_jobs(JobLL *l);

#endif