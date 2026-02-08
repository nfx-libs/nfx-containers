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
 * @file OrderedHashSet.inl
 * @brief Template implementation file for OrderedHashSet with Robin Hood hashing and insertion-order preservation
 * @details Contains template method implementations for hash table with Robin Hood displacement algorithm,
 *          heterogeneous lookup, and doubly-linked list for stable iteration order
 */

namespace nfx::containers
{
    //=====================================================================
    // OrderedHashSet class
    //=====================================================================

    //----------------------------------------------
    // Construction
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::OrderedHashSet()
        : m_capacity{ INITIAL_CAPACITY },
          m_mask{ INITIAL_CAPACITY - 1 }
    {
        m_buckets.resize( INITIAL_CAPACITY );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::OrderedHashSet( std::initializer_list<TKey> init )
        : OrderedHashSet{}
    {
        reserve( init.size() );
        for ( const auto& key : init )
        {
            insert( key );
        }
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename InputIt>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::OrderedHashSet( InputIt first, InputIt last )
        : OrderedHashSet{}
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
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::OrderedHashSet( size_t initialCapacity )
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
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::OrderedHashSet( OrderedHashSet&& other ) noexcept
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>&
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::operator=( OrderedHashSet&& other ) noexcept
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::OrderedHashSet( const OrderedHashSet& other )
        : m_capacity{ other.m_capacity },
          m_mask{ other.m_mask },
          m_hasher{ other.m_hasher },
          m_keyEqual{ other.m_keyEqual }
    {
        m_buckets.resize( m_capacity );

        // Deep copy: iterate in insertion order and insert
        for ( const auto& key : other )
        {
            insert( key );
        }
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>&
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::operator=( const OrderedHashSet& other )
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
            for ( const auto& key : other )
            {
                insert( key );
            }
        }
        return *this;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::~OrderedHashSet()
    {
        clear();
    }

    //----------------------------------------------
    // Core operations
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline const TKey* OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::find( const KeyType& key ) const noexcept
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

            if ( bucket.hash == hash && keysEqual( bucket.node->data, key ) )
            {
                return &bucket.node->data;
            }

            ++distance;
            pos = ( pos + 1 ) & m_mask;
        }
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::contains( const KeyType& key ) const noexcept
    {
        return find( key ) != nullptr;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline const TKey& OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::at( const KeyType& key ) const
    {
        const TKey* value = find( key );
        if ( !value )
        {
            throw std::out_of_range( "OrderedHashSet::at: key not found" );
        }
        return *value;
    }

    //----------------------------------------------
    // Insertion
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::insert( const TKey& key )
    {
        return insertInternal( key );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::insert( TKey&& key )
    {
        return insertInternal( std::move( key ) );
    }

    //----------------------------------------------
    // Emplace operations
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::emplace( Args&&... args )
    {
        return insertInternal( TKey( std::forward<Args>( args )... ) );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename... Args>
    inline std::pair<typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator, bool>
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::tryEmplace( Args&&... args )
    {
        TKey key( std::forward<Args>( args )... );
        const HashType hash{ m_hasher( key ) };
        size_t pos{ static_cast<size_t>( hash & m_mask ) };
        uint32_t distance = 0;

        // Check if key already exists
        while ( m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].node->data, key ) )
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
        Node* newNode = new Node{ std::move( key ), nullptr, nullptr, hash };
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::reserve( size_t minCapacity )
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::erase( const KeyType& key ) noexcept
    {
        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        while ( distance <= m_buckets[pos].distance && m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].node->data, key ) )
            {
                eraseAtPosition( pos );
                return true;
            }
            pos = ( pos + 1 ) & m_mask;
            ++distance;
        }

        return false;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::erase( ConstIterator pos ) noexcept
    {
        if ( pos.m_node == nullptr )
        {
            return end();
        }

        Node* nextNode = pos.m_node->next;

        // Find the bucket containing this node
        const HashType hash( pos.m_node->hash );
        size_t bucketPos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance = 0;

        while ( m_buckets[bucketPos].occupied )
        {
            if ( m_buckets[bucketPos].node == pos.m_node )
            {
                eraseAtPosition( bucketPos );
                break;
            }
            bucketPos = ( bucketPos + 1 ) & m_mask;
            ++distance;
        }

        return Iterator{ nextNode, this };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::erase( ConstIterator first, ConstIterator last ) noexcept
    {
        while ( first != last )
        {
            first = erase( first );
        }
        return Iterator{ first.m_node, this };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::clear() noexcept
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType>
    inline std::optional<TKey> OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::extract( const KeyType& key )
    {
        const HashType hash( m_hasher( key ) );

        size_t pos( static_cast<size_t>( hash & m_mask ) );
        uint32_t distance( 0 );

        while ( distance <= m_buckets[pos].distance && m_buckets[pos].occupied )
        {
            if ( m_buckets[pos].hash == hash && keysEqual( m_buckets[pos].node->data, key ) )
            {
                Node* nodeToExtract = m_buckets[pos].node;

                // Extract the key before unlinking
                std::optional<TKey> extracted{ std::move( nodeToExtract->data ) };

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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::merge( OrderedHashSet& other )
    {
        auto it = other.begin();
        while ( it != other.end() )
        {
            if ( !contains( *it ) )
            {
                // Extract from other and insert into this
                auto extracted = other.extract( *it );
                if ( extracted )
                {
                    insert( std::move( *extracted ) );
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::merge( OrderedHashSet&& other )
    {
        merge( other );
    }

    //----------------------------------------------
    // State inspection
    //----------------------------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline size_t OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::size() const noexcept
    {
        return m_size;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline size_t OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::capacity() const noexcept
    {
        return m_capacity;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::isEmpty() const noexcept
    {
        return m_size == 0;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::swap( OrderedHashSet& other ) noexcept
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::begin() noexcept
    {
        return Iterator{ m_head, this };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::begin() const noexcept
    {
        return ConstIterator{ m_head, this };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::end() noexcept
    {
        return Iterator{ nullptr, this };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::end() const noexcept
    {
        return ConstIterator{ nullptr, this };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::cbegin() const noexcept
    {
        return ConstIterator{ m_head, this };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::cend() const noexcept
    {
        return ConstIterator{ nullptr, this };
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::operator==( const OrderedHashSet& other ) const noexcept
    {
        if ( m_size != other.m_size )
        {
            return false;
        }

        // Order-independent comparison: check all keys exist
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
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::insertInternal( const TKey& key )
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
            if ( m_buckets[searchPos].hash == hash && keysEqual( m_buckets[searchPos].node->data, key ) )
            {
                // Key already exists
                return false;
            }

            if ( searchDist > m_buckets[searchPos].distance )
            {
                break;
            }

            searchPos = ( searchPos + 1 ) & m_mask;
            ++searchDist;
        }

        // Key doesn't exist, create new node and append to list
        Node* newNode = new Node{ key, nullptr, nullptr, hash };
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
        return true;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::insertInternal( TKey&& key )
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
            if ( m_buckets[searchPos].hash == hash && keysEqual( m_buckets[searchPos].node->data, key ) )
            {
                // Key already exists
                return false;
            }

            if ( searchDist > m_buckets[searchPos].distance )
            {
                break;
            }

            searchPos = ( searchPos + 1 ) & m_mask;
            ++searchDist;
        }

        // Key doesn't exist, create new node and append to list
        Node* newNode = new Node{ std::move( key ), nullptr, nullptr, hash };
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
        return true;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::shouldResize() const noexcept
    {
        return ( m_size * 100 ) >= ( m_capacity * MAX_LOAD_FACTOR_PERCENT );
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::resize()
    {
        const size_t oldCapacity{ m_capacity };
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::eraseAtPosition( size_t pos ) noexcept
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::unlinkNode( Node* node ) noexcept
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline void OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::appendNode( Node* node ) noexcept
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    template <typename KeyType1, typename KeyType2>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::keysEqual( const KeyType1& k1, const KeyType2& k2 ) const noexcept
    {
        return m_keyEqual( k1, k2 );
    }

    //----------------------------------------------
    // OrderedHashSet::Iterator class
    //----------------------------------------------

    //---------------------------
    // Construction
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::Iterator( Node* node )
        : m_node{ node }, m_container{ nullptr }
    {
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::Iterator( Node* node, const OrderedHashSet* container )
        : m_node{ node }, m_container{ container }
    {
    }

    //---------------------------
    // Operations
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::reference
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator*() const
    {
        return m_node->data;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::pointer
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator->() const
    {
        return &m_node->data;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator&
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator++()
    {
        m_node = m_node->next;
        return *this;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator++( int )
    {
        Iterator tmp = *this;
        ++( *this );
        return tmp;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator&
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator--()
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator--( int )
    {
        Iterator tmp = *this;
        --( *this );
        return tmp;
    }

    //---------------------------
    // Comparison
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator==( const Iterator& other ) const
    {
        return m_node == other.m_node;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::Iterator::operator!=( const Iterator& other ) const
    {
        return m_node != other.m_node;
    }

    //----------------------------------------------
    // OrderedHashSet::ConstIterator class
    //----------------------------------------------

    //---------------------------
    // Construction
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Node* node )
        : m_node{ node }, m_container{ nullptr }
    {
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Node* node, const OrderedHashSet* container )
        : m_node{ node }, m_container{ container }
    {
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::ConstIterator( const Iterator& it )
        : m_node{ it.m_node }, m_container{ it.m_container }
    {
    }

    //---------------------------
    // Operations
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::reference
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator*() const
    {
        return m_node->data;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::pointer
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator->() const
    {
        return &m_node->data;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator&
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator++()
    {
        m_node = m_node->next;
        return *this;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator++( int )
    {
        ConstIterator tmp = *this;
        ++( *this );
        return tmp;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator&
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator--()
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

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline typename OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator
    OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator--( int )
    {
        ConstIterator tmp = *this;
        --( *this );
        return tmp;
    }

    //---------------------------
    // Comparison
    //---------------------------

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator==( const ConstIterator& other ) const
    {
        return m_node == other.m_node;
    }

    template <typename TKey, hashing::Hash32or64 HashType, HashType Seed, typename THasher, typename KeyEqual>
    inline bool OrderedHashSet<TKey, HashType, Seed, THasher, KeyEqual>::ConstIterator::operator!=( const ConstIterator& other ) const
    {
        return m_node != other.m_node;
    }
} // namespace nfx::containers
