#include <stdatomic.h>
#include <stdbool.h>

// Must be a power of 2 so we can piggyback off of uint8 overflow
#define CANDIDATE_LIST_SIZE 8

atomic_uint_fast32_t rocket_state_current_release_index;
struct RocketStateStruct {
    uint8_t rocket_state;
    bool firing_channel_1;
    bool firing_channel_2;
    bool firing_channel_3;
} rocket_state_struct_candidate_list[CANDIDATE_LIST_SIZE];

const struct RocketStateStruct* get_current_rocket_state_values() {
    return &(rocket_state_struct_candidate_list[atomic_load(&rocket_state_current_release_index) % CANDIDATE_LIST_SIZE]);
}

struct RocketStateStruct* get_rocket_state_release_candidate() {
    return &(rocket_state_struct_candidate_list[(1 + atomic_load(&rocket_state_current_release_index)) % CANDIDATE_LIST_SIZE]);
}

void publish_rocket_state_release_candidate() {
    atomic_fetch_add(&rocket_state_current_release_index, 1);
}
