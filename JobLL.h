//JobLL.h

#ifndef JOB_LL_H
#define JOB_LL_H

#include "node.h"
#include "Job.h"

// Define data structures and prototypes
typedef struct {
    int size; //size of LL
    struct Node *head; //head of LL
    struct Node *tail; //tail of LL
} JobLL;

JobLL *new_list(); //Creates a new empty list
int size(const JobLL *l); //returns size of LL
int empty(const JobLL *l); //returns if LL is empty (1=empty, 0=notempty)
Job *remove_first_job(JobLL *l); //removes head and returns Job
void add_job(JobLL *l, Job *j); //add Job at end of LL
void print_jobs(const JobLL *l); //prints all jobs in LL

#endif
