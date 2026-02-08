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
 * @file OrderedHashMap.h
 * @brief Hash map with Robin Hood hashing, heterogeneous lookup, and insertion-order preservation
 * @details Provides a high-performance hash map using Robin Hood algorithm for bounded probe distances
 *          with zero-copy heterogeneous lookup support and stable insertion-order iteration via
 *          intrusive doubly-linked list
 */

#pragma once

#include "nfx/detail/containers/CompilerSupport.h"

#include <nfx/Hashing.h>

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

namespace nfx::containers
{
    //=====================================================================
    // OrderedHashMap class
    //=====================================================================

    /**
     * @brief Hash map with Robin Hood hashing and insertion-order preservation
     * @tparam TKey Key type (supports heterogeneous lookup for compatible types)
     * @tparam TValue Value type
     * @tparam HashType Hash type - uint32_t or uint64_t (default: uint32_t)
     * @tparam Seed Hash seed value for initialization (default: FNV offset basis for HashType)
     * @tparam THasher Hash functor type (default: hashing::Hasher<HashType, Seed>)
     * @tparam KeyEqual Key equality comparator (default: std::equal_to<> for transparent comparison)
     *
     * @details Combines O(1) hash table lookups with stable insertion-order iteration.
     *          Uses Robin Hood hashing for the hash table and an intrusive doubly-linked list
     *          to maintain insertion order. Iteration follows insertion order, not bucket order.
     */
    template <typename TKey,
        typename TValue,
        hashing::Hash32or64 HashType = uint32_t,
        HashType Seed = ( sizeof( HashType ) == 4 ? hashing::constants::FNV_OFFSET_BASIS_32 : hashing::constants::FNV_OFFSET_BASIS_64 ),
        typename THasher = hashing::Hasher<HashType, Seed>,
        typename KeyEqual = std::equal_to<>>
    class OrderedHashMap final
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

        /** @brief Type alias for mapped value type */
        using mapped_type = TValue;

        /** @brief Type alias for key-value pair type */
        using value_type = std::pair<const TKey, TValue>;

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
         */
        inline OrderedHashMap();

        /**
         * @brief Construct map from initializer_list
         * @param init Initializer list of key/value pairs
         */
        inline OrderedHashMap( std::initializer_list<std::pair<TKey, TValue>> init );

        /**
         * @brief Construct map from iterator range
         * @tparam InputIt Input iterator type (must dereference to std::pair-like type)
         * @param first Beginning of range to copy from
         * @param last End of range (exclusive)
         */
        template <typename InputIt>
        inline OrderedHashMap( InputIt first, InputIt last );

        /**
         * @brief Constructor with specified initial capacity
         * @param initialCapacity Minimum initial capacity (rounded up to power of 2)
         */
        inline explicit OrderedHashMap( size_t initialCapacity );

        /**
         * @brief Move constructor
         * @param other Map to move from
         */
        OrderedHashMap( OrderedHashMap&& other ) noexcept;

        /**
         * @brief Move assignment operator
         * @param other Map to move from
         * @return Reference to this map
         */
        OrderedHashMap& operator=( OrderedHashMap&& other ) noexcept;

        /**
         * @brief Copy constructor
         * @param other Map to copy from (preserves insertion order)
         */
        OrderedHashMap( const OrderedHashMap& other );

        /**
         * @brief Copy assignment operator
         * @param other Map to copy from (preserves insertion order)
         * @return Reference to this map
         */
        OrderedHashMap& operator=( const OrderedHashMap& other );

        /**
         * @brief Destructor
         */
        ~OrderedHashMap();

        //----------------------------------------------
        // Core operations
        //----------------------------------------------

        /**
         * @brief Fast lookup with heterogeneous key types (C++ idiom: pointer return)
         * @tparam KeyType Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return Pointer to the value if found, nullptr otherwise
         */
        template <typename KeyType = TKey>
        [[nodiscard]] inline TValue* find( const KeyType& key ) noexcept;

