#include <atomic>

class ReaderAccessTicket {
    
    std::atomic_flag& parent_synchronized;

    void enter_critical_section() {

    }

};

template<int N> class ReaderSynchronizationLock {
    ReaderAccessTicket readers[N];

    void 
}; 