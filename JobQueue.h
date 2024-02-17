//JobQueue.h

#ifndef QUEUE_H
#define QUEUE_H

#include "JobLL.h"
#include "Job.h"

typedef struct {
    JobLL *L; //LL of Queue
} JobQueue;

//Creates a new Queue
JobQueue *new_queue();

//basic necessities
int sizeQ(const JobQueue *q); //returns size of Queue
int emptyQ(const JobQueue *q); //returns if queue is empty

//core functions
void insertQ(JobQueue *q, Job *j); //insert job at end of queue
void *removeQ(JobQueue* q); //removes job from head of queue

#endif