#include "database_async.h"

DatabaseAsync::DatabaseAsync(const std::string& filename)
    : db(filename), running(false) {}

DatabaseAsync::~DatabaseAsync() {
    stop();
}

void DatabaseAsync::start() {
    if (running.exchange(true)) return;
    worker = std::thread([this] {
        db.run_storage_loop();
    });
}

void DatabaseAsync::stop() {
    if (!running.exchange(false)) return;
    db.terminate_storage_loop();
    if (worker.joinable()) {
        worker.join();
    }
}

void DatabaseAsync::enqueue(const DataEntry& entry, timestamp_t ts) {
    db.add_entry(entry, ts);
}