/*
 * MIT License
 *
 * Copyright (c) 2026 nfx
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
 * @file StackHashMap.h
 * @brief Hash map with small buffer optimization (SBO) and heap fallback
 * @details Stack-allocated storage for small maps (≤N elements) with linear search,
 *          automatic transition to FastHashMap for larger datasets
 */

#pragma once

#include <nfx/containers/FastHashMap.h>

#include <array>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <utility>

namespace nfx::containers
{
    //=====================================================================
    // StackHashMap class
    //=====================================================================

    /**
     * @brief Hash map with small buffer optimization for stack-allocated storage
     * @tparam TKey Key type (supports heterogeneous lookup for compatible types)
     * @tparam TValue Value type
     * @tparam N Maximum stack capacity before heap allocation (default: 8)
     * @tparam KeyEqual Key equality comparator (default: std::equal_to<> for transparent comparison)
     *
     * @details StackHashMap uses a hybrid storage strategy:
     *          - Small capacity (≤N): Linear search in stack-allocated array (cache-friendly, zero heap allocations)
     *          - Large capacity (>N): Automatic transition to FastHashMap (Robin Hood hashing on heap)
     *
     *          This is ideal for small maps (config options, local caches) where heap allocation overhead
     *          would dominate performance. The transition to heap is transparent and automatic.
     */
    template <typename TKey,
        typename TValue,
        size_t N = 8,
        typename KeyEqual = std::equal_to<>>
    class StackHashMap final
    {
    public:
        //----------------------------------------------
        // STL-compatible type aliases
        //----------------------------------------------

        /** @brief Type alias for key type */
        using key_type = TKey;

        /** @brief Type alias for mapped value type */
        using mapped_type = TValue;

        /** @brief Type alias for key-value pair type */
        using value_type = std::pair<const TKey, TValue>;

        /** @brief Type alias for size type */
        using size_type = size_t;

        /** @brief Type alias for difference type */
        using difference_type = std::ptrdiff_t;

        /** @brief Type alias for key equality comparator */
        using key_equal = KeyEqual;

        //----------------------------------------------
        // Construction
        //----------------------------------------------

        /**
         * @brief Default constructor with empty stack storage
         */
        inline StackHashMap();

        /**
         * @brief Construct map from initializer_list
         * @param init Initializer list of key/value pairs
         */
        inline StackHashMap( std::initializer_list<std::pair<TKey, TValue>> init );

        /**
         * @brief Construct map from iterator range
         * @tparam InputIt Input iterator type (must dereference to std::pair-like type)
         * @param first Beginning of range to copy from
         * @param last End of range (exclusive)
         */
        template <typename InputIt>
        inline StackHashMap( InputIt first, InputIt last );

        //----------------------------------------------
        // Capacity
        //----------------------------------------------

        /**
         * @brief Check if the map is empty
         * @return true if size() == 0, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        /**
         * @brief Get the number of elements in the map
         * @return Current number of key-value pairs stored
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline size_t size() const noexcept;

        /**
         * @brief Get the maximum stack capacity before heap allocation
         * @return Stack capacity (template parameter N)
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] static constexpr size_t stackCapacity() noexcept { return N; }

        //----------------------------------------------
        // Element access
        //----------------------------------------------

        /**
         * @brief Checked element access with bounds checking
         * @param key The key to access
         * @return Reference to the value associated with the key
         * @throws std::out_of_range if key is not found
         */
        inline TValue& at( const TKey& key );

        /**
         * @brief Checked const element access with bounds checking
         * @param key The key to access
         * @return Const reference to the value associated with the key
         * @throws std::out_of_range if key is not found
         */
        inline const TValue& at( const TKey& key ) const;

        /**
         * @brief STL-compatible subscript operator (insert-if-missing)
         * @param key The key to access or insert
         * @return Reference to the value associated with the key
         * @details If key doesn't exist, inserts default-constructed value.
         *          Requires TValue to be default-constructible.
         *          May trigger transition to heap storage if stack capacity exceeded.
         * @note This function is NOT marked noexcept as it may insert/resize
         */
        inline TValue& operator[]( const TKey& key );

        /**
         * @brief STL-compatible subscript operator with move semantics
         * @param key The key to access or insert (moved if new)
         * @return Reference to the value associated with the key
         * @details If key doesn't exist, inserts default-constructed value.
         *          Requires TValue to be default-constructible.
         *          May trigger transition to heap storage if stack capacity exceeded.
         * @note This function is NOT marked noexcept as it may insert/resize
         */
        inline TValue& operator[]( TKey&& key );

        //----------------------------------------------
        // Modifiers
        //----------------------------------------------

        /**
         * @brief Insert a key-value pair (copy semantics)
         * @param value The key-value pair to insert
         * @return Pair of pointer to element and bool indicating insertion success
         * @details If key already exists, returns pointer to existing element and false.
         *          Otherwise inserts new element and returns pointer with true.
         *          May trigger transition to heap storage.
         */
        inline std::pair<value_type*, bool> insert( const std::pair<TKey, TValue>& value );

        /**
         * @brief Insert a key-value pair (move semantics)
         * @param value The key-value pair to insert (moved)
         * @return Pair of pointer to element and bool indicating insertion success
         * @details If key already exists, returns pointer to existing element and false.
         *          Otherwise inserts new element and returns pointer with true.
         *          May trigger transition to heap storage.
         */
        inline std::pair<value_type*, bool> insert( std::pair<TKey, TValue>&& value );

