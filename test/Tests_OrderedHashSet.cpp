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
 * @file Tests_OrderedHashSet.cpp
 * @brief Tests for OrderedHashSet (Robin Hood hashing with insertion-order preservation)
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

    TEST( OrderedHashSetTests, InsertionOrder_BasicPreservation )
    {
        OrderedHashSet<std::string> set;

        set.insert( "third" );
        set.insert( "first" );
        set.insert( "second" );

        // Verify iteration follows insertion order, not key order
        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }

        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "third" );  // First inserted
        EXPECT_EQ( keys[1], "first" );  // Second inserted
        EXPECT_EQ( keys[2], "second" ); // Third inserted
    }

    TEST( OrderedHashSetTests, InsertionOrder_InitializerList )
    {
        OrderedHashSet<std::string> set = { "zebra", "alpha", "beta" };

        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }

        // Order matches initializer list, not alphabetical
        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "zebra" );
        EXPECT_EQ( keys[1], "alpha" );
        EXPECT_EQ( keys[2], "beta" );
    }

    TEST( OrderedHashSetTests, InsertionOrder_DuplicateDoesNotReorder )
    {
        OrderedHashSet<std::string> set;

        set.insert( "a" );
        set.insert( "b" );
        set.insert( "c" );
        bool inserted = set.insert( "b" ); // Try to insert duplicate

        EXPECT_FALSE( inserted ); // Should not insert duplicate

        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }

        // "b" stays in original position
        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "b" );
        EXPECT_EQ( keys[2], "c" );
    }

    TEST( OrderedHashSetTests, InsertionOrder_AfterErase )
    {
        OrderedHashSet<std::string> set;

        set.insert( "one" );
        set.insert( "two" );
        set.insert( "three" );
        set.insert( "four" );

        set.erase( "two" );

        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }

        // Order preserved after erase
        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "one" );
        EXPECT_EQ( keys[1], "three" );
        EXPECT_EQ( keys[2], "four" );
    }

    TEST( OrderedHashSetTests, InsertionOrder_IntegerKeys )
    {
        OrderedHashSet<int> set;

        set.insert( 100 );
        set.insert( 1 );
        set.insert( 50 );

        std::vector<int> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }

        // Insertion order, not sorted by value
        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], 100 );
        EXPECT_EQ( keys[1], 1 );
        EXPECT_EQ( keys[2], 50 );
    }

    //=====================================================================
    // Bidirectional iterator tests
    //=====================================================================

    TEST( OrderedHashSetTests, BidirectionalIterator_ForwardIteration )
    {
        OrderedHashSet<std::string> set = { "a", "b", "c" };

        std::vector<std::string> keys;
        for ( auto it = set.begin(); it != set.end(); ++it )
        {
            keys.push_back( *it );
        }

        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "b" );
        EXPECT_EQ( keys[2], "c" );
    }

    TEST( OrderedHashSetTests, BidirectionalIterator_BackwardIteration )
    {
        OrderedHashSet<std::string> set = { "a", "b", "c" };

        std::vector<std::string> keys;
        for ( auto it = --set.end();; --it )
        {
            keys.push_back( *it );
            if ( it == set.begin() )
                break;
        }

        ASSERT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "c" );
        EXPECT_EQ( keys[1], "b" );
        EXPECT_EQ( keys[2], "a" );
    }

    TEST( OrderedHashSetTests, BidirectionalIterator_PostIncrement )
    {
        OrderedHashSet<std::string> set = { "x", "y" };

        auto it = set.begin();
        auto prev = it++;

        EXPECT_EQ( *prev, "x" );
        EXPECT_EQ( *it, "y" );
    }

    TEST( OrderedHashSetTests, BidirectionalIterator_PostDecrement )
    {
        OrderedHashSet<std::string> set = { "x", "y" };

        auto it = --set.end();
        auto prev = it--;

        EXPECT_EQ( *prev, "y" );
        EXPECT_EQ( *it, "x" );
    }

    //=====================================================================
    // Constructor tests - initializer_list
    //=====================================================================

    TEST( OrderedHashSetTests, InitializerListConstructor_Basic )
    {
        OrderedHashSet<std::string> set = { "apple", "banana", "cherry" };

        EXPECT_EQ( set.size(), 3 );
        EXPECT_FALSE( set.isEmpty() );

        EXPECT_TRUE( set.contains( "apple" ) );
        EXPECT_TRUE( set.contains( "banana" ) );
        EXPECT_TRUE( set.contains( "cherry" ) );
    }

    TEST( OrderedHashSetTests, InitializerListConstructor_Empty )
    {
        OrderedHashSet<std::string> set = {};

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
    }

    TEST( OrderedHashSetTests, InitializerListConstructor_SingleElement )
    {
        OrderedHashSet<std::string> set = { "single" };

        EXPECT_EQ( set.size(), 1 );
        EXPECT_TRUE( set.contains( "single" ) );
    }

    TEST( OrderedHashSetTests, InitializerListConstructor_DuplicateKeys )
    {
        // Duplicates ignored
        OrderedHashSet<std::string> set = { "key", "key", "key" };

        EXPECT_EQ( set.size(), 1 );
        EXPECT_TRUE( set.contains( "key" ) );
    }

    //=====================================================================
    // Range constructor tests
    //=====================================================================

    TEST( OrderedHashSetTests, RangeConstructor_FromVector )
    {
        std::vector<std::string> data = { "apple", "banana", "cherry" };

        OrderedHashSet<std::string> set( data.begin(), data.end() );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( "apple" ) );
        EXPECT_TRUE( set.contains( "banana" ) );
        EXPECT_TRUE( set.contains( "cherry" ) );
    }

    //=====================================================================
    // String keys - Heterogeneous lookup tests
    //=====================================================================

    TEST( OrderedHashSetTests, StringKeys_BasicLookup )
    {
        OrderedHashSet<std::string> set;

        set.insert( "key1" );
        set.insert( "key2" );
        set.insert( "key3" );

        EXPECT_EQ( set.size(), 3 );

        auto* value1 = set.find( "key1" );
        auto* value2 = set.find( "key2" );
        auto* value3 = set.find( "key3" );
        auto* valueNotFound = set.find( "nonexistent" );

        ASSERT_NE( value1, nullptr );
        ASSERT_NE( value2, nullptr );
        ASSERT_NE( value3, nullptr );
        EXPECT_EQ( valueNotFound, nullptr );

        EXPECT_EQ( *value1, "key1" );
        EXPECT_EQ( *value2, "key2" );
        EXPECT_EQ( *value3, "key3" );

        EXPECT_TRUE( set.contains( "key1" ) );
        EXPECT_TRUE( set.contains( "key2" ) );
        EXPECT_FALSE( set.contains( "missing" ) );
    }

    TEST( OrderedHashSetTests, StringKeys_HeterogeneousLookup_StringView )
    {
        OrderedHashSet<std::string> set;

        set.insert( "test_key" );

        // Zero-copy lookup with string_view
        std::string_view sv = "test_key";
        auto* value = set.find( sv );
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, "test_key" );

        EXPECT_TRUE( set.contains( sv ) );
        EXPECT_EQ( set.at( sv ), "test_key" );

        std::string_view svMissing = "missing_key";
        EXPECT_EQ( set.find( svMissing ), nullptr );
        EXPECT_FALSE( set.contains( svMissing ) );
    }

    TEST( OrderedHashSetTests, StringKeys_HeterogeneousLookup_CString )
    {
        OrderedHashSet<std::string> set;
        set.insert( "cstring_key" );

        // Zero-copy lookup with const char*
        auto* value = set.find( "cstring_key" );
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, "cstring_key" );
    }

    //=====================================================================
    // at() - Checked element access tests
    //=====================================================================

    TEST( OrderedHashSetTests, At_BasicAccess )
    {
        OrderedHashSet<std::string> set;
        set.insert( "key" );

        EXPECT_EQ( set.at( "key" ), "key" );
    }

    TEST( OrderedHashSetTests, At_ThrowsOutOfRange )
    {
        OrderedHashSet<std::string> set;
        set.insert( "exists" );

        EXPECT_THROW( set.at( "missing" ), std::out_of_range );
    }

    TEST( OrderedHashSetTests, At_HeterogeneousLookup )
    {
        OrderedHashSet<std::string> set;
        set.insert( "key" );

        std::string_view sv = "key";
        EXPECT_EQ( set.at( sv ), "key" );

        const char* cstr = "key";
        EXPECT_EQ( set.at( cstr ), "key" );
    }

    //=====================================================================
    // Erase operations with order preservation
    //=====================================================================

    TEST( OrderedHashSetTests, Erase_BasicOperation )
    {
        OrderedHashSet<std::string> set;
        set.insert( "a" );
        set.insert( "b" );
        set.insert( "c" );

        EXPECT_EQ( set.size(), 3 );

        bool erased = set.erase( "b" );
        EXPECT_TRUE( erased );
        EXPECT_EQ( set.size(), 2 );

        EXPECT_TRUE( set.contains( "a" ) );
        EXPECT_FALSE( set.contains( "b" ) );
        EXPECT_TRUE( set.contains( "c" ) );

        // Verify order preserved
        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "c" );
    }

    TEST( OrderedHashSetTests, Erase_NonExistent )
    {
        OrderedHashSet<std::string> set;
        set.insert( "exists" );

        bool erased = set.erase( "missing" );
        EXPECT_FALSE( erased );
        EXPECT_EQ( set.size(), 1 );
    }

    TEST( OrderedHashSetTests, Erase_ByIterator )
    {
        OrderedHashSet<std::string> set = { "a", "b", "c" };

        auto it = set.begin();
        ++it; // Point to "b"

        auto next = set.erase( static_cast<OrderedHashSet<std::string>::ConstIterator>( it ) );

        EXPECT_EQ( set.size(), 2 );
        EXPECT_NE( next, set.end() );
        EXPECT_EQ( *next, "c" );

        // Verify order
        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "c" );
    }

    //=====================================================================
    // Capacity and memory management
    //=====================================================================

    TEST( OrderedHashSetTests, Capacity_DefaultConstruction )
    {
        OrderedHashSet<std::string> set;

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
        EXPECT_GE( set.capacity(), 0 );
    }

    TEST( OrderedHashSetTests, Capacity_Reserve )
    {
        OrderedHashSet<std::string> set;

        set.reserve( 100 );
        size_t capacityAfterReserve = set.capacity();
        EXPECT_GE( capacityAfterReserve, 100 );

        // Add elements
        for ( int i = 0; i < 50; ++i )
        {
            set.insert( std::to_string( i ) );
        }

        // Capacity should not change
        EXPECT_EQ( set.capacity(), capacityAfterReserve );
    }

    TEST( OrderedHashSetTests, Clear_RemovesAllElements )
    {
        OrderedHashSet<std::string> set = { "a", "b", "c" };

        EXPECT_EQ( set.size(), 3 );

        set.clear();

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
        EXPECT_EQ( set.begin(), set.end() );
    }

    //=====================================================================
    // Copy and move semantics with order preservation
    //=====================================================================

    TEST( OrderedHashSetTests, CopyConstructor_PreservesOrder )
    {
        OrderedHashSet<std::string> original;
        original.insert( "third" );
        original.insert( "first" );
        original.insert( "second" );

        OrderedHashSet<std::string> copy( original );

        EXPECT_EQ( copy.size(), 3 );

        std::vector<std::string> keys;
        for ( const auto& key : copy )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "third" );
        EXPECT_EQ( keys[1], "first" );
        EXPECT_EQ( keys[2], "second" );
    }

    TEST( OrderedHashSetTests, CopyAssignment_PreservesOrder )
    {
        OrderedHashSet<std::string> original;
        original.insert( "x" );
        original.insert( "y" );
        original.insert( "z" );

        OrderedHashSet<std::string> copy;
        copy.insert( "old" ); // Pre-existing data

        copy = original;

        EXPECT_EQ( copy.size(), 3 );
        EXPECT_FALSE( copy.contains( "old" ) ); // Old data replaced

        std::vector<std::string> keys;
        for ( const auto& key : copy )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "x" );
        EXPECT_EQ( keys[1], "y" );
        EXPECT_EQ( keys[2], "z" );

        // Original unchanged
        EXPECT_EQ( original.size(), 3 );
    }

    TEST( OrderedHashSetTests, MoveConstructor_PreservesOrder )
    {
        OrderedHashSet<std::string> original;
        original.insert( "z" );
        original.insert( "a" );
        original.insert( "m" );

        OrderedHashSet<std::string> moved( std::move( original ) );

        EXPECT_EQ( moved.size(), 3 );

        std::vector<std::string> keys;
        for ( const auto& key : moved )
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

    TEST( OrderedHashSetTests, MoveAssignment_PreservesOrder )
    {
        OrderedHashSet<std::string> original;
        original.insert( "first" );
        original.insert( "second" );
        original.insert( "third" );

        OrderedHashSet<std::string> moved;
        moved.insert( "old" ); // Pre-existing data

        moved = std::move( original );

        EXPECT_EQ( moved.size(), 3 );
        EXPECT_FALSE( moved.contains( "old" ) ); // Old data replaced

        std::vector<std::string> keys;
        for ( const auto& key : moved )
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

    TEST( OrderedHashSetTests, Equality_SameContent )
    {
        OrderedHashSet<std::string> set1 = { "a", "b" };
        OrderedHashSet<std::string> set2 = { "b", "a" };

        // Equal despite different insertion order
        EXPECT_EQ( set1, set2 );
    }

    TEST( OrderedHashSetTests, Equality_DifferentContent )
    {
        OrderedHashSet<std::string> set1 = { "a", "b" };
        OrderedHashSet<std::string> set2 = { "a", "c" };

        EXPECT_NE( set1, set2 );
    }

    //=====================================================================
    // Large dataset with order preservation
    //=====================================================================

    TEST( OrderedHashSetTests, LargeDataset_OrderPreserved )
    {
        OrderedHashSet<int> set;

        // Insert in specific order
        for ( int i = 1000; i > 0; --i )
        {
            set.insert( i );
        }

        EXPECT_EQ( set.size(), 1000 );

        // Verify order matches insertion (descending)
        int expected = 1000;
        for ( const auto& key : set )
        {
            EXPECT_EQ( key, expected );
            --expected;
        }
    }

    //=====================================================================
    // Resize preserves insertion order
    //=====================================================================

    TEST( OrderedHashSetTests, Resize_PreservesOrder )
    {
        OrderedHashSet<std::string> set;

        // Insert enough to trigger resize
        set.insert( "first" );
        set.insert( "second" );
        set.insert( "third" );

        for ( int i = 0; i < 100; ++i )
        {
            set.insert( "key_" + std::to_string( i ) );
        }

        // Verify first three maintain order
        auto it = set.begin();
        EXPECT_EQ( *it, "first" );
        ++it;
        EXPECT_EQ( *it, "second" );
        ++it;
        EXPECT_EQ( *it, "third" );
    }

    //=====================================================================
    // Emplace operations with order
    //=====================================================================

    TEST( OrderedHashSetTests, Emplace_AppendsToEnd )
    {
        OrderedHashSet<std::string> set;

        set.emplace( "first" );
        set.emplace( 5, 'x' ); // Constructs std::string(5, 'x')

        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "first" );
        EXPECT_EQ( keys[1], "xxxxx" );
    }

    TEST( OrderedHashSetTests, TryEmplace_OnlyInsertsIfNew )
    {
        OrderedHashSet<std::string> set;
        set.insert( "a" );
        set.insert( "b" );

        auto [it1, inserted1] = set.tryEmplace( "c" );
        EXPECT_TRUE( inserted1 );
        EXPECT_EQ( *it1, "c" );

        auto [it2, inserted2] = set.tryEmplace( "b" );
        EXPECT_FALSE( inserted2 );
        EXPECT_EQ( *it2, "b" );

        // Verify order
        std::vector<std::string> keys;
        for ( const auto& key : set )
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

    TEST( OrderedHashSetTests, EdgeCase_SingleElement )
    {
        OrderedHashSet<std::string> set;
        set.insert( "only" );

        EXPECT_EQ( set.size(), 1 );
        EXPECT_EQ( *set.begin(), "only" );
    }

    TEST( OrderedHashSetTests, EdgeCase_EraseAll )
    {
        OrderedHashSet<std::string> set = { "a", "b", "c" };

        set.erase( "a" );
        set.erase( "b" );
        set.erase( "c" );

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
        EXPECT_EQ( set.begin(), set.end() );
    }

    TEST( OrderedHashSetTests, EdgeCase_InsertEraseInsert )
    {
        OrderedHashSet<std::string> set;

        set.insert( "a" );
        set.insert( "b" );
        set.erase( "a" );
        set.insert( "c" );

        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys[0], "b" );
        EXPECT_EQ( keys[1], "c" );
    }

    //=====================================================================
    // Swap operation preserves order
    //=====================================================================

    TEST( OrderedHashSetTests, Swap_PreservesOrder )
    {
        OrderedHashSet<std::string> set1;
        set1.insert( "a" );
        set1.insert( "b" );

        OrderedHashSet<std::string> set2;
        set2.insert( "x" );
        set2.insert( "y" );

        set1.swap( set2 );

        std::vector<std::string> keys1;
        for ( const auto& key : set1 )
        {
            keys1.push_back( key );
        }

        std::vector<std::string> keys2;
        for ( const auto& key : set2 )
        {
            keys2.push_back( key );
        }

        EXPECT_EQ( keys1[0], "x" );
        EXPECT_EQ( keys1[1], "y" );
        EXPECT_EQ( keys2[0], "a" );
        EXPECT_EQ( keys2[1], "b" );
    }

    //=====================================================================
    // Insert return value tests
    //=====================================================================

    TEST( OrderedHashSetTests, Insert_ReturnValue )
    {
        OrderedHashSet<std::string> set;

        bool inserted1 = set.insert( "new" );
        EXPECT_TRUE( inserted1 );

        bool inserted2 = set.insert( "new" );
        EXPECT_FALSE( inserted2 ); // Duplicate

        EXPECT_EQ( set.size(), 1 );
    }

    //=====================================================================
    // Extract operation tests
    //=====================================================================

    TEST( OrderedHashSetTests, Extract_BasicOperation )
    {
        OrderedHashSet<std::string> set = { "apple", "banana", "cherry" };

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( "banana" ) );

        auto extracted = set.extract( "banana" );

        ASSERT_TRUE( extracted.has_value() );
        EXPECT_EQ( *extracted, "banana" );
        EXPECT_EQ( set.size(), 2 );
        EXPECT_FALSE( set.contains( "banana" ) );
    }

    TEST( OrderedHashSetTests, Extract_PreservesOrder )
    {
        OrderedHashSet<std::string> set = { "first", "second", "third", "fourth" };

        (void)set.extract( "second" );

        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys.size(), 3 );
        EXPECT_EQ( keys[0], "first" );
        EXPECT_EQ( keys[1], "third" );
        EXPECT_EQ( keys[2], "fourth" );
    }

    TEST( OrderedHashSetTests, Extract_NonExistent )
    {
        OrderedHashSet<std::string> set = { "apple", "banana" };

        auto extracted = set.extract( "cherry" );

        EXPECT_FALSE( extracted.has_value() );
        EXPECT_EQ( set.size(), 2 );
    }

    TEST( OrderedHashSetTests, Extract_HeterogeneousLookup )
    {
        OrderedHashSet<std::string> set = { "apple", "banana" };

        auto extracted = set.extract( std::string_view( "banana" ) );

        ASSERT_TRUE( extracted.has_value() );
        EXPECT_EQ( *extracted, "banana" );
    }

    TEST( OrderedHashSetTests, Extract_MoveSemantics )
    {
        OrderedHashSet<std::string> set = { "apple", "banana" };

        auto extracted = set.extract( "banana" );

        ASSERT_TRUE( extracted.has_value() );
        std::string moved = std::move( *extracted );
        EXPECT_EQ( moved, "banana" );
    }

    TEST( OrderedHashSetTests, Extract_FromEmpty )
    {
        OrderedHashSet<std::string> set;

        auto extracted = set.extract( "anything" );

        EXPECT_FALSE( extracted.has_value() );
        EXPECT_EQ( set.size(), 0 );
    }

    TEST( OrderedHashSetTests, Extract_AllElements )
    {
        OrderedHashSet<std::string> set = { "a", "b", "c" };

        auto e1 = set.extract( "a" );
        auto e2 = set.extract( "b" );
        auto e3 = set.extract( "c" );

        EXPECT_TRUE( e1.has_value() );
        EXPECT_TRUE( e2.has_value() );
        EXPECT_TRUE( e3.has_value() );
        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
    }

    //=====================================================================
    // Merge operation tests
    //=====================================================================

    TEST( OrderedHashSetTests, Merge_BasicOperation )
    {
        OrderedHashSet<std::string> set1 = { "apple", "banana" };
        OrderedHashSet<std::string> set2 = { "cherry", "date" };

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 4 );
        EXPECT_EQ( set2.size(), 0 );
        EXPECT_TRUE( set1.contains( "apple" ) );
        EXPECT_TRUE( set1.contains( "banana" ) );
        EXPECT_TRUE( set1.contains( "cherry" ) );
        EXPECT_TRUE( set1.contains( "date" ) );
    }

    TEST( OrderedHashSetTests, Merge_PreservesOrder )
    {
        OrderedHashSet<std::string> set1 = { "a", "b" };
        OrderedHashSet<std::string> set2 = { "c", "d" };

        set1.merge( set2 );

        std::vector<std::string> keys;
        for ( const auto& key : set1 )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys.size(), 4 );
        EXPECT_EQ( keys[0], "a" );
        EXPECT_EQ( keys[1], "b" );
        EXPECT_EQ( keys[2], "c" );
        EXPECT_EQ( keys[3], "d" );
    }

    TEST( OrderedHashSetTests, Merge_WithDuplicates )
    {
        OrderedHashSet<std::string> set1 = { "apple", "banana" };
        OrderedHashSet<std::string> set2 = { "cherry", "apple", "date" };

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 4 );
        EXPECT_EQ( set2.size(), 1 );
        EXPECT_TRUE( set2.contains( "apple" ) );
        EXPECT_FALSE( set2.contains( "cherry" ) );
        EXPECT_FALSE( set2.contains( "date" ) );
    }

    TEST( OrderedHashSetTests, Merge_EmptySource )
    {
        OrderedHashSet<std::string> set1 = { "apple", "banana" };
        OrderedHashSet<std::string> set2;

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 2 );
        EXPECT_EQ( set2.size(), 0 );
    }

    TEST( OrderedHashSetTests, Merge_EmptyDestination )
    {
        OrderedHashSet<std::string> set1;
        OrderedHashSet<std::string> set2 = { "apple", "banana" };

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 2 );
        EXPECT_EQ( set2.size(), 0 );
        EXPECT_TRUE( set1.contains( "apple" ) );
        EXPECT_TRUE( set1.contains( "banana" ) );
    }

    TEST( OrderedHashSetTests, Merge_BothEmpty )
    {
        OrderedHashSet<std::string> set1;
        OrderedHashSet<std::string> set2;

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 0 );
        EXPECT_EQ( set2.size(), 0 );
    }

    TEST( OrderedHashSetTests, Merge_RvalueReference )
    {
        OrderedHashSet<std::string> set1 = { "apple" };
        OrderedHashSet<std::string> set2 = { "banana" };

        set1.merge( std::move( set2 ) );

        EXPECT_EQ( set1.size(), 2 );
        EXPECT_TRUE( set1.contains( "apple" ) );
        EXPECT_TRUE( set1.contains( "banana" ) );
    }

    TEST( OrderedHashSetTests, Merge_AllDuplicates )
    {
        OrderedHashSet<std::string> set1 = { "apple", "banana", "cherry" };
        OrderedHashSet<std::string> set2 = { "apple", "banana", "cherry" };

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 3 );
        EXPECT_EQ( set2.size(), 3 );
    }

    TEST( OrderedHashSetTests, Merge_LargeDataset )
    {
        OrderedHashSet<int> set1;
        OrderedHashSet<int> set2;

        for ( int i = 0; i < 100; ++i )
        {
            set1.insert( i );
        }

        for ( int i = 50; i < 150; ++i )
        {
            set2.insert( i );
        }

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 150 );
        EXPECT_EQ( set2.size(), 50 );

        for ( int i = 50; i < 100; ++i )
        {
            EXPECT_TRUE( set2.contains( i ) );
        }
    }

    TEST( OrderedHashSetTests, Merge_ExtractMergeWorkflow )
    {
        OrderedHashSet<std::string> set1 = { "apple", "banana" };
        OrderedHashSet<std::string> set2 = { "cherry", "date" };
        OrderedHashSet<std::string> set3;

        auto extracted = set1.extract( "banana" );
        if ( extracted )
        {
            set3.insert( std::move( *extracted ) );
        }

        set3.merge( set2 );

        EXPECT_EQ( set1.size(), 1 );
        EXPECT_EQ( set2.size(), 0 );
        EXPECT_EQ( set3.size(), 3 );
        EXPECT_TRUE( set3.contains( "banana" ) );
        EXPECT_TRUE( set3.contains( "cherry" ) );
        EXPECT_TRUE( set3.contains( "date" ) );
    }

} // namespace nfx::containers::test
