#include "sql_task_queue.h"
#include "sqlite3.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdatomic.h>

#define TASK_QUEUE_STATE_EMPTY 0
#define TASK_QUEUE_STATE_AQUIRED 1
#define TASK_QUEUE_STATE_WRITTEN 2

#define NUM_POSSIBLE_SQL_STATEMENTS 12

sqlite3_stmt *compiled_sql_statements[NUM_POSSIBLE_SQL_STATEMENTS];


uint8_t execute_task(const Task* task) {
    if (task->task_id < 0 || task->task_id >= NUM_POSSIBLE_SQL_STATEMENTS) {
        return; // TODO: error code?
    }

    sqlite3_stmt* statement = compiled_sql_statements[task->task_id];
    sqlite3_reset(statement);

    switch (task->task_id) {
        case 10:
            sqlite3_bind_double(statement, 1, task->data.task_10.item0);
            sqlite3_bind_double(statement, 2, task->data.task_10.item1);
            sqlite3_bind_double(statement, 3, task->data.task_10.item2);
            sqlite3_bind_int(statement, 4, task->data.task_10.item3);
            sqlite3_bind_int(statement, 5, task->data.task_10.item4);
            sqlite3_bind_int(statement, 6, task->data.task_10.item5);
            sqlite3_bind_int(statement, 7, task->data.task_10.item6);
            break;
        case 11:
            sqlite3_bind_int(statement, 1, task->data.task_11.item0);
            sqlite3_bind_int(statement, 2, task->data.task_11.item1);
            sqlite3_bind_int(statement, 3, task->data.task_11.item2);
            sqlite3_bind_int(statement, 4, task->data.task_11.item3);
            break;
    }
    
    // TODO: handle errors?
    sqlite3_step(statement);
}


void step_sql_task_queue() {
    Task task;
    if (try_dequeue(&task)) {
        // There is a task for us to do!
        execute_task(&task);
    }
}


bool initialize_database_executer(sqlite3 *db) {
    int result;
    
    result = sqlite3_prepare_v2(db, "INSERT INTO RocketKinematics VALUES (?, ?, ?, ?, ?, ?, ?);", 59, &(compiled_sql_statements[10]), NULL);

    if (result != SQLITE_OK) {
        printf("COMPILING PREPARED STATEMENT 0 FAILED WITH SQL ERROR CODE %d", result);
        return false;
    }
    
    result = sqlite3_prepare_v2(db, "INSERT INTO RocketState VALUES (?, ?, ?, ?);", 45, &(compiled_sql_statements[11]), NULL);

    if (result != SQLITE_OK) {
        printf("COMPILING PREPARED STATEMENT 1 FAILED WITH SQL ERROR CODE %d", result);
        return false;
    }
    

    return true;
}


void finalize_database_executer() {
    for (int i = 0; i < NUM_POSSIBLE_SQL_STATEMENTS; i++) {
        sqlite3_finalize(compiled_sql_statements[i]);
    }
}


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

    if (!atomic_compare_exchange_strong(
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
) {
    struct SQL_TASK_ROCKET_KINEMATICS task_data;
    task_data.item0 = x_position;
    task_data.item1 = y_position;
    task_data.item2 = z_position;
    task_data.item3 = x_velocity;
    task_data.item4 = y_velocity;
    task_data.item5 = z_velocity;
    task_data.item6 = acceleration_magnitude;

    Task task;
    task.task_id = 10;
    task.data.task_10 = task_data;

    return try_enqueue(&task);
}

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
) {
    struct SQL_TASK_ROCKET_STATE task_data;
    task_data.item0 = state;
    task_data.item1 = firing_channel_1;
    task_data.item2 = firing_channel_2;
    task_data.item3 = firing_channel_3;

    Task task;
    task.task_id = 11;
    task.data.task_11 = task_data;

    return try_enqueue(&task);
}
