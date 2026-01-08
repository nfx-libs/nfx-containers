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
 * @file FastHashSet.h
 * @brief Hash set with Robin Hood hashing and heterogeneous lookup
 * @details Provides a high-performance hash set using Robin Hood algorithm for bounded probe distances
 *          with zero-copy heterogeneous lookup support and configurable hash types (32/64-bit)
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <nfx/Hashing.h>

#include "nfx/detail/containers/CompilerSupport.h"

namespace nfx::containers
{
    //=====================================================================
    // FastHashSet class
    //=====================================================================

    /**
     * @brief Hash set with Robin Hood hashing for bounded probe distances and predictable performance
     * @tparam TKey Key type (supports heterogeneous lookup for compatible types like string_view)
     * @tparam HashType Hash type - either uint32_t or uint64_t (default: uint32_t)
     * @tparam Seed Hash seed value for initialization (default: FNV offset basis for HashType)
     * @tparam THasher Hash functor type (default: hashing::Hasher<HashType, Seed>)
     * @tparam KeyEqual Key equality comparator (default: std::equal_to<> for transparent comparison)
     */
    template <typename TKey,
        hashing::Hash32or64 HashType = uint32_t,
        HashType Seed = ( sizeof( HashType ) == 4 ? hashing::constants::FNV_OFFSET_BASIS_32 : hashing::constants::FNV_OFFSET_BASIS_64 ),
        typename THasher = hashing::Hasher<HashType, Seed>,
        typename KeyEqual = std::equal_to<>>
    class FastHashSet final
    {
        //----------------------------------------------
        // Compile-time type constraints
        //----------------------------------------------

        static_assert( std::is_same_v<HashType, uint32_t> || std::is_same_v<HashType, uint64_t>,
            "HashType must be uint32_t or uint64_t" );

        static_assert( std::is_invocable_r_v<HashType, THasher, TKey>,
            "THasher must be callable with TKey and return HashType" );

        static_assert( std::is_invocable_r_v<bool, KeyEqual, TKey, TKey>,
            "KeyEqual must be callable with two TKey arguments and return bool" );

    public:
        //----------------------------------------------
        // Forward declarations for iterator support
        //----------------------------------------------

        class Iterator;
        class ConstIterator;

        //----------------------------------------------
        // STL-compatible type aliases
        //----------------------------------------------

        /** @brief Type alias for key type */
        using key_type = TKey;

        /** @brief Type alias for value type (same as key for sets) */
        using value_type = TKey;

        /** @brief Type alias for hasher type */
        using hasher = THasher;

        /** @brief Type alias for key equality comparator */
        using key_equal = KeyEqual;

        /** @brief Type alias for hash type (uint32_t or uint64_t) */
        using hash_type = HashType;

        /** @brief Type alias for size type */
        using size_type = size_t;

        /** @brief Type alias for difference type */
        using difference_type = std::ptrdiff_t;

        /** @brief Type alias for iterator */
        using iterator = Iterator;

        /** @brief Type alias for const iterator */
        using const_iterator = ConstIterator;

        //----------------------------------------------
        // Construction
        //----------------------------------------------

        /**
         * @brief Default constructor with initial capacity of 32 elements
         * @details Initializes hash table with power-of-2 capacity for optimal
         *          bitwise operations and cache-friendly memory layout
         */
        inline FastHashSet();

        /**
         * @brief Construct set from initializer_list
         * @param init Initializer list of keys
         */
        inline FastHashSet( std::initializer_list<TKey> init );

        /**
         * @brief Construct set from iterator range
         * @tparam InputIt Input iterator type (must dereference to TKey)
         * @param first Beginning of range to copy from
         * @param last End of range (exclusive)
         */
        template <typename InputIt>
        inline FastHashSet( InputIt first, InputIt last );

        /**
         * @brief Constructor with specified initial capacity
         * @param initialCapacity Minimum initial capacity (rounded up to power of 2)
         * @details Capacity will be rounded up to next power of 2 for optimal
         *          hash distribution and bitwise mask operations
         */
        inline explicit FastHashSet( size_t initialCapacity );

        /**
         * @brief Move constructor
         * @param other Set to move from (will be left in valid but empty state)
         */
        inline FastHashSet( FastHashSet&& other ) noexcept;

        /**
         * @brief Move assignment operator
         * @param other Set to move from (will be left in valid but empty state)
         * @return Reference to this set
         */
        inline FastHashSet& operator=( FastHashSet&& other ) noexcept;

        /**
         * @brief Copy constructor
         */
        FastHashSet( const FastHashSet& ) = default;

        /**
         * @brief Copy assignment operator
         * @return Reference to this set
         */
        FastHashSet& operator=( const FastHashSet& ) = default;

        /**
         * @brief Destructor
         */
        ~FastHashSet() = default;

