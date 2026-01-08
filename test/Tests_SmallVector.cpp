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
 * @file Tests_SmallVector.cpp
 * @brief Unit tests for SmallVector (Small Vector Optimization with stack storage)
 */

#include <gtest/gtest.h>

#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nfx/containers/SmallVector.h>

namespace nfx::containers::test
{
    //=====================================================================
    // Constructor tests
    //=====================================================================

    TEST( SmallVectorTests, DefaultConstructor_Empty )
    {
        SmallVector<int, 4> vec;
        EXPECT_EQ( vec.size(), 0 );
        EXPECT_TRUE( vec.isEmpty() );
        EXPECT_EQ( vec.capacity(), 4 );
    }

    TEST( SmallVectorTests, DefaultConstructor_StringType )
    {
        SmallVector<std::string, 8> vec;
        EXPECT_EQ( vec.size(), 0 );
        EXPECT_TRUE( vec.isEmpty() );
        EXPECT_EQ( vec.capacity(), 8 );
    }

    //=====================================================================
    // Push back operations - Stack storage
    //=====================================================================

    TEST( SmallVectorTests, PushBack_StaysOnStack )
    {
        SmallVector<int, 8> vec;

        vec.push_back( 10 );
        vec.push_back( 20 );
        vec.push_back( 30 );

        EXPECT_EQ( vec.size(), 3 );
        EXPECT_EQ( vec.capacity(), 8 ); // Still on stack
        EXPECT_EQ( vec[0], 10 );
        EXPECT_EQ( vec[1], 20 );
        EXPECT_EQ( vec[2], 30 );
    }

    TEST( SmallVectorTests, PushBack_MoveSemantics )
    {
        SmallVector<std::string, 4> vec;

        std::string s1 = "hello";
        std::string s2 = "world";

        vec.push_back( std::move( s1 ) );
        vec.push_back( std::move( s2 ) );

        EXPECT_EQ( vec.size(), 2 );
        EXPECT_EQ( vec[0], "hello" );
        EXPECT_EQ( vec[1], "world" );
    }

    TEST( SmallVectorTests, PushBack_FillsStack )
    {
        SmallVector<int, 4> vec;

        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 );
        vec.push_back( 4 );

