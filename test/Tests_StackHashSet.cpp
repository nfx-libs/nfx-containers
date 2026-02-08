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
 * @file Tests_StackHashSet.cpp
 * @brief Tests for StackHashSet (SBO hash set)
 */

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nfx/containers/StackHashSet.h>

namespace nfx::containers::test
{
    //=====================================================================
    // Constructor tests
    //=====================================================================

    TEST( StackHashSetTests, DefaultConstructor_Empty )
    {
        StackHashSet<std::string> set;

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
    }

    TEST( StackHashSetTests, InitializerListConstructor_Basic )
    {
        StackHashSet<std::string> set{ "apple", "banana", "cherry" };

        EXPECT_EQ( set.size(), 3 );
        EXPECT_FALSE( set.isEmpty() );
        EXPECT_TRUE( set.contains( "apple" ) );
        EXPECT_TRUE( set.contains( "banana" ) );
        EXPECT_TRUE( set.contains( "cherry" ) );
    }

    TEST( StackHashSetTests, InitializerListConstructor_Empty )
    {
        StackHashSet<std::string> set{};

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
    }

    TEST( StackHashSetTests, InitializerListConstructor_SingleElement )
    {
        StackHashSet<std::string> set{ "single" };

        EXPECT_EQ( set.size(), 1 );
        EXPECT_TRUE( set.contains( "single" ) );
    }