        //----------------------------------------------
        // Core operations
        //----------------------------------------------

        /**
         * @brief Fast lookup with heterogeneous key types (C++ idiom: pointer return)
         * @tparam KeyType Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return Pointer to the key if found, nullptr otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        template <typename KeyType = TKey>
        [[nodiscard]] inline const TKey* find( const KeyType& key ) const noexcept;

        /**
         * @brief Check if a key exists in the set
         * @tparam KeyType Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return true if key exists, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        template <typename KeyType = TKey>
        [[nodiscard]] inline bool contains( const KeyType& key ) const noexcept;

        /**
         * @brief Checked element access with bounds checking
         * @tparam KeyType Key type (supports heterogeneous lookup)
         * @param key The key to access
         * @return Const reference to the key
         * @throws std::out_of_range if key is not found
         * @note For sets, at() returns the key itself (useful for retrieving stored key)
         */
        template <typename KeyType = TKey>
        inline const TKey& at( const KeyType& key ) const;

        //----------------------------------------------
        // Insertion
        //----------------------------------------------

        /**
         * @brief Insert a key into the set (copy semantics)
         * @param key The key to insert
         * @return true if key was inserted, false if key already exists
         */
        inline bool insert( const TKey& key );

        /**
         * @brief Insert a key into the set (move semantics)
         * @param key The key to insert (moved)
         * @return true if key was inserted, false if key already exists
         */
        inline bool insert( TKey&& key );

        //----------------------------------------------
        // Emplace operations
        //----------------------------------------------

        /**
         * @brief Construct and insert a key in-place
         * @tparam Args Constructor argument types for TKey
         * @param args Arguments forwarded to TKey constructor
         * @return true if key was inserted, false if key already exists
         * @details Constructs key in-place using perfect forwarding, avoiding unnecessary copies
         */
        template <typename... Args>
        inline bool emplace( Args&&... args );

        /**
         * @brief Try to emplace a key if it doesn't exist
         * @tparam Args Variadic template for key constructor arguments
         * @param args Arguments forwarded to TKey constructor (only used if key doesn't exist)
         * @return Pair of iterator to element and bool (true if inserted, false if key existed)
         * @note More efficient than emplace() - doesn't construct key if it already exists
         */
        template <typename... Args>
        inline std::pair<Iterator, bool> tryEmplace( Args&&... args );

        //----------------------------------------------
        // Capacity and memory management
        //----------------------------------------------
        /**
         * @brief Reserve capacity for at least the specified number of elements
         * @param minCapacity Minimum capacity to reserve
         * @details Resizes hash table to accommodate at least minCapacity elements
         *          without triggering automatic resize. Capacity rounded to power of 2.
         *          Rehashes all existing elements to new table layout.
         */
        inline void reserve( size_t minCapacity );

        /**
         * @brief Remove a key from the set
         * @tparam KeyType Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to remove
         * @return true if the key was found and removed, false otherwise
         */
        template <typename KeyType = TKey>
        inline bool erase( const KeyType& key ) noexcept;

        /**
         * @brief Erase element at iterator position
         * @param pos Iterator to element to erase
         * @return Iterator to the element following the erased element
         * @note Iterator becomes invalid after erase
         */
        inline Iterator erase( ConstIterator pos ) noexcept;

        /**
         * @brief Erase range of elements
         * @param first Beginning of range to erase
         * @param last End of range to erase (exclusive)
         * @return Iterator to the element following the last erased element
         */
        inline Iterator erase( ConstIterator first, ConstIterator last ) noexcept;

        /**
         * @brief Clear all elements from the set
         */
        inline void clear() noexcept;

        //----------------------------------------------
        // State inspection
        //----------------------------------------------

        /**
         * @brief Get the number of elements in the set
         * @return Current number of keys stored
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline size_t size() const noexcept;

        /**
         * @brief Get the current capacity of the hash table
         * @return Maximum elements before resize (always power of 2)
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline size_t capacity() const noexcept;

        /**
         * @brief Check if the set is empty
         * @return true if size() == 0, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        /**
         * @brief Swap contents with another set
         * @param other Set to swap with
         * @note noexcept operation - just swaps internal pointers/values
         */
        inline void swap( FastHashSet& other ) noexcept;

        //----------------------------------------------
        // STL-compatible iteration support
        //----------------------------------------------

