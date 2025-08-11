#ifndef BUFFERED_DB
#define BUFFERED_DB

#include "sql_commands.h"
#include "datatypes.h"
#include "config.h"
#include "MPMCQueue.h"

#include <atomic>
#include <utility>

#include <chrono>
#include <type_traits>

// Verify the resolution of steady_clock
static_assert(
    std::is_same<std::chrono::steady_clock::duration, std::chrono::microseconds>::value ||
    std::is_same<std::chrono::steady_clock::duration, std::chrono::nanoseconds>::value,
    "Steady Clock resolution is not at least in microseconds"
);
static_assert(std::chrono::steady_clock::is_steady, "Steady Clock is not guaranteed to be steady");

/** 
 * A threadsafe wrapper over #SQLDatabase which adds inserts to a bounded length queue instead of 
 * inserting them immediately.
 * 
 * Inserts are handled in batches via the #BufferedSQLDatabase::run_storage_loop method, which
 * should be ran in it's own thread.    
*/
class BufferedSQLDatabase {

private:
    /** Internal SQL Database Handler */
    SQLDatabase db;

    /** Flag to indicate when the update loop should terminate. */
    std::atomic_flag terminate_flag;

    /** MPMC task queue to handle SQL insert operations */
    MPMCQueue<std::pair<DataEntry, timestamp_t>> work_queue;

    /** Last timestamp when a transaction commit occurred */
    std::chrono::time_point<std::chrono::steady_clock> last_transaction_commit;

    /** The number of slots in the work queue used to hold pending insert operations */
    constexpr static uint32_t INTERNAL_WORK_QUEUE_SIZE = 4096;

    /** The maximum period between transaction commits */
    constexpr static std::chrono::nanoseconds transaction_period = std::chrono::nanoseconds((long long)1e7);

    /** Commits all changes in this transaction and begins a new one */
    void complete_transaction();

public:
    BufferedSQLDatabase(const std::string& name);
    BufferedSQLDatabase(const BufferedSQLDatabase&) = delete;

    /** Adds an entry to the queue of insert operations. 
     * 
     * This method is thread-safe and can be run from any number of threads.
     * This method will never block unless the queue is full.
     */
    void add_entry(const DataEntry& data, const timestamp_t timestamp);

    /** Signals that the storage loop should commit any pending changes and gracefully exit. */
    inline void terminate_storage_loop() {
        // We use relaxed memory ordering because the termination signal has no 
        // bearing on anything else in memory.
        terminate_flag.test_and_set(std::memory_order_relaxed); 
    }
    
    /** Runs the storage loop.
     * 
     * This method should run in its own thread. Once started, it will loop infinitely until the 
     * #BufferedSQLDatabase::terminate_storage_loop method is called in another thread.
     */
    void run_storage_loop();
};

#endif