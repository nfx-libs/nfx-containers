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
 * @file FastHashSet.inl
 * @brief Template implementation file for FastHashSet Robin Hood hashing container
 * @details Contains template method implementations for cache-friendly hash table
 *          with Robin Hood displacement algorithm, heterogeneous string lookup,
 *          and aggressive performance optimizations
 */

namespace nfx::containers
{
    //=====================================================================
    // FastHashSet class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::FastHashSet()
        : m_capacity{ INITIAL_CAPACITY },
          m_mask{ INITIAL_CAPACITY - 1 }
    {
        m_buckets.resize( INITIAL_CAPACITY );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::FastHashSet( std::initializer_list<TKey> init )
        : FastHashSet{}
    {
        reserve( init.size() );
        for ( const auto& key : init )
        {
            insert( key );
        }
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename InputIt>
    inline FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::FastHashSet( InputIt first, InputIt last )
        : FastHashSet{}
    {
        if constexpr ( std::is_same_v<typename std::iterator_traits<InputIt>::iterator_category, std::random_access_iterator_tag> )
        {
            reserve( std::distance( first, last ) );
        }
        for ( auto it = first; it != last; ++it )
        {
            insert( *it );
        }
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::FastHashSet( size_t initialCapacity )
    {
        size_t capacity{ 1 };
        while ( capacity < initialCapacity )
        {
            capacity <<= 1;
        }
        m_capacity = capacity;
        m_mask = capacity - 1;
        m_buckets.resize( capacity );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::FastHashSet( FastHashSet&& other ) noexcept
        : m_buckets( std::move( other.m_buckets ) ),
          m_size( other.m_size ),
          m_capacity( other.m_capacity ),
          m_mask( other.m_mask ),
          m_hasher( std::move( other.m_hasher ) ),
          m_keyEqual( std::move( other.m_keyEqual ) )
    {
        // Reset source to valid empty state
        other.m_size = 0;
        other.m_capacity = INITIAL_CAPACITY;
        other.m_mask = INITIAL_CAPACITY - 1;
        other.m_buckets.clear();
        other.m_buckets.resize( INITIAL_CAPACITY );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>&
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::operator=( FastHashSet&& other ) noexcept
    {
        if ( this != &other )
        {
            m_buckets = std::move( other.m_buckets );
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            m_mask = other.m_mask;
            m_hasher = std::move( other.m_hasher );
            m_keyEqual = std::move( other.m_keyEqual );

            // Reset source to valid empty state
            other.m_size = 0;
            other.m_capacity = INITIAL_CAPACITY;
            other.m_mask = INITIAL_CAPACITY - 1;
            other.m_buckets.clear();
            other.m_buckets.resize( INITIAL_CAPACITY );
        }
        return *this;
    }

    //----------------------------------------------
    // Core operations
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline const TKey* FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::find( const KeyType& key ) const noexcept
    {
        const HashType hash( m_hasher( key ) );
        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance = 0;

        while ( true )
        {
            const Bucket& bucket( m_buckets[pos] );

            // Check Robin Hood invariant and occupancy in single condition
            if ( !bucket.occupied || distance > bucket.distance )
            {
                return nullptr;
            }

            // Hot path: hash comparison first, then key equality
            if ( bucket.hash == hash && keysEqual( *bucket.data, key ) )
            {
                return &bucket.data.value();
            }

            ++distance;
            pos = ( pos + 1 ) & m_mask;
        }
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::contains( const KeyType& key ) const noexcept
    {
        return find( key ) != nullptr;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline const TKey& FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::at( const KeyType& key ) const
    {
        const TKey* found = find( key );
        if ( !found )
        {
            throw std::out_of_range( "FastHashSet::at: key not found" );
        }
        return *found;
    }

    //----------------------------------------------
    // Insertion
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::insert( const TKey& key )
    {
        return insertInternal( key );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::insert( TKey&& key )
    {
        return insertInternal( std::move( key ) );
    }

    //----------------------------------------------
    // Emplace operations
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::emplace( Args&&... args )
    {
        return insertInternal( TKey( std::forward<Args>( args )... ) );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline std::pair<typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator, bool>
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::tryEmplace( Args&&... args )
    {
        TKey key( std::forward<Args>( args )... );

        const HashType hash{ m_hasher( key ) };
        size_t idx{ hash & m_mask };

        for ( ;; )
        {
            Bucket& bucket{ m_buckets[idx] };

            if ( !bucket.occupied )
            {
                if ( shouldResize() )
                {
                    resize();
                    idx = hash & m_mask;
                    while ( m_buckets[idx].occupied )
                    {
                        idx = ( idx + 1 ) & m_mask;
                    }
                }

                m_buckets[idx].data.emplace( std::move( key ) );
                m_buckets[idx].hash = hash;
                m_buckets[idx].distance = 0;
                m_buckets[idx].occupied = true;
                ++m_size;

                return { Iterator{ &m_buckets[idx], m_buckets.data() + m_capacity }, true };
            }

            if ( bucket.hash == hash && keysEqual( bucket.data.value(), key ) )
            {
                // Key exists - return existing element
                return { Iterator{ &bucket, m_buckets.data() + m_capacity }, false };
            }

            idx = ( idx + 1 ) & m_mask;
        }
    }

    //----------------------------------------------
    // Capacity and memory management
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::reserve( size_t minCapacity )
    {
        if ( minCapacity > m_capacity )
        {
            size_t newCapacity{ 1 };
            while ( newCapacity < minCapacity )
            {
                newCapacity <<= 1;
            }

            if ( newCapacity > m_capacity )
            {
                std::vector<Bucket> oldBuckets{ std::move( m_buckets ) };
                const size_t oldCapacity{ m_capacity };

                m_capacity = newCapacity;
                m_mask = newCapacity - 1;
                m_buckets.clear();
                m_buckets.resize( newCapacity );
                m_size = 0;

                for ( size_t i = 0; i < oldCapacity; ++i )
                {
                    if ( oldBuckets[i].occupied )
                    {
                        insertInternal( std::move( oldBuckets[i].data.value() ) );
                    }
                }
            }
        }
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::erase( const KeyType& key ) noexcept
    {
        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        while ( distance <= m_buckets[pos].distance && m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( *m_buckets[pos].data, key ) )
            {
                eraseAtPosition( pos );
                --m_size;
                return true;
            }
            pos = ( pos + 1 ) & m_mask;
            ++distance;
        }

        return false;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::erase( ConstIterator pos ) noexcept
    {
        if ( pos.m_bucket == nullptr || pos.m_bucket >= m_buckets.data() + m_capacity || !pos.m_bucket->occupied )
        {
            return end();
        }

        size_t bucketPos = pos.m_bucket - m_buckets.data();
        eraseAtPosition( bucketPos );
        --m_size;

        return Iterator{ const_cast<Bucket*>( pos.m_bucket ), m_buckets.data() + m_capacity };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::erase( ConstIterator first, ConstIterator last ) noexcept
    {
        while ( first != last )
        {
            first = erase( first );
        }
        return Iterator{ const_cast<Bucket*>( last.m_bucket ), m_buckets.data() + m_capacity };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::clear() noexcept
    {
        for ( size_t i = 0; i < m_capacity; ++i )
        {
            m_buckets[i].occupied = false;
            m_buckets[i].distance = 0;
        }
        m_size = 0;
    }

    //----------------------------------------------
    // State inspection
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline size_t FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::size() const noexcept
    {
        return m_size;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline size_t FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::capacity() const noexcept
    {
        return m_capacity;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::isEmpty() const noexcept
    {
        return m_size == 0;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::swap( FastHashSet& other ) noexcept
    {
        std::swap( m_buckets, other.m_buckets );
        std::swap( m_size, other.m_size );
        std::swap( m_capacity, other.m_capacity );
        std::swap( m_mask, other.m_mask );
        std::swap( m_hasher, other.m_hasher );
        std::swap( m_keyEqual, other.m_keyEqual );
    }

    //----------------------------------------------
    // STL-compatible iteration support
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::begin() noexcept
    {
        return Iterator{ m_buckets.data(), m_buckets.data() + m_capacity };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::begin() const noexcept
    {
        return ConstIterator{ m_buckets.data(), m_buckets.data() + m_capacity };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::end() noexcept
    {
        return Iterator{ m_buckets.data() + m_capacity, m_buckets.data() + m_capacity };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::end() const noexcept
    {
        return ConstIterator{ m_buckets.data() + m_capacity, m_buckets.data() + m_capacity };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::cbegin() const noexcept
    {
        return ConstIterator{ m_buckets.data(), m_buckets.data() + m_capacity };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::cend() const noexcept
    {
        return ConstIterator{ m_buckets.data() + m_capacity, m_buckets.data() + m_capacity };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::operator==( const FastHashSet& other ) const noexcept
    {
        if ( m_size != other.m_size )
        {
            return false;
        }

        // Compare all keys using O(n) lookup
        for ( const auto& key : *this )
        {
            if ( !other.contains( key ) )
            {
                return false;
            }
        }

        return true;
    }

    //----------------------------------------------
    // Internal implementation
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::insertInternal( const TKey& key )
    {
        if ( shouldResize() )
        {
            resize();
        }

        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        // Fast path: check if slot is empty
        if ( !m_buckets[pos].occupied )
        {
            m_buckets[pos] = Bucket{ { key }, hash, distance, true };
            ++m_size;
            return true;
        }

        // First pass: check for existing key or find insertion point
        while ( m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].data.value(), key ) )
            {
                // Key already exists - don't insert
                return false;
            }

            if ( distance > m_buckets[pos].distance )
            {
                // Robin Hood: we need to displace this bucket
                break;
            }

            pos = ( pos + 1 ) & m_mask;
            ++distance;
        }

        // If we're here, we need to insert a new bucket
        Bucket newBucket{ { key }, hash, distance, true };

        // Robin Hood displacement loop (optimized swap)
        while ( m_buckets[pos].occupied )
        {
            if ( newBucket.distance > m_buckets[pos].distance )
            {
                // Optimized Robin Hood swap: direct moves instead of std::swap
                Bucket temp{ std::move( m_buckets[pos] ) };
                m_buckets[pos] = std::move( newBucket );
                newBucket = std::move( temp );
            }

            pos = ( pos + 1 ) & m_mask;
            ++newBucket.distance;
        }

        // Insert the final bucket
        m_buckets[pos] = std::move( newBucket );
        ++m_size;
        return true;
    }

    // Overload for rvalue key (perfect forwarding optimization)
    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::insertInternal( TKey&& key )
    {
        if ( shouldResize() )
        {
            resize();
        }

        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        // Fast path: check if slot is empty
        if ( !m_buckets[pos].occupied )
        {
            // Move key directly into bucket (no copy!)
            m_buckets[pos] = Bucket{ std::move( key ), hash, distance, true };
            ++m_size;
            return true;
        }

        // First pass: check for existing key or find insertion point
        while ( m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].data.value(), key ) )
            {
                // Key already exists - don't insert
                return false;
            }

            if ( distance > m_buckets[pos].distance )
            {
                // Robin Hood: we need to displace this bucket
                break;
            }

            pos = ( pos + 1 ) & m_mask;
            ++distance;
        }

        // If we're here, we need to insert a new bucket (move key!)
        Bucket newBucket{ { std::move( key ) }, hash, distance, true };

        // Robin Hood displacement loop (optimized swap)
        while ( m_buckets[pos].occupied )
        {
            if ( newBucket.distance > m_buckets[pos].distance )
            {
                // Optimized Robin Hood swap: direct moves instead of std::swap
                Bucket temp{ std::move( m_buckets[pos] ) };
                m_buckets[pos] = std::move( newBucket );
                newBucket = std::move( temp );
            }

            pos = ( pos + 1 ) & m_mask;
            ++newBucket.distance;
        }

        // Insert the final bucket
        m_buckets[pos] = std::move( newBucket );
        ++m_size;
        return true;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::shouldResize() const noexcept
    {
        return ( m_size * 100 ) >= ( m_capacity * MAX_LOAD_FACTOR_PERCENT );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::resize()
    {
        const size_t oldCapacity{ m_capacity };
        m_capacity <<= 1;
        m_mask = m_capacity - 1;

        std::vector<Bucket> oldBuckets{ std::move( m_buckets ) };
        m_buckets.clear();
        m_buckets.resize( m_capacity );
        m_size = 0;

        for ( size_t i = 0; i < oldCapacity; ++i )
        {
            if ( oldBuckets[i].occupied )
            {
                insertInternal( std::move( oldBuckets[i].data.value() ) );
            }
        }
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::eraseAtPosition( size_t pos ) noexcept
    {
        size_t nextPos{ ( pos + 1 ) & m_mask };

        while ( m_buckets[nextPos].occupied && m_buckets[nextPos].distance > 0 )
        {
            m_buckets[pos] = std::move( m_buckets[nextPos] );
            --m_buckets[pos].distance; // Adjust distance!
            pos = nextPos;
            nextPos = ( nextPos + 1 ) & m_mask;
        }

        m_buckets[pos] = Bucket{};
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType1, typename KeyType2>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::keysEqual( const KeyType1& k1, const KeyType2& k2 ) const noexcept
    {
        return m_keyEqual( k1, k2 );
    }

    //----------------------------------------------
    // FastHashSet::Iterator class
    //----------------------------------------------

    //---------------------------
    // Construction
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::Iterator( Bucket* bucket, Bucket* end )
        : m_bucket{ bucket },
          m_end{ end }
    {
        skipToOccupied();
    }

    //---------------------------
    // Operations
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::reference
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator*() const
    {
        return m_bucket->data.value();
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::pointer
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator->() const
    {
        return &m_bucket->data.value();
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator&
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator++()
    {
        ++m_bucket;
        skipToOccupied();
        return *this;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator++( int )
    {
        Iterator tmp = *this;
        ++( *this );
        return tmp;
    }

    //---------------------------
    // Comparison
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator==( const Iterator& other ) const
    {
        return m_bucket == other.m_bucket;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator!=( const Iterator& other ) const
    {
        return m_bucket != other.m_bucket;
    }

    //---------------------------
    // Private methods
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::skipToOccupied()
    {
        while ( m_bucket != m_end && !m_bucket->occupied )
        {
            ++m_bucket;
        }
    }

    //----------------------------------------------
    // FastHashSet::ConstIterator class
    //----------------------------------------------

    //---------------------------
    // Construction
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Bucket* bucket, const Bucket* end )
        : m_bucket{ bucket },
          m_end{ end }
    {
        skipToOccupied();
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Iterator& it )
        : m_bucket{ it.m_bucket },
          m_end{ it.m_end }
    {
    }

    //---------------------------
    // Operations
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::reference
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator*() const
    {
        return m_bucket->data.value();
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::pointer
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator->() const
    {
        return &m_bucket->data.value();
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator&
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator++()
    {
        ++m_bucket;
        skipToOccupied();
        return *this;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator++( int )
    {
        ConstIterator tmp = *this;
        ++( *this );
        return tmp;
    }

    //---------------------------
    // Comparison
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator==( const ConstIterator& other ) const
    {
        return m_bucket == other.m_bucket;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator!=( const ConstIterator& other ) const
    {
        return m_bucket != other.m_bucket;
    }

    //---------------------------
    // Private methods
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::skipToOccupied()
    {
        while ( m_bucket != m_end && !m_bucket->occupied )
        {
            ++m_bucket;
        }
    }
} // namespace nfx::containers
