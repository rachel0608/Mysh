//JobLL.c

#include "JobLL.h"
#include "Job.h"
#include "node.h"

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

Job *find_job_jid(const JobLL *l, int jid) {
    struct Node *n = l->head;
    int index = 0;
    //traverse LL until value is found or end is reached
    while (n != NULL) {
        if (n->data->jid == jid) {
            return n->data; //value found, return the current index
        }
        //move to next node
        n = n->next;
        index++;
    }
    //value not found
    free(n);
    return NULL;

}

Job *get_nth_job(JobLL *l, int n) {
    if (n < 0 || n >= l->size || l->head == NULL) {
        return NULL; 
    }

    // traverse the list until the nth node
    struct Node *current = l->head;
    for (int i = 0; i < n; i++) {
        current = current->next;
    }

    return current->data;
}

// Removes nth node and returns Job in node
Job *remove_nth_job(JobLL *l, int n) {
    if (n < 0 || n > l->size || l->head == NULL) {
        return NULL;
    }

    if (n == 0) {
        return remove_first_job(l);
    }

    // traverse to the node to be removed
    struct Node *current = l->head;
    for (int i = 1; i < n; i++) {
        current = current->next;
    }

    Job *removed_job = current->data;
    if (current->prev != NULL) { // prev is not head
        current->prev->next = current->next;
    } else { // prev is head
        l->head = current->next; 
    }

    if (current->next != NULL) {
        current->next->prev = current->prev;
    } else {
        l->tail = current->prev;
    }

    free(current);

    if (l->size == ONE)
        reset_job_count(); //if last job in list, update size and reset job count

    l->size--;
    return removed_job;
} //removes nth node and returns Job in node

Job *remove_first_job(JobLL *l) {
    if (l->size == EMPTY) {
        fprintf(stderr, "Error: List is empty.\n");
        return NULL;
    }
    struct Node *head_node = l->head;
    Job *removed_job = head_node->data;
    if (l->size == ONE) {
        l->head = NULL;
        l->tail = NULL;
        reset_job_count();
    } else {
        l->head = head_node->next;
        l->head->prev = NULL;
    }
    free(head_node);
    l->size--;
    return removed_job;
}

// Add Job at end of LL
void add_job(JobLL *l, Job *j) {
    struct Node *n = new_node(j); 
    n->data = j;
    n->next = NULL;
    if (empty(l)) {
        n->prev = NULL;
        l->head = n;
        l->tail = n;
    } else {
        n->prev = l->tail;
        l->tail->next = n;
        l->tail = n;
    }
    l->size++;
}

Job *get_last_suspended_job(JobLL *job_list) {
    Job *last_bg_job = NULL;
    for (int i = job_list->size - 1; i >= 0; i++) {
        if (get_nth_job(job_list, i)->status == 1) { 
            last_bg_job = get_nth_job(job_list, i);
        }
    }
    return last_bg_job;
}

void print_jobs(const JobLL *l) {
    struct Node *n = l->head;
    printf("L(%d) \n", l->size);
    while (n != NULL) {
        char *job = job_to_string(n->data);
        printf("%s", job);
        free(job);
        n = n->next;
    }
    free(n);
    printf("\n");
} // print_jobs()

void free_all_jobs(JobLL *l) {
    struct Node *current = l->head;
    while (current != NULL) {
        struct Node *next = current->next; // Store the next node before freeing the current one
        free(current->data); // Free the job associated with the current node
        free(current); // Free the current node
        current = next; // Move to the next node
    }
} // free_all_jobs()
