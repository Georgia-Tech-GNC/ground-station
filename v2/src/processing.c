#include <stdint.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdbool.h>

#include "processing.h"
#include "configuration.h"

// Both must be a power of 2 so we can piggyback off of uint8 overflow
#define CANDIDATE_LIST_SIZE 8
#define MAX_STORED_LOG_MESSAGES 256

/********************** ************ **********************/
/********************** LOG MESSAGES **********************/
/********************** ************ **********************/

#define LOG_MESSAGE_BUFFER_ELEMENT_STATE_IDLE    0
#define LOG_MESSAGE_BUFFER_ELEMENT_STATE_READING 1
#define LOG_MESSAGE_BUFFER_ELEMENT_STATE_WRITING 2

struct {
    atomic_uint_fast32_t tail_index;
    struct LogMessageBufferElement {
        LogMessage message;
        uint32_t rotation;
        atomic_uint_fast8_t active_readers;
        atomic_uint_fast8_t waiting_writers;
    } message_ring[MAX_STORED_LOG_MESSAGES];
} log_message_buffer;



struct {
    atomic_uint_fast32_t tail_index;
    struct LogMessageBufferElementV2 {
        volatile LogMessage message;
        atomic_uint_fast32_t rotation;
        atomic_flag in_write_mode;
    } message_ring[MAX_STORED_LOG_MESSAGES];
} lv2;


void write_v2(LogMessageType message_type, uint32_t message_parameter) {
    uint_fast32_t write_counter = atomic_fetch_add(&(lv2.tail_index), 1);

    struct LogMessageBufferElementV2* write_container = &(lv2.message_ring[write_counter % MAX_STORED_LOG_MESSAGES]);
    uint_fast32_t write_rotation_start = 2 * (write_counter / MAX_STORED_LOG_MESSAGES + 1);

    // Wait for existing writers to finish writing (there should be none)
    while (atomic_flag_test_and_set_explicit(&(write_container->in_write_mode), memory_order_seq_cst));

    // Only update the value if the current write rotation is less than the desired one (ie. our data is newer)
    if (atomic_load_explicit(&(write_container->rotation), memory_order_seq_cst) <= write_rotation_start) { 
        atomic_store_explicit(&(write_container->rotation), write_rotation_start + 1, memory_order_release);
        // I'm going to argue that having atomic inner values is only neccessary for further calculation in critical section.
        // For our purposes, tearing on the reader end doesn't matter because we don't require proper
        // data until we get a loop where the rotation didn't change (implying that the data must not have torn)
        // Since the variables are volatile, the compiler won't move them out of the loop or cache them anyway.
        write_container->message.log_message = message_type;             
        write_container->message.message_parameter = message_parameter;   
        atomic_store_explicit(&(write_container->rotation), write_rotation_start + 2, memory_order_release);
    }

    atomic_flag_clear_explicit(&(write_container->in_write_mode), memory_order_seq_cst); // Signal to other waiting writers that we are done writing.
}

void read_v2(uint32_t message_index, LogMessage* message) {
    uint_fast32_t r0, r1;
    LogMessageType message_type;
    uint32_t message_parameter;

    struct LogMessageBufferElementV2* target_container = &(lv2.message_ring[message_index % MAX_STORED_LOG_MESSAGES]);

    // Repeat until the before/after rotation is the same (no write operation mid-read)
    // and the rotation number is even (meaning that the read was not done mid-write)
    do {
        r0 = atomic_load_explicit(&(target_container->rotation), memory_order_acquire);
        message_type = target_container->message.log_message;
        message_parameter = target_container->message.message_parameter;
        atomic_thread_fence(memory_order_acquire);
        r1 = atomic_load_explicit(&(target_container->rotation), memory_order_relaxed);
    } while (r0 != r1 || r0 & 1); 
    
    message->log_message = message_type;
    message->message_parameter = message_parameter;
}

void internal_add_log_message_to_buffer(LogMessageType message_type, uint32_t message_parameter) {
    uint_fast32_t write_counter = atomic_fetch_add(&(log_message_buffer.tail_index), 1);

    struct LogMessageBufferElement* write_container = &(log_message_buffer.message_ring[write_counter % MAX_STORED_LOG_MESSAGES]);
    uint_fast32_t write_rotation = write_counter / MAX_STORED_LOG_MESSAGES + 1;

    // Increment waiting writers to stop new readers from accessing value
    atomic_fetch_add(&(write_container->waiting_writers), 1);

    // Wait for any active readers to finish up.
    // Since reads are very fast operations (copying two integers), the overhead of this loop is minimized.
    // Futhermore, this buffer has write-priority, so the this thread is guaranteed to continue after a finite period of time.
    while (atomic_load(&(write_container->active_readers)) > 0); 

    // The reason we can split up the reader wait and the writer wait is because we know
    // that no new readers will attempt to read until we finally decrement waiting_writers

    // Wait for any other writers to finish up
    // In general, the change of the entire buffer being written too in the time it takes for the readers to
    // finish reading is quite small, so there should really never be much/any contention between writers.
    while (atomic_compare_exchange_weak(&(write_container->in_write_mode), false, true)); 

    // At this point we have gauranteed that this is the only thread with access to the contents
    // of write container for as long as write_container->in_write_mode is true.
    if (write_container->rotation <= write_rotation) { 
        // In the case of writer contention, this ensures that we don't overwrite a more recent log message.
        write_container->message.log_message = message_type;
        write_container->message.message_parameter = message_parameter;
    }
    
    // Set in_write_mode to false and decrement the waiting_writers variable to signal to other threads
    // that this container is open for new reads/writes
    atomic_store(&(write_container->in_write_mode), false);
    atomic_fetch_sub(&(write_container->waiting_writers), 1);
}

