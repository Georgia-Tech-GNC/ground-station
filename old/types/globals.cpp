#include "globals.h"
#include <atomic>

DataContainer default_data_values;
ContainerDirectory most_recent_data(default_data_values);
 
struct {
    std::atomic_flag synchronization_period = false;
    std::atomic_uint64_t entry_count = 0, exit_count = 0;
    std::atomic_uint64_t head = 1, tail = 0;

    DataContainerUnion queue[INTERNAL_DATA_QUEUE_MEM_SIZE];
} data_queue;


/** Reads data from the queue. This method makes no guarantee about the freshness of the data. 
 * The only guarantee made is that the data will always be valid, if not neccessarily the most recent.
 * 
 * In general, data should be at least as recent as the moment this method is called. 
 * Further guarantees are left to the ordering of atomic reads and writes by the scheduler.
 */
template<typename T>
void read_queue(T& read_target) {
    // Ensure that the synchronization period flag is not high
    // Memory order acquire ensures that the reading portion is not rearranged above this lock
    while (data_queue.synchronization_period.test(std::memory_order_acquire)) { 
        data_queue.synchronization_period.wait(true, std::memory_order_relaxed);
    }

    // We use acq_rel to stop the read from being arranged above this call
    data_queue.entry_count.fetch_add(1, std::memory_order_acq_rel); 

    std::memcpy(read_target, most_recent_data.get<T>().load(std::memory_order_acquire), sizeof(T));

    // We use acq_rel to stop the read from being arranged below this call
    data_queue.exit_count.fetch_add(1, std::memory_order_acq_rel);
}

/** Moves the tail pointer in the queue while ensuring that no stale readers are active during that time.
 * This function does NOT guarantee that all readers are inactive. It only guarantees that readers that began their read
 * before this method was called will finish their reads before the tail pointer is moved.
 */
void delete_queue(uint_fast32_t new_tail_value) {
    data_queue.synchronization_period.test_and_set(std::memory_order_release);

    // Wait for the entry count to equal the exit count.
    // We use acquire on the exit count load to make that updates to entry_count are loaded in order relative to exit_count
    // The reason we spin lock instead of waiting is because this method is called infrequently and usually only locks
    // for very short periods of time.
    // TODO: test if waiting is faster
    while (data_queue.entry_count.load(std::memory_order_relaxed) != data_queue.exit_count.load(std::memory_order_acquire));

    // We use memory order release to make sure that this doesn't happen until the lock above is satisfied 
    // (indicating that there are no stale readers)
    data_queue.tail.store(new_tail_value, std::memory_order_release); 

    data_queue.synchronization_period.clear(std::memory_order_release);
    data_queue.synchronization_period.notify_all();
}

/** Increments the head pointer (potentially blocking if the queue if full) and 
 * returns the index of the newly freed element. 
 */
template<typename T>
void write_queue(const T& data) {
    uint64_t head_cand;
    do {
        head_cand = data_queue.head.load(std::memory_order_relaxed);
        if ((head_cand + 1) % INTERNAL_DATA_QUEUE_MEM_SIZE == data_queue.tail.load(std::memory_order_acquire)) {
            data_queue.tail.wait((head_cand + 1) % INTERNAL_DATA_QUEUE_MEM_SIZE, std::memory_order_relaxed);
        } else {
            if (data_queue.head.compare_exchange_weak(
                head_cand, (head_cand + 1) % INTERNAL_DATA_QUEUE_MEM_SIZE, std::memory_order_relaxed
            )) {
                break;
            }
        }
    } while(true);

    uint_fast32_t target_idx = head_cand;
    std::memcpy(data_queue.queue[target_idx].get<T>(), data, sizeof(T));
    most_recent_data.get<T>().store(data_queue.queue[target_idx].get<T>(), std::memory_order_release);
}