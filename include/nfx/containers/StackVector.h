/*
 * MIT License
 *
 * Copyright (c) 2025 nfx
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file StackVector.h
 * @brief Stack vector optimization with stack storage and heap fallback
 * @details Provides vector-like container optimized for small sizes (N elements on stack).
 *          Automatically falls back to heap allocation if size exceeds N.
 */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace nfx::containers
{
    /**
     * @brief Vector with stack storage optimization
     * @tparam T Element type
     * @tparam N Maximum number of elements to store on stack before heap allocation
     * @details Stores up to N elements on the stack for optimal cache locality and zero heap allocations.
     *          Automatically transitions to heap storage when size exceeds N.
     */
    template <typename T, std::size_t N = 8>
    class StackVector final
    {
    public:
        //----------------------------------------------
        // Type aliases
        //----------------------------------------------

        /** @brief Type alias for element type */
        using value_type = T;

        /** @brief Type alias for size type */
        using size_type = std::size_t;

        /** @brief Type alias for difference type */
        using difference_type = std::ptrdiff_t;

        /** @brief Type alias for reference to element */
        using reference = T&;

        /** @brief Type alias for const reference to element */
        using const_reference = const T&;

        /** @brief Type alias for pointer to element */
        using pointer = T*;

        /** @brief Type alias for const pointer to element */
        using const_pointer = const T*;

        //----------------------------------------------
        // Construction
        //----------------------------------------------

        /** @brief Default constructor */
        inline StackVector() noexcept;

        /** @brief Construct from initializer list
         *  @param init Initializer list of elements
         */
        inline StackVector( std::initializer_list<T> init );

        /** @brief Copy constructor
         *  @param other The StackVector to copy from
         */
        inline StackVector( const StackVector& other );

        /** @brief Move constructor
         *  @param other The StackVector to move from
         */
        inline StackVector( StackVector&& other ) noexcept;

        /** @brief Destructor */
        inline ~StackVector() noexcept;

        /**
         * @brief Copy assignment operator
         * @param other The StackVector to copy from
         * @return Reference to this object
         */
        inline StackVector& operator=( const StackVector& other );

        /**
         * @brief Move assignment operator
         * @param other The StackVector to move from
         * @return Reference to this object
         */
        inline StackVector& operator=( StackVector&& other ) noexcept;

        /**
         * @brief Equality comparison operator
         * @param other The StackVector to compare with
         * @return true if equal, false otherwise
         */
        inline bool operator==( const StackVector& other ) const noexcept;

        /**
         * @brief Inequality comparison operator
         * @param other The StackVector to compare with
         * @return true if not equal, false otherwise
         */
        inline bool operator!=( const StackVector& other ) const noexcept;

        /**
         * @brief Add element to end (copy)
         * @param value The element to add
         */
        inline void push_back( const T& value );

        /**
         * @brief Add element to end (move)
         * @param value The element to add
         */
        inline void push_back( T&& value );

        /**
         * @brief Construct element in-place at end
         * @tparam Args Types of arguments to forward to constructor
         * @param args Arguments to forward to element constructor
         */
        template <typename... Args>
        inline void emplace_back( Args&&... args );

        /**
         * @brief Access element at position (no bounds checking)
         * @param pos Position of element
         * @return Reference to element
         */
        inline reference operator[]( size_type pos ) noexcept;

        /**
         * @brief Access element at position (no bounds checking) const
         * @param pos Position of element
         * @return Const reference to element
         */
        inline const_reference operator[]( size_type pos ) const noexcept;

        /**
         * @brief Access element at position (with bounds checking)
         * @param pos Position of element
         * @return Reference to element
         * @throws std::out_of_range if pos >= size()
         */
        inline reference at( size_type pos );

        /**
         * @brief Access element at position (with bounds checking) const
         * @param pos Position of element
         * @return Const reference to element
         * @throws std::out_of_range if pos >= size()
         */
        inline const_reference at( size_type pos ) const;

        /**
         * @brief Access last element
         * @return Reference to last element
         */
        inline reference back() noexcept;

        /**
         * @brief Access last element const
         * @return Const reference to last element
         */
        inline const_reference back() const noexcept;

        /**
         * @brief Access first element
         * @return Reference to first element
         */
        inline reference front() noexcept;

        /**
         * @brief Access first element const
         * @return Const reference to first element
         */
        inline const_reference front() const noexcept;

        /**
         * @brief Get number of elements
         * @return Number of elements in container
         */
        inline typename StackVector<T, N>::size_type size() const noexcept;

        /**
         * @brief Check if container is empty
         * @return true if size() == 0, false otherwise
         */
        inline bool isEmpty() const noexcept;

        /**
         * @brief Get current capacity
         * @return Number of elements that can be held without reallocation
         */
        inline size_type capacity() const noexcept;

        /**
         * @brief Resize container to specified size
         * @param count New size
         */
        inline void resize( size_type count );

        /**
         * @brief Resize container to specified size with default value
         * @param count New size
         * @param value Value to initialize new elements with
         */
        inline void resize( size_type count, const T& value );

        /**
         * @brief Reserve storage for at least specified capacity
         * @param newCapacity Minimum capacity to reserve
         */
        inline void reserve( size_type newCapacity );

        /**
         * @brief Get pointer to underlying data
         * @return Pointer to first element
         */
        inline pointer data() noexcept;

        /**
         * @brief Get const pointer to underlying data
         * @return Const pointer to first element
         */
        inline const_pointer data() const noexcept;

        /**
         * @brief Get iterator to beginning
         * @return Iterator to first element
         */
        inline pointer begin() noexcept;

        /**
         * @brief Get const iterator to beginning
         * @return Const iterator to first element
         */
        inline const_pointer begin() const noexcept;

        /**
         * @brief Get iterator to end
         * @return Iterator to one past last element
         */
        inline pointer end() noexcept;

        /**
         * @brief Get const iterator to end
         * @return Const iterator to one past last element
         */
        inline const_pointer end() const noexcept;

        /**
         * @brief Get const iterator to beginning
         * @return Const iterator to first element
         */
        inline const_pointer cbegin() const noexcept;

        /**
         * @brief Get const iterator to end
         * @return Const iterator to one past last element
         */
        inline const_pointer cend() const noexcept;

        /**
         * @brief Clear all elements
         */
        inline void clear() noexcept;

        /**
         * @brief Remove last element
         */
        inline void pop_back() noexcept;

    private:
        /** @brief Transition from stack to heap storage */
        inline void transitionToHeap();

        /** @brief Get pointer to stack storage */
        inline T* stackData() noexcept;

        /** @brief Get const pointer to stack storage */
        inline const T* stackData() const noexcept;

        /** @brief Get reference to heap storage */
        inline std::vector<T>& heapData() noexcept;

        /** @brief Get const reference to heap storage */
        inline const std::vector<T>& heapData() const noexcept;

        size_type m_size;  ///< Number of elements currently stored
        bool m_usingStack; ///< True if using stack storage, false if using heap

        union
        {
            alignas( T ) unsigned char m_stackStorage[N * sizeof( T )];                      ///< Stack-based storage for small arrays
            alignas( std::vector<T> ) unsigned char m_heapStorage[sizeof( std::vector<T> )]; ///< Heap-based storage (std::vector) for large arrays
        };
    };
} // namespace nfx::containers

#include "nfx/detail/containers/StackVector.inl"
