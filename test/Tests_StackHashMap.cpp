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
 * @file Tests_StackHashMap.cpp
 * @brief Tests for StackHashMap (SBO hash map)
 */

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nfx/containers/StackHashMap.h>

namespace nfx::containers::test
{
    //=====================================================================
    // Constructor tests
    //=====================================================================

    TEST( StackHashMapTests, DefaultConstructor_Empty )
    {
        StackHashMap<std::string, int> map;

        EXPECT_EQ( map.size(), 0 );
        EXPECT_TRUE( map.isEmpty() );
    }

    TEST( StackHashMapTests, InitializerListConstructor_Basic )
    {
        StackHashMap<std::string, int> map{ { "apple", 1 }, { "banana", 2 }, { "cherry", 3 } };

        EXPECT_EQ( map.size(), 3 );
        EXPECT_FALSE( map.isEmpty() );
        EXPECT_EQ( map.at( "apple" ), 1 );
        EXPECT_EQ( map.at( "banana" ), 2 );
        EXPECT_EQ( map.at( "cherry" ), 3 );
    }

    TEST( StackHashMapTests, InitializerListConstructor_Empty )
    {
        StackHashMap<std::string, int> map{};

        EXPECT_EQ( map.size(), 0 );
        EXPECT_TRUE( map.isEmpty() );
    }

    TEST( StackHashMapTests, InitializerListConstructor_SingleElement )
    {
        StackHashMap<std::string, int> map{ { "single", 42 } };

        EXPECT_EQ( map.size(), 1 );
        EXPECT_EQ( map.at( "single" ), 42 );
    }

    TEST( StackHashMapTests, RangeConstructor_FromVector )
    {
        std::vector<std::pair<std::string, int>> data{
            { "one", 1 },
            { "two", 2 },
            { "three", 3 } };

        StackHashMap<std::string, int> map( data.begin(), data.end() );

        EXPECT_EQ( map.size(), 3 );
        EXPECT_EQ( map.at( "one" ), 1 );
        EXPECT_EQ( map.at( "two" ), 2 );
        EXPECT_EQ( map.at( "three" ), 3 );
    }

    TEST( StackHashMapTests, RangeConstructor_EmptyRange )
    {
        std::vector<std::pair<int, int>> data;
        StackHashMap<int, int> map( data.begin(), data.end() );

        EXPECT_EQ( map.size(), 0 );
        EXPECT_TRUE( map.isEmpty() );
    }

    //=====================================================================
    // Capacity tests
    //=====================================================================

    TEST( StackHashMapTests, StackCapacity_ReturnsTemplateParameter )
    {
        StackHashMap<int, int, 8> map8;
        StackHashMap<int, int, 16> map16;
        StackHashMap<int, int, 32> map32;

        EXPECT_EQ( map8.stackCapacity(), 8 );
        EXPECT_EQ( map16.stackCapacity(), 16 );
        EXPECT_EQ( map32.stackCapacity(), 32 );
    }

    TEST( StackHashMapTests, Size_TracksCurrent )
    {
        StackHashMap<int, int> map;

        EXPECT_EQ( map.size(), 0 );

        map.insert( { 1, 100 } );
        EXPECT_EQ( map.size(), 1 );

        map.insert( { 2, 200 } );
        EXPECT_EQ( map.size(), 2 );

        map.erase( 1 );
        EXPECT_EQ( map.size(), 1 );

        map.clear();
        EXPECT_EQ( map.size(), 0 );
    }

    TEST( StackHashMapTests, Empty_ReflectsSize )
    {
        StackHashMap<int, int> map;

        EXPECT_TRUE( map.isEmpty() );

        map.insert( { 1, 100 } );
        EXPECT_FALSE( map.isEmpty() );

        map.clear();
        EXPECT_TRUE( map.isEmpty() );
    }

    //=====================================================================
    // Element access - at()
    //=====================================================================

    TEST( StackHashMapTests, At_ReturnsValue )
    {
        StackHashMap<std::string, int> map{ { "key1", 10 }, { "key2", 20 } };

        EXPECT_EQ( map.at( "key1" ), 10 );
        EXPECT_EQ( map.at( "key2" ), 20 );
    }