        /**
         * @brief Fast const lookup with heterogeneous key types (C++ idiom: pointer return)
         * @tparam KeyType Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return Const pointer to the value if found, nullptr otherwise
         */
        template <typename KeyType = TKey>
        [[nodiscard]] inline const TValue* find( const KeyType& key ) const noexcept;

        /**
         * @brief Check if a key exists in the map
         * @tparam KeyType Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return true if key exists, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        template <typename KeyType = TKey>
        [[nodiscard]] inline bool contains( const KeyType& key ) const noexcept;

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
         * @details If key doesn't exist, inserts default-constructed value at end of insertion order.
         *          Requires TValue to be default-constructible.
         * @note This function is NOT marked noexcept as it may insert/resize
         */
        inline TValue& operator[]( const TKey& key );

        /**
         * @brief STL-compatible subscript operator with move semantics
         * @param key The key to access or insert (moved if new)
         * @return Reference to the value associated with the key
         * @details If key doesn't exist, inserts default-constructed value at end of insertion order.
         *          Requires TValue to be default-constructible.
         * @note This function is NOT marked noexcept as it may insert/resize
         */
        inline TValue& operator[]( TKey&& key );

        /**
         * @brief Checked element access with bounds checking
         * @tparam KeyType Key type (supports heterogeneous lookup)
         * @param key The key to access
         * @return Reference to the value associated with the key
         * @throws std::out_of_range if key is not found
         */
        template <typename KeyType = TKey>
        inline TValue& at( const KeyType& key );

        /**
         * @brief Checked const element access with bounds checking
         * @tparam KeyType Key type (supports heterogeneous lookup)
         * @param key The key to access
         * @return Const reference to the value associated with the key
         * @throws std::out_of_range if key is not found
         */
        template <typename KeyType = TKey>
        inline const TValue& at( const KeyType& key ) const;

        //----------------------------------------------
        // Insertion
        //----------------------------------------------

        /**
         * @brief Insert a key-value pair only if key doesn't exist (copy semantics)
         * @param key The key to insert
         * @param value The value to associate with the key (copied)
         * @return true if key was inserted, false if key already exists (value unchanged)
         * @note Does NOT overwrite existing values (unlike insertOrAssign)
         * @note New keys are appended to end of insertion order
         */
        inline bool insert( const TKey& key, const TValue& value );

        /**
         * @brief Insert a key-value pair only if key doesn't exist (move semantics)
         * @param key The key to insert
         * @param value The value to associate with the key (moved)
         * @return true if key was inserted, false if key already exists (value unchanged)
         * @note Does NOT overwrite existing values (unlike insertOrAssign)
         * @note New keys are appended to end of insertion order
         */
        inline bool insert( const TKey& key, TValue&& value );

        /**
         * @brief Insert a key-value pair only if key doesn't exist (perfect forwarding)
         * @param key The key to insert (moved)
         * @param value The value to associate with the key (moved)
         * @return true if key was inserted, false if key already exists (value unchanged)
         * @note Does NOT overwrite existing values (unlike insertOrAssign)
         * @note New keys are appended to end of insertion order
         */
        inline bool insert( TKey&& key, TValue&& value );

        /**
         * @brief Insert or update a key-value pair (move semantics)
         * @param key The key to insert or update
         * @param value The value to associate with the key (moved)
         * @note Updating existing keys does NOT change their position in insertion order
         * @note New keys are appended to end of insertion order
         */
        inline void insertOrAssign( const TKey& key, TValue&& value );

        /**
         * @brief Insert or update a key-value pair (copy semantics)
         * @param key The key to insert or update
         * @param value The value to associate with the key (copied)
         * @note Updating existing keys does NOT change their position in insertion order
         * @note New keys are appended to end of insertion order
         */
        inline void insertOrAssign( const TKey& key, const TValue& value );

        /**
         * @brief Insert or update a key-value pair (perfect forwarding for both key and value)
         * @param key The key to insert or update (forwarded)
         * @param value The value to associate with the key (forwarded)
         * @note Updating existing keys does NOT change their position in insertion order
         * @note New keys are appended to end of insertion order
         */
        inline void insertOrAssign( TKey&& key, TValue&& value );

