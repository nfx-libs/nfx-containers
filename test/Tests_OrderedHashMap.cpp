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
 * @file Tests_OrderedHashMap.cpp
 * @brief Tests for OrderedHashMap (Robin Hood hashing with insertion-order preservation)
 */

#include <gtest/gtest.h>

#include <nfx/Containers.h>

#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace nfx::containers::test
{
    using namespace nfx::hashing;

    //=====================================================================
    // Insertion-order preservation tests
    //=====================================================================

    TEST( OrderedHashMapTests, InsertionOrder_BasicPreservation )
    {
        OrderedHashMap<std::string, int> map;

        map.insertOrAssign( "third", 3 );
        map.insertOrAssign( "first", 1 );
        map.insertOrAssign( "second", 2 );

        // Verify iteration follows insertion order, not key order
        std::vector<std::string> keys;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
        }

        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "third" );  // First inserted
        EXPECT_EQ( keys[1], "first" );  // Second inserted
        EXPECT_EQ( keys[2], "second" ); // Third inserted
    }

    TEST( OrderedHashMapTests, InsertionOrder_InitializerList )
    {
        OrderedHashMap<std::string, int> map = {
            { "zebra", 26 },
            { "alpha", 1 },
            { "beta", 2 } };

        std::vector<std::string> keys;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
        }

        // Order matches initializer list, not alphabetical
        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "zebra" );
        EXPECT_EQ( keys[1], "alpha" );
        EXPECT_EQ( keys[2], "beta" );
    }

    TEST( OrderedHashMapTests, InsertionOrder_UpdateDoesNotReorder )
    {
        OrderedHashMap<std::string, int> map;

        map.insertOrAssign( "a", 1 );
        map.insertOrAssign( "b", 2 );
        map.insertOrAssign( "c", 3 );
        map.insertOrAssign( "b", 999 ); // Update existing key

        std::vector<std::string> keys;
        std::vector<int> values;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
            values.push_back( value );
        }

        // "b" stays in original position despite update
        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "b" );
        EXPECT_EQ( keys[2], "c" );
        EXPECT_EQ( values[1], 999 ); // Value updated
    }

    TEST( OrderedHashMapTests, InsertionOrder_AfterErase )
    {
        OrderedHashMap<std::string, int> map;

        map.insertOrAssign( "one", 1 );
        map.insertOrAssign( "two", 2 );
        map.insertOrAssign( "three", 3 );
        map.insertOrAssign( "four", 4 );

        map.erase( "two" );

        std::vector<std::string> keys;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
        }

        // Order preserved after erase
        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "one" );
        EXPECT_EQ( keys[1], "three" );
        EXPECT_EQ( keys[2], "four" );
    }

    TEST( OrderedHashMapTests, InsertionOrder_IntegerKeys )
    {
        OrderedHashMap<int, std::string> map;

        map.insertOrAssign( 100, "hundred" );
        map.insertOrAssign( 1, "one" );
        map.insertOrAssign( 50, "fifty" );

        std::vector<int> keys;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
        }

        // Insertion order, not sorted by key value
        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], 100 );
        EXPECT_EQ( keys[1], 1 );
        EXPECT_EQ( keys[2], 50 );
    }

    //=====================================================================
    // Bidirectional iterator tests
    //=====================================================================

    TEST( OrderedHashMapTests, BidirectionalIterator_ForwardIteration )
    {
        OrderedHashMap<std::string, int> map = { { "a", 1 }, { "b", 2 }, { "c", 3 } };

        std::vector<std::string> keys;
        for ( auto it = map.begin(); it != map.end(); ++it )
        {
            keys.push_back( it->first );
        }

        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "b" );
        EXPECT_EQ( keys[2], "c" );
    }

    TEST( OrderedHashMapTests, BidirectionalIterator_BackwardIteration )
    {
        OrderedHashMap<std::string, int> map = { { "a", 1 }, { "b", 2 }, { "c", 3 } };

        std::vector<std::string> keys;
        for ( auto it = --map.end();; --it )
        {
            keys.push_back( it->first );
            if ( it == map.begin() )
                break;
        }

        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "c" );
        EXPECT_EQ( keys[1], "b" );
        EXPECT_EQ( keys[2], "a" );
    }

    TEST( OrderedHashMapTests, BidirectionalIterator_PostIncrement )
    {
        OrderedHashMap<std::string, int> map = { { "x", 1 }, { "y", 2 } };

        auto it = map.begin();
        auto prev = it++;

        EXPECT_EQ( prev->first, "x" );
        EXPECT_EQ( it->first, "y" );
    }

    TEST( OrderedHashMapTests, BidirectionalIterator_PostDecrement )
    {
        OrderedHashMap<std::string, int> map = { { "x", 1 }, { "y", 2 } };

        auto it = --map.end();
        auto prev = it--;

        EXPECT_EQ( prev->first, "y" );
        EXPECT_EQ( it->first, "x" );
    }

    //=====================================================================
    // Constructor tests - initializer_list
    //=====================================================================

    TEST( OrderedHashMapTests, InitializerListConstructor_Basic )
    {
        OrderedHashMap<std::string, int> map = { { "apple", 1 }, { "banana", 2 }, { "cherry", 3 } };

        EXPECT_EQ( map.size(), 3 );
        EXPECT_FALSE( map.isEmpty() );

        auto* value1 = map.find( "apple" );
        auto* value2 = map.find( "banana" );
        auto* value3 = map.find( "cherry" );

        ASSERT_NE( value1, nullptr );
        ASSERT_NE( value2, nullptr );
        ASSERT_NE( value3, nullptr );

        EXPECT_EQ( *value1, 1 );
        EXPECT_EQ( *value2, 2 );
        EXPECT_EQ( *value3, 3 );
    }

    TEST( OrderedHashMapTests, InitializerListConstructor_Empty )
    {
        OrderedHashMap<std::string, int> map = {};

        EXPECT_EQ( map.size(), 0 );
        EXPECT_TRUE( map.isEmpty() );
    }

    TEST( OrderedHashMapTests, InitializerListConstructor_SingleElement )
    {
        OrderedHashMap<std::string, int> map = { { "single", 42 } };

        EXPECT_EQ( map.size(), 1 );
        auto* value = map.find( "single" );
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, 42 );
    }

    TEST( OrderedHashMapTests, InitializerListConstructor_DuplicateKeys )
    {
        // Last value wins with duplicate keys
        OrderedHashMap<std::string, int> map = { { "key", 1 }, { "key", 2 }, { "key", 3 } };

        EXPECT_EQ( map.size(), 1 );
        auto* value = map.find( "key" );
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, 3 ); // Last value should win
    }

    //=====================================================================
    // Range constructor tests
    //=====================================================================

    TEST( OrderedHashMapTests, RangeConstructor_FromVector )
    {
        std::vector<std::pair<std::string, int>> data = {
            { "apple", 1 },
            { "banana", 2 },
            { "cherry", 3 } };

        OrderedHashMap<std::string, int> map( data.begin(), data.end() );

        EXPECT_EQ( map.size(), 3 );
        EXPECT_EQ( *map.find( "apple" ), 1 );
        EXPECT_EQ( *map.find( "banana" ), 2 );
        EXPECT_EQ( *map.find( "cherry" ), 3 );
    }

    //=====================================================================
    // String keys - Heterogeneous lookup tests
    //=====================================================================

    TEST( OrderedHashMapTests, StringKeys_BasicLookup )
    {
        OrderedHashMap<std::string, int> map;

        map.insertOrAssign( "key1", 100 );
        map.insertOrAssign( "key2", 200 );
        map.insertOrAssign( "key3", 300 );

        EXPECT_EQ( map.size(), 3 );

        auto* value1 = map.find( "key1" );
        auto* value2 = map.find( "key2" );
        auto* value3 = map.find( "key3" );
        auto* valueNotFound = map.find( "nonexistent" );

        ASSERT_NE( value1, nullptr );
        ASSERT_NE( value2, nullptr );
        ASSERT_NE( value3, nullptr );
        EXPECT_EQ( valueNotFound, nullptr );

        EXPECT_EQ( *value1, 100 );
        EXPECT_EQ( *value2, 200 );
        EXPECT_EQ( *value3, 300 );

        EXPECT_TRUE( map.contains( "key1" ) );
        EXPECT_TRUE( map.contains( "key2" ) );
        EXPECT_FALSE( map.contains( "missing" ) );
    }

    TEST( OrderedHashMapTests, StringKeys_HeterogeneousLookup_StringView )
    {
        OrderedHashMap<std::string, int> map;

        map.insertOrAssign( "test_key", 42 );

        // Zero-copy lookup with string_view
        std::string_view sv = "test_key";
        auto* value = map.find( sv );
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, 42 );

        EXPECT_TRUE( map.contains( sv ) );
        EXPECT_EQ( map.at( sv ), 42 );

        std::string_view svMissing = "missing_key";
        EXPECT_EQ( map.find( svMissing ), nullptr );
        EXPECT_FALSE( map.contains( svMissing ) );
    }

    TEST( OrderedHashMapTests, StringKeys_HeterogeneousLookup_CString )
    {
        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "cstring_key", 999 );

        // Zero-copy lookup with const char*
        auto* value = map.find( "cstring_key" );
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, 999 );
    }

    //=====================================================================
    // operator[] - Subscript access tests
    //=====================================================================

    TEST( OrderedHashMapTests, OperatorSubscript_InsertIfMissing )
    {
        OrderedHashMap<std::string, int> map;

        map["key1"] = 100;
        EXPECT_EQ( map.size(), 1 );
        EXPECT_EQ( map["key1"], 100 );

        map["key2"] = 200;
        EXPECT_EQ( map.size(), 2 );
        EXPECT_EQ( map["key2"], 200 );
    }

    TEST( OrderedHashMapTests, OperatorSubscript_UpdateExisting )
    {
        OrderedHashMap<std::string, int> map;

        map["key"] = 1;
        EXPECT_EQ( map["key"], 1 );

        map["key"] = 2;
        EXPECT_EQ( map.size(), 1 );
        EXPECT_EQ( map["key"], 2 );
    }

    //=====================================================================
    // at() - Checked element access tests
    //=====================================================================

    TEST( OrderedHashMapTests, At_BasicAccess )
    {
        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "key", 42 );

        EXPECT_EQ( map.at( "key" ), 42 );
    }

    TEST( OrderedHashMapTests, At_ThrowsOutOfRange )
    {
        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "exists", 1 );

        EXPECT_THROW( map.at( "missing" ), std::out_of_range );
    }

    TEST( OrderedHashMapTests, At_HeterogeneousLookup )
    {
        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "key", 100 );

        std::string_view sv = "key";
        EXPECT_EQ( map.at( sv ), 100 );

        const char* cstr = "key";
        EXPECT_EQ( map.at( cstr ), 100 );
    }

    //=====================================================================
    // Erase operations with order preservation
    //=====================================================================

    TEST( OrderedHashMapTests, Erase_BasicOperation )
    {
        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "a", 1 );
        map.insertOrAssign( "b", 2 );
        map.insertOrAssign( "c", 3 );

        EXPECT_EQ( map.size(), 3 );

        bool erased = map.erase( "b" );
        EXPECT_TRUE( erased );
        EXPECT_EQ( map.size(), 2 );

        EXPECT_TRUE( map.contains( "a" ) );
        EXPECT_FALSE( map.contains( "b" ) );
        EXPECT_TRUE( map.contains( "c" ) );

        // Verify order preserved
        std::vector<std::string> keys;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
        }
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "c" );
    }

    TEST( OrderedHashMapTests, Erase_NonExistent )
    {
        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "exists", 1 );

        bool erased = map.erase( "missing" );
        EXPECT_FALSE( erased );
        EXPECT_EQ( map.size(), 1 );
    }

    TEST( OrderedHashMapTests, Erase_ByIterator )
    {
        OrderedHashMap<std::string, int> map = { { "a", 1 }, { "b", 2 }, { "c", 3 } };

        auto it = map.begin();
        ++it; // Point to "b"

        auto next = map.erase( static_cast<OrderedHashMap<std::string, int>::ConstIterator>( it ) );

        EXPECT_EQ( map.size(), 2 );
        EXPECT_NE( next, map.end() );
        EXPECT_EQ( next->first, "c" );

        // Verify order
        std::vector<std::string> keys;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
        }
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "c" );
    }

    //=====================================================================
    // Capacity and memory management
    //=====================================================================

    TEST( OrderedHashMapTests, Capacity_DefaultConstruction )
    {
        OrderedHashMap<std::string, int> map;

        EXPECT_EQ( map.size(), 0 );
        EXPECT_TRUE( map.isEmpty() );
        EXPECT_GE( map.capacity(), 0 );
    }

    TEST( OrderedHashMapTests, Capacity_Reserve )
    {
        OrderedHashMap<std::string, int> map;

        map.reserve( 100 );
        size_t capacityAfterReserve = map.capacity();
        EXPECT_GE( capacityAfterReserve, 100 );

        // Add elements
        for ( int i = 0; i < 50; ++i )
        {
            map.insertOrAssign( std::to_string( i ), i );
        }

        // Capacity should not change
        EXPECT_EQ( map.capacity(), capacityAfterReserve );
    }

    TEST( OrderedHashMapTests, Clear_RemovesAllElements )
    {
        OrderedHashMap<std::string, int> map = { { "a", 1 }, { "b", 2 }, { "c", 3 } };

        EXPECT_EQ( map.size(), 3 );

        map.clear();

        EXPECT_EQ( map.size(), 0 );
        EXPECT_TRUE( map.isEmpty() );
        EXPECT_EQ( map.begin(), map.end() );
    }

    //=====================================================================
    // Copy and move semantics with order preservation
    //=====================================================================

    TEST( OrderedHashMapTests, CopyConstructor_PreservesOrder )
    {
        OrderedHashMap<std::string, int> original;
        original.insertOrAssign( "third", 3 );
        original.insertOrAssign( "first", 1 );
        original.insertOrAssign( "second", 2 );

        OrderedHashMap<std::string, int> copy( original );

        EXPECT_EQ( copy.size(), 3 );

        std::vector<std::string> keys;
        for ( const auto& [key, value] : copy )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "third" );
        EXPECT_EQ( keys[1], "first" );
        EXPECT_EQ( keys[2], "second" );
    }

    TEST( OrderedHashMapTests, CopyAssignment_PreservesOrder )
    {
        OrderedHashMap<std::string, int> original;
        original.insertOrAssign( "x", 24 );
        original.insertOrAssign( "y", 25 );
        original.insertOrAssign( "z", 26 );

        OrderedHashMap<std::string, int> copy;
        copy.insertOrAssign( "old", 999 ); // Pre-existing data

        copy = original;

        EXPECT_EQ( copy.size(), 3 );
        EXPECT_FALSE( copy.contains( "old" ) ); // Old data replaced

        std::vector<std::string> keys;
        for ( const auto& [key, value] : copy )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "x" );
        EXPECT_EQ( keys[1], "y" );
        EXPECT_EQ( keys[2], "z" );

        // Original unchanged
        EXPECT_EQ( original.size(), 3 );
    }

    TEST( OrderedHashMapTests, MoveConstructor_PreservesOrder )
    {
        OrderedHashMap<std::string, int> original;
        original.insertOrAssign( "z", 26 );
        original.insertOrAssign( "a", 1 );
        original.insertOrAssign( "m", 13 );

        OrderedHashMap<std::string, int> moved( std::move( original ) );

        EXPECT_EQ( moved.size(), 3 );

        std::vector<std::string> keys;
        for ( const auto& [key, value] : moved )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "z" );
        EXPECT_EQ( keys[1], "a" );
        EXPECT_EQ( keys[2], "m" );

        // Original moved-from (should be empty)
        EXPECT_EQ( original.size(), 0 );
        EXPECT_TRUE( original.isEmpty() );
    }

    TEST( OrderedHashMapTests, MoveAssignment_PreservesOrder )
    {
        OrderedHashMap<std::string, int> original;
        original.insertOrAssign( "first", 1 );
        original.insertOrAssign( "second", 2 );
        original.insertOrAssign( "third", 3 );

        OrderedHashMap<std::string, int> moved;
        moved.insertOrAssign( "old", 999 ); // Pre-existing data

        moved = std::move( original );

        EXPECT_EQ( moved.size(), 3 );
        EXPECT_FALSE( moved.contains( "old" ) ); // Old data replaced

        std::vector<std::string> keys;
        for ( const auto& [key, value] : moved )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "first" );
        EXPECT_EQ( keys[1], "second" );
        EXPECT_EQ( keys[2], "third" );

        // Original moved-from (should be empty)
        EXPECT_EQ( original.size(), 0 );
        EXPECT_TRUE( original.isEmpty() );
    }

    //=====================================================================
    // Equality comparison (order-independent)
    //=====================================================================

    TEST( OrderedHashMapTests, Equality_SameContent )
    {
        OrderedHashMap<std::string, int> map1 = { { "a", 1 }, { "b", 2 } };
        OrderedHashMap<std::string, int> map2 = { { "b", 2 }, { "a", 1 } };

        // Equal despite different insertion order
        EXPECT_EQ( map1, map2 );
    }

    TEST( OrderedHashMapTests, Equality_DifferentContent )
    {
        OrderedHashMap<std::string, int> map1 = { { "a", 1 }, { "b", 2 } };
        OrderedHashMap<std::string, int> map2 = { { "a", 1 }, { "b", 3 } };

        EXPECT_NE( map1, map2 );
    }

    //=====================================================================
    // Large dataset with order preservation
    //=====================================================================

    TEST( OrderedHashMapTests, LargeDataset_OrderPreserved )
    {
        OrderedHashMap<int, int> map;

        // Insert in specific order
        for ( int i = 1000; i > 0; --i )
        {
            map.insertOrAssign( i, i * 2 );
        }

        EXPECT_EQ( map.size(), 1000 );

        // Verify order matches insertion (descending)
        int expected = 1000;
        for ( const auto& [key, value] : map )
        {
            EXPECT_EQ( key, expected );
            EXPECT_EQ( value, expected * 2 );
            --expected;
        }
    }

    //=====================================================================
    // Resize preserves insertion order
    //=====================================================================

    TEST( OrderedHashMapTests, Resize_PreservesOrder )
    {
        OrderedHashMap<std::string, int> map;

        // Insert enough to trigger resize
        map.insertOrAssign( "first", 1 );
        map.insertOrAssign( "second", 2 );
        map.insertOrAssign( "third", 3 );

        for ( int i = 0; i < 100; ++i )
        {
            map.insertOrAssign( "key_" + std::to_string( i ), i );
        }

        // Verify first three maintain order
        auto it = map.begin();
        EXPECT_EQ( it->first, "first" );
        ++it;
        EXPECT_EQ( it->first, "second" );
        ++it;
        EXPECT_EQ( it->first, "third" );
    }

    //=====================================================================
    // Emplace operations with order
    //=====================================================================

    TEST( OrderedHashMapTests, Emplace_AppendsToEnd )
    {
        OrderedHashMap<std::string, std::vector<int>> map;

        map.emplace( "first", std::vector<int>{ 1, 2, 3 } );
        map.emplace( "second", std::vector<int>{ 4, 5, 6 } );

        std::vector<std::string> keys;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "first" );
        EXPECT_EQ( keys[1], "second" );
    }

    TEST( OrderedHashMapTests, TryEmplace_OnlyInsertsIfNew )
    {
        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "a", 1 );
        map.insertOrAssign( "b", 2 );

        auto [it1, inserted1] = map.tryEmplace( "c", 3 );
        EXPECT_TRUE( inserted1 );
        EXPECT_EQ( it1->second, 3 );

        auto [it2, inserted2] = map.tryEmplace( "b", 999 );
        EXPECT_FALSE( inserted2 );
        EXPECT_EQ( it2->second, 2 ); // Not changed

        // Verify order
        std::vector<std::string> keys;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
        }
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "b" );
        EXPECT_EQ( keys[2], "c" );
    }

    //=====================================================================
    // Edge cases
    //=====================================================================

    TEST( OrderedHashMapTests, EdgeCase_SingleElement )
    {
        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "only", 42 );

        EXPECT_EQ( map.size(), 1 );
        EXPECT_EQ( map.begin()->first, "only" );
        EXPECT_EQ( map.begin()->second, 42 );
    }

    TEST( OrderedHashMapTests, EdgeCase_EraseAll )
    {
        OrderedHashMap<std::string, int> map = { { "a", 1 }, { "b", 2 }, { "c", 3 } };

        map.erase( "a" );
        map.erase( "b" );
        map.erase( "c" );

        EXPECT_EQ( map.size(), 0 );
        EXPECT_TRUE( map.isEmpty() );
        EXPECT_EQ( map.begin(), map.end() );
    }

    TEST( OrderedHashMapTests, EdgeCase_InsertEraseInsert )
    {
        OrderedHashMap<std::string, int> map;

        map.insertOrAssign( "a", 1 );
        map.insertOrAssign( "b", 2 );
        map.erase( "a" );
        map.insertOrAssign( "c", 3 );

        std::vector<std::string> keys;
        for ( const auto& [key, value] : map )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "b" );
        EXPECT_EQ( keys[1], "c" );
    }

    //=====================================================================
    // Swap operation preserves order
    //=====================================================================

    TEST( OrderedHashMapTests, Swap_PreservesOrder )
    {
        OrderedHashMap<std::string, int> map1;
        map1.insertOrAssign( "a", 1 );
        map1.insertOrAssign( "b", 2 );

        OrderedHashMap<std::string, int> map2;
        map2.insertOrAssign( "x", 24 );
        map2.insertOrAssign( "y", 25 );

        map1.swap( map2 );

        std::vector<std::string> keys1;
        for ( const auto& [key, value] : map1 )
        {
            keys1.push_back( key );
        }

        std::vector<std::string> keys2;
        for ( const auto& [key, value] : map2 )
        {
            keys2.push_back( key );
        }

        EXPECT_EQ( keys1[0], "x" );
        EXPECT_EQ( keys1[1], "y" );
        EXPECT_EQ( keys2[0], "a" );
        EXPECT_EQ( keys2[1], "b" );
    }
} // namespace nfx::containers::test
