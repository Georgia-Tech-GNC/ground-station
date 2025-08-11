#ifndef BUFFERED_DB
#define BUFFERED_DB

#include "sql_commands.h"

#include <atomic>

class BufferedSQLDatabase {

private:
    SQLDatabase db;
    std::atomic_flag terminate;

public:
    BufferedSQLDatabase(const std::string& name): db(name), terminate(false) { }
    BufferedSQLDatabase(const BufferedSQLDatabase&) = delete;

    void add_entry(const DataEntry& data);
    
    void buffered_storage_loop() {
        terminate.clear();

        while (!terminate.test()) {
            // Try to emplace an item.
        }
    }
};

#endif