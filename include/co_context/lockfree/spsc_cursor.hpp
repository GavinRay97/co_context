#pragma once

#include <co_context/config.hpp>
#include <co_context/utility/as_atomic.hpp>
#include <co_context/utility/bit.hpp>

namespace co_context {

template<std::unsigned_integral T, T capacity>
struct spsc_cursor {
    static_assert(std::has_single_bit(capacity));
    // Check config::swap_capacity_size_t if this assertion fails.
    static_assert(std::atomic<T>::is_always_lock_free);
    inline static constexpr T mask = capacity - 1;
    inline static constexpr bool need_thread_safe =
        config::worker_threads_number != 0;

    T m_head = 0;
    T m_tail = 0;

    [[nodiscard]] inline T head() const noexcept { return m_head & mask; }

    [[nodiscard]] inline T tail() const noexcept { return m_tail & mask; }

    [[nodiscard]] inline T raw_head() const noexcept { return m_head; }

    [[nodiscard]] inline T raw_tail() const noexcept { return m_tail; }

    [[nodiscard]] inline bool is_empty() const noexcept {
        return m_head == m_tail;
    }

    [[nodiscard]] inline T size() const noexcept { return m_tail - m_head; }

    [[nodiscard]] inline T available_number() const noexcept {
        return capacity - (m_tail - m_head);
    }

    [[nodiscard]] inline bool is_available() const noexcept {
        return bool(capacity - (m_tail - m_head));
    }

    [[nodiscard]] inline T load_head() const noexcept {
        if constexpr (need_thread_safe) {
            return as_c_atomic(m_head).load(std::memory_order_acquire) & mask;
        } else {
            return head();
        }
    }

    [[nodiscard]] inline T load_tail() const noexcept {
        if constexpr (need_thread_safe) {
            return as_c_atomic(m_tail).load(std::memory_order_acquire) & mask;
        } else {
            return tail();
        }
    }

    [[nodiscard]] inline T load_raw_head() const noexcept {
        if constexpr (need_thread_safe) {
            return as_c_atomic(m_head).load(std::memory_order_acquire);
        } else {
            return raw_head();
        }
    }

    [[nodiscard]] inline T load_raw_tail() const noexcept {
        if constexpr (need_thread_safe) {
            return as_c_atomic(m_tail).load(std::memory_order_acquire);
        } else {
            return raw_tail();
        }
    }

    [[nodiscard]] inline T load_raw_tail_relaxed() const noexcept {
        if constexpr (need_thread_safe) {
            return as_c_atomic(m_tail).load(std::memory_order_relaxed);
        } else {
            return raw_tail();
        }
    }

    // inline void set_raw_tail(T tail) const noexcept { m_tail = tail; }

    inline void store_raw_tail(T tail) noexcept {
        if constexpr (need_thread_safe) {
            as_atomic(m_tail).store(tail, std::memory_order_release);
        } else {
            m_tail = tail;
        }
    }

    [[nodiscard]] inline bool is_empty_load_head() const noexcept {
        return m_tail == load_raw_head();
    }

    [[nodiscard]] inline bool is_empty_load_tail() const noexcept {
        return m_head == load_raw_tail();
    }

    [[nodiscard]] inline bool is_empty_load_tail_relaxed() const noexcept {
        return m_head == load_raw_tail_relaxed();
    }

    [[nodiscard]] inline bool is_available_load_head() const noexcept {
        return bool(capacity - (m_tail - load_raw_head()));
    }

    inline void wait_for_available() const noexcept {
        const T head_full = m_tail - capacity;
        if constexpr (!need_thread_safe) {
            assert(head_full != load_raw_head());
            return;
        }
        if constexpr (config::is_using_wait_and_notify) {
            as_c_atomic(m_head).wait(head_full, std::memory_order_acquire);
        } else {
            while (head_full == load_raw_head()) {}
        }
    }

    inline void wait_for_not_empty() const noexcept {
        const T tail_empty = m_head;
        if constexpr (!need_thread_safe) {
            assert(tail_empty != load_raw_tail());
            return;
        }
        if constexpr (config::is_using_wait_and_notify) {
            as_c_atomic(m_tail).wait(tail_empty, std::memory_order_acquire);
        } else {
            while (tail_empty == load_raw_tail()) {}
        }
    }

    inline void push(T num = 1) noexcept {
        if constexpr (need_thread_safe) {
            as_atomic(m_tail).store(m_tail + num, std::memory_order_release);
        } else {
            m_tail += num;
        }
    }

    inline void pop(T num = 1) noexcept {
        if constexpr (need_thread_safe) {
            as_atomic(m_head).store(m_head + num, std::memory_order_release);
        } else {
            m_head += num;
        }
    }

    inline void push_notify(T num = 1) noexcept {
        push(num);
        if constexpr (need_thread_safe && config::is_using_wait_and_notify) {
            as_atomic(m_tail).notify_one();
        }
    }

    inline void pop_notify(T num = 1) noexcept {
        pop(num);
        if constexpr (need_thread_safe && config::is_using_wait_and_notify) {
            as_atomic(m_head).notify_one();
        }
    }
};

} // namespace co_context
