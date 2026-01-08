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
 * @file PerfectHashMap.inl
 * @brief Template implementation file for PerfectHashMap perfect hashing container
 * @details Contains template method implementations for CHD (Compress, Hash, Displace) algorithm
 *          providing O(1) guaranteed lookups for immutable datasets with minimal memory overhead
 */

#include <algorithm>
#include <stdexcept>
#include <unordered_set>

namespace nfx::containers
{
    //=====================================================================
    // PerfectHashMap class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    template <typename TKey,
        typename TValue,
        hashing::Hash32or64 HashType,
        HashType Seed,
        typename Hasher,
        typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::PerfectHashMap(
        std::vector<std::pair<TKey, TValue>>&& items )
        : m_itemCount{ items.size() },
          m_hasher{},
          m_keyEqual{}
    {
        const size_t itemCount = items.size();

        if ( itemCount == 0 )
        {
            return;
        }

        // O(n) duplicate check to prevent infinite loops during CHD construction
        std::unordered_set<TKey, hasher, key_equal> seen( itemCount, m_hasher, m_keyEqual );
        for ( size_t i = 0; i < itemCount; ++i )
        {
            if ( !seen.insert( items[i].first ).second )
            {
                throw std::invalid_argument( "PerfectHashMap: duplicate keys detected" );
            }
        }

        size_t tableSize = 1;
        while ( tableSize < itemCount )
        {
            tableSize <<= 1;
        }
        tableSize <<= 1;

        m_table.resize( tableSize );
        m_seeds.resize( tableSize, 0 );
        m_occupied.resize( tableSize, 0 );

        std::vector<std::vector<std::pair<size_t, hash_type>>> buckets( tableSize );
        // Compute hashes and fill buckets in one pass
        for ( size_t i = 0; i < itemCount; ++i )
        {
            hash_type h = m_hasher( items[i].first );
            size_t bucketIndex = h & ( tableSize - 1 );
            buckets[bucketIndex].emplace_back( i, h );
        }

        // Create indexed buckets for sorting (preserve original bucket indices)
        std::vector<std::pair<size_t, std::vector<std::pair<size_t, hash_type>>*>> indexedBuckets;
        indexedBuckets.reserve( itemCount );
        for ( size_t i = 0; i < tableSize; ++i )
        {
            if ( !buckets[i].empty() )
            {
                indexedBuckets.emplace_back( i, &buckets[i] );
            }
        }

        // Sort by bucket size (largest first)
        std::sort( indexedBuckets.begin(), indexedBuckets.end(),
            []( const auto& a, const auto& b ) {
                return a.second->size() > b.second->size();
            } );

        // Process buckets in sorted order (largest to smallest)
        for ( const auto& [bucketIndex, bucketPtr] : indexedBuckets )
        {
            const auto& bucket = *bucketPtr;

            if ( bucket.size() == 1 )
            {
                // Single-item bucket: place directly at bucket index
                size_t itemIndex = bucket[0].first;
                m_table[bucketIndex].emplace( std::move( items[itemIndex] ) );
                m_occupied[bucketIndex] = 1;
                m_seeds[bucketIndex] = -static_cast<seed_type>( bucketIndex + 1 );
            }
            else
            {
                // Multi-item bucket: find seed for perfect hashing
                bool seedFound = false;
                size_t originalBucketIndex = bucket[0].second & ( tableSize - 1 );

                for ( hash_type seed = 1; !seedFound; ++seed )
                {
                    std::vector<size_t> positions;
                    positions.reserve( bucket.size() );
                    bool collision = false;

                    for ( const auto& [itemIndex, hashValue] : bucket )
                    {
                        hash_type finalHash = hashing::seedMix<hash_type>( seed, hashValue, tableSize );
                        size_t pos = finalHash & ( tableSize - 1 );

                        // Check if position is occupied by another bucket's item
                        if ( m_occupied[pos] )
                        {
                            collision = true;
                            break;
                        }

                        // Check if bucket at this position is reserved for single-item placement
                        if ( !buckets[pos].empty() && buckets[pos].size() == 1 )
                        {
                            collision = true;
                            break;
                        }

                        // Check for intra-bucket collision (multiple items in this bucket hashing to same position)
                        if ( std::find( positions.begin(), positions.end(), pos ) != positions.end() )
                        {
                            collision = true;
                            break;
                        }

                        positions.push_back( pos );
                    }

                    if ( !collision )
                    {
                        // Place all items from this bucket
                        for ( size_t i = 0; i < bucket.size(); ++i )
                        {
                            size_t itemIndex = bucket[i].first;
                            size_t pos = positions[i];
                            m_table[pos].emplace( std::move( items[itemIndex] ) );
                            m_occupied[pos] = 1;
                        }
                        m_seeds[originalBucketIndex] = static_cast<seed_type>( seed );
                        seedFound = true;
                    }
                }
            }
        }
    }

