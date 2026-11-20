// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/control_queue.hpp"
#include <atomic>
#include <cstdio>
#include <stdexcept>
#include <thread>

int main() {
    grainmm::ControlQueue<unsigned, 4> queue;
    unsigned out = 99;
    if (queue.pop(out) || out != 99 || !queue.push(1) || !queue.push(2) || !queue.push(3) ||
        queue.push(4))
        return 1;
    for (unsigned expected = 1; expected <= 3; ++expected)
        if (!queue.pop(out) || out != expected)
            return 1;
    std::atomic<bool> failed{false};
    std::thread producer([&] {
        for (unsigned i = 0; i < 100000; ++i)
            while (!queue.push(i))
                std::this_thread::yield();
    });
    for (unsigned i = 0; i < 100000; ++i) {
        while (!queue.pop(out))
            std::this_thread::yield();
        if (out != i)
            failed.store(true);
    }
    producer.join();
    if (failed.load() || queue.pop(out))
        return 1;
    std::puts("control queue: full/empty/wrap and 100000 concurrent ordered messages passed");
}
