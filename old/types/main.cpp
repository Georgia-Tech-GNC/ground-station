#include "globals.h"
#include <thread>
#include <atomic>
#include <iostream>
#include <syncstream>

const int NUM_WRITES = 5;
const int NUM_READS = 30;

template <typename T> using gen_func_t = void (*)(T&, int);
template <typename T, gen_func_t<T> g> void writer(std::atomic_flag start) {
    while (!start.test(std::memory_order_acquire)) start.wait(false, std::memory_order_relaxed);

    for (int i = 0; i < NUM_WRITES; i++) {
        T value;
        g(T, i);
        write_queue(T);
    }
}

template <typename T> void reader(std::atomic_flag start) {
    while (!start.test(std::memory_order_acquire)) start.wait(false, std::memory_order_relaxed);

    T oldval, newval;
    int count = 0;

    for (int i = 0; i < NUM_READS; i++) {
        read_queue(newval);
        if (newval != oldval) count++;
        oldval = newval;

    }
    std::osyncstream(std::cout) << "Read count: " << count << '\n';
}

template void deleter(std::atomic_flag start) {
    while (!start.test(std::memory_order_acquire)) start.wait(false, std::memory_order_relaxed);

    for (int i = 0; i < NUM_WRITES * 4; i++) {
        while ()
    }

}

template <typename T, gen_func_t<T> g> void writer {
    for (int i = 0; i < NUM_WRITES; i++) {
        T value;
        g(T);
        write_queue(T);
    }
}

int main() {

}