    TEST( StackHashSetTests, InitializerListConstructor_Duplicates )
    {
        StackHashSet<int> set{ 1, 2, 3, 2, 1 }; // Duplicates should be ignored

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( 1 ) );
        EXPECT_TRUE( set.contains( 2 ) );
        EXPECT_TRUE( set.contains( 3 ) );
    }

    TEST( StackHashSetTests, RangeConstructor_FromVector )
    {
        std::vector<std::string> data{ "one", "two", "three" };

        StackHashSet<std::string> set( data.begin(), data.end() );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( "one" ) );
        EXPECT_TRUE( set.contains( "two" ) );
        EXPECT_TRUE( set.contains( "three" ) );
    }

    TEST( StackHashSetTests, RangeConstructor_EmptyRange )
    {
        std::vector<int> data;
        StackHashSet<int> set( data.begin(), data.end() );

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
    }

    //=====================================================================
    // Capacity tests
    //=====================================================================

    TEST( StackHashSetTests, StackCapacity_ReturnsTemplateParameter )
    {
        StackHashSet<int, 8> set8;
        StackHashSet<int, 16> set16;
        StackHashSet<int, 32> set32;

        EXPECT_EQ( set8.stackCapacity(), 8 );
        EXPECT_EQ( set16.stackCapacity(), 16 );
        EXPECT_EQ( set32.stackCapacity(), 32 );
    }

    TEST( StackHashSetTests, Size_TracksCurrent )
    {
        StackHashSet<int> set;

        EXPECT_EQ( set.size(), 0 );

        set.insert( 1 );
        EXPECT_EQ( set.size(), 1 );

        set.insert( 2 );
        EXPECT_EQ( set.size(), 2 );

        set.erase( 1 );
        EXPECT_EQ( set.size(), 1 );

        set.clear();
        EXPECT_EQ( set.size(), 0 );
    }

    TEST( StackHashSetTests, Empty_ReflectsSize )
    {
        StackHashSet<int> set;

        EXPECT_TRUE( set.isEmpty() );

        set.insert( 1 );
        EXPECT_FALSE( set.isEmpty() );

        set.clear();
        EXPECT_TRUE( set.isEmpty() );
    }

    //=====================================================================
    // Modifiers - insert()
    //=====================================================================

    TEST( StackHashSetTests, Insert_NewElement )
    {
        StackHashSet<int> set;

        auto [ptr, inserted]{ set.insert( 10 ) };

        EXPECT_TRUE( inserted );
        EXPECT_NE( ptr, nullptr );
        EXPECT_EQ( *ptr, 10 );
        EXPECT_EQ( set.size(), 1 );
    }

    TEST( StackHashSetTests, Insert_DuplicateKey )
    {
        StackHashSet<int> set;

        auto [ptr1, inserted1]{ set.insert( 10 ) };
        EXPECT_TRUE( inserted1 );

        auto [ptr2, inserted2]{ set.insert( 10 ) };
        EXPECT_FALSE( inserted2 );
        EXPECT_EQ( *ptr2, 10 );
        EXPECT_EQ( set.size(), 1 );
    }

    TEST( StackHashSetTests, Insert_MoveSemantics )
    {
        StackHashSet<std::string> set;

        std::string key{ "movable" };
        auto [ptr, inserted]{ set.insert( std::move( key ) ) };

        EXPECT_TRUE( inserted );
        EXPECT_EQ( *ptr, "movable" );
    }

    TEST( StackHashSetTests, Insert_MultipleElements )
    {
        StackHashSet<int> set;

        for ( int i{ 1 }; i <= 5; ++i )
        {
            auto [ptr, inserted]{ set.insert( i ) };
            EXPECT_TRUE( inserted );
        }

        EXPECT_EQ( set.size(), 5 );
        for ( int i{ 1 }; i <= 5; ++i )
        {
            EXPECT_TRUE( set.contains( i ) );
        }
    }

    //=====================================================================
    // Modifiers - emplace()
    //=====================================================================

    TEST( StackHashSetTests, Emplace_NewElement )
    {
        StackHashSet<std::string> set;

        auto [ptr, inserted]{ set.emplace( "emplaced" ) };

        EXPECT_TRUE( inserted );
        EXPECT_NE( ptr, nullptr );
        EXPECT_EQ( *ptr, "emplaced" );
    }

    TEST( StackHashSetTests, Emplace_DuplicateKey )
    {
        StackHashSet<int> set;

        set.emplace( 5 );
        auto [ptr, inserted]{ set.emplace( 5 ) };

        EXPECT_FALSE( inserted );
        EXPECT_EQ( *ptr, 5 );
    }

    //=====================================================================
    // Modifiers - erase()
    //=====================================================================

    TEST( StackHashSetTests, Erase_ExistingKey )
    {
        StackHashSet<int> set{ 1, 2, 3 };

        size_t erased{ set.erase( 2 ) };

        EXPECT_EQ( erased, 1 );
        EXPECT_EQ( set.size(), 2 );
        EXPECT_TRUE( set.contains( 1 ) );
        EXPECT_FALSE( set.contains( 2 ) );
        EXPECT_TRUE( set.contains( 3 ) );
    }

    TEST( StackHashSetTests, Erase_MissingKey )
    {
        StackHashSet<int> set{ 1 };

        size_t erased{ set.erase( 99 ) };

        EXPECT_EQ( erased, 0 );
        EXPECT_EQ( set.size(), 1 );
    }

    TEST( StackHashSetTests, Erase_AllElements )
    {
        StackHashSet<int> set{ 1, 2, 3 };

        set.erase( 1 );
        set.erase( 2 );
        set.erase( 3 );

        EXPECT_TRUE( set.isEmpty() );
        EXPECT_EQ( set.size(), 0 );
    }

    //=====================================================================
    // Modifiers - clear()
    //=====================================================================

    TEST( StackHashSetTests, Clear_RemovesAllElements )
    {
        StackHashSet<int> set{ 1, 2, 3 };

        set.clear();

        EXPECT_TRUE( set.isEmpty() );
        EXPECT_EQ( set.size(), 0 );
        EXPECT_FALSE( set.contains( 1 ) );
        EXPECT_FALSE( set.contains( 2 ) );
        EXPECT_FALSE( set.contains( 3 ) );
    }

    TEST( StackHashSetTests, Clear_OnEmptySet )
    {
        StackHashSet<int> set;

        set.clear();

        EXPECT_TRUE( set.isEmpty() );
        EXPECT_EQ( set.size(), 0 );
    }

    //=====================================================================
    // Lookup - count()
    //=====================================================================

    TEST( StackHashSetTests, Count_ExistingKey )
    {
        StackHashSet<std::string> set{ "exists" };

        EXPECT_EQ( set.count( "exists" ), 1 );
    }

    TEST( StackHashSetTests, Count_MissingKey )
    {
        StackHashSet<std::string> set{ "exists" };

        EXPECT_EQ( set.count( "missing" ), 0 );
    }

    //=====================================================================
    // Lookup - contains()
    //=====================================================================

    TEST( StackHashSetTests, Contains_ExistingKey )
    {
        StackHashSet<int> set{ 10, 20 };

        EXPECT_TRUE( set.contains( 10 ) );
        EXPECT_TRUE( set.contains( 20 ) );
    }

    TEST( StackHashSetTests, Contains_MissingKey )
    {
        StackHashSet<int> set{ 10 };

        EXPECT_FALSE( set.contains( 99 ) );
    }

    TEST( StackHashSetTests, Contains_HeterogeneousLookup )
    {
        StackHashSet<std::string> set{ "hello" };

        std::string_view key{ "hello" };
        EXPECT_TRUE( set.contains( key ) );

        std::string_view missing{ "world" };
        EXPECT_FALSE( set.contains( missing ) );
    }

    //=====================================================================
    // Stack to heap transition tests
    //=====================================================================

    TEST( StackHashSetTests, StackToHeapTransition_ViaInsert )
    {
        StackHashSet<int, 4> set; // Small capacity: 4

        // Fill stack
        set.insert( 1 );
        set.insert( 2 );
        set.insert( 3 );
        set.insert( 4 );

        EXPECT_EQ( set.size(), 4 );

        // Trigger transition to heap
        set.insert( 5 );

        EXPECT_EQ( set.size(), 5 );
        EXPECT_TRUE( set.contains( 1 ) );
        EXPECT_TRUE( set.contains( 2 ) );
        EXPECT_TRUE( set.contains( 3 ) );
        EXPECT_TRUE( set.contains( 4 ) );
        EXPECT_TRUE( set.contains( 5 ) );
    }

    TEST( StackHashSetTests, StackToHeapTransition_PreservesData )
    {
        StackHashSet<std::string, 2> set;

        set.insert( "a" );
        set.insert( "b" );
        set.insert( "c" ); // Transition

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( "a" ) );
        EXPECT_TRUE( set.contains( "b" ) );
        EXPECT_TRUE( set.contains( "c" ) );
    }

    TEST( StackHashSetTests, StackToHeapTransition_AllOperationsWork )
    {
        StackHashSet<int, 2> set;

        set.insert( 1 );
        set.insert( 2 );
        set.insert( 3 ); // Now on heap

        // Test operations after transition
        EXPECT_TRUE( set.contains( 1 ) );
        EXPECT_TRUE( set.contains( 2 ) );
        EXPECT_TRUE( set.contains( 3 ) );

        set.erase( 2 );
        EXPECT_FALSE( set.contains( 2 ) );
        EXPECT_EQ( set.size(), 2 );

        set.insert( 4 );
        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( 4 ) );

        set.clear();
        EXPECT_TRUE( set.isEmpty() );
    }

    //=====================================================================
    // STL C++17 compatibility - extract()
    //=====================================================================

    TEST( StackHashSetTests, Extract_ExistingKey_Stack )
    {
        StackHashSet<std::string, 8> set{ "one", "two", "three" };

        auto result{ set.extract( "two" ) };

        EXPECT_TRUE( result.has_value() );
        EXPECT_EQ( result.value(), "two" );
        EXPECT_EQ( set.size(), 2 );
        EXPECT_FALSE( set.contains( "two" ) );
        EXPECT_TRUE( set.contains( "one" ) );
        EXPECT_TRUE( set.contains( "three" ) );
    }

    TEST( StackHashSetTests, Extract_MissingKey_Stack )
    {
        StackHashSet<std::string, 8> set{ "one" };

        auto result{ set.extract( "missing" ) };

        EXPECT_FALSE( result.has_value() );
        EXPECT_EQ( set.size(), 1 );
    }

    TEST( StackHashSetTests, Extract_ExistingKey_Heap )
    {
        StackHashSet<int, 2> set;

        set.insert( 1 );
        set.insert( 2 );
        set.insert( 3 ); // Transition to heap

        auto result{ set.extract( 2 ) };

        EXPECT_TRUE( result.has_value() );
        EXPECT_EQ( result.value(), 2 );
        EXPECT_EQ( set.size(), 2 );
        EXPECT_FALSE( set.contains( 2 ) );
    }

    TEST( StackHashSetTests, Extract_AllElements )
    {
        StackHashSet<int, 8> set{ 1, 2, 3 };

        auto v1{ set.extract( 1 ) };
        auto v2{ set.extract( 2 ) };
        auto v3{ set.extract( 3 ) };

        EXPECT_TRUE( v1.has_value() && v1.value() == 1 );
        EXPECT_TRUE( v2.has_value() && v2.value() == 2 );
        EXPECT_TRUE( v3.has_value() && v3.value() == 3 );
        EXPECT_TRUE( set.isEmpty() );
    }

    //=====================================================================
    // STL C++17 compatibility - merge()
    //=====================================================================

    TEST( StackHashSetTests, Merge_BothOnStack )
    {
        StackHashSet<int, 8> set1{ 1, 2 };
        StackHashSet<int, 8> set2{ 3, 4 };

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 4 );
        EXPECT_TRUE( set1.contains( 1 ) );
        EXPECT_TRUE( set1.contains( 2 ) );
        EXPECT_TRUE( set1.contains( 3 ) );
        EXPECT_TRUE( set1.contains( 4 ) );
    }

    TEST( StackHashSetTests, Merge_DuplicateKeys )
    {
        StackHashSet<int, 8> set1{ 1, 2 };
        StackHashSet<int, 8> set2{ 1, 3 };

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 3 );
        EXPECT_TRUE( set1.contains( 1 ) );
        EXPECT_TRUE( set1.contains( 2 ) );
        EXPECT_TRUE( set1.contains( 3 ) );
    }

    TEST( StackHashSetTests, Merge_BothOnHeap )
    {
        StackHashSet<int, 2> set1;
        StackHashSet<int, 2> set2;

        // Force both to heap
        set1.insert( 1 );
        set1.insert( 2 );
        set1.insert( 3 );

        set2.insert( 4 );
        set2.insert( 5 );
        set2.insert( 6 );

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 6 );
        EXPECT_TRUE( set1.contains( 1 ) );
        EXPECT_TRUE( set1.contains( 6 ) );
    }

    TEST( StackHashSetTests, Merge_StackToHeap )
    {
        StackHashSet<int, 2> set1;
        set1.insert( 1 );
        set1.insert( 2 );
        set1.insert( 3 ); // set1 on heap

        StackHashSet<int, 2> set2;
        set2.insert( 4 );
        set2.insert( 5 ); // set2 on stack

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 5 );
        EXPECT_TRUE( set1.contains( 4 ) );
        EXPECT_TRUE( set1.contains( 5 ) );
    }

    TEST( StackHashSetTests, Merge_HeapToStack )
    {
        StackHashSet<int, 8> set1; // On stack
        set1.insert( 1 );

        StackHashSet<int, 8> set2; // Will be on heap
        for ( int i{ 2 }; i <= 10; ++i )
        {
            set2.insert( i ); // Fill beyond stack capacity to force heap
        }

        set1.merge( set2 ); // Should trigger transition of set1 to heap

        EXPECT_EQ( set1.size(), 10 );
        EXPECT_TRUE( set1.contains( 1 ) );
        EXPECT_TRUE( set1.contains( 10 ) );
    }

    TEST( StackHashSetTests, Merge_EmptySource )
    {
        StackHashSet<int> set1{ 1 };
        StackHashSet<int> set2;

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 1 );
        EXPECT_TRUE( set1.contains( 1 ) );
    }

    TEST( StackHashSetTests, Merge_EmptyDestination )
    {
        StackHashSet<int> set1;
        StackHashSet<int> set2{ 1, 2 };

        set1.merge( set2 );

        EXPECT_EQ( set1.size(), 2 );
        EXPECT_TRUE( set1.contains( 1 ) );
        EXPECT_TRUE( set1.contains( 2 ) );
    }

    TEST( StackHashSetTests, Merge_RvalueOverload )
    {
        StackHashSet<int> set1{ 1 };
        StackHashSet<int> set2{ 2, 3 };

        set1.merge( std::move( set2 ) ); // Rvalue overload

        EXPECT_EQ( set1.size(), 3 );
        EXPECT_TRUE( set1.contains( 1 ) );
        EXPECT_TRUE( set1.contains( 2 ) );
        EXPECT_TRUE( set1.contains( 3 ) );
    }

    //=====================================================================
    // Edge cases
    //=====================================================================

    TEST( StackHashSetTests, LargeDataset_AfterTransition )
    {
        StackHashSet<int, 8> set;

        // Insert many elements
        for ( int i{ 0 }; i < 100; ++i )
        {
            set.insert( i );
        }

        EXPECT_EQ( set.size(), 100 );

        // Verify all elements
        for ( int i{ 0 }; i < 100; ++i )
        {
            EXPECT_TRUE( set.contains( i ) );
        }
    }

    TEST( StackHashSetTests, StringKeys_StackAndHeap )
    {
        StackHashSet<std::string, 3> set;

        set.insert( "one" );
        set.insert( "two" );
        set.insert( "three" );
        set.insert( "four" ); // Transition

        EXPECT_TRUE( set.contains( "one" ) );
        EXPECT_TRUE( set.contains( "two" ) );
        EXPECT_TRUE( set.contains( "three" ) );
        EXPECT_TRUE( set.contains( "four" ) );

        // Heterogeneous lookup after transition
        std::string_view key{ "one" };
        EXPECT_TRUE( set.contains( key ) );
    }

    TEST( StackHashSetTests, EraseFromStackMaintainsOrder )
    {
        StackHashSet<int, 8> set{ 1, 2, 3, 4 };

        set.erase( 2 );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( 1 ) );
        EXPECT_FALSE( set.contains( 2 ) );
        EXPECT_TRUE( set.contains( 3 ) );
        EXPECT_TRUE( set.contains( 4 ) );
    }

    TEST( StackHashSetTests, ClearAfterTransition )
    {
        StackHashSet<int, 2> set;

        set.insert( 1 );
        set.insert( 2 );
        set.insert( 3 ); // Transition to heap

        set.clear();

        EXPECT_TRUE( set.isEmpty() );
        EXPECT_EQ( set.size(), 0 );

        // Can still insert after clear
        set.insert( 5 );
        EXPECT_EQ( set.size(), 1 );
        EXPECT_TRUE( set.contains( 5 ) );
    }

} // namespace nfx::containers::test
