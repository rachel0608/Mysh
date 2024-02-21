//node.c

#include <stdlib.h>
#include "node.h"

struct Node *new_node(Job *job) { //creates new node with job
    struct Node *n = malloc(sizeof(struct Node)); //stdlib.h is needed for malloc/sizeof
    n->data = job;
    n->next = NULL;
} //new_node()