    TEST( StackHashMapTests, At_ThrowsOnMissing )
    {
        StackHashMap<std::string, int> map{ { "exists", 42 } };

        EXPECT_THROW( map.at( "missing" ), std::out_of_range );
    }

    TEST( StackHashMapTests, At_ModifiesValue )
    {
        StackHashMap<std::string, int> map{ { "key", 100 } };

        map.at( "key" ) = 200;
        EXPECT_EQ( map.at( "key" ), 200 );
    }

    TEST( StackHashMapTests, At_ConstAccess )
    {
        const StackHashMap<std::string, int> map{ { "const_key", 42 } };

        EXPECT_EQ( map.at( "const_key" ), 42 );
        EXPECT_THROW( map.at( "missing" ), std::out_of_range );
    }

    //=====================================================================
    // Element access - operator[]
    //=====================================================================

    TEST( StackHashMapTests, OperatorSubscript_InsertIfMissing )
    {
        StackHashMap<std::string, int> map;

        map["key1"] = 100;
        map["key2"] = 200;
        map["key3"] = 300;

        EXPECT_EQ( map.size(), 3 );
        EXPECT_EQ( map["key1"], 100 );
        EXPECT_EQ( map["key2"], 200 );
        EXPECT_EQ( map["key3"], 300 );
    }

    TEST( StackHashMapTests, OperatorSubscript_DefaultConstruction )
    {
        StackHashMap<std::string, int> map;

        int& value{ map["new_key"] };
        EXPECT_EQ( value, 0 );
        EXPECT_EQ( map.size(), 1 );

        value = 42;
        EXPECT_EQ( map["new_key"], 42 );
    }

    TEST( StackHashMapTests, OperatorSubscript_MoveKey )
    {
        StackHashMap<std::string, int> map;

        std::string key{ "movable" };
        map[std::move( key )] = 123;

        EXPECT_EQ( map["movable"], 123 );
    }

    //=====================================================================
    // Modifiers - insert()
    //=====================================================================

    TEST( StackHashMapTests, Insert_NewElement )
    {
        StackHashMap<int, std::string> map;

        auto [ptr, inserted]{ map.insert( { 10, "ten" } ) };

        EXPECT_TRUE( inserted );
        EXPECT_NE( ptr, nullptr );
        EXPECT_EQ( ptr->second, "ten" );
        EXPECT_EQ( map.size(), 1 );
    }

    TEST( StackHashMapTests, Insert_DuplicateKey )
    {
        StackHashMap<int, std::string> map;

        auto [ptr1, inserted1]{ map.insert( { 10, "original" } ) };
        EXPECT_TRUE( inserted1 );

        auto [ptr2, inserted2]{ map.insert( { 10, "duplicate" } ) };
        EXPECT_FALSE( inserted2 );
        EXPECT_EQ( ptr2->second, "original" );
        EXPECT_EQ( map.size(), 1 );
    }

    TEST( StackHashMapTests, Insert_MoveSemantics )
    {
        StackHashMap<std::string, std::string> map;

        std::pair<std::string, std::string> pair{ "key", "value" };
        auto [ptr, inserted]{ map.insert( std::move( pair ) ) };

        EXPECT_TRUE( inserted );
        EXPECT_EQ( ptr->second, "value" );
    }

    //=====================================================================
    // Modifiers - emplace()
    //=====================================================================

    TEST( StackHashMapTests, Emplace_NewElement )
    {
        StackHashMap<int, std::string> map;

        auto [ptr, inserted]{ map.emplace( 10, "ten" ) };

        EXPECT_TRUE( inserted );
        EXPECT_NE( ptr, nullptr );
        EXPECT_EQ( ptr->second, "ten" );
    }

    TEST( StackHashMapTests, Emplace_DuplicateKey )
    {
        StackHashMap<int, std::string> map;

        map.emplace( 5, "five" );
        auto [ptr, inserted]{ map.emplace( 5, "duplicate" ) };

        EXPECT_FALSE( inserted );
        EXPECT_EQ( ptr->second, "five" );
    }