        //----------------------------------------------
        // Emplace operations
        //----------------------------------------------

        /**
         * @brief Emplace a value in-place for the given key
         * @tparam Args Variadic template for value constructor arguments
         * @param key The key to insert or update
         * @param args Arguments forwarded to TValue constructor
         * @note New keys are appended to end of insertion order
         */
        template <typename... Args>
        inline void emplace( const TKey& key, Args&&... args );

        /**
         * @brief Emplace a value in-place for the given key (move key)
         * @tparam Args Variadic template for value constructor arguments
         * @param key The key to insert or update (rvalue reference - moved)
         * @param args Arguments forwarded to TValue constructor
         * @note New keys are appended to end of insertion order
         */
        template <typename... Args>
        inline void emplace( TKey&& key, Args&&... args );

        /**
         * @brief Try to emplace a value if key doesn't exist
         * @tparam Args Variadic template for value constructor arguments
         * @param key The key to insert (const reference)
         * @param args Arguments forwarded to TValue constructor (only used if key doesn't exist)
         * @return Pair of iterator to element and bool (true if inserted, false if key existed)
         * @note More efficient than emplace() - doesn't construct value if key exists
         * @note New keys are appended to end of insertion order
         */
        template <typename... Args>
        inline std::pair<Iterator, bool> tryEmplace( const TKey& key, Args&&... args );

        /**
         * @brief Try to emplace a value if key doesn't exist
         * @tparam Args Variadic template for value constructor arguments
         * @param key The key to insert (rvalue reference - moved only if inserted)
         * @param args Arguments forwarded to TValue constructor (only used if key doesn't exist)
         * @return Pair of iterator to element and bool (true if inserted, false if key existed)
         * @note More efficient than emplace() - doesn't construct value if key exists
         * @note New keys are appended to end of insertion order
         */
        template <typename... Args>
        inline std::pair<Iterator, bool> tryEmplace( TKey&& key, Args&&... args );

        //----------------------------------------------
        // Capacity and memory management
        //----------------------------------------------

        /**
         * @brief Reserve capacity for at least the specified number of elements
         * @param minCapacity Minimum capacity to reserve
         * @note Preserves insertion order during rehashing
         */
        inline void reserve( size_t minCapacity );

        /**
         * @brief Remove a key-value pair from the map
         * @tparam KeyType Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to remove
         * @return true if the key was found and removed, false otherwise
         * @note Preserves insertion order of remaining elements
         */
        template <typename KeyType = TKey>
        inline bool erase( const KeyType& key ) noexcept;

        /**
         * @brief Erase element at iterator position
         * @param pos Iterator to element to erase
         * @return Iterator to the element following the erased element in insertion order
         * @note Iterator becomes invalid after erase
         */
        inline Iterator erase( ConstIterator pos ) noexcept;

        /**
         * @brief Erase range of elements
         * @param first Beginning of range to erase (in insertion order)
         * @param last End of range to erase (exclusive)
         * @return Iterator to the element following the last erased element
         */
        inline Iterator erase( ConstIterator first, ConstIterator last ) noexcept;

        /**
         * @brief Clear all elements from the map
         */
        inline void clear() noexcept;

        /**
         * @brief Extract and remove a key-value pair from the map
         * @tparam KeyType Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to extract
         * @return std::optional containing the extracted key-value pair if found, std::nullopt otherwise
         * @details Removes the key-value pair from the map and returns it. If the key is not found,
         *          returns std::nullopt. This operation invalidates iterators to the extracted element.
         *          The insertion order of remaining elements is preserved.
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        template <typename KeyType = TKey>
        [[nodiscard]] inline std::optional<std::pair<TKey, TValue>> extract( const KeyType& key );

        /**
         * @brief Merge elements from another map into this map
         * @param other Source map to merge from (modified - unique elements are moved out)
         * @details Attempts to move each element from 'other' into this map in insertion order.
         *          Elements that already exist in this map (duplicates) remain in 'other'.
         *          After merge, 'other' contains only elements that were duplicates.
         *          Insertion order is preserved for both maps.
         */
        inline void merge( OrderedHashMap& other );