        /**
         * @brief Get iterator to beginning of occupied buckets
         * @return Iterator pointing to first key
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] Iterator begin() noexcept;

        /**
         * @brief Get const iterator to beginning of occupied buckets
         * @return Const iterator pointing to first key
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] ConstIterator begin() const noexcept;

        /**
         * @brief Get iterator to end (past last occupied bucket)
         * @return Iterator pointing past the last key
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] Iterator end() noexcept;

        /**
         * @brief Get const iterator to end (past last occupied bucket)
         * @return Const iterator pointing past the last key
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] ConstIterator end() const noexcept;

        /**
         * @brief Get const iterator to beginning of occupied buckets (explicit const)
         * @return Const iterator pointing to first key
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] ConstIterator cbegin() const noexcept;

        /**
         * @brief Get const iterator to end (explicit const)
         * @return Const iterator pointing past the last key
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] ConstIterator cend() const noexcept;

        /**
         * @brief Compare two FastHashSets for equality
         * @param other The other FastHashSet to compare with
         * @return true if both sets contain the same keys, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] bool operator==( const FastHashSet& other ) const noexcept;

    private:
        //----------------------------------------------
        // Robin Hood Hashing bucket structure
        //----------------------------------------------

        /**
         * @brief Internal bucket structure for Robin Hood hashing
         */
        struct Bucket
        {
            std::optional<TKey> data; ///< The stored key
            HashType hash{};          ///< Cached hash value for fast comparison (32 or 64-bit)
            uint32_t distance{};      ///< Robin Hood displacement distance from ideal position
            bool occupied{};          ///< Bucket occupancy flag (true = occupied, false = empty)
        };

        /**
         * @brief Initial hash table capacity (power of 2 for bitwise operations)
         * @details 32 elements provides good balance between memory usage and
         *          resize frequency for typical use cases
         */
        static constexpr size_t INITIAL_CAPACITY = 32;

        /**
         * @brief Load factor threshold as percentage (75%)
         * @details Resize triggered when (size * 100) >= (capacity * 75)
         *          Balances space efficiency with probe sequence performance
         */
        static constexpr size_t MAX_LOAD_FACTOR_PERCENT = 75;

        /**
         * @brief Main bucket storage with contiguous memory layout
         * @details Vector provides cache-friendly linear probing and automatic
         *          memory management with strong exception safety guarantees
         */
        std::vector<Bucket> m_buckets;

        size_t m_size{};                       ///< Current number of elements
        size_t m_capacity{ INITIAL_CAPACITY }; ///< Current hash table capacity
        size_t m_mask{ INITIAL_CAPACITY - 1 }; ///< Bitwise mask for hash modulo

        /**
         * @brief Hash function object with zero-space optimization
         * @details Uses high-performance hashing::Hasher functor providing string hashing
         *          and proper integer mixing for optimal Robin Hood performance.
         *          Supports heterogeneous lookup while maintaining excellent hash distribution.
         *          [[no_unique_address]] ensures zero overhead for stateless hashers.
         */
        NFX_CONTAINERS_NO_UNIQUE_ADDRESS hasher m_hasher;

        /**
         * @brief Key equality comparator with zero-space optimization
         * @details Supports transparent heterogeneous comparison (e.g., std::string vs string_view).
         *          [[no_unique_address]] ensures zero overhead for stateless comparators.
         */
        NFX_CONTAINERS_NO_UNIQUE_ADDRESS KeyEqual m_keyEqual;

        //----------------------------------------------
        // Internal implementation
        //----------------------------------------------

        /**
         * @brief Internal insert implementation for const key reference
         * @param key The key to insert (const reference)
         * @return true if key was inserted, false if key already exists
         */
        inline bool insertInternal( const TKey& key );

        /**
         * @brief Internal insert implementation for rvalue key reference
         * @param key The key to insert (rvalue reference - moved)
         * @return true if key was inserted, false if key already exists
         */
        inline bool insertInternal( TKey&& key );

        /**
         * @brief Check if resize is needed based on load factor
         * @return true if current load exceeds MAX_LOAD_FACTOR_PERCENT threshold
         */
        inline bool shouldResize() const noexcept;

        /**
         * @brief Resize hash table to double capacity and rehash all elements
         */
        inline void resize();

        /**
         * @brief Erase element at specific position using backward shift deletion
         * @param pos Position in bucket array to erase
         */
        inline void eraseAtPosition( size_t pos ) noexcept;

        /**
         * @brief Compare keys with heterogeneous lookup support for string types
         * @tparam KeyType1 First key type
         * @tparam KeyType2 Second key type
         * @param k1 First key to compare
         * @param k2 Second key to compare
         * @return true if keys are equal, false otherwise
         */
        template <typename KeyType1, typename KeyType2>
        inline bool keysEqual( const KeyType1& k1, const KeyType2& k2 ) const noexcept;

    public:
        //----------------------------------------------
        // FastHashSet::Iterator class
        //----------------------------------------------

        /**
         * @brief Forward iterator for FastHashSet that skips empty buckets
         * @details Provides STL-compatible iteration over occupied buckets only.
         *          Automatically skips empty buckets during traversal for efficiency.
         */
        class Iterator
        {
            friend class ConstIterator;

        public:
            /** @brief STL iterator category (forward iterator) */
            using iterator_category = std::forward_iterator_tag;