    //=====================================================================
    // Modifiers - erase()
    //=====================================================================

    TEST( StackHashMapTests, Erase_ExistingKey )
    {
        StackHashMap<int, int> map{ { 1, 10 }, { 2, 20 }, { 3, 30 } };

        size_t erased{ map.erase( 2 ) };

        EXPECT_EQ( erased, 1 );
        EXPECT_EQ( map.size(), 2 );
        EXPECT_TRUE( map.contains( 1 ) );
        EXPECT_FALSE( map.contains( 2 ) );
        EXPECT_TRUE( map.contains( 3 ) );
    }

    TEST( StackHashMapTests, Erase_MissingKey )
    {
        StackHashMap<int, int> map{ { 1, 10 } };

        size_t erased{ map.erase( 99 ) };

        EXPECT_EQ( erased, 0 );
        EXPECT_EQ( map.size(), 1 );
    }

    TEST( StackHashMapTests, Erase_AllElements )
    {
        StackHashMap<int, int> map{ { 1, 10 }, { 2, 20 }, { 3, 30 } };

        map.erase( 1 );
        map.erase( 2 );
        map.erase( 3 );

        EXPECT_TRUE( map.isEmpty() );
        EXPECT_EQ( map.size(), 0 );
    }

    //=====================================================================
    // Modifiers - clear()
    //=====================================================================

    TEST( StackHashMapTests, Clear_RemovesAllElements )
    {
        StackHashMap<int, int> map{ { 1, 10 }, { 2, 20 }, { 3, 30 } };

        map.clear();

        EXPECT_TRUE( map.isEmpty() );
        EXPECT_EQ( map.size(), 0 );
        EXPECT_FALSE( map.contains( 1 ) );
        EXPECT_FALSE( map.contains( 2 ) );
        EXPECT_FALSE( map.contains( 3 ) );
    }

    TEST( StackHashMapTests, Clear_OnEmptyMap )
    {
        StackHashMap<int, int> map;

        map.clear();

        EXPECT_TRUE( map.isEmpty() );
        EXPECT_EQ( map.size(), 0 );
    }

    //=====================================================================
    // Lookup - find()
    //=====================================================================

    TEST( StackHashMapTests, Find_ExistingKey_Stack )
    {
        StackHashMap<std::string, int> map{ { "key1", 10 }, { "key2", 20 } };

        int* value{ map.find( "key1" ) };
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, 10 );

