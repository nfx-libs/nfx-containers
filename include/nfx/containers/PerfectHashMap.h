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
 * @file PerfectHashMap.h
 * @brief Perfect hash map using CHD (Compress, Hash, Displace) algorithm
 * @details Provides O(1) guaranteed lookups for immutable datasets with minimal memory overhead
 *          using the CHD perfect hashing algorithm
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <nfx/Hashing.h>

namespace nfx::containers
{
    //=====================================================================
    // PerfectHashMap class
    //=====================================================================

    /**
     * @brief Perfect hash map using CHD (Compress, Hash, Displace) algorithm for immutable datasets
     * @tparam TKey Key type (supports heterogeneous lookup for compatible types)
     * @tparam TValue Mapped value type
     * @tparam HashType Hash type - either uint32_t or uint64_t (default: uint32_t)
     * @tparam Seed Hash seed value for initialization (default: FNV offset basis for HashType)
     * @tparam Hasher Hash functor type (default: hashing::Hasher<HashType, Seed>)
     * @tparam KeyEqual Key equality comparator (default: std::equal_to<> for transparent comparison)
     *
     * @details Provides O(1) guaranteed lookups with minimal memory overhead using the CHD algorithm.
     *          CHD (Compress, Hash, Displace) creates a perfect hash function where each key maps
     *          to a unique table position with no collisions, enabling true constant-time lookups.
     * @note The map is immutable after construction. Use FastHashMap for mutable scenarios.
     */
    template <typename TKey,
        typename TValue,
        hashing::Hash32or64 HashType = uint32_t,
        HashType Seed = ( sizeof( HashType ) == 4 ? hashing::constants::FNV_OFFSET_BASIS_32 : hashing::constants::FNV_OFFSET_BASIS_64 ),
        typename Hasher = hashing::Hasher<HashType, Seed>,
        typename KeyEqual = std::equal_to<>>
    class PerfectHashMap final
    {
    public:
        //----------------------------------------------
        // Forward declarations for iterator support
        //----------------------------------------------

        class Iterator;

        //----------------------------------------------
        // Type aliases
        //----------------------------------------------

        /** @brief Type alias for key type */
        using key_type = TKey;

        /** @brief Type alias for mapped value type */
        using mapped_type = TValue;

        /** @brief Type alias for key-value pair type */
        using value_type = std::pair<TKey, TValue>;

        /** @brief Type alias for hasher type */
        using hasher = Hasher;

        /** @brief Type alias for key equality comparator */
        using key_equal = KeyEqual;

        /** @brief Type alias for hash type (uint32_t or uint64_t) */
        using hash_type = HashType;

        /** @brief Type alias for signed seed type (used internally for displacement seeds) */
        using seed_type = std::make_signed_t<hash_type>;

        /** @brief Type alias for size type */
        using size_type = size_t;

        /** @brief Type alias for difference type */
        using difference_type = std::ptrdiff_t;

        /** @brief Primary iterator class */
        using iterator = Iterator;

        /** @brief Type alias for const iterator (same as iterator since map is immutable) */
        using const_iterator = Iterator;

        /** @brief Alias for const iterator */
        using ConstIterator = Iterator;

        //----------------------------------------------
        // Construction
        //----------------------------------------------

        /**
         * @brief Constructs a perfect hash map from a vector of key-value pairs
         * @param items Vector of key-value pairs (moved into the map)
         * @throws std::invalid_argument if duplicate keys are detected in items
         * @details Uses CHD (Compress, Hash, Displace) algorithm to build a perfect hash table.
         *          Construction is O(n) expected time. The resulting map is immutable.
         */
        inline explicit PerfectHashMap( std::vector<std::pair<TKey, TValue>>&& items );

        /**
         * @brief Default constructor creates an empty map
         * @details Creates an empty PerfectHashMap with no elements. Use the explicit constructor
         *          with a vector of key-value pairs to build a functional perfect hash map.
         */
        PerfectHashMap() = default;

        /**
         * @brief Copy constructor
         */
        PerfectHashMap( const PerfectHashMap& ) = default;

        /**
         * @brief Move constructor
         */
        PerfectHashMap( PerfectHashMap&& ) noexcept = default;

        //----------------------------------------------
        // Destruction
        //----------------------------------------------

        /**
         * @brief Destructor
         */
        ~PerfectHashMap() = default;

        //----------------------------------------------
        // Assignment
        //----------------------------------------------

        /**
         * @brief Copy assignment operator
         * @return Reference to this map
         */
        PerfectHashMap& operator=( const PerfectHashMap& ) = default;

        /**
         * @brief Move assignment operator
         * @return Reference to this map
         */
        PerfectHashMap& operator=( PerfectHashMap&& ) noexcept = default;

        //----------------------------------------------
        // Comparison
        //----------------------------------------------

        /**
         * @brief Equality comparison operator
         * @param other PerfectHashMap to compare with
         * @return true if both maps contain the same key-value pairs, false otherwise
         */
        [[nodiscard]] inline bool operator==( const PerfectHashMap& other ) const noexcept;

        /**
         * @brief Inequality comparison operator
         * @param other PerfectHashMap to compare with
         * @return true if maps differ in size or contents, false otherwise
         */
        [[nodiscard]] inline bool operator!=( const PerfectHashMap& other ) const noexcept;

        //----------------------------------------------
        // Element access
        //----------------------------------------------

        /**
         * @brief Access element with bounds checking
         * @tparam K Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return Const reference to the mapped value
         * @throws std::out_of_range if key is not found in the map
         * @details Provides checked access to elements. For unchecked access, use find().
         *          Supports heterogeneous lookup (e.g., string_view for string keys).
         */
        template <typename K>
        inline const TValue& at( const K& key ) const;

