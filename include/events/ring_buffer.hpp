// ring_buffer.hpp, created by Andrew Gossen.

// ----
// This is a custom fixed capacity ring buffer, used to queue events for the dispatcher 
// ---- 

#pragma once 
#include <cstddef>

template <typename T, std::size_t capacity>
class RingBuffer{

    public:

    bool push(T&& el){ 
        if (full()) return false;
        m_buffer[m_tail]=std::move(el); // Move into the buffer 
        m_tail=increment(m_tail);
        ++m_count;
        return true;
    }

    bool push(T const& el){  
        if (full()) return false;
        m_buffer[m_tail]=el; // Copies into the buffer 
        m_tail=increment(m_tail);
        ++m_count;
        return true;
    }

    bool pop(T& el){
        if (empty()) return false; 
        el=std::move(m_buffer[m_head]);
        m_head=increment(m_head);
        --m_count;
        return true;
    }

    // Checks
    bool full() const{
        return (m_count==capacity);
    }
    bool empty() const { 
        return (m_count==0);
    }

    // Getter
    std::size_t size() const { 
        return m_count;
    }

    static constexpr std::size_t getCapacity(){
        return capacity;
    }

    private:

    static constexpr std::size_t increment(std::size_t i){
        return (i+1)%capacity; // Wrap around indexing
    }

    std::array<T,capacity> m_buffer{};
    std::size_t m_head{0}; // Index of the oldest element ( i.e. element to pop ) 
    std::size_t m_tail{0}; // Index to the next free slot ( i.e. where to push to ) 
    std::size_t m_count{0}; // Nnumber of elements in the buffer 

};