bool try_get_log_message(int message_index, LogMessage* message) {
    struct LogMessageBufferElement* target_container = &(log_message_buffer.message_ring[message_index % MAX_STORED_LOG_MESSAGES]);
    uint_fast32_t target_rotation = message_index / MAX_STORED_LOG_MESSAGES + 1;

    // Increment the reader count first as a "fake increment". 
    // This ensures that a writer thread does not begin writing until we checked and re-decremented the value
    atomic_fetch_add(&(target_container->active_readers), 1);

    // Now check if a writer is active. 
    // If it is, we re-decrement the active-reader and wait until i
}


/********************** ************ **********************/
/********************** DATA PACKETS **********************/
/********************** ************ **********************/

/************* ROCKET KINEMATICS *************/

atomic_uint_fast32_t rocket_kinematics_current_release_index;
RocketKinematics rocket_kinematics_struct_candidate_list[CANDIDATE_LIST_SIZE];

const struct RocketKinematics* get_current_rocket_kinematics_values() {
    return &(rocket_state_struct_candidate_list[atomic_load(&rocket_state_current_release_index) % CANDIDATE_LIST_SIZE]);
}

struct RocketKinematics* get_rocket_kinematics_release_candidate() {
    return &(rocket_state_struct_candidate_list[(1 + atomic_load(&rocket_state_current_release_index)) % CANDIDATE_LIST_SIZE]);
}

void publish_rocket_kinematics_release_candidate() {
    atomic_fetch_add(&rocket_state_current_release_index, 1);
}


/************* ROCKET STATE *************/

atomic_uint_fast32_t rocket_state_current_release_index;
RocketState rocket_state_struct_candidate_list[CANDIDATE_LIST_SIZE];

const struct RocketState* get_current_rocket_state_values() {
    return &(rocket_state_struct_candidate_list[atomic_load(&rocket_state_current_release_index) % CANDIDATE_LIST_SIZE]);
}

struct RocketState* get_rocket_state_release_candidate() {
    return &(rocket_state_struct_candidate_list[(1 + atomic_load(&rocket_state_current_release_index)) % CANDIDATE_LIST_SIZE]);
}

void publish_rocket_state_release_candidate() {
    atomic_fetch_add(&rocket_state_current_release_index, 1);
}


/********************** ****************** **********************/
/********************** PAYLOAD PROCESSING **********************/
/********************** ****************** **********************/

void process_rocket_kinematics_payload(uint8_t* payload) {
    RocketKinematics* storage_location = get_rocket_kinematics_release_candidate();
    memcpy(&storage_location->x_position, payload + 0, 4);
    memcpy(&storage_location->y_position, payload + 4, 4);
    memcpy(&storage_location->z_position, payload + 8, 4);
    memcpy(&storage_location->x_velocity, payload + 12, 2);
    memcpy(&storage_location->y_velocity, payload + 14, 2);
    memcpy(&storage_location->z_velocity, payload + 16, 2);
    memcpy(&storage_location->acceleration_magnitude, payload + 18, 4);
    publish_rocket_kinematics_release_candidate();

    // TODO: log something when the queue is full.
    try_enqueue_rocket_kinematics(
        storage_location->x_position,
		storage_location->y_position,
		storage_location->z_position,
		storage_location->x_velocity,
		storage_location->y_velocity,
		storage_location->z_velocity,
		storage_location->acceleration_magnitude
    );
}   

void process_rocket_state_payload(uint8_t* payload) {
    RocketState* storage_location = get_rocket_state_release_candidate();
    memcpy(&storage_location->state, payload + 0, 1);
    memcpy(&storage_location->firing_channel_1, payload + 1, 1);
    memcpy(&storage_location->firing_channel_2, payload + 2, 1);
    memcpy(&storage_location->firing_channel_3, payload + 3, 1);
    publish_rocket_state_release_candidate();

    try_enqueue_rocket_state(
        storage_location->state,
		storage_location->firing_channel_1,
		storage_location->firing_channel_2,
		storage_location->firing_channel_3
    );
}   


bool process_payload_with_id(uint8_t* payload, size_t size, uint8_t message_id) {
    switch (message_id) {	
        case DATA_MSG_ROCKET_KINEMATICS_MSG_ID: // Data Message Packet -> Rocket Kinematics
            if (size != DATA_MSG_ROCKET_KINEMATICS_PACKET_SIZE) return false; // Wrong size
            process_rocket_kinematics_packet_payload(payload);
            return true;	

        case DATA_MSG_ROCKET_STATE_MSG_ID: // Data Message Packet -> Rocket State
            if (size != DATA_MSG_ROCKET_STATE_PACKET_SIZE) return false; // Wrong size
            process_rocket_state_packet_payload(payload);
            return true;

        default:
            return false; // Unknown message id
    }
}