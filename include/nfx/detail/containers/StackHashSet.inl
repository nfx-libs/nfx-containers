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
 * @file StackHashSet.inl
 * @brief Implementation of StackHashSet template methods
 */

#pragma once

#include <stdexcept>

namespace nfx::containers
{
    //=====================================================================
    // Construction
    //=====================================================================

    template <typename TKey, size_t N, typename KeyEqual>
    inline StackHashSet<TKey, N, KeyEqual>::StackHashSet()
        : m_stack{}, m_stackSize{ 0 }, m_heap{ nullptr }
    {
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline StackHashSet<TKey, N, KeyEqual>::StackHashSet( std::initializer_list<TKey> init )
        : StackHashSet{}
    {
        for ( const auto& item : init )
        {
            insert( item );
        }
    }

    template <typename TKey, size_t N, typename KeyEqual>
    template <typename InputIt>
    inline StackHashSet<TKey, N, KeyEqual>::StackHashSet( InputIt first, InputIt last )
        : StackHashSet()
    {
        for ( auto it = first; it != last; ++it )
        {
            insert( *it );
        }
    }

    //=====================================================================
    // Capacity
    //=====================================================================

    template <typename TKey, size_t N, typename KeyEqual>
    inline bool StackHashSet<TKey, N, KeyEqual>::isEmpty() const noexcept
    {
        return isOnStack() ? m_stackSize == 0 : m_heap->isEmpty();
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline size_t StackHashSet<TKey, N, KeyEqual>::size() const noexcept
    {
        return isOnStack() ? m_stackSize : m_heap->size();
    }

    //=====================================================================
    // Modifiers
    //=====================================================================

    template <typename TKey, size_t N, typename KeyEqual>
    inline std::pair<const typename StackHashSet<TKey, N, KeyEqual>::value_type*, bool>
    StackHashSet<TKey, N, KeyEqual>::insert( const TKey& key )
    {
        if ( isOnStack() )
        {
            // Check if exists
            for ( size_t i{ 0 }; i < m_stackSize; ++i )
            {
                if ( m_stack[i].data.has_value() && KeyEqual{}( *m_stack[i].data, key ) )
                {
                    // Key exists, return existing
                    return { &m_stack[i].data.value(), false };
                }
            }

            // Insert new
            if ( m_stackSize < N )
            {
                m_stack[m_stackSize].data = key;
                return { &m_stack[m_stackSize++].data.value(), true };
            }

            // Transition to heap
            transitionToHeap();
        }

        auto result = m_heap->insert( key );
        auto* val = m_heap->find( key );
        return { val, result };
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline std::pair<const typename StackHashSet<TKey, N, KeyEqual>::value_type*, bool>
    StackHashSet<TKey, N, KeyEqual>::insert( TKey&& key )
    {
        if ( isOnStack() )
        {
            // Check if exists
            for ( size_t i{ 0 }; i < m_stackSize; ++i )
            {
                if ( m_stack[i].data.has_value() && KeyEqual{}( *m_stack[i].data, key ) )
                {
                    return { &m_stack[i].data.value(), false };
                }
            }

            // Insert new
            if ( m_stackSize < N )
            {
                m_stack[m_stackSize].data = std::move( key );
                return { &m_stack[m_stackSize++].data.value(), true };
            }

            // Transition to heap
            transitionToHeap();
        }

        auto result = m_heap->insert( std::move( key ) );
        auto* val = m_heap->find( key );
        return { val, result };
    }

    template <typename TKey, size_t N, typename KeyEqual>
    template <typename... Args>
    inline std::pair<const typename StackHashSet<TKey, N, KeyEqual>::value_type*, bool>
    StackHashSet<TKey, N, KeyEqual>::emplace( Args&&... args )
    {
        return insert( TKey( std::forward<Args>( args )... ) );
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline size_t StackHashSet<TKey, N, KeyEqual>::erase( const TKey& key )
    {
        if ( isOnStack() )
        {
            for ( size_t i{ 0 }; i < m_stackSize; ++i )
            {
                if ( m_stack[i].data.has_value() && KeyEqual{}( *m_stack[i].data, key ) )
                {
                    // Shift remaining elements
                    for ( size_t j{ i }; j < m_stackSize - 1; ++j )
                    {
                        m_stack[j] = std::move( m_stack[j + 1] );
                    }
                    m_stack[m_stackSize - 1].data.reset();
                    --m_stackSize;
                    return 1;
                }
            }
            return 0;
        }
        return m_heap->erase( key ) ? 1 : 0;
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline void StackHashSet<TKey, N, KeyEqual>::clear() noexcept
    {
        if ( isOnStack() )
        {
            for ( size_t i{ 0 }; i < m_stackSize; ++i )
            {
                m_stack[i].data.reset();
            }
            m_stackSize = 0;
        }
        else
        {
            m_heap->clear();
        }
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline std::optional<TKey> StackHashSet<TKey, N, KeyEqual>::extract( const TKey& key )
    {
        if ( isOnStack() )
        {
            for ( size_t i{ 0 }; i < m_stackSize; ++i )
            {
                if ( m_stack[i].data.has_value() && KeyEqual{}( *m_stack[i].data, key ) )
                {
                    // Extract the key
                    TKey extractedKey{ std::move( *m_stack[i].data ) };

                    // Shift remaining elements
                    for ( size_t j{ i }; j < m_stackSize - 1; ++j )
                    {
                        m_stack[j] = std::move( m_stack[j + 1] );
                    }
                    m_stack[m_stackSize - 1].data.reset();
                    --m_stackSize;

                    return extractedKey;
                }
            }
            return std::nullopt;
        }
        return m_heap->extract( key );
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline void StackHashSet<TKey, N, KeyEqual>::merge( StackHashSet& other )
    {
        if ( isOnStack() && other.isOnStack() )
        {
            // Both on stack - manual transfer
            for ( size_t i{ 0 }; i < other.m_stackSize; ++i )
            {
                if ( other.m_stack[i].data.has_value() )
                {
                    // Try to insert (only if key doesn't exist)
                    if ( !contains( *other.m_stack[i].data ) )
                    {
                        insert( std::move( *other.m_stack[i].data ) );
                    }
                }
            }
        }
        else if ( !isOnStack() && !other.isOnStack() )
        {
            // Both on heap - delegate
            m_heap->merge( *other.m_heap );
        }
        else if ( !isOnStack() && other.isOnStack() )
        {
            // This on heap, other on stack
            for ( size_t i{ 0 }; i < other.m_stackSize; ++i )
            {
                if ( other.m_stack[i].data.has_value() )
                {
                    if ( !m_heap->contains( *other.m_stack[i].data ) )
                    {
                        m_heap->insert( std::move( *other.m_stack[i].data ) );
                    }
                }
            }
        }
        else
        {
            // This on stack, other on heap - transition this to heap first
            transitionToHeap();
            m_heap->merge( *other.m_heap );
        }
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline void StackHashSet<TKey, N, KeyEqual>::merge( StackHashSet&& other )
    {
        merge( other ); // Delegate to lvalue overload
    }

    //=====================================================================
    // Lookup
    //=====================================================================

    template <typename TKey, size_t N, typename KeyEqual>
    inline const TKey* StackHashSet<TKey, N, KeyEqual>::find( const TKey& key ) const noexcept
    {
        if ( isOnStack() )
        {
            // Linear search on stack storage
            for ( const auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( *entry.data, key ) )
                {
                    return &*entry.data;
                }
            }
            return nullptr;
        }
        // Delegate to heap storage
        return m_heap->find( key );
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline bool StackHashSet<TKey, N, KeyEqual>::contains( const TKey& key ) const
    {
        return find( key ) != nullptr;
    }

    template <typename TKey, size_t N, typename KeyEqual>
    template <typename K>
    inline bool StackHashSet<TKey, N, KeyEqual>::contains( const K& key ) const
        requires requires( KeyEqual eq, const K& k, const TKey& t ) { eq( k, t ); }
    {
        if ( isOnStack() )
        {
            for ( const auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( key, *entry.data ) )
                {
                    return true;
                }
            }
            return false;
        }
        return m_heap->contains( key );
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline const TKey& StackHashSet<TKey, N, KeyEqual>::at( const TKey& key ) const
    {
        const TKey* result{ find( key ) };
        if ( !result )
        {
            throw std::out_of_range{ "StackHashSet::at: key not found" };
        }
        return *result;
    }

    //=====================================================================
    // Private implementation
    //=====================================================================

    template <typename TKey, size_t N, typename KeyEqual>
    inline bool StackHashSet<TKey, N, KeyEqual>::isOnStack() const noexcept
    {
        return m_heap == nullptr;
    }

    template <typename TKey, size_t N, typename KeyEqual>
    inline void StackHashSet<TKey, N, KeyEqual>::transitionToHeap()
    {
        m_heap = std::make_unique<FastHashSet<TKey>>();
        m_heap->reserve( N * 2 );

        // Move all stack entries to heap
        for ( auto& entry : m_stack )
        {
            if ( entry.data.has_value() )
            {
                m_heap->insert( std::move( *entry.data ) );
                entry.data.reset();
            }
        }
        m_stackSize = 0;
    }
} // namespace nfx::containers