        /**
         * @brief Emplace a key-value pair with in-place construction
         * @tparam Args Argument types for constructing the pair
         * @param args Arguments forwarded to construct the pair
         * @return Pair of pointer to element and bool indicating insertion success
         */
        template <typename... Args>
        inline std::pair<value_type*, bool> emplace( Args&&... args );

        /**
         * @brief Insert or assign a key-value pair (copy value)
         * @param key The key to insert or assign to
         * @param value The value to assign
         * @details If key exists, assigns new value. Otherwise inserts new pair.
         *          May trigger transition to heap storage.
         */
        inline void insertOrAssign( const TKey& key, const TValue& value );

        /**
         * @brief Insert or assign a key-value pair (move value)
         * @param key The key to insert or assign to
         * @param value The value to assign (moved)
         * @details If key exists, assigns new value. Otherwise inserts new pair.
         *          May trigger transition to heap storage.
         */
        inline void insertOrAssign( const TKey& key, TValue&& value );

        /**
         * @brief Insert or assign a key-value pair (move key and value)
         * @param key The key to insert or assign to (moved)
         * @param value The value to assign (moved)
         * @details If key exists, assigns new value. Otherwise inserts new pair.
         *          May trigger transition to heap storage.
         */
        inline void insertOrAssign( TKey&& key, TValue&& value );

        /**
         * @brief Erase element by key
         * @param key The key to erase
         * @return Number of elements erased (0 or 1)
         */
        inline size_t erase( const TKey& key );

        /**
         * @brief Clear all elements from the map
         * @details Clears stack storage or heap storage, does not deallocate heap map
         */
        inline void clear() noexcept;

        /**
         * @brief Extract a key-value pair from the map without destroying it
         * @param key The key to extract
         * @return std::optional containing the extracted key-value pair if found, std::nullopt otherwise
         * @details Removes the element from the map and returns the key-value pair.
         *          If the key is not found, returns std::nullopt.
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline std::optional<std::pair<TKey, TValue>> extract( const TKey& key );

        /**
         * @brief Merge another StackHashMap into this one
         * @param other The map to merge from (elements are moved, not copied)
         * @details Attempts to insert each element from other into this map.
         *          Elements that already exist in this map are left in other.
         *          After the operation, other contains only elements that were not inserted.
         */
        inline void merge( StackHashMap& other );

        /**
         * @brief Merge another StackHashMap into this one (rvalue overload)
         * @param other The map to merge from (elements are moved)
         * @details Same as merge(StackHashMap&) but accepts rvalue references.
         */
        inline void merge( StackHashMap&& other );

        //----------------------------------------------
        // Lookup
        //----------------------------------------------

        /**
         * @brief Find a value by key
         * @param key The key to search for
         * @return Pointer to the value if found, nullptr otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline TValue* find( const TKey& key ) noexcept;

        /**
         * @brief Find a value by key (const version)
         * @param key The key to search for
         * @return Const pointer to the value if found, nullptr otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline const TValue* find( const TKey& key ) const noexcept;

        /**
         * @brief Count occurrences of key in map
         * @param key The key to search for
         * @return 1 if key exists, 0 otherwise (maps have unique keys)
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline size_t count( const TKey& key ) const;

        /**
         * @brief Check if a key exists in the map
         * @param key The key to search for
         * @return true if key exists, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline bool contains( const TKey& key ) const;

        /**
         * @brief Check if a key exists in the map (heterogeneous lookup)
         * @tparam K Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return true if key exists, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         * @note Enabled only if KeyEqual supports comparing K with TKey
         */
        template <typename K>
        [[nodiscard]] inline bool contains( const K& key ) const
            requires requires( KeyEqual eq, const K& k, const TKey& t ) { eq( k, t ); };

        //----------------------------------------------
        // Iteration
        //----------------------------------------------

        /**
         * @brief Apply a function to each key-value pair
         * @tparam Func Function type with signature void(const TKey&, TValue&) or compatible
         * @param func Function to apply to each element
         * @details Iterates over all elements in unspecified order.
         *          For stack storage, iteration order matches insertion order (up to N elements).
         *          For heap storage, iteration order is unspecified (hash table order).
         */
        template <typename Func>
        inline void forEach( Func&& func );

        /**
         * @brief Apply a function to each key-value pair (const version)
         * @tparam Func Function type with signature void(const TKey&, const TValue&) or compatible
         * @param func Function to apply to each element
         * @details Iterates over all elements in unspecified order.
         *          For stack storage, iteration order matches insertion order (up to N elements).
         *          For heap storage, iteration order is unspecified (hash table order).
         */
        template <typename Func>
        inline void forEach( Func&& func ) const;

    private:
        //----------------------------------------------
        // Internal storage
        //----------------------------------------------

        /**
         * @brief Stack storage entry with optional pair
         */
        struct StackEntry
        {
            std::optional<std::pair<TKey, TValue>> data; ///< Optional key-value pair
        };

        /**
         * @brief Stack-allocated array for small maps
         */
        std::array<StackEntry, N> m_stack;

        /**
         * @brief Current number of elements in stack storage
         */
        size_t m_stackSize;

        /**
         * @brief Heap-allocated FastHashMap for large maps (nullptr when on stack)
         */
        std::unique_ptr<FastHashMap<TKey, TValue>> m_heap;

        /**
         * @brief Check if currently using stack storage
         * @return true if on stack, false if on heap
         */
        [[nodiscard]] inline bool isOnStack() const noexcept;

        /**
         * @brief Transition from stack to heap storage
         * @details Allocates FastHashMap, moves all stack entries to heap, clears stack
         */
        inline void transitionToHeap();
    };
} // namespace nfx::containers

#include "nfx/detail/containers/StackHashMap.inl"
