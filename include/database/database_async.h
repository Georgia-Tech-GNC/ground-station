#ifndef DATABASE_ASYNC_H
#define DATABASE_ASYNC_H

#include "buffered_db.h"
#include <string>
#include <thread>
#include <atomic>

class DatabaseAsync {
public:
    explicit DatabaseAsync(const std::string& filename);
    ~DatabaseAsync();

    DatabaseAsync(const DatabaseAsync&) = delete;
    DatabaseAsync& operator=(const DatabaseAsync&) = delete;

    void start();
    void stop();

    void enqueue(const DataEntry& entry, timestamp_t ts);

    template<typename T>
    T latest() { return db.see_most_recent_entry<T>(); }

private:
    BufferedSQLDatabase db;
    std::thread worker;
    std::atomic<bool> running;
};


#endif