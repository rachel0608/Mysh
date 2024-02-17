//JobLL.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "JobLL.h"

JobLL *new_list() { // Creates a new empty JobLL
    JobLL *L = malloc(sizeof(JobLL));
    L->head = NULL;
    L->tail = NULL;
    L->size = 0;
    return L;
} // new_list()

int size(const JobLL *l) { // Returns the size of JobLL
    return l->size;
} // size()

int empty(const JobLL *l) { // is the JobLL empty?
    return l->size == 0;
} // empty()

Job *remove_first_job(JobLL *l) {
    if (l->size = 0) { //list is empty cannot do anything
        fprintf(stderr, "Error: List is empty\n");
        exit(EXIT_FAILURE);
    }
    if (l->size == 1) {
        struct Node *n = l->head;
        l->head = l->tail = NULL;
        Job *job = n->data;
        free(n);
        l->size = 0;
        return job;
    }
    else {
        struct Node *n = l->head;
        l->head = n->next;
        l->head->prev = NULL;
        Job *job = n->data;
        free(n);
        l->size--;
        return job;
    }
} //remove_first_job()

void add_job(JobLL *l, Job *j) { // Add item at end of JobLL
    struct Node *n = new_node(j); // Create a new node (item, NULL)
    if (l->size == 0) // Inserting in empty JobLL
        l->head = l->tail = n;
    else { // Inserting in a non-empty JobLL
        l->tail->next = n; //set curr tail's next to n
        n->prev = l->tail; //set n's prev to curr tail
        l->tail = n; //set tail label to n, n becomes tail
    }
    l->size++; // We just inserted an item
} // add_job()

void print_jobs(const JobLL *l) {
    struct Node *n = l->head;
    printf("L(%d) = ", l->size);
    while (n != NULL) {
        printf("%s ", job_to_string(&n->data));
        n = n->next;
    }
    printf("\n");
} // print_jobs()