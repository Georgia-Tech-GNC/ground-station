#ifndef QUEUE_H
#define QUEUE_H

#include <cstdint>
#include <cstddef>
#include <atomic>
#include <array>

#if defined(__cpp_lib_hardware_interference_size) && !defined(__APPLE__)
    static constexpr size_t hardwareInterferenceSize =
        std::hardware_destructive_interference_size;
#else
    static constexpr size_t hardwareInterferenceSize = 64;
    #pragma message("WARNING: Defaulting hardware interference size to 64 bits")
#endif

/** A fully thread-safe container that stores an item of type T.  
 * 
 * @param T The type of data stored in this struct.
*/
template <typename T> struct MPSCSlot {
    private:
        alignas(hardwareInterferenceSize) std::atomic_uint8_t rotation;

    public:
        T value;

        MPSCSlot() : rotation(0) { }

        /** Sets the value of this slot and modifies this slot's rotation to indicate that a write is active.
         * Thread-safe. This method may block until the target rotation has been reached.
         */
        void write(const T& new_value, uint_fast8_t target_rotation);

        /** Sets the state of this slot to empty. This method will never block. */
        void clear() {
            rotation.fetch_add(1, std::memory_order_release);
            rotation.notify_all();
        }

        /** Returns true if this slot actively stores a value, and false otherwise.
         * This method will never block.
         */
        bool is_written() {
            return rotation.load(std::memory_order_acquire) % 2 == 1;
        }
};


/** consumer_t is a function type that takes in a single const reference to an object T and returns nothing. */
template <typename T> using consumer_t = void (*)(const T&);


/** A simple and efficient multi-producer, single-consumer queue optimized for weakly-ordered memory models.
 * 
 * The consumer method must be passed in as a template parameter to the constructor.
 * To dequeue an item, call try_optimist_dequeue. This method will call the consumer on the 
 * first-enqueued item (if it exists).
 * 
 * Note that the memory for this item will not be reclaimed.
 * To reclaim memory, call flush_dequeue(). This will reclaim the memory of all items dequeued using the 
 * try_optimist_dequeue method.
 * 
 * @param T                 The type of data stored in the queue
 * @param QUEUE_SIZE_BITS   The log2 value of the queue size. Note that the actual size is 2^QUEUE_SIZE_BITS
 * @param optimist_consumer The method called when dequeuing.
 */
template <typename T, uint8_t QUEUE_SIZE_BITS, consumer_t<T> optimist_consumer>
class MPSCQueue {
    /** Internal array of slots */
    std::array<MPSCSlot<T>, (1<<QUEUE_SIZE_BITS) + 1> slots;

    /** The head ticket pointer. The last QUEUE_SIZE_BITS of this value point to the head, 
     * while the seven bits before it indicate the rotation. */
    alignas(hardwareInterferenceSize) std::atomic_uint_fast32_t head;
    /** The reclamation tail pointer. Only the last QUEUE_SIZE_BITS matter. */
    uint32_t tail;
    /** The tail pointer */
    uint32_t optimist_tail;

    static constexpr uint_fast32_t TICKET_INDEX_MASK = (1<<(QUEUE_SIZE_BITS)) - 1;
    static constexpr uint_fast32_t SKEWED_TICKET_ROTATION_MASK = ((1<<(QUEUE_SIZE_BITS + 7)) - 1) & ~TICKET_INDEX_MASK;

    static inline uint_fast32_t TICKET_ROTATION_MASK(uint_fast32_t val) {
        return (SKEWED_TICKET_ROTATION_MASK & val) >> QUEUE_SIZE_BITS;
    }

    public:
        MPSCQueue(): head(0), tail(0), optimist_tail(0) { }

        /** Disallow copying or moving this class */
        MPSCQueue(const MPSCQueue& other) = delete;
        MPSCQueue(MPSCQueue&& other) = delete;

        /** Enqueues a value into the queue. This method may block if the queue is full */
        void enqueue(const T& value);

        /** "Dequeues" a node without actually reclaiming its memory. 
         * Call flush_dequeue to reclaim memory.
         * 
         * @return true if a value was "dequeued", or false if the queue is empty.
         */
        bool try_optimist_dequeue();

        /** Matches the tail to the optimist tail and reclaims the memory of the dequeued nodes 
         * This method does nothing if try_optimist_dequeue has not been called.
         */
        void flush_dequeue();
};
#endif