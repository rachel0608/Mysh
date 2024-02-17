//JobQueue.c

#include <stdlib.h>
#include <stdio.h>
#include "JobQueue.h"

// Creates a new Queue
JobQueue *new_queue() {
    JobQueue *Q = malloc(sizeof(JobQueue));
    Q->L = new_list();
    return Q;
} //new_queue()

// Basic necesseties...
int sizeQ(const JobQueue *q) {
    return size(q->L);
} //sizeQ()

int emptyQ(const JobQueue *q) {
    return empty(q->L);
}//emptyQ()

// Core functions for a Queue
void insertQ(JobQueue *q, Job *j) {
    add_job(q->L, j);
} //inserts data at the end of the queue (List tail)

void *removeQ(JobQueue *q) {
    if (emptyQ(q)) {
        printf("Tried to remove from empty Queue\n");
        exit(EXIT_FAILURE);
    }
    return remove_first_job(q->L);
} //removes data at the front of the queue (List head)