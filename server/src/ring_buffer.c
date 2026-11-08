
#include <stdlib.h>

#include "ring_buffer.h"


bool initialize_ring_buffer(ring_buffer_t* this, size_t num_slots) {
    if (num_slots < 2) { return false; }

    this->num_slots = num_slots;
    this->head = 0;
    this->tail = 0;
    this->backing_array = malloc(num_slots * sizeof(ring_buffer_slot_t));
    this->current_size = 0;
    if (this->backing_array == nullptr) { return false; }

    if (pthread_mutex_init(&this->buffer_lock, nullptr) != 0) {
        free(this->backing_array);
        return false;
    }

    if (pthread_cond_init(&this->push_notification, NULL) != 0) {
        pthread_mutex_destroy(&this->buffer_lock);
        free(this->backing_array);
        return false;
    }

    if (pthread_cond_init(&this->pop_notification, NULL) != 0) {
        pthread_cond_destroy(&this->push_notification);
        pthread_mutex_destroy(&this->buffer_lock);
        free(this->backing_array);
        return false;
    }

    return true;
}

void free_ring_buffer(ring_buffer_t* this) {
    if (this->backing_array) { free(this->backing_array); }
    pthread_cond_destroy(&this->push_notification);
    pthread_cond_destroy(&this->pop_notification);
    pthread_mutex_destroy(&this->buffer_lock);
}

static size_t get_next_buffer_index(const ring_buffer_t* this, size_t buffer_index) {
    return (buffer_index + 1) % this->num_slots;
}

void push_to_buffer(ring_buffer_t* this, uint8_t id, void* data) {
    if (pthread_mutex_lock(&this->buffer_lock) == 0) {
        while (this->current_size == this->num_slots) {
            pthread_cond_wait(&this->pop_notification, &this->buffer_lock);
        }
        ring_buffer_slot_t* slot = &this->backing_array[this->head];
        slot->id = id;
        slot->data = data;
        this->head = get_next_buffer_index(this, this->head);
        this->current_size++;
        pthread_cond_signal(&this->push_notification);
        pthread_mutex_unlock(&this->buffer_lock);
    }
}

void* pop_from_buffer(ring_buffer_t* this, uint8_t* popped_id) {
    if (pthread_mutex_lock(&this->buffer_lock) == 0) {
        while (this->current_size == 0) {
            pthread_cond_wait(&this->push_notification, &this->buffer_lock);
        }
        ring_buffer_slot_t* slot = &this->backing_array[this->tail];
        *popped_id = slot->id;
        void* data = slot->data;
        slot->data = nullptr;
        this->tail = get_next_buffer_index(this, this->tail);
        this->current_size--;
        pthread_cond_signal(&this->pop_notification);
        pthread_mutex_unlock(&this->buffer_lock);
        return data;
    }
    return nullptr;
}