            /** @brief STL iterator value type (key) */
            using value_type = const TKey;

            /** @brief STL iterator difference type */
            using difference_type = std::ptrdiff_t;

            /** @brief STL iterator pointer type */
            using pointer = const TKey*;

            /** @brief STL iterator reference type */
            using reference = const TKey&;

            //---------------------------
            // Construction
            //---------------------------

            /**
             * @brief Default constructor creates an invalid iterator
             */
            Iterator() = default;

            /**
             * @brief Construct iterator from bucket range
             * @param bucket Starting bucket pointer
             * @param end End bucket pointer (one past last bucket)
             */
            inline Iterator( Bucket* bucket, Bucket* end );

            //---------------------------
            // Operations
            //---------------------------

            /**
             * @brief Dereference operator to access key
             * @return Reference to current key
             */
            inline reference operator*() const;

            /**
             * @brief Arrow operator to access key
             * @return Pointer to current key
             */
            inline pointer operator->() const;

            /**
             * @brief Pre-increment operator to advance to next occupied bucket
             * @return Reference to this iterator after advancement
             */
            inline Iterator& operator++();

            /**
             * @brief Post-increment operator to advance to next occupied bucket
             * @return Copy of iterator before advancement
             */
            inline Iterator operator++( int );

            //---------------------------
            // Comparison
            //---------------------------

            /**
             * @brief Equality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to the same bucket
             */
            inline bool operator==( const Iterator& other ) const;

            /**
             * @brief Inequality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to different buckets
             */
            inline bool operator!=( const Iterator& other ) const;

        private:
            //---------------------------
            // Private methods
            //---------------------------

            /**
             * @brief Skip to next occupied bucket
             * @details Advances bucket pointer until an occupied bucket is found or end is reached
             */
            inline void skipToOccupied();

            //---------------------------
            // Private members
            //---------------------------

            Bucket* m_bucket = nullptr; ///< Pointer to current bucket
            Bucket* m_end = nullptr;    ///< Pointer to end sentinel (one past last bucket)
        };

        //----------------------------------------------
        // FastHashSet::ConstIterator class
        //----------------------------------------------

        /**
         * @brief Const forward iterator for FastHashSet that skips empty buckets
         * @details Provides STL-compatible const iteration over occupied buckets only.
         *          Automatically skips empty buckets during traversal for efficiency.
         */
        class ConstIterator
        {
            friend class FastHashSet;

        public:
            /** @brief STL iterator category (forward iterator) */
            using iterator_category = std::forward_iterator_tag;

            /** @brief STL iterator value type (const key) */
            using value_type = const TKey;

            /** @brief STL iterator difference type */
            using difference_type = std::ptrdiff_t;

            /** @brief STL iterator pointer type */
            using pointer = const TKey*;

            /** @brief STL iterator reference type */
            using reference = const TKey&;

            //---------------------------
            // Construction
            //---------------------------

            /**
             * @brief Default constructor creates an invalid iterator
             */
            ConstIterator() = default;

            /**
             * @brief Construct const iterator from bucket range
             * @param bucket Starting bucket pointer
             * @param end End bucket pointer (one past last bucket)
             */
            inline ConstIterator( const Bucket* bucket, const Bucket* end );

            /**
             * @brief Convert from non-const iterator
             * @param it Non-const iterator to convert from
             */
            inline ConstIterator( const Iterator& it );

            //---------------------------
            // Operations
            //---------------------------

            /**
             * @brief Dereference operator to access key
             * @return Const reference to current key
             */
            inline reference operator*() const;

            /**
             * @brief Arrow operator to access key
             * @return Const pointer to current key
             */
            inline pointer operator->() const;

            /**
             * @brief Pre-increment operator to advance to next occupied bucket
             * @return Reference to this iterator after advancement
             */
            inline ConstIterator& operator++();

            /**
             * @brief Post-increment operator to advance to next occupied bucket
             * @return Copy of iterator before advancement
             */
            inline ConstIterator operator++( int );

            //---------------------------
            // Comparison
            //---------------------------

            /**
             * @brief Equality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to the same bucket
             */
            inline bool operator==( const ConstIterator& other ) const;

            /**
             * @brief Inequality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to different buckets
             */
            inline bool operator!=( const ConstIterator& other ) const;

        private:
            //---------------------------
            // Private methods
            //---------------------------

            /**
             * @brief Skip to next occupied bucket
             * @details Advances bucket pointer until an occupied bucket is found or end is reached
             */
            inline void skipToOccupied();

            //---------------------------
            // Private members
            //---------------------------

            const Bucket* m_bucket = nullptr; ///< Pointer to current bucket (const)
            const Bucket* m_end = nullptr;    ///< Pointer to end sentinel (one past last bucket, const)
        };
    };
} // namespace nfx::containers

#include "nfx/detail/containers/FastHashSet.inl"
