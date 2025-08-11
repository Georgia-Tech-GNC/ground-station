
#include <atomic>

template<typename T, int N> class RCU {
    T items[N];
    std::atomic_uint32_t next_write;
};

