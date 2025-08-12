#include <iostream>

#include "buffered_db.h"
#include "datatypes.h"
#include "web_server.h"

#include <chrono>
#include <thread>
#include <atomic>
#include <type_traits>

int main() {
    std::cout << "Hello World!\n";

    BufferedSQLDatabase db("data.db");

    std::thread t1(&BufferedSQLDatabase::run_storage_loop, std::ref(db));

    db.add_entry(DataEntry(RocketKinematicsData(.1, .2, .3, 12, 13)), 0);
    db.add_entry(DataEntry(RocketKinematicsData(.2, .2, .3, 12, 13)), 1);
    db.add_entry(DataEntry(RocketKinematicsData(.3, .2, .3, 12, 13)), 2);

    std::this_thread::sleep_for(std::chrono::nanoseconds((long long)1e5));
    web_server::initialize_web_server("0.0.0.0:8000", db);

    db.terminate_storage_loop();
    t1.join();

    std::atomic_flag stop_flag(false);
    web_server::run_web_server_loop(stop_flag);

    std::cout << "complete!";
}