#ifndef GROUND_STATION_RING_BUFFER_H
#define GROUND_STATION_RING_BUFFER_H

#include <stdint.h>
#include <pthread.h>

typedef struct RingBufferSlot {
    uint8_t id;
    void* data;
} ring_buffer_slot_t;

typedef struct RingBuffer {
    ring_buffer_slot_t* backing_array;
    size_t head, tail, num_slots, current_size;
    pthread_mutex_t buffer_lock;
    pthread_cond_t push_notification, pop_notification;
} ring_buffer_t;

bool initialize_ring_buffer(ring_buffer_t* this, size_t num_slots);
void free_ring_buffer(ring_buffer_t* this);
void push_to_buffer(ring_buffer_t* this, uint8_t id, void* data);
void* pop_from_buffer(ring_buffer_t* this, uint8_t* popped_id);

#endif //GROUND_STATION_RING_BUFFER_H