        /**
         * @brief Merge elements from another map into this map (rvalue overload)
         * @param other Source map to merge from (modified - unique elements are moved out)
         */
        inline void merge( OrderedHashMap&& other );

        //----------------------------------------------
        // State inspection
        //----------------------------------------------

        /**
         * @brief Get the number of elements in the map
         * @return Current number of key-value pairs stored
         */
        [[nodiscard]] inline size_t size() const noexcept;

        /**
         * @brief Get the current capacity of the hash table
         * @return Maximum elements before resize (always power of 2)
         */
        [[nodiscard]] inline size_t capacity() const noexcept;

        /**
         * @brief Check if the map is empty
         * @return true if size() == 0, false otherwise
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        /**
         * @brief Swap contents with another map
         * @param other Map to swap with
         * @note noexcept operation - just swaps internal pointers/values
         */
        inline void swap( OrderedHashMap& other ) noexcept;

        //----------------------------------------------
        // STL-compatible iteration support (insertion order)
        //----------------------------------------------

        /**
         * @brief Get iterator to first element in insertion order
         * @return Iterator pointing to first inserted key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] Iterator begin() noexcept;

        /**
         * @brief Get const iterator to first element in insertion order
         * @return Const iterator pointing to first inserted key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] ConstIterator begin() const noexcept;

        /**
         * @brief Get iterator to end (past last element in insertion order)
         * @return Iterator pointing past the last key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] Iterator end() noexcept;

        /**
         * @brief Get const iterator to end (past last element in insertion order)
         * @return Const iterator pointing past the last key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] ConstIterator end() const noexcept;

        /**
         * @brief Get const iterator to first element in insertion order (explicit const)
         * @return Const iterator pointing to first inserted key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] ConstIterator cbegin() const noexcept;

        /**
         * @brief Get const iterator to end (explicit const)
         * @return Const iterator pointing past the last key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] ConstIterator cend() const noexcept;

        /**
         * @brief Compare two OrderedHashMaps for equality
         * @param other The other OrderedHashMap to compare with
         * @return true if both maps contain the same key-value pairs in the same insertion order
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] bool operator==( const OrderedHashMap& other ) const noexcept;

    private:
        //----------------------------------------------
        // Node structure for doubly-linked list
        //----------------------------------------------

        /**
         * @brief Node structure for intrusive doubly-linked list (insertion order)
         */
        struct Node
        {
            std::pair<TKey, TValue> data; ///< Key-value pair storage
            Node* prev{};                 ///< Previous node in insertion order
            Node* next{};                 ///< Next node in insertion order
            HashType hash{};              ///< Cached hash value (32 or 64-bit)
        };

        //----------------------------------------------
        // Robin Hood Hashing bucket structure
        //----------------------------------------------

        /**
         * @brief Bucket structure for Robin Hood hashing algorithm
         */
        struct Bucket
        {
            Node* node{};        ///< Pointer to node in linked list (nullptr for empty buckets)
            HashType hash{};     ///< Cached hash value (32 or 64-bit)
            uint32_t distance{}; ///< Robin Hood displacement distance
            bool occupied{};     ///< Bucket occupancy flag (fast path check)
        };

        /**
         * @brief Initial hash table capacity (power of 2 for bitwise operations)
         */
        static constexpr size_t INITIAL_CAPACITY = 32;

        /**
         * @brief Load factor threshold as percentage (75%)
         */
        static constexpr size_t MAX_LOAD_FACTOR_PERCENT = 75;

        /**
         * @brief Main bucket storage with contiguous memory layout
         */
        std::vector<Bucket> m_buckets;

