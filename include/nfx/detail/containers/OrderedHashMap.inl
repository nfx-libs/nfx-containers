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
 * @file OrderedHashMap.inl
 * @brief Template implementation file for OrderedHashMap with Robin Hood hashing and insertion-order preservation
 * @details Contains template method implementations for hash table with Robin Hood displacement algorithm,
 *          heterogeneous lookup, and doubly-linked list for stable iteration order
 */

namespace nfx::containers
{
    //=====================================================================
    // OrderedHashMap class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::OrderedHashMap()
        : m_capacity{ INITIAL_CAPACITY },
          m_mask{ INITIAL_CAPACITY - 1 }
    {
        m_buckets.resize( INITIAL_CAPACITY );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::OrderedHashMap( std::initializer_list<std::pair<TKey, TValue>> init )
        : OrderedHashMap{}
    {
        reserve( init.size() );
        for ( const auto& p : init )
        {
            insertOrAssign( p.first, p.second );
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename InputIt>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::OrderedHashMap( InputIt first, InputIt last )
        : OrderedHashMap{}
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
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::OrderedHashMap( size_t initialCapacity )
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
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::OrderedHashMap( OrderedHashMap&& other ) noexcept
        : m_buckets( std::move( other.m_buckets ) ),
          m_head( other.m_head ),
          m_tail( other.m_tail ),
          m_size( other.m_size ),
          m_capacity( other.m_capacity ),
          m_mask( other.m_mask ),
          m_hasher( std::move( other.m_hasher ) ),
          m_keyEqual( std::move( other.m_keyEqual ) )
    {
        // Reset source to valid empty state
        other.m_head = nullptr;
        other.m_tail = nullptr;
        other.m_size = 0;
        other.m_capacity = INITIAL_CAPACITY;
        other.m_mask = INITIAL_CAPACITY - 1;
        other.m_buckets.clear();
        other.m_buckets.resize( INITIAL_CAPACITY );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>&
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::operator=( OrderedHashMap&& other ) noexcept
    {
        if ( this != &other )
        {
            // Clear existing nodes
            clear();

            m_buckets = std::move( other.m_buckets );
            m_head = other.m_head;
            m_tail = other.m_tail;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            m_mask = other.m_mask;
            m_hasher = std::move( other.m_hasher );
            m_keyEqual = std::move( other.m_keyEqual );

            // Reset source to valid empty state
            other.m_head = nullptr;
            other.m_tail = nullptr;
            other.m_size = 0;
            other.m_capacity = INITIAL_CAPACITY;
            other.m_mask = INITIAL_CAPACITY - 1;
            other.m_buckets.clear();
            other.m_buckets.resize( INITIAL_CAPACITY );
        }
        return *this;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::OrderedHashMap( const OrderedHashMap& other )
        : m_capacity{ other.m_capacity },
          m_mask{ other.m_mask },
          m_hasher{ other.m_hasher },
          m_keyEqual{ other.m_keyEqual }
    {
        m_buckets.resize( m_capacity );

        // Deep copy: iterate in insertion order and insert
        for ( const auto& pair : other )
        {
            insertOrAssign( pair.first, pair.second );
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>&
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::operator=( const OrderedHashMap& other )
    {
        if ( this != &other )
        {
            // Clear existing state
            clear();

            m_capacity = other.m_capacity;
            m_mask = other.m_mask;
            m_hasher = other.m_hasher;
            m_keyEqual = other.m_keyEqual;
            m_buckets.clear();
            m_buckets.resize( m_capacity );

            // Deep copy: iterate in insertion order and insert
            for ( const auto& pair : other )
            {
                insertOrAssign( pair.first, pair.second );
            }
        }
        return *this;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::~OrderedHashMap()
    {
        clear();
    }

    //----------------------------------------------
    // Core operations
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline TValue* OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::find( const KeyType& key ) noexcept
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

            if ( bucket.hash == hash && keysEqual( bucket.node->data.first, key ) )
            {
                return &bucket.node->data.second;
            }

            ++distance;
            pos = ( pos + 1 ) & m_mask;
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline const TValue* OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::find( const KeyType& key ) const noexcept
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

            if ( bucket.hash == hash && keysEqual( bucket.node->data.first, key ) )
            {
                return &bucket.node->data.second;
            }

            ++distance;
            pos = ( pos + 1 ) & m_mask;
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::contains( const KeyType& key ) const noexcept
    {
        return find( key ) != nullptr;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline TValue& OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::at( const TKey& key )
    {
        TValue* value{ find( key ) };
        if ( !value )
        {
            throw std::out_of_range{ "OrderedHashMap::at: key not found" };
        }
        return *value;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline const TValue& OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::at( const TKey& key ) const
    {
        const TValue* value{ find( key ) };
        if ( !value )
        {
            throw std::out_of_range{ "OrderedHashMap::at: key not found" };
        }
        return *value;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline TValue& OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::operator[]( const TKey& key )
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
    inline TValue& OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::operator[]( TKey&& key )
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
    inline TValue& OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::at( const KeyType& key )
    {
        TValue* value{ find( key ) };
        if ( !value )
        {
            throw std::out_of_range{ "OrderedHashMap::at: key not found" };
        }
        return *value;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline const TValue& OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::at( const KeyType& key ) const
    {
        const TValue* value{ find( key ) };
        if ( !value )
        {
            throw std::out_of_range{ "OrderedHashMap::at: key not found" };
        }
        return *value;
    }

    //----------------------------------------------
    // Insertion
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insert( const TKey& key, const TValue& value )
    {
        if ( find( key ) != nullptr )
        {
            return false;
        }
        insertOrAssignInternal( key, value );
        return true;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insert( const TKey& key, TValue&& value )
    {
        if ( find( key ) != nullptr )
        {
            return false;
        }
        insertOrAssignInternal( key, std::forward<TValue>( value ) );
        return true;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insert( TKey&& key, TValue&& value )
    {
        if ( find( key ) != nullptr )
        {
            return false;
        }
        insertOrAssignInternal( std::forward<TKey>( key ), std::forward<TValue>( value ) );
        return true;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssign( const TKey& key, TValue&& value )
    {
        insertOrAssignInternal( key, std::forward<TValue>( value ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssign( const TKey& key, const TValue& value )
    {
        insertOrAssignInternal( key, value );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssign( TKey&& key, TValue&& value )
    {
        insertOrAssignInternal( std::forward<TKey>( key ), std::forward<TValue>( value ) );
    }

    //----------------------------------------------
    // Emplace operations
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::emplace( const TKey& key, Args&&... args )
    {
        insertOrAssignInternal( key, TValue( std::forward<Args>( args )... ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::emplace( TKey&& key, Args&&... args )
    {
        insertOrAssignInternal( std::move( key ), TValue( std::forward<Args>( args )... ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline std::pair<typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator, bool>
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::tryEmplace( const TKey& key, Args&&... args )
    {
        const HashType hash{ m_hasher( key ) };
        size_t pos{ static_cast<size_t>( hash & m_mask ) };
        uint32_t distance = 0;

        // Check if key already exists
        while ( m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].node->data.first, key ) )
            {
                return { Iterator{ m_buckets[pos].node, this }, false };
            }

            if ( distance > m_buckets[pos].distance )
            {
                break;
            }

            pos = ( pos + 1 ) & m_mask;
            ++distance;
        }

        // Key doesn't exist, insert new node
        if ( shouldResize() )
        {
            resize();
            // Recalculate position after resize
            pos = static_cast<size_t>( hash & m_mask );
            distance = 0;
        }

        // Create new node and append to list
        Node* newNode = new Node{ { key, TValue( std::forward<Args>( args )... ) }, nullptr, nullptr, hash };
        appendNode( newNode );

        // Insert into hash table using Robin Hood
        Bucket newBucket{ newNode, hash, distance, true };

        while ( m_buckets[pos].occupied )
        {
            if ( newBucket.distance > m_buckets[pos].distance )
            {
                // Robin Hood swap
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

        return { Iterator{ newNode, this }, true };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline std::pair<typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator, bool>
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::tryEmplace( TKey&& key, Args&&... args )
    {
        const HashType hash{ m_hasher( key ) };
        size_t pos{ static_cast<size_t>( hash & m_mask ) };
        uint32_t distance = 0;

        // Check if key already exists
        while ( m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].node->data.first, key ) )
            {
                return { Iterator{ m_buckets[pos].node, this }, false };
            }

            if ( distance > m_buckets[pos].distance )
            {
                break;
            }

            pos = ( pos + 1 ) & m_mask;
            ++distance;
        }

        // Key doesn't exist, insert new node
        if ( shouldResize() )
        {
            resize();
            // Recalculate position after resize
            pos = static_cast<size_t>( hash & m_mask );
            distance = 0;
        }

        // Create new node and append to list
        Node* newNode = new Node{ { std::move( key ), TValue( std::forward<Args>( args )... ) }, nullptr, nullptr, hash };
        appendNode( newNode );

        // Insert into hash table using Robin Hood
        Bucket newBucket{ newNode, hash, distance, true };

        while ( m_buckets[pos].occupied )
        {
            if ( newBucket.distance > m_buckets[pos].distance )
            {
                // Robin Hood swap
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

        return { Iterator{ newNode, this }, true };
    }

    //----------------------------------------------
    // Capacity and memory management
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::reserve( size_t minCapacity )
    {
        if ( minCapacity > m_capacity )
        {
            size_t newCapacity{ m_capacity };
            while ( newCapacity < minCapacity )
            {
                newCapacity <<= 1;
            }

            // Set capacity to the new value before rehashing
            m_capacity = newCapacity;
            m_mask = m_capacity - 1;

            std::vector<Bucket> oldBuckets{ std::move( m_buckets ) };
            m_buckets.clear();
            m_buckets.resize( m_capacity );

            // Rehash all nodes (preserves insertion order via linked list)
            Node* current = m_head;
            while ( current )
            {
                const HashType hash = current->hash;
                size_t pos = static_cast<size_t>( hash & m_mask );
                uint32_t distance = 0;

                Bucket newBucket{ current, hash, distance, true };

                while ( m_buckets[pos].occupied )
                {
                    if ( newBucket.distance > m_buckets[pos].distance )
                    {
                        Bucket temp{ std::move( m_buckets[pos] ) };
                        m_buckets[pos] = std::move( newBucket );
                        newBucket = std::move( temp );
                    }

                    pos = ( pos + 1 ) & m_mask;
                    ++newBucket.distance;
                }

                m_buckets[pos] = std::move( newBucket );
                current = current->next;
            }
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::erase( const KeyType& key ) noexcept
    {
        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        while ( distance <= m_buckets[pos].distance && m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].node->data.first, key ) )
            {
                eraseAtPosition( pos );
                return true;
            }
            pos = ( pos + 1 ) & m_mask;
            ++distance;
        }

        return false;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::erase( ConstIterator pos ) noexcept
    {
        if ( pos.m_node == nullptr )
        {
            return end();
        }

        Node* nextNode = pos.m_node->next;

        // Find the bucket containing this node
        const HashType hash( pos.m_node->hash );
        size_t bucketPos( static_cast<size_t>( hash & m_mask ) );

        while ( m_buckets[bucketPos].occupied )
        {
            if ( m_buckets[bucketPos].node == pos.m_node )
            {
                eraseAtPosition( bucketPos );
                break;
            }
            bucketPos = ( bucketPos + 1 ) & m_mask;
        }

        return Iterator{ nextNode, this };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::erase( ConstIterator first, ConstIterator last ) noexcept
    {
        while ( first != last )
        {
            first = erase( first );
        }
        return Iterator{ first.m_node, this };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::clear() noexcept
    {
        // Delete all nodes in the linked list
        Node* current = m_head;
        while ( current )
        {
            Node* next = current->next;
            delete current;
            current = next;
        }

        // Reset state
        m_head = nullptr;
        m_tail = nullptr;
        m_size = 0;

        // Clear buckets
        for ( size_t i = 0; i < m_capacity; ++i )
        {
            m_buckets[i] = Bucket{};
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline std::optional<std::pair<TKey, TValue>> OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::extract( const KeyType& key )
    {
        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        while ( distance <= m_buckets[pos].distance && m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].node->data.first, key ) )
            {
                Node* nodeToExtract = m_buckets[pos].node;

                // Extract the key-value pair before unlinking
                std::optional<std::pair<TKey, TValue>> extracted{ std::move( nodeToExtract->data ) };

                // Unlink from doubly-linked list
                unlinkNode( nodeToExtract );

                // Robin Hood backward shift deletion
                size_t nextPos{ ( pos + 1 ) & m_mask };

                while ( m_buckets[nextPos].occupied && m_buckets[nextPos].distance > 0 )
                {
                    m_buckets[pos] = std::move( m_buckets[nextPos] );
                    --m_buckets[pos].distance;
                    pos = nextPos;
                    nextPos = ( nextPos + 1 ) & m_mask;
                }

                m_buckets[pos] = Bucket{};

                // Delete the node
                delete nodeToExtract;
                --m_size;

                return extracted;
            }
            pos = ( pos + 1 ) & m_mask;
            ++distance;
        }

        return std::nullopt;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::merge( OrderedHashMap& other )
    {
        auto it = other.begin();
        while ( it != other.end() )
        {
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
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::merge( OrderedHashMap&& other )
    {
        merge( other );
    }

    //----------------------------------------------
    // State inspection
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline size_t OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::size() const noexcept
    {
        return m_size;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline size_t OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::capacity() const noexcept
    {
        return m_capacity;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::isEmpty() const noexcept
    {
        return m_size == 0;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::swap( OrderedHashMap& other ) noexcept
    {
        std::swap( m_buckets, other.m_buckets );
        std::swap( m_head, other.m_head );
        std::swap( m_tail, other.m_tail );
        std::swap( m_size, other.m_size );
        std::swap( m_capacity, other.m_capacity );
        std::swap( m_mask, other.m_mask );
        std::swap( m_hasher, other.m_hasher );
        std::swap( m_keyEqual, other.m_keyEqual );
    }

    //----------------------------------------------
    // STL-compatible iteration support (insertion order)
    //----------------------------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::begin() noexcept
    {
        return Iterator{ m_head, this };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::begin() const noexcept
    {
        return ConstIterator{ m_head, this };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::end() noexcept
    {
        return Iterator{ nullptr, this };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::end() const noexcept
    {
        return ConstIterator{ nullptr, this };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::cbegin() const noexcept
    {
        return ConstIterator{ m_head, this };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::cend() const noexcept
    {
        return ConstIterator{ nullptr, this };
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::operator==( const OrderedHashMap& other ) const noexcept
    {
        if ( m_size != other.m_size )
        {
            return false;
        }

        // Order-independent comparison: check all keys exist with same values
        for ( const auto& [key, value] : *this )
        {
            const TValue* otherValue = other.find( key );
            if ( otherValue == nullptr || *otherValue != value )
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
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssignInternal( const TKey& key, ValueType&& value )
    {
        if ( shouldResize() )
        {
            resize();
        }

        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        // First pass: check for existing key
        size_t searchPos = pos;
        uint32_t searchDist = 0;

        while ( m_buckets[searchPos].occupied )
        {
            if ( m_buckets[searchPos].hash == hash && keysEqual( m_buckets[searchPos].node->data.first, key ) )
            {
                // Update existing key (does NOT change insertion order)
                m_buckets[searchPos].node->data.second = std::forward<ValueType>( value );
                return;
            }

            if ( searchDist > m_buckets[searchPos].distance )
            {
                break;
            }

            searchPos = ( searchPos + 1 ) & m_mask;
            ++searchDist;
        }

        // Key doesn't exist, create new node and append to list
        Node* newNode = new Node{ { key, std::forward<ValueType>( value ) }, nullptr, nullptr, hash };
        appendNode( newNode );

        // Robin Hood insertion
        Bucket newBucket{ newNode, hash, distance, true };

        while ( m_buckets[pos].occupied )
        {
            if ( newBucket.distance > m_buckets[pos].distance )
            {
                // Robin Hood swap
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
    template <typename ValueType>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::insertOrAssignInternal( TKey&& key, ValueType&& value )
    {
        if ( shouldResize() )
        {
            resize();
        }

        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        // First pass: check for existing key
        size_t searchPos = pos;
        uint32_t searchDist = 0;

        while ( m_buckets[searchPos].occupied )
        {
            if ( m_buckets[searchPos].hash == hash && keysEqual( m_buckets[searchPos].node->data.first, key ) )
            {
                // Update existing key (does NOT change insertion order, key is discarded)
                m_buckets[searchPos].node->data.second = std::forward<ValueType>( value );
                return;
            }

            if ( searchDist > m_buckets[searchPos].distance )
            {
                break;
            }

            searchPos = ( searchPos + 1 ) & m_mask;
            ++searchDist;
        }

        // Key doesn't exist, create new node and append to list
        Node* newNode = new Node{ { std::move( key ), std::forward<ValueType>( value ) }, nullptr, nullptr, hash };
        appendNode( newNode );

        // Robin Hood insertion
        Bucket newBucket{ newNode, hash, distance, true };

        while ( m_buckets[pos].occupied )
        {
            if ( newBucket.distance > m_buckets[pos].distance )
            {
                // Robin Hood swap
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
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::shouldResize() const noexcept
    {
        return ( m_size * 100 ) >= ( m_capacity * MAX_LOAD_FACTOR_PERCENT );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::resize()
    {
        m_capacity <<= 1;
        m_mask = m_capacity - 1;

        std::vector<Bucket> oldBuckets{ std::move( m_buckets ) };
        m_buckets.clear();
        m_buckets.resize( m_capacity );

        // Rehash all nodes (preserves insertion order via linked list)
        Node* current = m_head;
        while ( current )
        {
            const HashType hash = current->hash;
            size_t pos = static_cast<size_t>( hash & m_mask );
            uint32_t distance = 0;

            Bucket newBucket{ current, hash, distance, true };

            while ( m_buckets[pos].occupied )
            {
                if ( newBucket.distance > m_buckets[pos].distance )
                {
                    Bucket temp{ std::move( m_buckets[pos] ) };
                    m_buckets[pos] = std::move( newBucket );
                    newBucket = std::move( temp );
                }

                pos = ( pos + 1 ) & m_mask;
                ++newBucket.distance;
            }

            m_buckets[pos] = std::move( newBucket );
            current = current->next;
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::eraseAtPosition( size_t pos ) noexcept
    {
        Node* nodeToDelete = m_buckets[pos].node;

        // Unlink from doubly-linked list
        unlinkNode( nodeToDelete );

        // Robin Hood backward shift deletion
        size_t nextPos{ ( pos + 1 ) & m_mask };

        while ( m_buckets[nextPos].occupied && m_buckets[nextPos].distance > 0 )
        {
            m_buckets[pos] = std::move( m_buckets[nextPos] );
            --m_buckets[pos].distance;
            pos = nextPos;
            nextPos = ( nextPos + 1 ) & m_mask;
        }

        m_buckets[pos] = Bucket{};

        // Delete the node
        delete nodeToDelete;
        --m_size;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::unlinkNode( Node* node ) noexcept
    {
        if ( node->prev )
        {
            node->prev->next = node->next;
        }
        else
        {
            m_head = node->next;
        }

        if ( node->next )
        {
            node->next->prev = node->prev;
        }
        else
        {
            m_tail = node->prev;
        }
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::appendNode( Node* node ) noexcept
    {
        node->prev = m_tail;
        node->next = nullptr;

        if ( m_tail )
        {
            m_tail->next = node;
        }
        else
        {
            m_head = node;
        }

        m_tail = node;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType1, typename KeyType2>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::keysEqual( const KeyType1& k1, const KeyType2& k2 ) const noexcept
    {
        return m_keyEqual( k1, k2 );
    }

    //----------------------------------------------
    // OrderedHashMap::Iterator class
    //----------------------------------------------

    //---------------------------
    // Construction
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::Iterator( Node* node )
        : m_node{ node }, m_container{ nullptr }
    {
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::Iterator( Node* node, const OrderedHashMap* container )
        : m_node{ node }, m_container{ container }
    {
    }

    //---------------------------
    // Operations
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::reference
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator*() const
    {
        return *std::launder( reinterpret_cast<pointer>( &m_node->data ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::pointer
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator->() const
    {
        return std::launder( reinterpret_cast<pointer>( &m_node->data ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator&
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator++()
    {
        m_node = m_node->next;
        return *this;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator++( int )
    {
        Iterator tmp = *this;
        ++( *this );
        return tmp;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator&
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator--()
    {
        // Special case: decrementing end() should go to tail
        if ( m_node == nullptr && m_container != nullptr )
        {
            m_node = m_container->m_tail;
        }
        else if ( m_node != nullptr )
        {
            m_node = m_node->prev;
        }
        return *this;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator--( int )
    {
        Iterator tmp = *this;
        --( *this );
        return tmp;
    }

    //---------------------------
    // Comparison
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator==( const Iterator& other ) const
    {
        return m_node == other.m_node;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::Iterator::operator!=( const Iterator& other ) const
    {
        return m_node != other.m_node;
    }

    //----------------------------------------------
    // OrderedHashMap::ConstIterator class
    //----------------------------------------------

    //---------------------------
    // Construction
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Node* node )
        : m_node{ node }, m_container{ nullptr }
    {
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Node* node, const OrderedHashMap* container )
        : m_node{ node }, m_container{ container }
    {
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Iterator& it )
        : m_node{ it.m_node }, m_container{ it.m_container }
    {
    }

    //---------------------------
    // Operations
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::reference
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator*() const
    {
        return *std::launder( reinterpret_cast<pointer>( &m_node->data ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::pointer
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator->() const
    {
        return std::launder( reinterpret_cast<pointer>( &m_node->data ) );
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator&
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator++()
    {
        m_node = m_node->next;
        return *this;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator++( int )
    {
        ConstIterator tmp = *this;
        ++( *this );
        return tmp;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator&
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator--()
    {
        // Special case: decrementing end() should go to tail
        if ( m_node == nullptr && m_container != nullptr )
        {
            m_node = m_container->m_tail;
        }
        else if ( m_node != nullptr )
        {
            m_node = m_node->prev;
        }
        return *this;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator--( int )
    {
        ConstIterator tmp = *this;
        --( *this );
        return tmp;
    }

    //---------------------------
    // Comparison
    //---------------------------

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator==( const ConstIterator& other ) const
    {
        return m_node == other.m_node;
    }

    template <typename TKey, typename TValue, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashMap<TKey, TValue, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator!=( const ConstIterator& other ) const
    {
        return m_node != other.m_node;
    }
} // namespace nfx::containers
