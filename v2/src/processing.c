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
    _Atomic struct LogMessage message_ring[MAX_STORED_LOG_MESSAGES];
} log_message_buffer;

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
