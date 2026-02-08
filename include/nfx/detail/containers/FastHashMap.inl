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
 * @file FastHashMap.inl
 * @brief Template implementation file for HashMap Robin Hood hashing container
 * @details Contains template method implementations for cache-friendly hash table
 *          with Robin Hood displacement algorithm, heterogeneous string lookup,
 *          and aggressive performance optimizations
 */

namespace nfx::containers
{
    //=====================================================================
    // FastHashMap class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::FastHashMap()
        : m_capacity{ INITIAL_CAPACITY },
          m_mask{ INITIAL_CAPACITY - 1 }
    {
        m_buckets.resize( INITIAL_CAPACITY );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::FastHashMap( std::initializer_list<std::pair<TKey, TValue>> init )
        : FastHashMap{}
    {
        reserve( init.size() );
        for ( const auto& p : init )
        {
            insertOrAssign( p.first, p.second );
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename InputIt>
    inline FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::FastHashMap( InputIt first, InputIt last )
        : FastHashMap{}
    {
        if constexpr ( std::is_same_v<typename std::iterator_traits<InputIt>::iterator_category, std::random_access_iterator_tag> )
        {
            reserve( std::distance( first, last ) );
        }
        for ( auto it = first; it != last; ++it )
        {
            insertOrAssign( it->first, it->second );
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::FastHashMap( size_t initialCapacity )
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

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::FastHashMap( FastHashMap&& other ) noexcept
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

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>&
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::operator=( FastHashMap&& other ) noexcept
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

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline TValue* FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::find( const KeyType& key ) noexcept
    {
        const HashType hash( m_hasher( key ) );
        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance = 0;

        while ( true )
        {
            Bucket& bucket( m_buckets[pos] );

            // Check Robin Hood invariant and occupancy in single condition
            if ( !bucket.occupied || distance > bucket.distance )
            {
                return nullptr;
            }

            if ( bucket.hash == hash && keysEqual( bucket.data->first, key ) )
            {
                return &bucket.data->second;
            }

            ++distance;
            pos = ( pos + 1 ) & m_mask;
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline const TValue* FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::find( const KeyType& key ) const noexcept
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

            if ( bucket.hash == hash && keysEqual( bucket.data->first, key ) )
            {
                return &bucket.data->second;
            }

            ++distance;
            pos = ( pos + 1 ) & m_mask;
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::contains( const KeyType& key ) const noexcept
    {
        return find( key ) != nullptr;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline TValue& FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::at( const TKey& key )
    {
        TValue* value{ find( key ) };
        if ( !value )
        {
            throw std::out_of_range{ "FastHashMap::at: key not found" };
        }
        return *value;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline const TValue& FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::at( const TKey& key ) const
    {
        const TValue* value{ find( key ) };
        if ( !value )
        {
            throw std::out_of_range{ "FastHashMap::at: key not found" };
        }
        return *value;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline TValue& FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::operator[]( const TKey& key )
    {
        TValue* existing = find( key );
        if ( existing )
        {
            return *existing;
        }

        insertOrAssign( key, TValue{} );
        return *find( key );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline TValue& FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::operator[]( TKey&& key )
    {
        TValue* existing = find( key );
        if ( existing )
        {
            return *existing;
        }

        TKey keyCopy = key;
        insertOrAssign( std::move( key ), TValue{} );

        return *find( keyCopy );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline TValue& FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::at( const KeyType& key )
    {
        TValue* value{ find( key ) };
        if ( !value )
        {
            throw std::out_of_range{ "FastHashMap::at: key not found" };
        }
        return *value;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline const TValue& FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::at( const KeyType& key ) const
    {
        const TValue* value{ find( key ) };
        if ( !value )
        {
            throw std::out_of_range{ "FastHashMap::at: key not found" };
        }
        return *value;
    }

    //----------------------------------------------
    // Insertion
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insert( const TKey& key, const TValue& value )
    {
        if ( find( key ) != nullptr )
        {
            return false;
        }
        insertOrAssignInternal( key, value );
        return true;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insert( const TKey& key, TValue&& value )
    {
        if ( find( key ) != nullptr )
        {
            return false;
        }
        insertOrAssignInternal( key, std::forward<TValue>( value ) );
        return true;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insert( TKey&& key, TValue&& value )
    {
        if ( find( key ) != nullptr )
        {
            return false;
        }
        insertOrAssignInternal( std::forward<TKey>( key ), std::forward<TValue>( value ) );
        return true;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssign( const TKey& key, TValue&& value )
    {
        insertOrAssignInternal( key, std::forward<TValue>( value ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssign( const TKey& key, const TValue& value )
    {
        insertOrAssignInternal( key, value );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssign( TKey&& key, TValue&& value )
    {
        insertOrAssignInternal( std::forward<TKey>( key ), std::forward<TValue>( value ) );
    }

    //----------------------------------------------
    // Emplace operations
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::emplace( const TKey& key, Args&&... args )
    {
        insertOrAssignInternal( key, TValue( std::forward<Args>( args )... ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::emplace( TKey&& key, Args&&... args )
    {
        insertOrAssignInternal( std::move( key ), TValue( std::forward<Args>( args )... ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline std::pair<typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator, bool>
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::tryEmplace( const TKey& key, Args&&... args )
    {
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

                m_buckets[idx].data.emplace( key, TValue( std::forward<Args>( args )... ) );
                m_buckets[idx].hash = hash;
                m_buckets[idx].distance = 0;
                m_buckets[idx].occupied = true;
                ++m_size;

                return { Iterator{ &m_buckets[idx], m_buckets.data() + m_capacity }, true };
            }

            if ( bucket.hash == hash && keysEqual( bucket.data->first, key ) )
            {
                return { Iterator{ &bucket, m_buckets.data() + m_capacity }, false };
            }

            idx = ( idx + 1 ) & m_mask;
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline std::pair<typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator, bool>
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::tryEmplace( TKey&& key, Args&&... args )
    {
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

                m_buckets[idx].data.emplace( std::move( key ), TValue( std::forward<Args>( args )... ) );
                m_buckets[idx].hash = hash;
                m_buckets[idx].distance = 0;
                m_buckets[idx].occupied = true;
                ++m_size;

                return { Iterator{ &m_buckets[idx], m_buckets.data() + m_capacity }, true };
            }

            if ( bucket.hash == hash && keysEqual( bucket.data->first, key ) )
            {
                return { Iterator{ &bucket, m_buckets.data() + m_capacity }, false };
            }

            idx = ( idx + 1 ) & m_mask;
        }
    }

    //----------------------------------------------
    // Capacity and memory management
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::reserve( size_t minCapacity )
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
                        insertOrAssignInternal( std::move( oldBuckets[i].data->first ), std::move( oldBuckets[i].data->second ) );
                    }
                }
            }
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::erase( const KeyType& key ) noexcept
    {
        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        while ( distance <= m_buckets[pos].distance && m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].data->first, key ) )
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

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::erase( ConstIterator pos ) noexcept
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

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::erase( ConstIterator first, ConstIterator last ) noexcept
    {
        while ( first != last )
        {
            first = erase( first );
        }
        return Iterator{ const_cast<Bucket*>( last.m_bucket ), m_buckets.data() + m_capacity };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::clear() noexcept
    {
        for ( size_t i = 0; i < m_capacity; ++i )
        {
            m_buckets[i].occupied = false;
            m_buckets[i].distance = 0;
        }
        m_size = 0;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline std::optional<std::pair<TKey, TValue>>
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::extract( const KeyType& key )
    {
        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        while ( distance <= m_buckets[pos].distance && m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].data->first, key ) )
            {
                // Extract the key-value pair before erasing
                std::pair<TKey, TValue> extracted( std::move( m_buckets[pos].data->first ), std::move( m_buckets[pos].data->second ) );

                // Erase the element
                eraseAtPosition( pos );
                --m_size;

                return extracted;
            }
            pos = ( pos + 1 ) & m_mask;
            ++distance;
        }

        return std::nullopt;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::merge( FastHashMap& other )
    {
        // Try to insert each element from other into this map
        for ( auto it = other.begin(); it != other.end(); )
        {
            // Try to insert without overwriting existing keys
            if ( !contains( it->first ) )
            {
                // Extract from other and insert into this
                auto extracted = other.extract( it->first );
                if ( extracted )
                {
                    insertOrAssign( std::move( extracted->first ), std::move( extracted->second ) );
                }
                // Iterator is now invalid, restart from begin
                it = other.begin();
            }
            else
            {
                ++it;
            }
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::merge( FastHashMap&& other )
    {
        merge( other );
    }

    //----------------------------------------------
    // State inspection
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline size_t FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::size() const noexcept
    {
        return m_size;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline size_t FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::capacity() const noexcept
    {
        return m_capacity;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::isEmpty() const noexcept
    {
        return m_size == 0;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::swap( FastHashMap& other ) noexcept
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

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::begin() noexcept
    {
        return Iterator{ m_buckets.data(), m_buckets.data() + m_capacity };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::begin() const noexcept
    {
        return ConstIterator{ m_buckets.data(), m_buckets.data() + m_capacity };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::end() noexcept
    {
        return Iterator{ m_buckets.data() + m_capacity, m_buckets.data() + m_capacity };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::end() const noexcept
    {
        return ConstIterator{ m_buckets.data() + m_capacity, m_buckets.data() + m_capacity };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::cbegin() const noexcept
    {
        return ConstIterator{ m_buckets.data(), m_buckets.data() + m_capacity };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::cend() const noexcept
    {
        return ConstIterator{ m_buckets.data() + m_capacity, m_buckets.data() + m_capacity };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::operator==( const FastHashMap& other ) const noexcept
    {
        if ( m_size != other.m_size )
        {
            return false;
        }

        // Compare all key-value pairs using O(n) average-case lookup
        for ( const auto& pair : *this )
        {
            const TValue* otherValue = other.find( pair.first );
            if ( !otherValue || *otherValue != pair.second )
            {
                return false;
            }
        }

        return true;
    }

    //----------------------------------------------
    // Internal implementation
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename ValueType>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssignInternal( const TKey& key, ValueType&& value )
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
            m_buckets[pos] = Bucket{ { { key, std::forward<ValueType>( value ) } }, hash, distance, true };
            ++m_size;
            return;
        }

        // First pass: check for existing key or find insertion point
        while ( m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].data->first, key ) )
            {
                // Update existing key
                m_buckets[pos].data->second = std::forward<ValueType>( value );
                return;
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
        Bucket newBucket{ { { key, std::forward<ValueType>( value ) } }, hash, distance, true };

        // Robin Hood displacement loop (optimized swap)
        while ( m_buckets[pos].occupied )
        {
            if ( newBucket.distance > m_buckets[pos].distance )
            {
                // Optimized Robin Hood swap: direct moves instead of std::swap
                // Avoids creating temporary Bucket (saves 1 move operation per swap)
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
    }

    // Overload for rvalue key (perfect forwarding optimization)
    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename ValueType>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssignInternal( TKey&& key, ValueType&& value )
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
            m_buckets[pos] = Bucket{ { { std::move( key ), std::forward<ValueType>( value ) } }, hash, distance, true };
            ++m_size;
            return;
        }

        // First pass: check for existing key or find insertion point
        while ( m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].data->first, key ) )
            {
                // Update existing key (key is discarded, value updated)
                m_buckets[pos].data->second = std::forward<ValueType>( value );
                return;
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
        Bucket newBucket{ { { std::move( key ), std::forward<ValueType>( value ) } }, hash, distance, true };

        // Robin Hood displacement loop (optimized swap)
        while ( m_buckets[pos].occupied )
        {
            if ( newBucket.distance > m_buckets[pos].distance )
            {
                // Optimized Robin Hood swap: direct moves instead of std::swap
                // Avoids creating temporary Bucket (saves 1 move operation per swap)
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
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::shouldResize() const noexcept
    {
        return ( m_size * 100 ) >= ( m_capacity * MAX_LOAD_FACTOR_PERCENT );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::resize()
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
                insertOrAssignInternal( std::move( oldBuckets[i].data->first ), std::move( oldBuckets[i].data->second ) );
            }
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::eraseAtPosition( size_t pos ) noexcept
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

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType1, typename KeyType2>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::keysEqual( const KeyType1& k1, const KeyType2& k2 ) const noexcept
    {
        return m_keyEqual( k1, k2 );
    }

    //----------------------------------------------
    // FastHashMap::Iterator class
    //----------------------------------------------

    //---------------------------
    // Construction
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::Iterator( Bucket* bucket, Bucket* end )
        : m_bucket{ bucket },
          m_end{ end }
    {
        skipToOccupied();
    }

    //---------------------------
    // Operations
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::reference
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator*() const
    {
        return *std::launder( reinterpret_cast<pointer>( &m_bucket->data.value() ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::pointer
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator->() const
    {
        return reinterpret_cast<pointer>( m_bucket );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator&
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator++()
    {
        ++m_bucket;
        skipToOccupied();

        return *this;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator++( int )
    {
        Iterator tmp = *this;
        ++( *this );

        return tmp;
    }

    //---------------------------
    // Comparison
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator==( const Iterator& other ) const
    {
        return m_bucket == other.m_bucket;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator!=( const Iterator& other ) const
    {
        return m_bucket != other.m_bucket;
    }

    //---------------------------
    // Private methods
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::skipToOccupied()
    {
        while ( m_bucket != m_end && !m_bucket->occupied )
        {
            ++m_bucket;
        }
    }

    //----------------------------------------------
    // FastHashMap::ConstIterator class
    //----------------------------------------------

    //---------------------------
    // Construction
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Bucket* bucket, const Bucket* end )
        : m_bucket{ bucket },
          m_end{ end }
    {
        skipToOccupied();
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Iterator& it )
        : m_bucket{ it.m_bucket },
          m_end{ it.m_end }
    {
    }

    //---------------------------
    // Operations
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::reference
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator*() const
    {
        return *std::launder( reinterpret_cast<pointer>( &m_bucket->data.value() ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::pointer
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator->() const
    {
        return reinterpret_cast<pointer>( m_bucket );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator&
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator++()
    {
        ++m_bucket;
        skipToOccupied();

        return *this;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator++( int )
    {
        ConstIterator tmp = *this;
        ++( *this );

        return tmp;
    }

    //---------------------------
    // Comparison
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator==( const ConstIterator& other ) const
    {
        return m_bucket == other.m_bucket;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator!=( const ConstIterator& other ) const
    {
        return m_bucket != other.m_bucket;
    }

    //---------------------------
    // Private methods
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void FastHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::skipToOccupied()
    {
        while ( m_bucket != m_end && !m_bucket->occupied )
        {
            ++m_bucket;
        }
    }
} // namespace nfx::containers