    //----------------------------------------------
    // Comparison
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline bool PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::operator==( const PerfectHashMap& other ) const noexcept
    {
        if ( count() != other.count() )
        {
            return false;
        }

        if ( isEmpty() )
        {
            return true;
        }

        for ( const auto& [key, value] : *this )
        {
            const TValue* otherValue = other.find( key );
            if ( !otherValue || value != *otherValue )
            {
                return false;
            }
        }

        return true;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline bool PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::operator!=( const PerfectHashMap& other ) const noexcept
    {
        return !( *this == other );
    }

    //----------------------------------------------
    // Element access
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    template <typename K>
    inline const TValue& PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::at( const K& key ) const
    {
        if ( const TValue* value = find( key ) )
        {
            return *value;
        }
        throw std::out_of_range( "PerfectHashMap::at: key not found" );
    }

    //----------------------------------------------
    // Lookup
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    template <typename K>
    inline bool PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::contains( const K& key ) const noexcept
    {
        if ( m_table.empty() )
        {
            return false;
        }

        const size_t tableSize = m_table.size();
        const hash_type hashValue = m_hasher( key );
        const size_t bucketIndex = hashValue & ( tableSize - 1 );
        const seed_type seed = m_seeds[bucketIndex];

        const size_t position = ( seed < 0 )
                                    ? static_cast<size_t>( -seed - 1 )
                                    : hashing::seedMix<hash_type>( static_cast<hash_type>( seed ), hashValue, tableSize );

        return m_occupied[position] && m_keyEqual( m_table[position]->first, key );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    template <typename K>
    inline const TValue* PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::find( const K& key ) const noexcept
    {
        if ( m_table.empty() )
        {
            return nullptr;
        }

        const size_t tableSize = m_table.size();
        const hash_type hashValue = m_hasher( key );
        const size_t bucketIndex = hashValue & ( tableSize - 1 );
        const seed_type seed = m_seeds[bucketIndex];

        const size_t position = ( seed < 0 )
                                    ? static_cast<size_t>( -seed - 1 )
                                    : hashing::seedMix<hash_type>( static_cast<hash_type>( seed ), hashValue, tableSize );

        if ( m_occupied[position] && m_keyEqual( m_table[position]->first, key ) )
        {
            return &m_table[position]->second;
        }

        return nullptr;
    }

    //----------------------------------------------
    // Capacity
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::size_type PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::size() const noexcept
    {
        return m_table.size();
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::size_type PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::count() const noexcept
    {
        return m_itemCount;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline bool PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::isEmpty() const noexcept
    {
        return m_itemCount == 0;
    }

    //----------------------------------------------
    // Iterators
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::begin() const noexcept
    {
        return Iterator{ &m_table, &m_occupied, 0 };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::end() const noexcept
    {
        return Iterator{ &m_table, &m_occupied, m_table.size() };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::ConstIterator PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::cbegin() const noexcept
    {
        return begin();
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::ConstIterator PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::cend() const noexcept
    {
        return end();
    }

    //----------------------------------------------
    // Hash policy
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::hasher PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::hash_function() const
    {
        return m_hasher;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::key_equal PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::key_eq() const
    {
        return m_keyEqual;
    }

    //----------------------------------------------
    // PerfectHashMap::Iterator class
    //----------------------------------------------

    //---------------------------
    // Construction
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::Iterator( const std::vector<std::optional<std::pair<TKey, TValue>>>* table, const std::vector<uint8_t>* occupied, size_t index )
        : m_table{ table },
          m_occupied{ occupied },
          m_index{ index }
    {
        skipEmpty();
    }

    //---------------------------
    // Operations
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::reference PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::operator*() const
    {
        return *m_table->operator[]( m_index );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::pointer PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::operator->() const
    {
        return &*m_table->operator[]( m_index );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator& PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::operator++()
    {
        ++m_index;
        skipEmpty();
        return *this;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::operator++( int )
    {
        Iterator temp = *this;
        ++m_index;
        skipEmpty();

        return temp;
    }

    //---------------------------
    // Comparison
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline bool PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::operator==( const PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator& other ) const
    {
        return m_index == other.m_index;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline bool PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::operator!=( const PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator& other ) const
    {
        return m_index != other.m_index;
    }

    //---------------------------
    // Private methods
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename Hasher, typename KeyEqual>
    inline void PerfectHashMap<TKey, TValue, HashType, Seed, Hasher, KeyEqual>::Iterator::skipEmpty()
    {
        while ( m_index < m_table->size() && !( *m_occupied )[m_index] )
        {
            ++m_index;
        }
    }
} // namespace nfx::containers