        //----------------------------------------------
        // Lookup
        //----------------------------------------------

        /**
         * @brief Check if a key exists in the map
         * @tparam K Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return true if key exists in the map, false otherwise
         * @details O(1) guaranteed lookup time using perfect hashing.
         *          Supports heterogeneous lookup (e.g., string_view for string keys).
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        template <typename K>
        [[nodiscard]] inline bool contains( const K& key ) const noexcept;

        /**
         * @brief Fast lookup with heterogeneous key types (C++ idiom: pointer return)
         * @tparam K Key type (supports heterogeneous lookup for compatible types)
         * @param key The key to search for
         * @return Pointer to the value if found, nullptr otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        template <typename K>
        [[nodiscard]] inline const TValue* find( const K& key ) const noexcept;

        //----------------------------------------------
        // Capacity
        //----------------------------------------------

        /**
         * @brief Get the total size of the hash table
         * @return Total number of slots in the hash table (including empty slots)
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline size_type size() const noexcept;

        /**
         * @brief Get the number of elements in the map
         * @return Number of key-value pairs stored in the map
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline size_type count() const noexcept;

        /**
         * @brief Check if the map is empty
         * @return true if size() == 0, false otherwise
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline bool isEmpty() const noexcept;

        //----------------------------------------------
        // Iterators
        //----------------------------------------------

        /**
         * @brief Get iterator to beginning of occupied slots
         * @return Iterator pointing to first key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline Iterator begin() const noexcept;

        /**
         * @brief Get iterator to end (past last occupied slot)
         * @return Iterator pointing past the last key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline Iterator end() const noexcept;

        /**
         * @brief Get const iterator to beginning of occupied slots (explicit const)
         * @return Const iterator pointing to first key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline ConstIterator cbegin() const noexcept;

        /**
         * @brief Get const iterator to end (explicit const)
         * @return Const iterator pointing past the last key-value pair
         * @note This function is marked [[nodiscard]] - the return value should not be ignored
         */
        [[nodiscard]] inline ConstIterator cend() const noexcept;

        //----------------------------------------------
        // Hash policy
        //----------------------------------------------

        /**
         * @brief Get the hash function object
         * @return Copy of the hash function used by the map
         */
        inline hasher hash_function() const;

        /**
         * @brief Get the key equality comparison function object
         * @return Copy of the key equality comparator used by the map
         */
        inline key_equal key_eq() const;

        //----------------------------------------------
        // PerfectHashMap::Iterator class
        //----------------------------------------------

        /**
         * @brief Const forward iterator for PerfectHashMap
         * @details Iterates over occupied slots in the hash table, skipping empty slots.
         *          Since PerfectHashMap is immutable, Iterator is always const.
         */
        class Iterator final
        {
        public:
            /** @brief STL iterator category (forward iterator) */
            using iterator_category = std::forward_iterator_tag;

            /** @brief STL iterator value type (key-value pair) */
            using value_type = std::pair<TKey, TValue>;

            /** @brief STL iterator difference type */
            using difference_type = std::ptrdiff_t;

            /** @brief STL iterator pointer type (const) */
            using pointer = const value_type*;

            /** @brief STL iterator reference type (const) */
            using reference = const value_type&;

            //---------------------------
            // Construction
            //---------------------------

            /**
             * @brief Construct iterator from table pointers and index
             * @param table Pointer to the hash table storage
             * @param occupied Pointer to the occupancy bitmap
             * @param index Starting index in the table
             */
            inline Iterator( const std::vector<std::optional<std::pair<TKey, TValue>>>* table, const std::vector<uint8_t>* occupied, size_t index );

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
             * @brief Pre-increment operator to advance to next occupied slot
             * @return Reference to this iterator after advancement
             */
            inline Iterator& operator++();

            /**
             * @brief Post-increment operator to advance to next occupied slot
             * @return Copy of iterator before advancement
             */
            inline Iterator operator++( int );

            //---------------------------
            // Comparison
            //---------------------------

            /**
             * @brief Equality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to the same position
             */
            inline bool operator==( const Iterator& other ) const;

            /**
             * @brief Inequality comparison operator
             * @param other Iterator to compare with
             * @return true if iterators point to different positions
             */
            inline bool operator!=( const Iterator& other ) const;

        private:
            //---------------------------
            // Private methods
            //---------------------------

            /**
             * @brief Skip to next occupied slot in the table
             * @details Advances m_index until an occupied slot is found or end is reached
             */
            inline void skipEmpty();

            //---------------------------
            // Private members
            //---------------------------

            const std::vector<std::optional<std::pair<TKey, TValue>>>* m_table; ///< Pointer to hash table storage
            const std::vector<uint8_t>* m_occupied;                             ///< Pointer to occupancy bitmap
            size_t m_index;                                                     ///< Current index in the table
        };

    private:
        size_t m_itemCount = 0;                                      ///< Number of key-value pairs in the map
        std::vector<std::optional<std::pair<TKey, TValue>>> m_table; ///< Hash table storage (sparse)
        std::vector<seed_type> m_seeds;                              ///< Displacement seeds per bucket (negative = occupied)
        std::vector<uint8_t> m_occupied;                             ///< Occupancy bitmap (1 = occupied, 0 = empty)
        hasher m_hasher;                                             ///< Hash function object
        KeyEqual m_keyEqual;                                         ///< Key equality comparator
    };
} // namespace nfx::containers

#include "nfx/detail/containers/PerfectHashMap.inl"
