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
 * @file StackHashMap.inl
 * @brief Implementation of StackHashMap template methods
 */

#pragma once

#include <stdexcept>

namespace nfx::containers
{
    //=====================================================================
    // Construction
    //=====================================================================

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline StackHashMap<TKey, TValue, N, KeyEqual>::StackHashMap()
        : m_stack{}, m_stackSize{ 0 }, m_heap{ nullptr }
    {
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline StackHashMap<TKey, TValue, N, KeyEqual>::StackHashMap( std::initializer_list<std::pair<TKey, TValue>> init )
        : StackHashMap{}
    {
        for ( const auto& item : init )
        {
            insert( item );
        }
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    template <typename InputIt>
    inline StackHashMap<TKey, TValue, N, KeyEqual>::StackHashMap( InputIt first, InputIt last )
        : StackHashMap()
    {
        for ( auto it = first; it != last; ++it )
        {
            insert( *it );
        }
    }

    //=====================================================================
    // Capacity
    //=====================================================================

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline bool StackHashMap<TKey, TValue, N, KeyEqual>::isEmpty() const noexcept
    {
        return isOnStack() ? m_stackSize == 0 : m_heap->isEmpty();
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline size_t StackHashMap<TKey, TValue, N, KeyEqual>::size() const noexcept
    {
        return isOnStack() ? m_stackSize : m_heap->size();
    }

    //=====================================================================
    // Element access
    //=====================================================================

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline TValue& StackHashMap<TKey, TValue, N, KeyEqual>::at( const TKey& key )
    {
        if ( isOnStack() )
        {
            for ( auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( entry.data->first, key ) )
                {
                    return entry.data->second;
                }
            }
            throw std::out_of_range{ "StackHashMap::at: key not found" };
        }
        return m_heap->at( key );
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline const TValue& StackHashMap<TKey, TValue, N, KeyEqual>::at( const TKey& key ) const
    {
        if ( isOnStack() )
        {
            for ( const auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( entry.data->first, key ) )
                {
                    return entry.data->second;
                }
            }
            throw std::out_of_range{ "StackHashMap::at: key not found" };
        }
        return m_heap->at( key );
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline TValue& StackHashMap<TKey, TValue, N, KeyEqual>::operator[]( const TKey& key )
    {
        if ( isOnStack() )
        {
            // Search existing
            for ( auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( entry.data->first, key ) )
                {
                    return entry.data->second;
                }
            }

            // Insert new
            if ( m_stackSize < N )
            {
                m_stack[m_stackSize].data.emplace( key, TValue{} );
                return m_stack[m_stackSize++].data->second;
            }

            // Transition to heap
            transitionToHeap();
        }

        return ( *m_heap )[key];
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline TValue& StackHashMap<TKey, TValue, N, KeyEqual>::operator[]( TKey&& key )
    {
        if ( isOnStack() )
        {
            // Search existing
            for ( auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( entry.data->first, key ) )
                {
                    return entry.data->second;
                }
            }

            // Insert new
            if ( m_stackSize < N )
            {
                m_stack[m_stackSize].data.emplace( std::move( key ), TValue{} );
                return m_stack[m_stackSize++].data->second;
            }

            // Transition to heap
            transitionToHeap();
        }

        return ( *m_heap )[std::move( key )];
    }

    //=====================================================================
    // Modifiers
    //=====================================================================

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline std::pair<typename StackHashMap<TKey, TValue, N, KeyEqual>::value_type*, bool>
    StackHashMap<TKey, TValue, N, KeyEqual>::insert( const std::pair<TKey, TValue>& value )
    {
        if ( isOnStack() )
        {
            // Check if exists
            for ( size_t i{ 0 }; i < m_stackSize; ++i )
            {
                if ( m_stack[i].data.has_value() && KeyEqual{}( m_stack[i].data->first, value.first ) )
                {
                    // Key exists, return existing
                    return { reinterpret_cast<value_type*>( &m_stack[i].data.value() ), false };
                }
            }

            // Insert new
            if ( m_stackSize < N )
            {
                m_stack[m_stackSize].data = value;
                return { reinterpret_cast<value_type*>( &m_stack[m_stackSize++].data.value() ), true };
            }

            // Transition to heap
            transitionToHeap();
        }

        auto result = m_heap->insert( value.first, value.second );
        auto* val = m_heap->find( value.first );
        return { reinterpret_cast<value_type*>( val ), result };
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline std::pair<typename StackHashMap<TKey, TValue, N, KeyEqual>::value_type*, bool>
    StackHashMap<TKey, TValue, N, KeyEqual>::insert( std::pair<TKey, TValue>&& value )
    {
        if ( isOnStack() )
        {
            // Check if exists
            for ( size_t i{ 0 }; i < m_stackSize; ++i )
            {
                if ( m_stack[i].data.has_value() && KeyEqual{}( m_stack[i].data->first, value.first ) )
                {
                    return { reinterpret_cast<value_type*>( &m_stack[i].data.value() ), false };
                }
            }

            // Insert new
            if ( m_stackSize < N )
            {
                m_stack[m_stackSize].data = std::move( value );
                return { reinterpret_cast<value_type*>( &m_stack[m_stackSize++].data.value() ), true };
            }

            // Transition to heap
            transitionToHeap();
        }

        auto result = m_heap->insert( std::move( value.first ), std::move( value.second ) );
        auto* val = m_heap->find( value.first );
        return { reinterpret_cast<value_type*>( val ), result };
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    template <typename... Args>
    inline std::pair<typename StackHashMap<TKey, TValue, N, KeyEqual>::value_type*, bool>
    StackHashMap<TKey, TValue, N, KeyEqual>::emplace( Args&&... args )
    {
        return insert( std::pair<TKey, TValue>( std::forward<Args>( args )... ) );
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline void StackHashMap<TKey, TValue, N, KeyEqual>::insertOrAssign( const TKey& key, const TValue& value )
    {
        if ( TValue* existing = find( key ) )
        {
            *existing = value;
            return;
        }
        insert( std::pair<TKey, TValue>{ key, value } );
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline void StackHashMap<TKey, TValue, N, KeyEqual>::insertOrAssign( const TKey& key, TValue&& value )
    {
        if ( TValue* existing = find( key ) )
        {
            *existing = std::move( value );
            return;
        }
        insert( std::pair<TKey, TValue>{ key, std::move( value ) } );
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline void StackHashMap<TKey, TValue, N, KeyEqual>::insertOrAssign( TKey&& key, TValue&& value )
    {
        if ( TValue* existing = find( key ) )
        {
            *existing = std::move( value );
            return;
        }
        insert( std::pair<TKey, TValue>{ std::move( key ), std::move( value ) } );
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline size_t StackHashMap<TKey, TValue, N, KeyEqual>::erase( const TKey& key )
    {
        if ( isOnStack() )
        {
            for ( size_t i{ 0 }; i < m_stackSize; ++i )
            {
                if ( m_stack[i].data.has_value() && KeyEqual{}( m_stack[i].data->first, key ) )
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

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline void StackHashMap<TKey, TValue, N, KeyEqual>::clear() noexcept
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

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline std::optional<std::pair<TKey, TValue>> StackHashMap<TKey, TValue, N, KeyEqual>::extract( const TKey& key )
    {
        if ( isOnStack() )
        {
            for ( size_t i{ 0 }; i < m_stackSize; ++i )
            {
                if ( m_stack[i].data.has_value() && KeyEqual{}( m_stack[i].data->first, key ) )
                {
                    // Extract the entire pair
                    std::pair<TKey, TValue> pair{ std::move( *m_stack[i].data ) };

                    // Shift remaining elements
                    for ( size_t j{ i }; j < m_stackSize - 1; ++j )
                    {
                        m_stack[j] = std::move( m_stack[j + 1] );
                    }
                    m_stack[m_stackSize - 1].data.reset();
                    --m_stackSize;

                    return pair;
                }
            }
            return std::nullopt;
        }
        return m_heap->extract( key );
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline void StackHashMap<TKey, TValue, N, KeyEqual>::merge( StackHashMap& other )
    {
        if ( isOnStack() && other.isOnStack() )
        {
            // Both on stack - manual transfer
            for ( size_t i{ 0 }; i < other.m_stackSize; ++i )
            {
                if ( other.m_stack[i].data.has_value() )
                {
                    auto& [key, value]{ *other.m_stack[i].data };

                    // Try to insert (only if key doesn't exist)
                    if ( !contains( key ) )
                    {
                        insert( { std::move( key ), std::move( value ) } );
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
                    auto& [key, value]{ *other.m_stack[i].data };
                    if ( !m_heap->contains( key ) )
                    {
                        m_heap->insert( std::move( key ), std::move( value ) );
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

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline void StackHashMap<TKey, TValue, N, KeyEqual>::merge( StackHashMap&& other )
    {
        merge( other ); // Delegate to lvalue overload
    }

    //=====================================================================
    // Lookup
    //=====================================================================

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline TValue* StackHashMap<TKey, TValue, N, KeyEqual>::find( const TKey& key ) noexcept
    {
        if ( isOnStack() )
        {
            for ( auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( entry.data->first, key ) )
                {
                    return &entry.data->second;
                }
            }
            return nullptr;
        }
        return m_heap->find( key );
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline const TValue* StackHashMap<TKey, TValue, N, KeyEqual>::find( const TKey& key ) const noexcept
    {
        if ( isOnStack() )
        {
            for ( const auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( entry.data->first, key ) )
                {
                    return &entry.data->second;
                }
            }
            return nullptr;
        }
        return m_heap->find( key );
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline size_t StackHashMap<TKey, TValue, N, KeyEqual>::count( const TKey& key ) const
    {
        if ( isOnStack() )
        {
            for ( const auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( entry.data->first, key ) )
                {
                    return 1;
                }
            }
            return 0;
        }
        return m_heap->contains( key ) ? 1 : 0;
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline bool StackHashMap<TKey, TValue, N, KeyEqual>::contains( const TKey& key ) const
    {
        return count( key ) > 0;
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    template <typename K>
    inline bool StackHashMap<TKey, TValue, N, KeyEqual>::contains( const K& key ) const
        requires requires( KeyEqual eq, const K& k, const TKey& t ) { eq( k, t ); }
    {
        if ( isOnStack() )
        {
            for ( const auto& entry : m_stack )
            {
                if ( entry.data.has_value() && KeyEqual{}( key, entry.data->first ) )
                {
                    return true;
                }
            }
            return false;
        }
        return m_heap->contains( key );
    }

    //=====================================================================
    // Private implementation
    //=====================================================================

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline bool StackHashMap<TKey, TValue, N, KeyEqual>::isOnStack() const noexcept
    {
        return m_heap == nullptr;
    }

    template <typename TKey, typename TValue, size_t N, typename KeyEqual>
    inline void StackHashMap<TKey, TValue, N, KeyEqual>::transitionToHeap()
    {
        m_heap = std::make_unique<FastHashMap<TKey, TValue>>();
        m_heap->reserve( N * 2 );

        // Move all stack entries to heap
        for ( auto& entry : m_stack )
        {
            if ( entry.data.has_value() )
            {
                auto& [key, value]{ *entry.data };
                m_heap->insert( std::move( key ), std::move( value ) );
                entry.data.reset();
            }
        }
        m_stackSize = 0;
    }
} // namespace nfx::containers
