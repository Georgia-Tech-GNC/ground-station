#include <stdatomic.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "sql_task_queue.h"

#define TASK_QUEUE_STATE_EMPTY 0
#define TASK_QUEUE_STATE_AQUIRED 1
#define TASK_QUEUE_STATE_WRITTEN 2

typedef struct {
    atomic_uint_fast8_t task_queue_state;
    Task data;
} MPSCTaskWrapper;

struct {
    atomic_uint_fast32_t write_ticket;
    uint32_t read_ticket;
    MPSCTaskWrapper slots[TASK_QUEUE_CAPACITY];
} sql_task_queue;

/**
 * Non-blocking method which attempts to add an item to the global sql task queue
 * Returns false if the queue is full, or true if the operation was successful.
 * This method is thread-safe and non-blocking
 */
bool try_enqueue(const Task* item) {
    uint_fast32_t ticket = atomic_fetch_add(&(sql_task_queue.write_ticket), 1);
    MPSCTaskWrapper* slot = &(sql_task_queue.slots[ticket % TASK_QUEUE_CAPACITY]);

    uint_fast8_t expected_empty_state = TASK_QUEUE_STATE_EMPTY;

    if (!atomic_compare_exchange_weak(
            &(slot->task_queue_state),  // Check slot state
            &expected_empty_state,      // If empty
            TASK_QUEUE_STATE_AQUIRED    // Set to aquired
    )) {
        // This means that either 
        // (a) another thread was assigned this slot and we have managed to fill 
        //     every other entry in the queue before it finished writing
        // or (b) a thread previously wrote something in this slot, but it hasn't been read yet.
        
        // Either way, we return because the queue is full
        return false;
    }

    // Otherwise, the state of this slot has been shifted to aquired and we know for sure
    // that this is the only thread writing or reading to/from this slot right now 
    memcpy(&(slot->data), item, sizeof(Task));

    // Set the state to written to indicate to the reader that we are good to read now
    atomic_store(&(slot->task_queue_state), TASK_QUEUE_STATE_WRITTEN);
    return true;
}

/**
 * Attempts to remove the earliest added element from the queue and store it in the inputted value
 * 
 * This method may NOT be called from more then one thread at a time
 * This method can also NOT be called at the same time as get_estimated_task_queue_size
 * However, this method CAN be called at the same time as multiple try_enqueue operations without issue.
 * 
 */
bool try_dequeue(Task* task) {
    uint_fast32_t ticket = sql_task_queue.read_ticket;
    MPSCTaskWrapper* slot = &(sql_task_queue.slots[ticket % TASK_QUEUE_CAPACITY]);

    if (atomic_load(&(slot->task_queue_state)) != TASK_QUEUE_STATE_WRITTEN) {
        // This slot has not been written yet (i.e. the task queue is empty)
        return false;
    }

    // We know that once a slot enters the "written" state, nothing will attempt to modify it
    // until it is back in the "empty" state
    memcpy(task, &(slot->data), sizeof(Task));
    
    // Once we are done, we can set the slot state back to "empty" so that the writer can write to it
    // We also increment the read ticket
    atomic_store(&(slot->task_queue_state), TASK_QUEUE_STATE_EMPTY);
    sql_task_queue.read_ticket++;
    return true;
}

/** Returns the estimated size of the queue (including all partially-completed write operations) 
 * This method may NOT be called from multiple threads or be called at the same time as try_dequeue
 * 
 * Note that the returned value is only a rough estimate of magnitude. It should NOT be used for iteration
 * or looping purposes.
 * 
 * The only way to determine the exact size of the queue is to repeatedly call "try_dequeue" until 
 * the method returns false (indicating the queue is empty).
 * 
 * In particular, there may be instances where this method returns a non-zero value even when the queue
 * is actually empty. This generally means that a thread is in the middle of adding something to the queue, 
 * but isn't quite finished yet
 * 
 * @return the estimated size of the task queue.
*/
int32_t get_estimated_task_queue_size() {
    return (
        atomic_load(&(sql_task_queue.write_ticket)) - sql_task_queue.read_ticket + TASK_QUEUE_CAPACITY
    ) % TASK_QUEUE_CAPACITY;
} 

//#define IN_TESTING_MODE

#ifdef IN_TESTING_MODE

void* enqueue_thread_1() {
    for (int i = 0; i < 100; i++) {
        if (!try_enqueue(i * 2)) {
            printf("Enqueue %d OPERATION FAILED!\n", i*2);
        }
    }
    printf("Enqueue 1 complete!\n");
}

void* enqueue_thread_2() {
    for (int i = 0; i < 100; i++) {
        if (!try_enqueue(i*2 + 1)) {
            printf("Enqueue %d OPERATION FAILED!\n", i*2+1);
        }
    }
    printf("Enqueue 2 complete!\n");
}

void* dequeue_thread() {
    int i = 0;
    int j = 0;
    while (i < 100 || j < 100) {
        Task dequeued;
        dequeued.item = -1;
        if (try_dequeue(&dequeued)) {
            if (dequeued.item % 2 == 0) {
                if (dequeued.item != i*2) printf("Dequeue %d OPERATION FAILED! (recv %d)\n", i, dequeued.item);
                i++;
            } else {
                if (dequeued.item != j*2+1) printf("Dequeue %d OPERATION FAILED! (recv %d)\n", j, dequeued.item);
                j++;
            }
        } 
    }
    printf("Dequeue complete!\n");
}
int main() {
    
    pthread_t enqueue_thread_id_1;
    pthread_t enqueue_thread_id_2;
    pthread_t dequeue_thread_id;
    printf("Before Thread\n");
    pthread_create(&enqueue_thread_id_1, NULL, enqueue_thread_1, NULL);
    pthread_create(&enqueue_thread_id_2, NULL, enqueue_thread_2, NULL);
    pthread_create(&dequeue_thread_id, NULL, dequeue_thread, NULL);
    pthread_join(enqueue_thread_id_1, NULL);
    pthread_join(enqueue_thread_id_2, NULL);
    pthread_join(dequeue_thread_id, NULL);
    printf("After Thread\n");
    exit(0);
}

#endif