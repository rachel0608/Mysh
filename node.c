//node.c

#include <stdlib.h>
#include "node.h"
#include "Job.h"

struct Node *new_node(Job *job) { //creates new node with job
    struct Node *n = malloc(sizeof(struct Node)); //stdlib.h is needed for malloc/sizeof
    n->data = job;
    n->prev = NULL;
    n->next = NULL;
} //new_node()