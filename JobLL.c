//JobLL.c

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

int find_job(const JobLL *l, pid_t pid) {
    struct Node *n = l->head;
    int index = 0;
    //traverse LL until value is found or end is reached
    while (n != NULL) {
        if (n->data->pid == pid) {
            return index; //value found, return the current index
        }
        //move to next node
        n = n->next;
        index++;
    }
    //value not found
    free(n);
    return -1;

} //returns the index of the job with a certain pid (counting from end of LL), otherwise -1

Job *remove_nth_job(JobLL *l, int n) {
    if (l->head == NULL || n < 0) 
        return NULL; // Invalid input
    
    if (n == 0) 
        return remove_first_job(l);         // Special case: Removing the head node

    // Traverse the list to find the previous node of the one to be removed
    struct Node* prev = NULL;
    struct Node* current = l->head;
    int count = 0;

    while (current != NULL && count < n) {
        prev = current;
        current = current->next;
        count++;
    }
    if (current == NULL) {
        free(prev);
        free(current);
        return NULL; // Index out of bounds
    }

    // Remove the nth node
    prev->next = current->next;
    current->next = NULL; // Disconnect the removed node
    Job *temp = current->data;
    free(prev);
    free(current);
    if (l->size == ONE)
        reset_job_count(); //if last job in list, update size and reset job count
    l->size--;
    return temp;

} //removes nth node and returns Job in node

Job *remove_first_job(JobLL *l) {
    if (l->size = EMPTY) { //list is empty cannot do anything
        fprintf(stderr, "Error: List is empty\n");
        exit(EXIT_FAILURE);
    }
    if (l->size == ONE) {
        struct Node *n = l->head;
        l->head = l->tail = NULL;
        Job *job = n->data;
        free(n);
        l->size = EMPTY;
        reset_job_count();
        return job;
    }
    else {
        struct Node *n = l->head;
        l->head = n->next;
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
        l->tail = n; //set tail label to n, n becomes tail
    }
    l->size++; // We just inserted an item
} // add_job()

void print_jobs(const JobLL *l) {
    struct Node *n = l->head;
    printf("L(%d) = ", l->size);
    while (n != NULL) {
        char *job = job_to_string(n->data);
        printf("%s ", job);
        free(job);
        n = n->next;
    }
    free(n);
    printf("\n");
} // print_jobs()
