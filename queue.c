#include "queue.h"
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t full_buffer_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t empty_buffer_cond = PTHREAD_COND_INITIALIZER;
/**
struct queue {
  int head; //index of head
  int tail; //index of tail
  int size; //number of elements currently in the queue
  int curr_size; //max number of elements 
  void **items; //array of items
};
*/

struct queue {
    int head; //index of head
    int tail; //index of tail
    int size; //number of elements currently in the queue
    int curr_size; //max number of elements
    int null_val; //placeholder for empty val, will be assigned -1
    void **items; //array of items
};

// Allocates a queue with size `size`
queue_t *queue_new(int size) {

    queue_t *q = (queue_t *) malloc(sizeof(queue_t));

    if (q) {
        q->head = 0;
        q->tail = 0;
        q->curr_size = 0;
        q->size = size;
        q->null_val = -1;
        q->items = (void **) malloc(sizeof(void *) * size);
    }
    for (int i = 0; i < q->size; i++) {
        q->items[i] = &q->null_val;
    }
    return q;
}

// frees a queue (should assume the queue is empty)
void queue_delete(queue_t **q) {
    pthread_mutex_lock(&mutex);
    if (*q) {
        free(((*q)->items));
        ((*q)->items) = NULL;
        free(*q);
        *q = NULL;
    }
    pthread_mutex_unlock(&mutex);
    return;
}

// add an element to the queue
// blocks if queue is full
bool queue_push(queue_t *q, void *elem) {
    pthread_mutex_lock(&mutex);
    if ((q == NULL) || (!q)) {
        pthread_mutex_unlock(&mutex);
        sleep(1);
        return false;
    }
    while (q->size == q->curr_size) {
        pthread_cond_wait(&full_buffer_cond, &mutex);
    }
    q->curr_size += 1;
    q->items[q->tail] = elem;
    q->tail = (q->tail + 1) % q->size;
    pthread_cond_signal(&empty_buffer_cond);
    pthread_mutex_unlock(&mutex);
    return true;
}

// remove an element from the queue
// blocks if the queue is empty
bool queue_pop(queue_t *q, void **elem) {
    pthread_mutex_lock(&mutex);
    if ((q == NULL) || (!q)) {
        pthread_mutex_unlock(&mutex);
        sleep(1);
        return false;
    }
    while (q->curr_size <= 0) {
        pthread_cond_wait(&empty_buffer_cond, &mutex);
    }
    q->curr_size -= 1;
    (*elem) = q->items[q->head];
    q->items[q->head] = &q->null_val;
    q->head = (q->head + 1) % q->size;
    pthread_cond_signal(&empty_buffer_cond);
    pthread_mutex_unlock(&mutex);
    return true;
}

// returns head element
void *queue_peek(queue_t *q) {
    pthread_mutex_lock(&mutex);
    if ((q == NULL) || (!q) || (q->curr_size <= 0)) {
        pthread_mutex_unlock(&mutex);
        return (&(q->null_val));
    }
    void *head = q->items[q->head];
    pthread_cond_signal(&empty_buffer_cond);
    pthread_mutex_unlock(&mutex);
    return head;
}
