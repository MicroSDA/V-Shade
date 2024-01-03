#pragma once
#include <algorithm>
#include <functional>
#include <memory>

namespace shade
{
    // Linear map based on stack allocation.
    template <typename Value, size_t MaxCount>
    class LinearArray
    {
    public:
        // Constants and type alias
        static const std::size_t ValueSize = sizeof(Value);
        using DataStorage = std::array<std::uint8_t, ValueSize* MaxCount>;

        // Iterator class for LinearArray
        template<typename Value>
        class Iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = Value;
            using pointer = value_type*;
            using reference = value_type&;
            using const_reference = const reference;
            using const_pointer = const pointer;
        public:
            // Constructor
            Iterator(pointer first = nullptr, pointer last = nullptr, pointer current = nullptr) :
                m_First(first), m_Last(last), m_Current((current) ? current : first) {}
            ~Iterator() = default;
        public:
            // Iterator operations
            Iterator& operator++()                  noexcept { ++m_Current; return (*this); }
            Iterator& operator--()                  noexcept { --m_Current; return (*this); }

            Iterator& operator++(int)               noexcept { Iterator tmp = *this; ++m_Current; return tmp; }
            Iterator& operator--(int)               noexcept { Iterator tmp = *this; --m_Current; return tmp; }

            bool operator==(const Iterator& other)  const noexcept { return other.m_Current == m_Current; }
            bool operator!=(const Iterator& other)  const noexcept { return other.m_Current != m_Current; }

            value_type& operator*()                 noexcept { return *m_Current; }
            value_type* operator->()                noexcept { return m_Current; }

            const value_type& operator*()           const noexcept { return m_Current; }
            const value_type* operator->()          const noexcept { return &m_Current; }

        private:
            // Iterator state
            pointer const m_First;
            pointer const m_Last;
            pointer m_Current;
        };

    public:
        // Constructor and Destructor
        LinearArray() : m_Count(0)
        {
            ClearDataMemeory(MaxCount);
        }
        ~LinearArray() { Reset(); };

        // Iterator aliases
        using iterator = Iterator<Value>;
        using const_iterator = Iterator<const Value>;

        // Iterator access functions
        iterator        begin()     noexcept { return iterator(reinterpret_cast<Value*>(m_DataBuffer.data()), reinterpret_cast<Value*>(m_DataBuffer.data() + m_Count * ValueSize)); };
        iterator        end()       noexcept { return iterator(reinterpret_cast<Value*>(m_DataBuffer.data() + m_Count * ValueSize), reinterpret_cast<Value*>(m_DataBuffer.data() + m_Count * ValueSize)); };
        const_iterator  cbegin()    const noexcept { return const_iterator(reinterpret_cast<Value*>(m_DataBuffer.data()), reinterpret_cast<Value*>(m_DataBuffer.data() + m_Count * ValueSize)); };
        const_iterator  cend()      const noexcept { return const_iterator(reinterpret_cast<Value*>(m_DataBuffer.data() + m_Count * ValueSize), reinterpret_cast<Value*>(m_DataBuffer.data() + m_Count * ValueSize)); };

        // Operator overloads for array access
        Value& operator[](std::size_t index) { if (index >= m_Count) { throw std::out_of_range("LinearArray out of range!"); } return *reinterpret_cast<Value*>(&m_DataBuffer[index * ValueSize]); }
        const Value& operator[](std::size_t index) const { if (index >= m_Count) { throw std::out_of_range("LinearArray out of range!"); } *reinterpret_cast<Value*>(&m_DataBuffer[index * ValueSize]); }

        // Disallow copy and move operations
        LinearArray(const LinearArray&) = delete;
        LinearArray& operator=(const LinearArray&) = delete;
        LinearArray(LinearArray&&) = delete;
        LinearArray& operator=(const LinearArray&&) = delete;

    public:
        // Emplace function for adding elements
        template<typename... Args>
        Value& Emplace(Args&&... args)
        {
            if (m_Count >= MaxCount) { throw std::out_of_range("LinearArray is at maximum capacity"); }
            return *ConstructAt(ValueSize * m_Count++, std::forward<Args>(args)...);
        }

        // Reset function for clearing the container
        void Reset()
        {
            for (std::size_t i = 0; i < m_Count; i++)
            {
                Value* ptr = reinterpret_cast<Value*>(&m_DataBuffer[i * ValueSize]);
                if (ptr != nullptr)  std::destroy_at(ptr);
            }
            ClearDataMemeory(m_Count);
            m_Count = 0;
        }

    private:
        // Data members
        DataStorage m_DataBuffer;
        std::size_t m_Count;

    private:
        // Helper function to construct an element at a given index
        template<typename... Args>
        Value* ConstructAt(std::size_t index, Args&&... args) { return std::construct_at(reinterpret_cast<Value*>(&m_DataBuffer[index]), std::forward<Args>(args)...); }

        // Helper function to construct an element at a given pointer
        template<typename... Args>
        Value* ConstructAt(Value* ptr, Args&&... args) { return std::construct_at(ptr, std::forward<Args>(args)...); }

        // Helper function to clear data memory
        inline void ClearDataMemeory(std::size_t count) { memset(m_DataBuffer.data(), 0, count * ValueSize); }
    };
}