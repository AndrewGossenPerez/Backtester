// ring_buffer.hpp, created by Andrew Gossen.

// ----
// This is a custom fixed capacity ring buffer, used to queue events for the dispatcher 
// ---- 

#pragma once 
#include <cstddef>
#include <array>
#include <utility>

template <typename T, std::size_t capacity>
class RingBuffer{

    static_assert(capacity > 0, "Ring Buffer capacity must be > 0");

    public:

    template<class U> // For reference collapsing 
    bool push(U&& el) { // Perfect forwarding to preserve value category
        if (full()) return false;
        m_buffer[m_tail] = std::forward<U>(el); // If rvalue will move if possible, else copies (lvalue)
        m_tail = increment(m_tail);
        ++m_count;
        return true;
    }

    bool pop(T& el) {
        if (empty()) return false;
        el = std::move(m_buffer[m_head]);
        m_head = increment(m_head);
        --m_count;
        return true;
    }

    template<class U>
    void overwrite(U&& el) {
        if (!full()) {
            push(std::forward<U>(el));
        } else {
            // Overwrite oldest element and advance
            m_buffer[m_head] = std::forward<U>(el);
            m_head = increment(m_head);
            m_tail = increment(m_tail);
            // m_count stays the same
        }
    }

    // Checks
    constexpr bool full() const noexcept {
        return (m_count==capacity);
    }
    constexpr bool empty() const noexcept { 
        return (m_count==0);
    }

    // Getter
    std::size_t size() const noexcept { 
        return m_count;
    }

    T& front() noexcept { 
        return m_buffer[m_head];
    }

    static constexpr std::size_t getCapacity() noexcept {
        return capacity;
    }

    private:
    
    static constexpr std::size_t increment(std::size_t i) noexcept { 
        // Faster implementation of wrap-around indexing avoiding division
        ++i;
        if (i == capacity) i = 0;
        return i;
    }

    std::array<T,capacity> m_buffer{}; // Reserve a contiguous buffer of size capacity 
    std::size_t m_head{0}; // Index of the oldest element ( i.e. element to pop ) 
    std::size_t m_tail{0}; // Index to the next free slot ( i.e. where to push to ) 
    std::size_t m_count{0}; // Nnumber of elements in the buffer 

};