        Node* m_head{};                        ///< Head of doubly-linked list (insertion order)
        Node* m_tail{};                        ///< Tail of doubly-linked list (insertion order)
        size_t m_size{};                       ///< Current number of elements
        size_t m_capacity{ INITIAL_CAPACITY }; ///< Current hash table capacity
        size_t m_mask{ INITIAL_CAPACITY - 1 }; ///< Bitwise mask for hash modulo

        /**
         * @brief Hash function object with zero-space optimization
         */
        NFX_CONTAINERS_NO_UNIQUE_ADDRESS hasher m_hasher;

        /**
         * @brief Key equality comparator with zero-space optimization
         */
        NFX_CONTAINERS_NO_UNIQUE_ADDRESS KeyEqual m_keyEqual;

        //----------------------------------------------
        // Internal implementation
        //----------------------------------------------

        /**
         * @brief Internal insert or assign implementation with perfect forwarding (const key)
         * @tparam ValueType Deduced value type supporting move/copy semantics
         * @param key The key to insert or update (const reference)
         * @param value The value to forward (preserves value category)
         */
        template <typename ValueType>
        inline void insertOrAssignInternal( const TKey& key, ValueType&& value );

        /**
         * @brief Internal insert or assign implementation with perfect forwarding (rvalue key)
         * @tparam ValueType Deduced value type supporting move/copy semantics
         * @param key The key to insert or update (rvalue reference - moved)
         * @param value The value to forward (preserves value category)
         */
        template <typename ValueType>
        inline void insertOrAssignInternal( TKey&& key, ValueType&& value );

        /**
         * @brief Check if resize is needed based on load factor
         * @return true if current load exceeds MAX_LOAD_FACTOR_PERCENT threshold
         */
        inline bool shouldResize() const noexcept;

        /**
         * @brief Resize hash table to double capacity and rehash all elements
         * @note Preserves insertion order during rehashing
         */
        inline void resize();

        /**
         * @brief Erase element at specific bucket position using backward shift deletion
         * @param pos Position in bucket array to erase
         */
        inline void eraseAtPosition( size_t pos ) noexcept;

        /**
         * @brief Unlink node from doubly-linked list
         * @param node Node to unlink from the list
         */
        inline void unlinkNode( Node* node ) noexcept;

        /**
         * @brief Append node to end of doubly-linked list
         * @param node Node to append (becomes new tail)
         */
        inline void appendNode( Node* node ) noexcept;

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
        // OrderedHashMap::Iterator class
        //----------------------------------------------

        /**
         * @brief Iterator for OrderedHashMap that follows insertion order
         */
        class Iterator
        {
            friend class ConstIterator;
            friend class OrderedHashMap;

        public:
            /** @brief STL iterator category (bidirectional iterator) */
            using iterator_category = std::bidirectional_iterator_tag;

            /** @brief STL iterator value type (key-value pair) */
            using value_type = std::pair<const TKey, TValue>;

            /** @brief STL iterator difference type */
            using difference_type = std::ptrdiff_t;

            /** @brief STL iterator pointer type */
            using pointer = value_type*;

            /** @brief STL iterator reference type */
            using reference = value_type&;

            //---------------------------
            // Construction
            //---------------------------

            /**
             * @brief Default constructor creates an invalid iterator
             */
            Iterator() = default;

            /**
             * @brief Construct iterator from node
             * @param node Node pointer (nullptr for end iterator)
             */
            inline explicit Iterator( Node* node );

            //---------------------------
            // Operations
            //---------------------------

            /**
             * @brief Dereference operator to access key-value pair
             * @return Reference to current key-value pair
             */
            inline reference operator*() const;

            /**
             * @brief Arrow operator to access key-value pair members
             * @return Pointer to current key-value pair
             */
            inline pointer operator->() const;

            /**
             * @brief Pre-increment operator to advance to next element in insertion order
             * @return Reference to this iterator after advancement
             */
            inline Iterator& operator++();

            /**
             * @brief Post-increment operator to advance to next element in insertion order
             * @return Copy of iterator before advancement
             */
            inline Iterator operator++( int );

