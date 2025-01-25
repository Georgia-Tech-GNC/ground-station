#ifndef SQL_TASK_QUEUE_H
#define SQL_TASK_QUEUE_H

#include <stdint.h>
#include <stdbool.h>

// Capacity must be a power of 2
#define TASK_QUEUE_CAPACITY 4096


struct SQL_TASK_ROCKET_KINEMATICS {
    float item0;
    float item1;
    float item2;
    int16_t item3;
    int16_t item4;
    int16_t item5;
    int32_t item6;
};

struct SQL_TASK_ROCKET_STATE {
    uint8_t item0;
    uint8_t item1;
    uint8_t item2;
    uint8_t item3;
};


typedef struct {
    int task_id;
    union {
        struct SQL_TASK_ROCKET_KINEMATICS task_10;
        struct SQL_TASK_ROCKET_STATE task_11;
    } data;
} Task;

/**
 * Executes the given task using the prepared statement created using initialize_database_executer.
 * This method cannot be called unless initialize_database_executer has been called.
 * This method is NOT thread safe.
 *
 * @param task The task to execute
 */
uint8_t execute_task(const Task* task);

/**
 * Initializes all prepared sql statements
 * This method should be called once at the start of the program
 * This method is NOT thread safe.
 *
 * @param db A pointer to the sql database to use
 */ 
bool initialize_database_executer(sqlite3 *db);

void finalize_database_executer();

void step_sql_task_queue();

/**
 * Non-blocking method which attempts to add an item to the global sql task queue
 * Returns false if the queue is full, or true if the operation was successful.
 * This method is thread-safe and non-blocking
 */
bool try_enqueue(const Task* task);

/**
 * Attempts to remove the earliest added element from the queue and store it in the inputted value
 * 
 * This method may NOT be called from more then one thread at a time
 * This method can also NOT be called at the same time as get_estimated_task_queue_size
 * However, this method CAN be called at the same time as multiple try_enqueue operations without issue.
 * 
 */
bool try_dequeue(Task* task);

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
int32_t get_estimated_task_queue_size();


/**
 * Non-blocking method which attempts to add a task to the global sql task queue.
 * This task will add the specified Rocket Kinematics to the sql database.
 * Returns false if the queue is full, or true if the operation was successful.
 * This method is thread-safe and non-blocking
 */
bool try_enqueue_rocket_kinematics (
    float x_position, 
    float y_position, 
    float z_position, 
    int16_t x_velocity, 
    int16_t y_velocity, 
    int16_t z_velocity, 
    int32_t acceleration_magnitude
);

/**
 * Non-blocking method which attempts to add a task to the global sql task queue.
 * This task will add the specified Rocket State to the sql database.
 * Returns false if the queue is full, or true if the operation was successful.
 * This method is thread-safe and non-blocking
 */
bool try_enqueue_rocket_state (
    uint8_t state, 
    uint8_t firing_channel_1, 
    uint8_t firing_channel_2, 
    uint8_t firing_channel_3
);



#endif