        EXPECT_EQ( vec.size(), 4 );
        EXPECT_EQ( vec.capacity(), 4 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[3], 4 );
    }

    //=====================================================================
    // Heap transition tests
    //=====================================================================

    TEST( SmallVectorTests, HeapTransition_ExceedsStackCapacity )
    {
        SmallVector<int, 4> vec;

        // Fill stack
        for ( int i = 0; i < 4; ++i )
        {
            vec.push_back( i );
        }

        EXPECT_EQ( vec.capacity(), 4 );

        // Trigger heap transition
        vec.push_back( 4 );

        EXPECT_EQ( vec.size(), 5 );
        EXPECT_GT( vec.capacity(), 4 ); // Now on heap

        // Verify all elements intact
        for ( int i = 0; i < 5; ++i )
        {
            EXPECT_EQ( vec[i], i );
        }
    }

    TEST( SmallVectorTests, HeapTransition_GrowthStrategy )
    {
        SmallVector<int, 4> vec;

        for ( int i = 0; i < 4; ++i )
        {
            vec.push_back( i );
        }

        vec.push_back( 4 ); // Trigger transition

        // Should reserve 2x initial capacity
        EXPECT_GE( vec.capacity(), 8 );
    }

    TEST( SmallVectorTests, HeapTransition_StringData )
    {
        SmallVector<std::string, 2> vec;

        vec.push_back( "first" );
        vec.push_back( "second" );
        EXPECT_EQ( vec.capacity(), 2 );

        vec.push_back( "third" ); // Transition to heap

        EXPECT_EQ( vec.size(), 3 );
        EXPECT_GT( vec.capacity(), 2 );
        EXPECT_EQ( vec[0], "first" );
        EXPECT_EQ( vec[1], "second" );
        EXPECT_EQ( vec[2], "third" );
    }

    //=====================================================================
    // Emplace back operations
    //=====================================================================

    TEST( SmallVectorTests, EmplaceBack_StackStorage )
    {
        SmallVector<std::string, 4> vec;

        vec.emplace_back( "hello" );
        vec.emplace_back( 5, 'x' ); // "xxxxx"

        EXPECT_EQ( vec.size(), 2 );
        EXPECT_EQ( vec[0], "hello" );
        EXPECT_EQ( vec[1], "xxxxx" );
    }

    TEST( SmallVectorTests, EmplaceBack_HeapStorage )
    {
        SmallVector<std::pair<int, std::string>, 2> vec;

        vec.emplace_back( 1, "one" );
        vec.emplace_back( 2, "two" );
        vec.emplace_back( 3, "three" ); // Triggers heap

        EXPECT_EQ( vec.size(), 3 );
        EXPECT_EQ( vec[2].first, 3 );
        EXPECT_EQ( vec[2].second, "three" );
    }

    //=====================================================================
    // Element access tests
    //=====================================================================

    TEST( SmallVectorTests, OperatorSubscript_ReadWrite )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 10 );
        vec.push_back( 20 );

        EXPECT_EQ( vec[0], 10 );
        EXPECT_EQ( vec[1], 20 );

        vec[0] = 100;
        vec[1] = 200;

        EXPECT_EQ( vec[0], 100 );
        EXPECT_EQ( vec[1], 200 );
    }

    TEST( SmallVectorTests, OperatorSubscript_ConstAccess )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 42 );

        const auto& cvec = vec;
        EXPECT_EQ( cvec[0], 42 );
    }

    TEST( SmallVectorTests, Back_AccessLastElement )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 );

        EXPECT_EQ( vec.back(), 3 );

        vec.back() = 99;
        EXPECT_EQ( vec.back(), 99 );
    }

    TEST( SmallVectorTests, Back_ConstAccess )
    {
        SmallVector<std::string, 4> vec;
        vec.push_back( "test" );

        const auto& cvec = vec;
        EXPECT_EQ( cvec.back(), "test" );
    }

    TEST( SmallVectorTests, Front_AccessFirstElement )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 10 );
        vec.push_back( 20 );
        vec.push_back( 30 );

        EXPECT_EQ( vec.front(), 10 );

        vec.front() = 100;
        EXPECT_EQ( vec.front(), 100 );
    }

    TEST( SmallVectorTests, Front_ConstAccess )
    {
        SmallVector<std::string, 4> vec;
        vec.push_back( "first" );
        vec.push_back( "second" );

        const auto& cvec = vec;
        EXPECT_EQ( cvec.front(), "first" );
    }

    TEST( SmallVectorTests, Data_RawPointerAccess )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 );

        int* ptr = vec.data();
        EXPECT_EQ( ptr[0], 1 );
        EXPECT_EQ( ptr[1], 2 );
        EXPECT_EQ( ptr[2], 3 );

        ptr[1] = 99;
        EXPECT_EQ( vec[1], 99 );
    }

    //=====================================================================
    // Capacity tests
    //=====================================================================

    TEST( SmallVectorTests, Size_TracksElementCount )
    {
        SmallVector<int, 8> vec;
        EXPECT_EQ( vec.size(), 0 );

        vec.push_back( 1 );
        EXPECT_EQ( vec.size(), 1 );

        vec.push_back( 2 );
        EXPECT_EQ( vec.size(), 2 );

        vec.push_back( 3 );
        EXPECT_EQ( vec.size(), 3 );
    }

    TEST( SmallVectorTests, Empty_DetectsEmptyState )
    {
        SmallVector<int, 4> vec;
        EXPECT_TRUE( vec.isEmpty() );

        vec.push_back( 1 );
        EXPECT_FALSE( vec.isEmpty() );

        vec.pop_back();
        EXPECT_TRUE( vec.isEmpty() );
    }

    TEST( SmallVectorTests, Capacity_StackMode )
    {
        SmallVector<int, 16> vec;
        EXPECT_EQ( vec.capacity(), 16 );

        for ( int i = 0; i < 10; ++i )
        {
            vec.push_back( i );
        }

        EXPECT_EQ( vec.capacity(), 16 ); // Still on stack
    }

    TEST( SmallVectorTests, Capacity_HeapMode )
    {
        SmallVector<int, 4> vec;
        EXPECT_EQ( vec.capacity(), 4 );

        for ( int i = 0; i < 5; ++i )
        {
            vec.push_back( i );
        }

        EXPECT_GT( vec.capacity(), 4 ); // On heap now
    }

    TEST( SmallVectorTests, Reserve_PreallocateHeap )
    {
        SmallVector<int, 4> vec;

        vec.reserve( 100 );

        EXPECT_GT( vec.capacity(), 4 );
        EXPECT_GE( vec.capacity(), 100 );
        EXPECT_EQ( vec.size(), 0 );
    }

    TEST( SmallVectorTests, Reserve_NoEffectWhenSmall )
    {
        SmallVector<int, 8> vec;

        vec.reserve( 4 ); // Less than stack capacity

        EXPECT_EQ( vec.capacity(), 8 );
    }

    //=====================================================================
    // Iterator tests
    //=====================================================================

    TEST( SmallVectorTests, Iterator_BasicIteration )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 10 );
        vec.push_back( 20 );
        vec.push_back( 30 );

        std::vector<int> values;
        for ( int val : vec )
        {
            values.push_back( val );
        }

        ASSERT_EQ( values.size(), 3 );
        EXPECT_EQ( values[0], 10 );
        EXPECT_EQ( values[1], 20 );
        EXPECT_EQ( values[2], 30 );
    }

    TEST( SmallVectorTests, Iterator_EmptyVector )
    {
        SmallVector<int, 4> vec;

        EXPECT_EQ( vec.begin(), vec.end() );

        int count = 0;
        for ( [[maybe_unused]] int val : vec )
        {
            ++count;
        }
        EXPECT_EQ( count, 0 );
    }

    TEST( SmallVectorTests, Iterator_ConstIteration )
    {
        SmallVector<std::string, 4> vec;
        vec.push_back( "a" );
        vec.push_back( "b" );
        vec.push_back( "c" );

        const auto& cvec = vec;

        std::vector<std::string> values;
        for ( const auto& val : cvec )
        {
            values.push_back( val );
        }

        ASSERT_EQ( values.size(), 3 );
        EXPECT_EQ( values[0], "a" );
        EXPECT_EQ( values[1], "b" );
        EXPECT_EQ( values[2], "c" );
    }

    TEST( SmallVectorTests, Iterator_HeapStorage )
    {
        SmallVector<int, 2> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 ); // Transition to heap
        vec.push_back( 4 );

        std::vector<int> values;
        for ( int val : vec )
        {
            values.push_back( val );
        }

        ASSERT_EQ( values.size(), 4 );
        EXPECT_EQ( values[0], 1 );
        EXPECT_EQ( values[3], 4 );
    }

    //=====================================================================
    // Clear operation
    //=====================================================================

    TEST( SmallVectorTests, Clear_StackStorage )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 );

        EXPECT_EQ( vec.size(), 3 );

        vec.clear();

        EXPECT_EQ( vec.size(), 0 );
        EXPECT_TRUE( vec.isEmpty() );
    }

    TEST( SmallVectorTests, Clear_HeapStorage )
    {
        SmallVector<int, 2> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 ); // Heap

        vec.clear();

        EXPECT_EQ( vec.size(), 0 );
        EXPECT_TRUE( vec.isEmpty() );
    }

    TEST( SmallVectorTests, Clear_StringDestruction )
    {
        SmallVector<std::string, 4> vec;
        vec.push_back( "test1" );
        vec.push_back( "test2" );

        vec.clear();

        EXPECT_EQ( vec.size(), 0 );
        EXPECT_TRUE( vec.isEmpty() );

        // Can reuse after clear
        vec.push_back( "new" );
        EXPECT_EQ( vec.size(), 1 );
        EXPECT_EQ( vec[0], "new" );
    }

    //=====================================================================
    // Pop back operation
    //=====================================================================

    TEST( SmallVectorTests, PopBack_StackStorage )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 );

        EXPECT_EQ( vec.size(), 3 );

        vec.pop_back();

        EXPECT_EQ( vec.size(), 2 );
        EXPECT_EQ( vec.back(), 2 );

        vec.pop_back();

        EXPECT_EQ( vec.size(), 1 );
        EXPECT_EQ( vec.back(), 1 );
    }

    TEST( SmallVectorTests, PopBack_HeapStorage )
    {
        SmallVector<int, 2> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 ); // Heap
        vec.push_back( 4 );

        EXPECT_EQ( vec.size(), 4 );

        vec.pop_back();

        EXPECT_EQ( vec.size(), 3 );
        EXPECT_EQ( vec.back(), 3 );
    }

    TEST( SmallVectorTests, PopBack_StringDestruction )
    {
        SmallVector<std::string, 4> vec;
        vec.push_back( "one" );
        vec.push_back( "two" );
        vec.push_back( "three" );

        vec.pop_back();

        EXPECT_EQ( vec.size(), 2 );
        EXPECT_EQ( vec.back(), "two" );
    }

    //=====================================================================
    // Comparison operator tests
    //=====================================================================

    TEST( SmallVectorTests, Equality_IdenticalVectors )
    {
        SmallVector<int, 4> vec1;
        SmallVector<int, 4> vec2;

        vec1.push_back( 1 );
        vec1.push_back( 2 );
        vec1.push_back( 3 );

        vec2.push_back( 1 );
        vec2.push_back( 2 );
        vec2.push_back( 3 );

        EXPECT_TRUE( vec1 == vec2 );
        EXPECT_FALSE( vec1 != vec2 );
    }

    TEST( SmallVectorTests, Equality_DifferentValues )
    {
        SmallVector<int, 4> vec1;
        SmallVector<int, 4> vec2;

        vec1.push_back( 1 );
        vec1.push_back( 2 );

        vec2.push_back( 1 );
        vec2.push_back( 3 );

        EXPECT_FALSE( vec1 == vec2 );
        EXPECT_TRUE( vec1 != vec2 );
    }

    TEST( SmallVectorTests, Equality_DifferentSizes )
    {
        SmallVector<int, 4> vec1;
        SmallVector<int, 4> vec2;

        vec1.push_back( 1 );
        vec1.push_back( 2 );

        vec2.push_back( 1 );

        EXPECT_FALSE( vec1 == vec2 );
        EXPECT_TRUE( vec1 != vec2 );
    }

    TEST( SmallVectorTests, Equality_EmptyVectors )
    {
        SmallVector<int, 4> vec1;
        SmallVector<int, 4> vec2;

        EXPECT_TRUE( vec1 == vec2 );
        EXPECT_FALSE( vec1 != vec2 );
    }

    //=====================================================================
    // Move semantics tests
    //=====================================================================

    TEST( SmallVectorTests, MoveConstructor_StackStorage )
    {
        SmallVector<int, 4> vec1;
        vec1.push_back( 10 );
        vec1.push_back( 20 );
        vec1.push_back( 30 );

        SmallVector<int, 4> vec2( std::move( vec1 ) );

        EXPECT_EQ( vec2.size(), 3 );
        EXPECT_EQ( vec2[0], 10 );
        EXPECT_EQ( vec2[1], 20 );
        EXPECT_EQ( vec2[2], 30 );

        // Source should be empty
        EXPECT_EQ( vec1.size(), 0 );
        EXPECT_TRUE( vec1.isEmpty() );
    }

    TEST( SmallVectorTests, MoveConstructor_HeapStorage )
    {
        SmallVector<int, 2> vec1;
        vec1.push_back( 1 );
        vec1.push_back( 2 );
        vec1.push_back( 3 ); // Heap

        SmallVector<int, 2> vec2( std::move( vec1 ) );

        EXPECT_EQ( vec2.size(), 3 );
        EXPECT_EQ( vec2[0], 1 );
        EXPECT_EQ( vec2[2], 3 );

        EXPECT_EQ( vec1.size(), 0 );
        EXPECT_TRUE( vec1.isEmpty() );
    }

    TEST( SmallVectorTests, MoveAssignment_StackToStack )
    {
        SmallVector<int, 4> vec1;
        vec1.push_back( 10 );
        vec1.push_back( 20 );

        SmallVector<int, 4> vec2;
        vec2.push_back( 99 );

        vec2 = std::move( vec1 );

        EXPECT_EQ( vec2.size(), 2 );
        EXPECT_EQ( vec2[0], 10 );
        EXPECT_EQ( vec2[1], 20 );

        EXPECT_EQ( vec1.size(), 0 );
        EXPECT_TRUE( vec1.isEmpty() );
    }

    TEST( SmallVectorTests, MoveAssignment_HeapToHeap )
    {
        SmallVector<int, 2> vec1;
        vec1.push_back( 1 );
        vec1.push_back( 2 );
        vec1.push_back( 3 );

        SmallVector<int, 2> vec2;
        vec2.push_back( 7 );
        vec2.push_back( 8 );
        vec2.push_back( 9 );

        vec2 = std::move( vec1 );

        EXPECT_EQ( vec2.size(), 3 );
        EXPECT_EQ( vec2[0], 1 );

        EXPECT_EQ( vec1.size(), 0 );
        EXPECT_TRUE( vec1.isEmpty() );
    }

    //=====================================================================
    // Edge cases and stress tests
    //=====================================================================

    TEST( SmallVectorTests, EdgeCase_SingleElement )
    {
        SmallVector<int, 1> vec;

        vec.push_back( 42 );

        EXPECT_EQ( vec.size(), 1 );
        EXPECT_EQ( vec.capacity(), 1 );
        EXPECT_EQ( vec[0], 42 );

        // Transition to heap
        vec.push_back( 43 );

        EXPECT_EQ( vec.size(), 2 );
        EXPECT_GT( vec.capacity(), 1 );
        EXPECT_EQ( vec[0], 42 );
        EXPECT_EQ( vec[1], 43 );
    }

    TEST( SmallVectorTests, EdgeCase_LargeStackCapacity )
    {
        SmallVector<int, 128> vec;

        for ( int i = 0; i < 100; ++i )
        {
            vec.push_back( i );
        }

        EXPECT_EQ( vec.size(), 100 );
        EXPECT_EQ( vec.capacity(), 128 ); // Still on stack

        for ( int i = 0; i < 100; ++i )
        {
            EXPECT_EQ( vec[i], i );
        }
    }

    TEST( SmallVectorTests, EdgeCase_NonTrivialType )
    {
        struct NonTrivial
        {
            std::string data;
            int value;

            NonTrivial( std::string d, int v )
                : data( std::move( d ) ), value( v )
            {
            }
        };

        SmallVector<NonTrivial, 4> vec;

        vec.emplace_back( "test", 42 );
        vec.emplace_back( "hello", 100 );

        EXPECT_EQ( vec.size(), 2 );
        EXPECT_EQ( vec[0].data, "test" );
        EXPECT_EQ( vec[0].value, 42 );
        EXPECT_EQ( vec[1].data, "hello" );
        EXPECT_EQ( vec[1].value, 100 );
    }

    TEST( SmallVectorTests, Stress_ManyElements )
    {
        SmallVector<int, 8> vec;

        // Add many elements to force heap allocation and growth
        for ( int i = 0; i < 1000; ++i )
        {
            vec.push_back( i );
        }

        EXPECT_EQ( vec.size(), 1000 );
        EXPECT_GT( vec.capacity(), 8 );

        // Verify all elements
        for ( int i = 0; i < 1000; ++i )
        {
            EXPECT_EQ( vec[i], i );
        }
    }

    TEST( SmallVectorTests, Stress_PushPopPattern )
    {
        SmallVector<int, 4> vec;

        // Repeatedly push and pop
        for ( int cycle = 0; cycle < 100; ++cycle )
        {
            vec.push_back( cycle );
            vec.push_back( cycle + 1 );
            vec.push_back( cycle + 2 );

            EXPECT_EQ( vec.size(), 3 );

            vec.pop_back();

            EXPECT_EQ( vec.size(), 2 );

            vec.clear();

            EXPECT_TRUE( vec.isEmpty() );
        }
    }

    TEST( SmallVectorTests, Stress_StringOperations )
    {
        SmallVector<std::string, 4> vec;

        for ( int i = 0; i < 100; ++i )
        {
            vec.push_back( "string_" + std::to_string( i ) );
        }

        EXPECT_EQ( vec.size(), 100 );

        for ( int i = 0; i < 100; ++i )
        {
            EXPECT_EQ( vec[i], "string_" + std::to_string( i ) );
        }

        vec.clear();
        EXPECT_TRUE( vec.isEmpty() );
    }

    //=====================================================================
    // Type traits verification
    //=====================================================================

    TEST( SmallVectorTests, TypeTraits_TrivialType )
    {
        SmallVector<int, 4> vec;

        // Should use memcpy for trivial types
        vec.push_back( 1 );
        vec.push_back( 2 );

        SmallVector<int, 4> vec2( std::move( vec ) );

        EXPECT_EQ( vec2.size(), 2 );
        EXPECT_EQ( vec2[0], 1 );
        EXPECT_EQ( vec2[1], 2 );
    }

    TEST( SmallVectorTests, TypeTraits_ComplexType )
    {
        // Verify proper construction/destruction for complex types
        SmallVector<std::pair<int, std::string>, 2> vec;

        vec.emplace_back( 1, "one" );
        vec.emplace_back( 2, "two" );
        vec.emplace_back( 3, "three" ); // Heap transition

        EXPECT_EQ( vec.size(), 3 );
        EXPECT_EQ( vec[0].second, "one" );
        EXPECT_EQ( vec[2].second, "three" );
    }

    //=====================================================================
    // Initializer list constructor tests
    //=====================================================================

    TEST( SmallVectorTests, InitializerList_StackStorage )
    {
        SmallVector<int, 8> vec{ 1, 2, 3, 4 };

        EXPECT_EQ( vec.size(), 4 );
        EXPECT_EQ( vec.capacity(), 8 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[1], 2 );
        EXPECT_EQ( vec[2], 3 );
        EXPECT_EQ( vec[3], 4 );
    }

    TEST( SmallVectorTests, InitializerList_HeapStorage )
    {
        SmallVector<int, 4> vec{ 1, 2, 3, 4, 5, 6, 7, 8 };

        EXPECT_EQ( vec.size(), 8 );
        EXPECT_GT( vec.capacity(), 4 );
        for ( int i = 0; i < 8; ++i )
        {
            EXPECT_EQ( vec[i], i + 1 );
        }
    }

    TEST( SmallVectorTests, InitializerList_StringType )
    {
        SmallVector<std::string, 4> vec{ "hello", "world", "test" };

        EXPECT_EQ( vec.size(), 3 );
        EXPECT_EQ( vec[0], "hello" );
        EXPECT_EQ( vec[1], "world" );
        EXPECT_EQ( vec[2], "test" );
    }

    TEST( SmallVectorTests, InitializerList_Empty )
    {
        SmallVector<int, 4> vec{};

        EXPECT_EQ( vec.size(), 0 );
        EXPECT_TRUE( vec.isEmpty() );
    }

    //=====================================================================
    // Copy constructor and assignment tests
    //=====================================================================

    TEST( SmallVectorTests, CopyConstructor_StackStorage )
    {
        SmallVector<int, 4> vec1;
        vec1.push_back( 10 );
        vec1.push_back( 20 );
        vec1.push_back( 30 );

        SmallVector<int, 4> vec2( vec1 );

        EXPECT_EQ( vec2.size(), 3 );
        EXPECT_EQ( vec2[0], 10 );
        EXPECT_EQ( vec2[1], 20 );
        EXPECT_EQ( vec2[2], 30 );

        // Original should be unchanged
        EXPECT_EQ( vec1.size(), 3 );
        EXPECT_EQ( vec1[0], 10 );

        // Modify copy shouldn't affect original
        vec2[0] = 99;
        EXPECT_EQ( vec1[0], 10 );
        EXPECT_EQ( vec2[0], 99 );
    }

    TEST( SmallVectorTests, CopyConstructor_HeapStorage )
    {
        SmallVector<int, 2> vec1;
        vec1.push_back( 1 );
        vec1.push_back( 2 );
        vec1.push_back( 3 );
        vec1.push_back( 4 );

        SmallVector<int, 2> vec2( vec1 );

        EXPECT_EQ( vec2.size(), 4 );
        for ( int i = 0; i < 4; ++i )
        {
            EXPECT_EQ( vec2[i], i + 1 );
        }
    }

    TEST( SmallVectorTests, CopyConstructor_StringType )
    {
        SmallVector<std::string, 4> vec1;
        vec1.push_back( "test" );
        vec1.push_back( "copy" );

        SmallVector<std::string, 4> vec2( vec1 );

        EXPECT_EQ( vec2.size(), 2 );
        EXPECT_EQ( vec2[0], "test" );
        EXPECT_EQ( vec2[1], "copy" );
    }

    TEST( SmallVectorTests, CopyAssignment_StackToStack )
    {
        SmallVector<int, 4> vec1;
        vec1.push_back( 10 );
        vec1.push_back( 20 );

        SmallVector<int, 4> vec2;
        vec2.push_back( 99 );

        vec2 = vec1;

        EXPECT_EQ( vec2.size(), 2 );
        EXPECT_EQ( vec2[0], 10 );
        EXPECT_EQ( vec2[1], 20 );

        // Original unchanged
        EXPECT_EQ( vec1.size(), 2 );
    }

    TEST( SmallVectorTests, CopyAssignment_HeapToHeap )
    {
        SmallVector<int, 2> vec1;
        vec1.push_back( 1 );
        vec1.push_back( 2 );
        vec1.push_back( 3 );

        SmallVector<int, 2> vec2;
        vec2.push_back( 7 );
        vec2.push_back( 8 );
        vec2.push_back( 9 );

        vec2 = vec1;

        EXPECT_EQ( vec2.size(), 3 );
        EXPECT_EQ( vec2[0], 1 );
        EXPECT_EQ( vec2[1], 2 );
        EXPECT_EQ( vec2[2], 3 );
    }

    TEST( SmallVectorTests, CopyAssignment_SelfAssignment )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );

        vec = vec;

        EXPECT_EQ( vec.size(), 2 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[1], 2 );
    }

    //=====================================================================
    // at() method tests - bounds checking
    //=====================================================================

    TEST( SmallVectorTests, At_ValidAccess )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 10 );
        vec.push_back( 20 );
        vec.push_back( 30 );

        EXPECT_EQ( vec.at( 0 ), 10 );
        EXPECT_EQ( vec.at( 1 ), 20 );
        EXPECT_EQ( vec.at( 2 ), 30 );

        vec.at( 1 ) = 99;
        EXPECT_EQ( vec.at( 1 ), 99 );
    }

    TEST( SmallVectorTests, At_ConstAccess )
    {
        SmallVector<std::string, 4> vec;
        vec.push_back( "test" );

        const auto& cvec = vec;
        EXPECT_EQ( cvec.at( 0 ), "test" );
    }

    TEST( SmallVectorTests, At_ThrowsOutOfRange )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );

        EXPECT_THROW( vec.at( 2 ), std::out_of_range );
        EXPECT_THROW( vec.at( 10 ), std::out_of_range );
    }

    TEST( SmallVectorTests, At_ThrowsOutOfRange_Const )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );

        const auto& cvec = vec;
        EXPECT_THROW( cvec.at( 1 ), std::out_of_range );
    }

    TEST( SmallVectorTests, At_EmptyVector )
    {
        SmallVector<int, 4> vec;

        EXPECT_THROW( vec.at( 0 ), std::out_of_range );
    }

    //=====================================================================
    // resize() method tests
    //=====================================================================

    TEST( SmallVectorTests, Resize_ShrinkStack )
    {
        SmallVector<int, 8> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 );
        vec.push_back( 4 );

        vec.resize( 2 );

        EXPECT_EQ( vec.size(), 2 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[1], 2 );
    }

    TEST( SmallVectorTests, Resize_GrowStack_DefaultConstruct )
    {
        SmallVector<int, 8> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );

        vec.resize( 5 );

        EXPECT_EQ( vec.size(), 5 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[1], 2 );
        EXPECT_EQ( vec[2], 0 );
        EXPECT_EQ( vec[3], 0 );
        EXPECT_EQ( vec[4], 0 );
    }

    TEST( SmallVectorTests, Resize_GrowStack_WithValue )
    {
        SmallVector<int, 8> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );

        vec.resize( 5, 42 );

        EXPECT_EQ( vec.size(), 5 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[1], 2 );
        EXPECT_EQ( vec[2], 42 );
        EXPECT_EQ( vec[3], 42 );
        EXPECT_EQ( vec[4], 42 );
    }

    TEST( SmallVectorTests, Resize_GrowToHeap )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );

        vec.resize( 10 );

        EXPECT_EQ( vec.size(), 10 );
        EXPECT_GT( vec.capacity(), 4 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[1], 2 );
        for ( int i = 2; i < 10; ++i )
        {
            EXPECT_EQ( vec[i], 0 );
        }
    }

    TEST( SmallVectorTests, Resize_GrowToHeap_WithValue )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );

        vec.resize( 10, 99 );

        EXPECT_EQ( vec.size(), 10 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[1], 2 );
        for ( int i = 2; i < 10; ++i )
        {
            EXPECT_EQ( vec[i], 99 );
        }
    }

    TEST( SmallVectorTests, Resize_ShrinkHeap )
    {
        SmallVector<int, 2> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 );
        vec.push_back( 4 );
        vec.push_back( 5 );

        vec.resize( 3 );

        EXPECT_EQ( vec.size(), 3 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[1], 2 );
        EXPECT_EQ( vec[2], 3 );
    }

    TEST( SmallVectorTests, Resize_StringType )
    {
        SmallVector<std::string, 4> vec;
        vec.push_back( "one" );
        vec.push_back( "two" );

        vec.resize( 4, "test" );

        EXPECT_EQ( vec.size(), 4 );
        EXPECT_EQ( vec[0], "one" );
        EXPECT_EQ( vec[1], "two" );
        EXPECT_EQ( vec[2], "test" );
        EXPECT_EQ( vec[3], "test" );
    }

    TEST( SmallVectorTests, Resize_ToZero )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );
        vec.push_back( 3 );

        vec.resize( 0 );

        EXPECT_EQ( vec.size(), 0 );
        EXPECT_TRUE( vec.isEmpty() );
    }

    TEST( SmallVectorTests, Resize_NoChange )
    {
        SmallVector<int, 4> vec;
        vec.push_back( 1 );
        vec.push_back( 2 );

        vec.resize( 2 );

        EXPECT_EQ( vec.size(), 2 );
        EXPECT_EQ( vec[0], 1 );
        EXPECT_EQ( vec[1], 2 );
    }

} // namespace nfx::containers::test
