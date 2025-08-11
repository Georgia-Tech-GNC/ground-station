#include "buffered_db.h"

void BufferedSQLDatabase::add_entry(const DataEntry& data, const timestamp_t timestamp) {
    most_recent_entries.store(data);
    work_queue.push(std::make_pair(data, timestamp));
}

void BufferedSQLDatabase::run_storage_loop() {
    std::pair<DataEntry, timestamp_t> target;

    while (!terminate_flag.test(std::memory_order_relaxed)) {
        if (std::chrono::steady_clock::now() - last_transaction_commit > transaction_period) {
            complete_transaction();
        }

        if (work_queue.try_pop(target)) {
            db.insert_data(target.first, target.second);

        } else {
            // If the work queue is empty, end the transaction to commit all changes.
            complete_transaction();

            if (work_queue.pop(target, terminate_flag)) {
                db.insert_data(target.first, target.second);
            } else {
                break; // Termination flag was set
            }
        }
    }
    terminate_flag.clear(std::memory_order_seq_cst); // Clear the flag so we can run this loop again if needed.
}

BufferedSQLDatabase::BufferedSQLDatabase(const std::string& name): 
    db(name), terminate_flag(false), work_queue(INTERNAL_WORK_QUEUE_SIZE), last_transaction_commit(std::chrono::steady_clock::now()) 
{ 
    db.begin_transaction();
}

void BufferedSQLDatabase::complete_transaction() {
    db.close_transaction();
    db.begin_transaction();
    last_transaction_commit = std::chrono::steady_clock::now();
}
