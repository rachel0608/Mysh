//node.h

#ifndef NODE_H
#define NODE_H

#include "Job.h"

struct Node {
    Job *data; //Job in node
    struct Node *next; //next node in Queue
    struct Node *prev; //previous node in Queue
};

struct Node *newNode(Job *data);

#endif