            /**
             * @brief Pre-decrement operator to move to previous element in insertion order
             * @return Reference to this iterator after moving backward
             */
            inline Iterator& operator--();

            /**
             * @brief Post-decrement operator to move to previous element in insertion order
             * @return Copy of iterator before moving backward
             */
            inline Iterator operator--( int );

            //---------------------------
            // Comparison
            //---------------------------

            /**
             * @brief Equality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to the same node
             */
            inline bool operator==( const Iterator& other ) const;

            /**
             * @brief Inequality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to different nodes
             */
            inline bool operator!=( const Iterator& other ) const;

        private:
            //---------------------------
            // Private members
            //---------------------------

            /**
             * @brief Private constructor with container pointer for end() decrement support
             * @param node Node pointer (nullptr for end iterator)
             * @param container Pointer to the container (needed for --end())
             */
            inline Iterator( Node* node, const OrderedHashMap* container );

            Node* m_node = nullptr;
            const OrderedHashMap* m_container = nullptr;
        };

        //----------------------------------------------
        // OrderedHashMap::ConstIterator class
        //----------------------------------------------

        /**
         * @brief Const iterator for OrderedHashMap that follows insertion order
         */
        class ConstIterator
        {
            friend class OrderedHashMap;

        public:
            /** @brief STL iterator category (bidirectional iterator) */
            using iterator_category = std::bidirectional_iterator_tag;

            /** @brief STL iterator value type (const key-value pair) */
            using value_type = std::pair<const TKey, TValue>;

            /** @brief STL iterator difference type */
            using difference_type = std::ptrdiff_t;

            /** @brief STL iterator pointer type */
            using pointer = const value_type*;

            /** @brief STL iterator reference type */
            using reference = const value_type&;

            //---------------------------
            // Construction
            //---------------------------

            /**
             * @brief Default constructor creates an invalid iterator
             */
            ConstIterator() = default;

            /**
             * @brief Construct const iterator from node
             * @param node Node pointer (nullptr for end iterator)
             */
            inline explicit ConstIterator( const Node* node );

            /**
             * @brief Convert from non-const iterator
             * @param it Non-const iterator to convert from
             */
            inline ConstIterator( const Iterator& it );

            //---------------------------
            // Operations
            //---------------------------

            /**
             * @brief Dereference operator to access key-value pair
             * @return Const reference to current key-value pair
             */
            inline reference operator*() const;

            /**
             * @brief Arrow operator to access key-value pair members
             * @return Const pointer to current key-value pair
             */
            inline pointer operator->() const;

            /**
             * @brief Pre-increment operator to advance to next element in insertion order
             * @return Reference to this iterator after advancement
             */
            inline ConstIterator& operator++();

            /**
             * @brief Post-increment operator to advance to next element in insertion order
             * @return Copy of iterator before advancement
             */
            inline ConstIterator operator++( int );

            /**
             * @brief Pre-decrement operator to move to previous element in insertion order
             * @return Reference to this iterator after moving backward
             */
            inline ConstIterator& operator--();

            /**
             * @brief Post-decrement operator to move to previous element in insertion order
             * @return Copy of iterator before moving backward
             */
            inline ConstIterator operator--( int );

            //---------------------------
            // Comparison
            //---------------------------

            /**
             * @brief Equality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to the same node
             */
            inline bool operator==( const ConstIterator& other ) const;

            /**
             * @brief Inequality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to different nodes
             */
            inline bool operator!=( const ConstIterator& other ) const;

        private:
            //---------------------------
            // Private members
            //---------------------------

            /**
             * @brief Private constructor with container pointer for end() decrement support
             * @param node Node pointer (nullptr for end iterator)
             * @param container Pointer to the container (needed for --end())
             */
            inline ConstIterator( const Node* node, const OrderedHashMap* container );

            const Node* m_node = nullptr;
            const OrderedHashMap* m_container = nullptr;
        };
    };
} // namespace nfx::containers

#include "nfx/detail/containers/OrderedHashMap.inl"
