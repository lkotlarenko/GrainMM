// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <array>
#include <atomic>
#include <type_traits>

namespace grainmm {
// Exactly one producer and one consumer. One reserved slot; operations never wait.
template <typename T, unsigned Capacity = 8> class ControlQueue {
    static_assert(Capacity >= 2 && std::is_trivially_copyable<T>::value,
                  "bounded value messages only");
    static_assert(std::atomic<unsigned>::is_always_lock_free, "lock-free indices required");

  public:
    bool push(const T &value) noexcept {
        const auto at = write_.load(std::memory_order_relaxed);
        const auto next = (at + 1) % Capacity;
        if (next == read_.load(std::memory_order_acquire))
            return false;
        values_[at] = value;
        write_.store(next, std::memory_order_release);
        return true;
    }
    bool pop(T &value) noexcept {
        const auto at = read_.load(std::memory_order_relaxed);
        if (at == write_.load(std::memory_order_acquire))
            return false;
        value = values_[at];
        read_.store((at + 1) % Capacity, std::memory_order_release);
        return true;
    }

  private:
    std::array<T, Capacity> values_{};
    std::atomic<unsigned> read_{0}, write_{0};
};
} // namespace grainmm
