//node.h

#ifndef NODE_H
#define NODE_H

#include <stdlib.h>
#include "Job.h"

struct Node {
    Job *data; //Job in node
    struct Node *next; //next node in LL
    struct Node *prev; //prev node in List
};

struct Node *new_node(Job *data);

#endif