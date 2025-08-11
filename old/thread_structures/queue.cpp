#include "queue.h"

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <array>

#include <thread>
#include <iostream>

/** Sets the value of this slot and modifies this slot's rotation to indicate that a write is active.
 * Thread-safe. This method may block until the target rotation has been reached.
 */
template<typename T> void MPSCSlot<T>::write(const T& new_value, uint_fast8_t target_rotation) {
    uint16_t old_value;
    while ((old_value = rotation.load(std::memory_order_acquire)) != 2*target_rotation) {
        rotation.wait(old_value, std::memory_order_relaxed);
    }
    value = new_value;
    rotation.store(target_rotation * 2 + 1, std::memory_order_release);       
    rotation.notify_all();
}

/** Enqueues a value into the queue. This method may block if the queue is full */
template <typename T, uint8_t N, consumer_t<T> C>
void MPSCQueue<T, N, C>::enqueue(const T& value) {
    uint_fast32_t ticket = head.fetch_add(1); // TODO: does this need to be sequentially consistent???
    slots[ticket & TICKET_INDEX_MASK].write(value, TICKET_ROTATION_MASK(ticket));
}

/** "Dequeues" a node without actually reclaiming its memory. 
 * Call flush_dequeue to reclaim memory.
 * 
 * @return true if a value was "dequeued", or false if the queue is empty.
 */
template <typename T, uint8_t N, consumer_t<T> optimist_consumer>
bool MPSCQueue<T, N, optimist_consumer>::try_optimist_dequeue() {
    // Checks if there is something to read.
    // Note that this method automatically performs acquire semantics.
    if (!slots[optimist_tail & TICKET_INDEX_MASK].is_written()) return false; 
    optimist_consumer(slots[optimist_tail++ & TICKET_INDEX_MASK].value);
    return true;
}

/** Matches the tail to the optimist tail and reclaims the memory of the dequeued nodes 
 * This method does nothing if try_optimist_dequeue fails.
 */
template <typename T, uint8_t N, consumer_t<T> C>
void MPSCQueue<T, N, C>::flush_dequeue() {
    while (tail != optimist_tail) {
        slots[tail++ & TICKET_INDEX_MASK].clear();
    }
}