        value = map.find( "key2" );
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, 20 );
    }

    TEST( StackHashMapTests, Find_MissingKey_Stack )
    {
        StackHashMap<std::string, int> map{ { "key1", 10 } };

        int* value{ map.find( "missing" ) };
        EXPECT_EQ( value, nullptr );
    }

    TEST( StackHashMapTests, Find_ExistingKey_Heap )
    {
        StackHashMap<int, int, 2> map;
        // Force transition to heap by exceeding stack capacity
        map[1] = 10;
        map[2] = 20;
        map[3] = 30;

        int* value{ map.find( 2 ) };
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, 20 );
    }

    TEST( StackHashMapTests, Find_ConstVersion )
    {
        const StackHashMap<std::string, int> map{ { "test", 42 } };

        const int* value{ map.find( "test" ) };
        ASSERT_NE( value, nullptr );
        EXPECT_EQ( *value, 42 );

        value = map.find( "missing" );
        EXPECT_EQ( value, nullptr );
    }

    TEST( StackHashMapTests, Find_ModifyValue )
    {
        StackHashMap<std::string, int> map{ { "key", 10 } };

        int* value{ map.find( "key" ) };
        ASSERT_NE( value, nullptr );
        *value = 99;

        EXPECT_EQ( map["key"], 99 );
    }

    //=====================================================================
    // Modifiers - insertOrAssign()
    //=====================================================================

    TEST( StackHashMapTests, InsertOrAssign_NewKey_ConstRef )
    {
        StackHashMap<std::string, int> map;
        std::string key{ "new" };
        int value{ 42 };

        map.insertOrAssign( key, value );

        EXPECT_EQ( map.size(), 1 );
        EXPECT_EQ( map["new"], 42 );
    }

    TEST( StackHashMapTests, InsertOrAssign_ExistingKey_UpdatesValue )
    {
        StackHashMap<std::string, int> map{ { "existing", 10 } };

        map.insertOrAssign( "existing", 99 );

        EXPECT_EQ( map.size(), 1 );
        EXPECT_EQ( map["existing"], 99 );
    }

    TEST( StackHashMapTests, InsertOrAssign_MoveValue )
    {
        StackHashMap<std::string, std::string> map;

        std::string value{ "moved_value" };
        map.insertOrAssign( "key", std::move( value ) );

        EXPECT_EQ( map["key"], "moved_value" );
    }

    TEST( StackHashMapTests, InsertOrAssign_MoveKeyAndValue )
    {
        StackHashMap<std::string, std::string> map;

        map.insertOrAssign( std::string{ "key" }, std::string{ "value" } );

        EXPECT_EQ( map.size(), 1 );
        EXPECT_EQ( map["key"], "value" );
    }

    TEST( StackHashMapTests, InsertOrAssign_Heap )
    {
        StackHashMap<int, int, 2> map;
        map[1] = 10;
        map[2] = 20;
        map[3] = 30; // Forces heap transition

        map.insertOrAssign( 2, 999 );

        EXPECT_EQ( map[2], 999 );
        EXPECT_EQ( map.size(), 3 );
    }

    //=====================================================================
    // Lookup - count()
    //=====================================================================

    TEST( StackHashMapTests, Count_ExistingKey )
    {
        StackHashMap<std::string, int> map{ { "exists", 42 } };

        EXPECT_EQ( map.count( "exists" ), 1 );
    }

    TEST( StackHashMapTests, Count_MissingKey )
    {
        StackHashMap<std::string, int> map{ { "exists", 42 } };

        EXPECT_EQ( map.count( "missing" ), 0 );
    }

    //=====================================================================
    // Lookup - contains()
    //=====================================================================

    TEST( StackHashMapTests, Contains_ExistingKey )
    {
        StackHashMap<int, int> map{ { 10, 100 }, { 20, 200 } };

        EXPECT_TRUE( map.contains( 10 ) );
        EXPECT_TRUE( map.contains( 20 ) );
    }

    TEST( StackHashMapTests, Contains_MissingKey )
    {
        StackHashMap<int, int> map{ { 10, 100 } };

        EXPECT_FALSE( map.contains( 99 ) );
    }

    TEST( StackHashMapTests, Contains_HeterogeneousLookup )
    {
        StackHashMap<std::string, int> map{ { "hello", 42 } };

        std::string_view key{ "hello" };
        EXPECT_TRUE( map.contains( key ) );

        std::string_view missing{ "world" };
        EXPECT_FALSE( map.contains( missing ) );
    }

    //=====================================================================
    // Stack to heap transition tests
    //=====================================================================

    TEST( StackHashMapTests, StackToHeapTransition_ViaInsert )
    {
        StackHashMap<int, int, 4> map; // Small capacity: 4

        // Fill stack
        map.insert( { 1, 10 } );
        map.insert( { 2, 20 } );
        map.insert( { 3, 30 } );
        map.insert( { 4, 40 } );

        EXPECT_EQ( map.size(), 4 );

        // Trigger transition to heap
        map.insert( { 5, 50 } );

        EXPECT_EQ( map.size(), 5 );
        EXPECT_EQ( map.at( 1 ), 10 );
        EXPECT_EQ( map.at( 2 ), 20 );
        EXPECT_EQ( map.at( 3 ), 30 );
        EXPECT_EQ( map.at( 4 ), 40 );
        EXPECT_EQ( map.at( 5 ), 50 );
    }

    TEST( StackHashMapTests, StackToHeapTransition_ViaSubscript )
    {
        StackHashMap<int, int, 3> map;

        map[1] = 10;
        map[2] = 20;
        map[3] = 30;
        map[4] = 40; // Triggers transition

        EXPECT_EQ( map.size(), 4 );
        EXPECT_EQ( map[1], 10 );
        EXPECT_EQ( map[2], 20 );
        EXPECT_EQ( map[3], 30 );
        EXPECT_EQ( map[4], 40 );
    }

    TEST( StackHashMapTests, StackToHeapTransition_PreservesData )
    {
        StackHashMap<std::string, std::string, 2> map;

        map["a"] = "alpha";
        map["b"] = "beta";
        map["c"] = "gamma"; // Transition

        EXPECT_EQ( map.size(), 3 );
        EXPECT_EQ( map["a"], "alpha" );
        EXPECT_EQ( map["b"], "beta" );
        EXPECT_EQ( map["c"], "gamma" );
    }

    TEST( StackHashMapTests, StackToHeapTransition_AllOperationsWork )
    {
        StackHashMap<int, int, 2> map;

        map[1] = 10;
        map[2] = 20;
        map[3] = 30; // Now on heap

        // Test operations after transition
        EXPECT_TRUE( map.contains( 1 ) );
        EXPECT_TRUE( map.contains( 2 ) );
        EXPECT_TRUE( map.contains( 3 ) );

        map.erase( 2 );
        EXPECT_FALSE( map.contains( 2 ) );
        EXPECT_EQ( map.size(), 2 );

        map[4] = 40;
        EXPECT_EQ( map.size(), 3 );
        EXPECT_EQ( map[4], 40 );

        map.clear();
        EXPECT_TRUE( map.isEmpty() );
    }

    //=====================================================================
    // Edge cases
    //=====================================================================

    TEST( StackHashMapTests, LargeDataset_AfterTransition )
    {
        StackHashMap<int, int, 8> map;

        // Insert many elements
        for ( int i{ 0 }; i < 100; ++i )
        {
            map[i] = i * 10;
        }

        EXPECT_EQ( map.size(), 100 );

        // Verify all elements
        for ( int i{ 0 }; i < 100; ++i )
        {
            EXPECT_EQ( map[i], i * 10 );
        }
    }

    TEST( StackHashMapTests, StringKeys_StackAndHeap )
    {
        StackHashMap<std::string, int, 3> map;

        map["one"] = 1;
        map["two"] = 2;
        map["three"] = 3;
        map["four"] = 4; // Transition

        EXPECT_EQ( map["one"], 1 );
        EXPECT_EQ( map["two"], 2 );
        EXPECT_EQ( map["three"], 3 );
        EXPECT_EQ( map["four"], 4 );

        // Heterogeneous lookup after transition
        std::string_view key{ "one" };
        EXPECT_TRUE( map.contains( key ) );
    }

    TEST( StackHashMapTests, EraseFromStackMaintainsOrder )
    {
        StackHashMap<int, int, 8> map{ { 1, 10 }, { 2, 20 }, { 3, 30 }, { 4, 40 } };

        map.erase( 2 );

        EXPECT_EQ( map.size(), 3 );
        EXPECT_TRUE( map.contains( 1 ) );
        EXPECT_FALSE( map.contains( 2 ) );
        EXPECT_TRUE( map.contains( 3 ) );
        EXPECT_TRUE( map.contains( 4 ) );
    }

    TEST( StackHashMapTests, ClearAfterTransition )
    {
        StackHashMap<int, int, 2> map;

        map[1] = 10;
        map[2] = 20;
        map[3] = 30; // Transition to heap

        map.clear();

        EXPECT_TRUE( map.isEmpty() );
        EXPECT_EQ( map.size(), 0 );

        // Can still insert after clear
        map[5] = 50;
        EXPECT_EQ( map.size(), 1 );
        EXPECT_EQ( map[5], 50 );
    }

    //=====================================================================
    // STL C++17 compatibility - extract()
    //=====================================================================

    TEST( StackHashMapTests, Extract_ExistingKey_Stack )
    {
        StackHashMap<int, std::string, 8> map{ { 1, "one" }, { 2, "two" }, { 3, "three" } };

        auto result{ map.extract( 2 ) };

        EXPECT_TRUE( result.has_value() );
        EXPECT_EQ( result.value().first, 2 );
        EXPECT_EQ( result.value().second, "two" );
        EXPECT_EQ( map.size(), 2 );
        EXPECT_FALSE( map.contains( 2 ) );
        EXPECT_TRUE( map.contains( 1 ) );
        EXPECT_TRUE( map.contains( 3 ) );
    }

    TEST( StackHashMapTests, Extract_MissingKey_Stack )
    {
        StackHashMap<int, std::string, 8> map{ { 1, "one" } };

        auto result{ map.extract( 99 ) };

        EXPECT_FALSE( result.has_value() );
        EXPECT_EQ( map.size(), 1 );
    }

    TEST( StackHashMapTests, Extract_ExistingKey_Heap )
    {
        StackHashMap<int, std::string, 2> map;

        map[1] = "one";
        map[2] = "two";
        map[3] = "three"; // Transition to heap

        auto result{ map.extract( 2 ) };

        EXPECT_TRUE( result.has_value() );
        EXPECT_EQ( result.value().first, 2 );
        EXPECT_EQ( result.value().second, "two" );
        EXPECT_EQ( map.size(), 2 );
        EXPECT_FALSE( map.contains( 2 ) );
    }

    TEST( StackHashMapTests, Extract_AllElements )
    {
        StackHashMap<int, int, 8> map{ { 1, 10 }, { 2, 20 }, { 3, 30 } };

        auto v1{ map.extract( 1 ) };
        auto v2{ map.extract( 2 ) };
        auto v3{ map.extract( 3 ) };

        EXPECT_TRUE( v1.has_value() && v1.value().first == 1 && v1.value().second == 10 );
        EXPECT_TRUE( v2.has_value() && v2.value().first == 2 && v2.value().second == 20 );
        EXPECT_TRUE( v3.has_value() && v3.value().first == 3 && v3.value().second == 30 );
        EXPECT_TRUE( map.isEmpty() );
    }

    //=====================================================================
    // STL C++17 compatibility - merge()
    //=====================================================================

    TEST( StackHashMapTests, Merge_BothOnStack )
    {
        StackHashMap<int, std::string, 8> map1{ { 1, "one" }, { 2, "two" } };
        StackHashMap<int, std::string, 8> map2{ { 3, "three" }, { 4, "four" } };

        map1.merge( map2 );

        EXPECT_EQ( map1.size(), 4 );
        EXPECT_EQ( map1[1], "one" );
        EXPECT_EQ( map1[2], "two" );
        EXPECT_EQ( map1[3], "three" );
        EXPECT_EQ( map1[4], "four" );
    }

    TEST( StackHashMapTests, Merge_DuplicateKeys )
    {
        StackHashMap<int, std::string, 8> map1{ { 1, "original" }, { 2, "two" } };
        StackHashMap<int, std::string, 8> map2{ { 1, "duplicate" }, { 3, "three" } };

        map1.merge( map2 );

        EXPECT_EQ( map1.size(), 3 );
        EXPECT_EQ( map1[1], "original" ); // Not overwritten
        EXPECT_EQ( map1[2], "two" );
        EXPECT_EQ( map1[3], "three" );
    }

    TEST( StackHashMapTests, Merge_BothOnHeap )
    {
        StackHashMap<int, int, 2> map1;
        StackHashMap<int, int, 2> map2;

        // Force both to heap
        map1[1] = 10;
        map1[2] = 20;
        map1[3] = 30;

        map2[4] = 40;
        map2[5] = 50;
        map2[6] = 60;

        map1.merge( map2 );

        EXPECT_EQ( map1.size(), 6 );
        EXPECT_EQ( map1[1], 10 );
        EXPECT_EQ( map1[6], 60 );
    }

    TEST( StackHashMapTests, Merge_StackToHeap )
    {
        StackHashMap<int, int, 2> map1;
        map1[1] = 10;
        map1[2] = 20;
        map1[3] = 30; // map1 on heap

        StackHashMap<int, int, 2> map2;
        map2[4] = 40;
        map2[5] = 50; // map2 on stack

        map1.merge( map2 );

        EXPECT_EQ( map1.size(), 5 );
        EXPECT_EQ( map1[4], 40 );
        EXPECT_EQ( map1[5], 50 );
    }

    TEST( StackHashMapTests, Merge_HeapToStack )
    {
        StackHashMap<int, int, 8> map1; // On stack (capacity 8)
        map1[1] = 10;

        StackHashMap<int, int, 8> map2; // Same N - will force to heap by filling
        for ( int i{ 2 }; i <= 10; ++i )
        {
            map2[i] = i * 10; // Fill beyond stack capacity to force heap
        }

        map1.merge( map2 ); // Should trigger transition of map1 to heap

        EXPECT_EQ( map1.size(), 10 );
        EXPECT_EQ( map1[1], 10 );
        EXPECT_EQ( map1[2], 20 );
        EXPECT_EQ( map1[10], 100 );
    }

    TEST( StackHashMapTests, Merge_EmptySource )
    {
        StackHashMap<int, int> map1{ { 1, 10 } };
        StackHashMap<int, int> map2;

        map1.merge( map2 );

        EXPECT_EQ( map1.size(), 1 );
        EXPECT_EQ( map1[1], 10 );
    }

    TEST( StackHashMapTests, Merge_EmptyDestination )
    {
        StackHashMap<int, int> map1;
        StackHashMap<int, int> map2{ { 1, 10 }, { 2, 20 } };

        map1.merge( map2 );

        EXPECT_EQ( map1.size(), 2 );
        EXPECT_EQ( map1[1], 10 );
        EXPECT_EQ( map1[2], 20 );
    }

    TEST( StackHashMapTests, Merge_RvalueOverload )
    {
        StackHashMap<int, int> map1{ { 1, 10 } };
        StackHashMap<int, int> map2{ { 2, 20 }, { 3, 30 } };

        map1.merge( std::move( map2 ) ); // Rvalue overload

        EXPECT_EQ( map1.size(), 3 );
        EXPECT_EQ( map1[1], 10 );
        EXPECT_EQ( map1[2], 20 );
        EXPECT_EQ( map1[3], 30 );
    }

    //=====================================================================
    // Iteration - forEach()
    //=====================================================================

    TEST( StackHashMapTests, ForEach_Stack )
    {
        StackHashMap<std::string, int, 4> map{ { "a", 1 }, { "b", 2 }, { "c", 3 } };

        int sum{ 0 };
        map.forEach( [&sum]( const std::string&, int value ) { sum += value; } );

        EXPECT_EQ( sum, 6 );
    }

    TEST( StackHashMapTests, ForEach_Heap )
    {
        StackHashMap<std::string, int, 2> map{ { "a", 1 }, { "b", 2 }, { "c", 3 } };

        int sum{ 0 };
        map.forEach( [&sum]( const std::string&, int value ) { sum += value; } );

        EXPECT_EQ( sum, 6 );
    }

    TEST( StackHashMapTests, ForEach_Modify )
    {
        StackHashMap<std::string, int, 4> map{ { "a", 1 }, { "b", 2 } };

        map.forEach( []( const std::string&, int& value ) { value *= 2; } );

        EXPECT_EQ( map["a"], 2 );
        EXPECT_EQ( map["b"], 4 );
    }

    TEST( StackHashMapTests, ForEach_Const )
    {
        const StackHashMap<std::string, int, 4> map{ { "a", 1 }, { "b", 2 } };

        int sum{ 0 };
        map.forEach( [&sum]( const std::string&, const int& value ) { sum += value; } );

        EXPECT_EQ( sum, 3 );
    }

    TEST( StackHashMapTests, ForEach_Empty )
    {
        StackHashMap<std::string, int> map;

        int count{ 0 };
        map.forEach( [&count]( const std::string&, int ) { ++count; } );

        EXPECT_EQ( count, 0 );
    }
} // namespace nfx::containers::test
