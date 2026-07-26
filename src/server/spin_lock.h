#pragma once

#include <atomic>

struct SpinLock {
    void lock() {
        while (mutex.test_and_set()) {
        }
        return;
    }

    void unlock() { mutex.clear(); }

    std::atomic_flag mutex{ATOMIC_FLAG_INIT};
};