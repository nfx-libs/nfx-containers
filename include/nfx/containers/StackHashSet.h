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
 * @file StackHashSet.h
 * @brief Hash set with small buffer optimization (SBO) and heap fallback
 * @details Stack-allocated storage for small sets (≤N elements) with linear search,
 *          automatic transition to FastHashSet for larger datasets
 */

#pragma once

#include <nfx/containers/FastHashSet.h>

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
    // StackHashSet class
    //=====================================================================

    /**
     * @brief Hash set with small buffer optimization for stack-allocated storage
     * @tparam TKey Key type (supports heterogeneous lookup for compatible types)
     * @tparam N Maximum stack capacity before heap allocation (default: 8)
     * @tparam KeyEqual Key equality comparator (default: std::equal_to<> for transparent comparison)
     *
     * @details StackHashSet uses a hybrid storage strategy:
     *          - Small capacity (≤N): Linear search in stack-allocated array (cache-friendly, zero heap allocations)
     *          - Large capacity (>N): Automatic transition to FastHashSet (Robin Hood hashing on heap)
     *
     *          This is ideal for small sets (config flags, tags, local caches) where heap allocation overhead
     *          would dominate performance. The transition to heap is transparent and automatic.
     */
    template <typename TKey,
        size_t N = 8,
        typename KeyEqual = std::equal_to<>>
    class StackHashSet final
    {
    public:
        //----------------------------------------------
        // STL-compatible type aliases
        //----------------------------------------------

        /** @brief Type alias for key type */
        using key_type = TKey;

        /** @brief Type alias for value type (same as key_type for sets) */
        using value_type = TKey;

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
        inline StackHashSet();

        /**
         * @brief Construct set from initializer_list
         * @param init Initializer list of keys
         */
        inline StackHashSet( std::initializer_list<TKey> init );

        /**
         * @brief Construct set from iterator range
         * @tparam InputIt Input iterator type (must dereference to TKey)
         * @param first Beginning of range to copy from
         * @param last End of range (exclusive)
         */
        template <typename InputIt>
        inline StackHashSet( InputIt first, InputIt last );

        //----------------------------------------------
        // Capacity
        //----------------------------------------------

        /**
         * @brief Check if the set is empty
         * @return true if size() == 0, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        /**
         * @brief Get the number of elements in the set
         * @return Current number of keys stored
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
        // Modifiers
        //----------------------------------------------

        /**
         * @brief Insert a key (copy semantics)
         * @param key The key to insert
         * @return Pair of pointer to element and bool indicating insertion success
         * @details If key already exists, returns pointer to existing element and false.
         *          Otherwise inserts new element and returns pointer with true.
         *          May trigger transition to heap storage.
         */
        inline std::pair<const value_type*, bool> insert( const TKey& key );

        /**
         * @brief Insert a key (move semantics)
         * @param key The key to insert (moved)
         * @return Pair of pointer to element and bool indicating insertion success
         * @details If key already exists, returns pointer to existing element and false.
         *          Otherwise inserts new element and returns pointer with true.
         *          May trigger transition to heap storage.
         */
        inline std::pair<const value_type*, bool> insert( TKey&& key );

        /**
         * @brief Emplace a key with in-place construction
         * @tparam Args Argument types for constructing the key
         * @param args Arguments forwarded to construct the key
         * @return Pair of pointer to element and bool indicating insertion success
         */
        template <typename... Args>
        inline std::pair<const value_type*, bool> emplace( Args&&... args );

        /**
         * @brief Erase element by key
         * @param key The key to erase
         * @return Number of elements erased (0 or 1)
         */
        inline size_t erase( const TKey& key );

        /**
         * @brief Clear all elements from the set
         * @details Clears stack storage or heap storage, does not deallocate heap set
         */
        inline void clear() noexcept;

        /**
         * @brief Extract a key from the set without destroying it
         * @param key The key to extract
         * @return std::optional containing the extracted key if found, std::nullopt otherwise
         * @details Removes the element from the set and returns the key.
         *          If the key is not found, returns std::nullopt.
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline std::optional<TKey> extract( const TKey& key );

        /**
         * @brief Merge another StackHashSet into this one
         * @param other The set to merge from (elements are moved, not copied)
         * @details Attempts to insert each element from other into this set.
         *          Elements that already exist in this set are left in other.
         *          After the operation, other contains only elements that were not inserted.
         */
        inline void merge( StackHashSet& other );

        /**
         * @brief Merge another StackHashSet into this one (rvalue overload)
         * @param other The set to merge from (elements are moved)
         * @details Same as merge(StackHashSet&) but accepts rvalue references.
         */
        inline void merge( StackHashSet&& other );

        //----------------------------------------------
        // Lookup
        //----------------------------------------------

        /**
         * @brief Find key in the set (const pointer version)
         * @param key The key to search for
         * @return Pointer to the key if found, nullptr otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline const TKey* find( const TKey& key ) const noexcept;

        /**
         * @brief Check if a key exists in the set
         * @param key The key to search for
         * @return true if key exists, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline bool contains( const TKey& key ) const;

        /**
         * @brief Check if a key exists in the set (heterogeneous lookup)
         * @tparam K Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return true if key exists, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         * @note Enabled only if KeyEqual supports comparing K with TKey
         */
        template <typename K>
        [[nodiscard]] inline bool contains( const K& key ) const
            requires requires( KeyEqual eq, const K& k, const TKey& t ) { eq( k, t ); };

        /**
         * @brief Access key in the set with bounds checking
         * @param key The key to search for
         * @return Const reference to the stored key
         * @throws std::out_of_range if key is not found
         * @note For sets, at() returns the key itself (useful for retrieving stored key)
         */
        inline const TKey& at( const TKey& key ) const;

    private:
        //----------------------------------------------
        // Internal storage
        //----------------------------------------------

        /**
         * @brief Stack storage entry with optional key
         */
        struct StackEntry
        {
            std::optional<TKey> data; ///< Optional key
        };

        /**
         * @brief Stack-allocated array for small sets
         */
        std::array<StackEntry, N> m_stack;

        /**
         * @brief Current number of elements in stack storage
         */
        size_t m_stackSize;

        /**
         * @brief Heap-allocated FastHashSet for large sets (nullptr when on stack)
         */
        std::unique_ptr<FastHashSet<TKey>> m_heap;

        /**
         * @brief Check if currently using stack storage
         * @return true if on stack, false if on heap
         */
        [[nodiscard]] inline bool isOnStack() const noexcept;

        /**
         * @brief Transition from stack to heap storage
         * @details Allocates FastHashSet, moves all stack entries to heap, clears stack
         */
        inline void transitionToHeap();
    };
} // namespace nfx::containers

#include "nfx/detail/containers/StackHashSet.inl"
