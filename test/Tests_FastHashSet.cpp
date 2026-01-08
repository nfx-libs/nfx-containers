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
 * @file Tests_FastHashSet.cpp
 * @brief Unit tests for FastHashSet (Robin Hood hashing set container)
 */

#include <gtest/gtest.h>

#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nfx/containers/FastHashSet.h>

namespace nfx::containers::test
{
    using namespace nfx::hashing;

    //=====================================================================
    // Initializer list constructor tests
    //=====================================================================

    TEST( FastHashSetTests, InitializerList_BasicConstruction )
    {
        FastHashSet<std::string> set = { "apple", "banana", "cherry" };

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( "apple" ) );
        EXPECT_TRUE( set.contains( "banana" ) );
        EXPECT_TRUE( set.contains( "cherry" ) );
        EXPECT_FALSE( set.contains( "mango" ) );
    }

    TEST( FastHashSetTests, InitializerList_EmptyList )
    {
        FastHashSet<std::string> set = {};

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
    }

    TEST( FastHashSetTests, InitializerList_SingleElement )
    {
        FastHashSet<std::string> set = { "single" };

        EXPECT_EQ( set.size(), 1 );
        EXPECT_TRUE( set.contains( "single" ) );
    }

    TEST( FastHashSetTests, InitializerList_DuplicateKeys )
    {
        // Duplicate keys should be inserted only once (set semantics)
        FastHashSet<std::string> set = { "apple", "banana", "apple", "cherry", "banana" };

        EXPECT_EQ( set.size(), 3 ); // Only unique keys
        EXPECT_TRUE( set.contains( "apple" ) );
        EXPECT_TRUE( set.contains( "banana" ) );
        EXPECT_TRUE( set.contains( "cherry" ) );
    }

    TEST( FastHashSetTests, InitializerList_IntKeys )
    {
        FastHashSet<int> set = { 1, 2, 3, 4, 5 };

        EXPECT_EQ( set.size(), 5 );
        EXPECT_TRUE( set.contains( 1 ) );
        EXPECT_TRUE( set.contains( 3 ) );
        EXPECT_TRUE( set.contains( 5 ) );
        EXPECT_FALSE( set.contains( 6 ) );
    }

    //=====================================================================
    // Range constructor tests
    //=====================================================================

    TEST( FastHashSetTests, RangeConstructor_FromVector )
    {
        std::vector<std::string> data = { "apple", "banana", "cherry", "date" };

        FastHashSet<std::string> set( data.begin(), data.end() );

        EXPECT_EQ( set.size(), 4 );
        EXPECT_TRUE( set.contains( "apple" ) );
        EXPECT_TRUE( set.contains( "banana" ) );
        EXPECT_TRUE( set.contains( "cherry" ) );
        EXPECT_TRUE( set.contains( "date" ) );
    }

    TEST( FastHashSetTests, RangeConstructor_FromAnotherSet )
    {
        FastHashSet<int> original;
        original.insert( 10 );
        original.insert( 20 );
        original.insert( 30 );

        FastHashSet<int> copy( original.begin(), original.end() );

        EXPECT_EQ( copy.size(), 3 );
        EXPECT_TRUE( copy.contains( 10 ) );
        EXPECT_TRUE( copy.contains( 20 ) );
        EXPECT_TRUE( copy.contains( 30 ) );
    }

    TEST( FastHashSetTests, RangeConstructor_EmptyRange )
    {
        std::vector<int> data;
        FastHashSet<int> set( data.begin(), data.end() );

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
    }

    //=====================================================================
    // String keys - Heterogeneous lookup tests
    //=====================================================================

    TEST( FastHashSetTests, StringKeys_BasicInsertAndFind )
    {
        FastHashSet<std::string> set;

        // Insert new keys
        EXPECT_TRUE( set.insert( "apple" ) );
        EXPECT_TRUE( set.insert( "banana" ) );
        EXPECT_TRUE( set.insert( "cherry" ) );

        // Duplicate inserts should fail
        EXPECT_FALSE( set.insert( "apple" ) );
        EXPECT_FALSE( set.insert( "banana" ) );

        // Verify size
        EXPECT_EQ( set.size(), 3 );

        // Find existing keys
        auto* found1 = set.find( "apple" );
        EXPECT_NE( found1, nullptr );
        EXPECT_EQ( *found1, "apple" );

        auto* found2 = set.find( "banana" );
        EXPECT_NE( found2, nullptr );
        EXPECT_EQ( *found2, "banana" );

        // Find non-existent key
        auto* notFound = set.find( "mango" );
        EXPECT_EQ( notFound, nullptr );
    }

    TEST( FastHashSetTests, StringKeys_Contains )
    {
        FastHashSet<std::string> set;

        set.insert( "red" );
        set.insert( "green" );
        set.insert( "blue" );

        EXPECT_TRUE( set.contains( "red" ) );
        EXPECT_TRUE( set.contains( "green" ) );
        EXPECT_TRUE( set.contains( "blue" ) );
        EXPECT_FALSE( set.contains( "yellow" ) );
        EXPECT_FALSE( set.contains( "" ) );
    }

    TEST( FastHashSetTests, StringKeys_HeterogeneousLookup )
    {
        FastHashSet<std::string> set;

        set.insert( "hello" );
        set.insert( "world" );

        // Find with string_view (no temporary allocation)
        std::string_view sv = "hello";
        EXPECT_TRUE( set.contains( sv ) );
        auto* found = set.find( sv );
        EXPECT_NE( found, nullptr );
        EXPECT_EQ( *found, "hello" );

        // Find with const char*
        EXPECT_TRUE( set.contains( "world" ) );
        auto* found2 = set.find( "world" );
        EXPECT_NE( found2, nullptr );
        EXPECT_EQ( *found2, "world" );
    }

    TEST( FastHashSetTests, StringKeys_Erase )
    {
        FastHashSet<std::string> set;

        set.insert( "first" );
        set.insert( "second" );
        set.insert( "third" );

        EXPECT_EQ( set.size(), 3 );

        // Erase existing key
        EXPECT_TRUE( set.erase( "second" ) );
        EXPECT_EQ( set.size(), 2 );
        EXPECT_FALSE( set.contains( "second" ) );

        // Erase non-existent key
        EXPECT_FALSE( set.erase( "nonexistent" ) );
        EXPECT_EQ( set.size(), 2 );

        // Verify remaining keys
        EXPECT_TRUE( set.contains( "first" ) );
        EXPECT_TRUE( set.contains( "third" ) );
    }

    //=====================================================================
    // at() - Checked element access tests
    //=====================================================================

    TEST( FastHashSetTests, At_BasicAccess )
    {
        FastHashSet<std::string> set;

        set.insert( "key1" );
        set.insert( "key2" );
        set.insert( "key3" );

        EXPECT_EQ( set.at( "key1" ), "key1" );
        EXPECT_EQ( set.at( "key2" ), "key2" );
        EXPECT_EQ( set.at( "key3" ), "key3" );
    }

    TEST( FastHashSetTests, At_ThrowsOutOfRange )
    {
        FastHashSet<std::string> set;

        set.insert( "existing" );

        EXPECT_THROW( set.at( "non_existent" ), std::out_of_range );
        EXPECT_THROW( set.at( "missing_key" ), std::out_of_range );
    }

    TEST( FastHashSetTests, At_HeterogeneousLookup )
    {
        FastHashSet<std::string> set;

        set.insert( "hetero_key" );

        // string_view lookup
        std::string_view sv = "hetero_key";
        EXPECT_EQ( set.at( sv ), "hetero_key" );

        // const char* lookup
        EXPECT_EQ( set.at( "hetero_key" ), "hetero_key" );

        // Non-existent with string_view
        std::string_view missing_sv = "missing";
        EXPECT_THROW( set.at( missing_sv ), std::out_of_range );
    }

    TEST( FastHashSetTests, At_ReturnsStoredKey )
    {
        FastHashSet<int> set;

        set.insert( 42 );
        set.insert( 100 );
        set.insert( 999 );

        // For sets, at() returns the stored key itself
        EXPECT_EQ( set.at( 42 ), 42 );
        EXPECT_EQ( set.at( 100 ), 100 );
        EXPECT_EQ( set.at( 999 ), 999 );

        EXPECT_THROW( set.at( 500 ), std::out_of_range );
    }

    //=====================================================================
    // Integer keys - Multiplicative hashing tests
    //=====================================================================

    TEST( FastHashSetTests, IntegerKeys_BasicOperations )
    {
        FastHashSet<int> set;

        // Insert integers
        EXPECT_TRUE( set.insert( 42 ) );
        EXPECT_TRUE( set.insert( 100 ) );
        EXPECT_TRUE( set.insert( -5 ) );

        // Duplicate insert
        EXPECT_FALSE( set.insert( 42 ) );

        EXPECT_EQ( set.size(), 3 );

        // Find
        EXPECT_TRUE( set.contains( 42 ) );
        EXPECT_TRUE( set.contains( 100 ) );
        EXPECT_TRUE( set.contains( -5 ) );
        EXPECT_FALSE( set.contains( 999 ) );

        // Erase
        EXPECT_TRUE( set.erase( 100 ) );
        EXPECT_FALSE( set.contains( 100 ) );
        EXPECT_EQ( set.size(), 2 );
    }

    TEST( FastHashSetTests, UInt64Keys_BasicOperations )
    {
        FastHashSet<uint64_t> set;

        // Insert large numbers
        EXPECT_TRUE( set.insert( 0xFFFFFFFFULL ) );
        EXPECT_TRUE( set.insert( 0x123456789ABCDEFULL ) );
        EXPECT_TRUE( set.insert( 0ULL ) );

        EXPECT_EQ( set.size(), 3 );

        // Find
        EXPECT_TRUE( set.contains( 0xFFFFFFFFULL ) );
        EXPECT_TRUE( set.contains( 0x123456789ABCDEFULL ) );
        EXPECT_TRUE( set.contains( 0ULL ) );
        EXPECT_FALSE( set.contains( 1ULL ) );
    }

    //=====================================================================
    // Enum keys - Underlying type conversion tests
    //=====================================================================

    enum class Color
    {
        Red,
        Green,
        Blue,
        Yellow
    };

    TEST( FastHashSetTests, EnumKeys_BasicOperations )
    {
        FastHashSet<Color> set;

        EXPECT_TRUE( set.insert( Color::Red ) );
        EXPECT_TRUE( set.insert( Color::Green ) );
        EXPECT_TRUE( set.insert( Color::Blue ) );

        // Duplicate
        EXPECT_FALSE( set.insert( Color::Red ) );

        EXPECT_EQ( set.size(), 3 );

        // Find
        EXPECT_TRUE( set.contains( Color::Red ) );
        EXPECT_TRUE( set.contains( Color::Green ) );
        EXPECT_TRUE( set.contains( Color::Blue ) );
        EXPECT_FALSE( set.contains( Color::Yellow ) );
    }

    //=====================================================================
    // Floating-point keys - Normalization tests
    //=====================================================================

    TEST( FastHashSetTests, FloatKeys_BasicOperations )
    {
        FastHashSet<double> set;

        EXPECT_TRUE( set.insert( 3.14159 ) );
        EXPECT_TRUE( set.insert( 2.71828 ) );
        EXPECT_TRUE( set.insert( 1.41421 ) );

        // Duplicate insert
        EXPECT_FALSE( set.insert( 3.14159 ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( 3.14159 ) );
        EXPECT_TRUE( set.contains( 2.71828 ) );
        EXPECT_TRUE( set.contains( 1.41421 ) );
        EXPECT_FALSE( set.contains( 0.0 ) );
    }

    TEST( FastHashSetTests, FloatKeys_ZeroNormalization )
    {
        FastHashSet<double> set;

        // Insert positive zero
        EXPECT_TRUE( set.insert( 0.0 ) );

        // Negative zero should be normalized to positive zero
        EXPECT_FALSE( set.insert( -0.0 ) );
        EXPECT_EQ( set.size(), 1 );

        EXPECT_TRUE( set.contains( 0.0 ) );
        EXPECT_TRUE( set.contains( -0.0 ) );
    }

    //=====================================================================
    // Pointer keys - Address hashing tests
    //=====================================================================

    TEST( FastHashSetTests, PointerKeys_BasicOperations )
    {
        int a = 1, b = 2, c = 3;

        FastHashSet<int*> set;

        EXPECT_TRUE( set.insert( &a ) );
        EXPECT_TRUE( set.insert( &b ) );
        EXPECT_TRUE( set.insert( &c ) );

        // Duplicate pointer
        EXPECT_FALSE( set.insert( &a ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( &a ) );
        EXPECT_TRUE( set.contains( &b ) );
        EXPECT_TRUE( set.contains( &c ) );

        int* null_ptr = nullptr;
        EXPECT_FALSE( set.contains( null_ptr ) );
    }

    //=====================================================================
    // Pair keys - Hash combining tests
    //=====================================================================

    TEST( FastHashSetTests, PairKeys_IntPairs )
    {
        using Key = std::pair<int, int>;

        FastHashSet<Key> set;

        EXPECT_TRUE( set.insert( { 1, 2 } ) );
        EXPECT_TRUE( set.insert( { 3, 4 } ) );
        EXPECT_TRUE( set.insert( { 5, 6 } ) );

        // Duplicate
        EXPECT_FALSE( set.insert( { 1, 2 } ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( Key{ 1, 2 } ) );
        EXPECT_TRUE( set.contains( Key{ 3, 4 } ) );
        EXPECT_TRUE( set.contains( Key{ 5, 6 } ) );
        EXPECT_FALSE( set.contains( Key{ 7, 8 } ) );
    }

    TEST( FastHashSetTests, PairKeys_StringPairs )
    {
        using Key = std::pair<std::string, std::string>;

        FastHashSet<Key> set;

        EXPECT_TRUE( set.insert( { "hello", "world" } ) );
        EXPECT_TRUE( set.insert( { "foo", "bar" } ) );

        EXPECT_EQ( set.size(), 2 );
        EXPECT_TRUE( set.contains( Key{ "hello", "world" } ) );
        EXPECT_TRUE( set.contains( Key{ "foo", "bar" } ) );
        EXPECT_FALSE( set.contains( Key{ "missing", "key" } ) );
    }

    //=====================================================================
    // Erase operations
    //=====================================================================

    TEST( FastHashSetTests, Erase_BasicOperation )
    {
        FastHashSet<std::string> set;

        set.insert( "first" );
        set.insert( "second" );
        set.insert( "third" );

        EXPECT_EQ( set.size(), 3 );

        // Erase existing key
        EXPECT_TRUE( set.erase( "second" ) );
        EXPECT_EQ( set.size(), 2 );
        EXPECT_FALSE( set.contains( "second" ) );

        // Verify remaining keys
        EXPECT_TRUE( set.contains( "first" ) );
        EXPECT_TRUE( set.contains( "third" ) );
    }

    TEST( FastHashSetTests, Erase_NonExistent )
    {
        FastHashSet<int> set;

        set.insert( 10 );
        set.insert( 20 );

        // Erase non-existent key
        EXPECT_FALSE( set.erase( 999 ) );
        EXPECT_EQ( set.size(), 2 );
    }

    TEST( FastHashSetTests, Erase_HeterogeneousKey )
    {
        FastHashSet<std::string> set;

        set.insert( "heterogeneous" );

        // Erase with string_view
        std::string_view sv = "heterogeneous";
        EXPECT_TRUE( set.erase( sv ) );
        EXPECT_EQ( set.size(), 0 );
    }

    //=====================================================================
    // Capacity and memory management
    //=====================================================================

    TEST( FastHashSetTests, Reserve_PreallocateCapacity )
    {
        FastHashSet<int> set;

        // Reserve space for 100 elements
        set.reserve( 100 );
        EXPECT_GE( set.capacity(), 100 );

        // Insert elements (should not trigger resize)
        for ( int i = 0; i < 50; ++i )
        {
            set.insert( i );
        }

        EXPECT_EQ( set.size(), 50 );
        EXPECT_GE( set.capacity(), 100 );
    }

    TEST( FastHashSetTests, AutomaticResize_LoadFactorTriggered )
    {
        FastHashSet<int> set;

        const size_t initialCapacity = set.capacity();

        // Insert enough elements to trigger resize (>75% load factor)
        const size_t numElements = static_cast<size_t>( initialCapacity * 0.8 );
        for ( size_t i = 0; i < numElements; ++i )
        {
            set.insert( static_cast<int>( i ) );
        }

        // Capacity should have increased
        EXPECT_GT( set.capacity(), initialCapacity );
        EXPECT_EQ( set.size(), numElements );

        // Verify all elements still present
        for ( size_t i = 0; i < numElements; ++i )
        {
            EXPECT_TRUE( set.contains( static_cast<int>( i ) ) );
        }
    }

    //=====================================================================
    // Iterator tests
    //=====================================================================

    TEST( FastHashSetTests, Iterator_BasicIteration )
    {
        FastHashSet<std::string> set;

        set.insert( "alpha" );
        set.insert( "beta" );
        set.insert( "gamma" );

        std::vector<std::string> keys;
        for ( const auto& key : set )
        {
            keys.push_back( key );
        }

        // Should have all 3 keys (order may vary)
        EXPECT_EQ( keys.size(), 3 );
        EXPECT_NE( std::find( keys.begin(), keys.end(), "alpha" ), keys.end() );
        EXPECT_NE( std::find( keys.begin(), keys.end(), "beta" ), keys.end() );
        EXPECT_NE( std::find( keys.begin(), keys.end(), "gamma" ), keys.end() );
    }

    TEST( FastHashSetTests, Iterator_EmptySet )
    {
        FastHashSet<int> set;

        // Iterating over empty set should be safe
        int count = 0;
        for ( [[maybe_unused]] const auto& key : set )
        {
            ++count;
        }

        EXPECT_EQ( count, 0 );
    }

    TEST( FastHashSetTests, ConstIterator_BasicIteration )
    {
        FastHashSet<int> set;
        set.insert( 1 );
        set.insert( 2 );
        set.insert( 3 );

        const auto& constSet = set;

        std::vector<int> keys;
        for ( const auto& key : constSet )
        {
            keys.push_back( key );
        }

        EXPECT_EQ( keys.size(), 3 );
        EXPECT_NE( std::find( keys.begin(), keys.end(), 1 ), keys.end() );
        EXPECT_NE( std::find( keys.begin(), keys.end(), 2 ), keys.end() );
        EXPECT_NE( std::find( keys.begin(), keys.end(), 3 ), keys.end() );
    }

    //=====================================================================
    // Comparison operator tests
    //=====================================================================

    TEST( FastHashSetTests, Equality_IdenticalSets )
    {
        FastHashSet<std::string> set1;
        FastHashSet<std::string> set2;

        set1.insert( "apple" );
        set1.insert( "banana" );

        set2.insert( "banana" );
        set2.insert( "apple" );

        EXPECT_TRUE( set1 == set2 );
    }

    TEST( FastHashSetTests, Equality_DifferentSets )
    {
        FastHashSet<std::string> set1;
        FastHashSet<std::string> set2;

        set1.insert( "apple" );
        set1.insert( "banana" );

        set2.insert( "apple" );
        set2.insert( "cherry" );

        EXPECT_FALSE( set1 == set2 );
    }

    TEST( FastHashSetTests, Equality_DifferentSizes )
    {
        FastHashSet<std::string> set1;
        FastHashSet<std::string> set2;

        set1.insert( "apple" );
        set1.insert( "banana" );

        set2.insert( "apple" );

        EXPECT_FALSE( set1 == set2 );
    }

    //=====================================================================
    // swap() - Container swap tests
    //=====================================================================

    TEST( FastHashSetTests, Swap_BasicOperation )
    {
        FastHashSet<std::string> set1;
        FastHashSet<std::string> set2;

        set1.insert( "alpha" );
        set1.insert( "beta" );

        set2.insert( "gamma" );
        set2.insert( "delta" );
        set2.insert( "epsilon" );

        set1.swap( set2 );

        EXPECT_EQ( set1.size(), 3 );
        EXPECT_TRUE( set1.contains( "gamma" ) );
        EXPECT_TRUE( set1.contains( "delta" ) );
        EXPECT_TRUE( set1.contains( "epsilon" ) );

        EXPECT_EQ( set2.size(), 2 );
        EXPECT_TRUE( set2.contains( "alpha" ) );
        EXPECT_TRUE( set2.contains( "beta" ) );
    }

    TEST( FastHashSetTests, Swap_EmptyWithFilled )
    {
        FastHashSet<int> set1;
        FastHashSet<int> set2;

        set2.insert( 10 );
        set2.insert( 20 );
        set2.insert( 30 );

        set1.swap( set2 );

        EXPECT_EQ( set1.size(), 3 );
        EXPECT_TRUE( set1.contains( 10 ) );
        EXPECT_TRUE( set1.contains( 20 ) );
        EXPECT_TRUE( set1.contains( 30 ) );

        EXPECT_EQ( set2.size(), 0 );
        EXPECT_TRUE( set2.isEmpty() );
    }

    TEST( FastHashSetTests, Swap_BothEmpty )
    {
        FastHashSet<std::string> set1;
        FastHashSet<std::string> set2;

        set1.swap( set2 );

        EXPECT_TRUE( set1.isEmpty() );
        EXPECT_TRUE( set2.isEmpty() );
    }

    TEST( FastHashSetTests, Swap_DifferentCapacities )
    {
        FastHashSet<int> set1;
        FastHashSet<int> set2;

        set1.reserve( 100 );
        set1.insert( 42 );

        set2.reserve( 10 );
        set2.insert( 99 );

        size_t cap1_before = set1.capacity();
        size_t cap2_before = set2.capacity();

        set1.swap( set2 );

        // Capacities should be swapped
        EXPECT_EQ( set1.capacity(), cap2_before );
        EXPECT_EQ( set2.capacity(), cap1_before );

        // Contents should be swapped
        EXPECT_TRUE( set1.contains( 99 ) );
        EXPECT_TRUE( set2.contains( 42 ) );
    }

    TEST( FastHashSetTests, Swap_StdSwapCompatibility )
    {
        FastHashSet<std::string> set1;
        FastHashSet<std::string> set2;

        set1.insert( "one" );
        set2.insert( "two" );

        // std::swap should use the member swap
        std::swap( set1, set2 );

        EXPECT_TRUE( set1.contains( "two" ) );
        EXPECT_TRUE( set2.contains( "one" ) );
    }

    //=====================================================================
    // Move semantics
    //=====================================================================

    TEST( FastHashSetTests, MoveSemantics_Insert )
    {
        FastHashSet<std::string> set;

        std::string key = "movable";
        EXPECT_TRUE( set.insert( std::move( key ) ) );

        // Original string should be moved-from
        EXPECT_TRUE( key.empty() || key == "movable" ); // Implementation-defined

        EXPECT_TRUE( set.contains( "movable" ) );
    }

    TEST( FastHashSetTests, MoveSemantics_Constructor )
    {
        FastHashSet<std::string> set1;
        set1.insert( "alpha" );
        set1.insert( "beta" );

        FastHashSet<std::string> set2( std::move( set1 ) );

        EXPECT_EQ( set2.size(), 2 );
        EXPECT_TRUE( set2.contains( "alpha" ) );
        EXPECT_TRUE( set2.contains( "beta" ) );

        // Verify source is moved-from (should be empty)
        EXPECT_EQ( set1.size(), 0 );
        EXPECT_TRUE( set1.isEmpty() );
    }

    TEST( FastHashSetTests, MoveSemantics_Assignment )
    {
        FastHashSet<std::string> set1;
        set1.insert( "alpha" );
        set1.insert( "beta" );

        FastHashSet<std::string> set2;
        set2.insert( "gamma" );

        set2 = std::move( set1 );

        EXPECT_EQ( set2.size(), 2 );
        EXPECT_TRUE( set2.contains( "alpha" ) );
        EXPECT_TRUE( set2.contains( "beta" ) );
        EXPECT_FALSE( set2.contains( "gamma" ) );

        // Verify source is moved-from (should be empty)
        EXPECT_EQ( set1.size(), 0 );
        EXPECT_TRUE( set1.isEmpty() );
    }

    //=====================================================================
    // Clear operation
    //=====================================================================

    TEST( FastHashSetTests, Clear_BasicOperation )
    {
        FastHashSet<std::string> set;

        set.insert( "one" );
        set.insert( "two" );
        set.insert( "three" );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_FALSE( set.isEmpty() );

        set.clear();

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
        EXPECT_FALSE( set.contains( "one" ) );
        EXPECT_FALSE( set.contains( "two" ) );
        EXPECT_FALSE( set.contains( "three" ) );

        // Can insert after clear
        EXPECT_TRUE( set.insert( "new" ) );
        EXPECT_EQ( set.size(), 1 );
        EXPECT_TRUE( set.contains( "new" ) );
    }

    //=====================================================================
    // Insert operations
    //=====================================================================

    TEST( FastHashSetTests, Insert_BasicOperation )
    {
        FastHashSet<std::string> set;

        // Insert constructs in-place
        EXPECT_TRUE( set.insert( "hello" ) );
        EXPECT_TRUE( set.insert( "world" ) );

        // Duplicate insert fails
        EXPECT_FALSE( set.insert( "hello" ) );

        EXPECT_EQ( set.size(), 2 );
        EXPECT_TRUE( set.contains( "hello" ) );
        EXPECT_TRUE( set.contains( "world" ) );
    }

    TEST( FastHashSetTests, Insert_Pairs )
    {
        using Key = std::pair<int, std::string>;
        FastHashSet<Key> set;

        // Insert pair with initialization
        EXPECT_TRUE( set.insert( Key{ 1, "one" } ) );
        EXPECT_TRUE( set.insert( Key{ 2, "two" } ) );

        EXPECT_EQ( set.size(), 2 );
        EXPECT_TRUE( set.contains( Key{ 1, "one" } ) );
        EXPECT_TRUE( set.contains( Key{ 2, "two" } ) );
    }

    //=====================================================================
    // Edge cases and stress tests
    //=====================================================================

    TEST( FastHashSetTests, EdgeCases_EmptySet )
    {
        FastHashSet<std::string> set;

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
        EXPECT_FALSE( set.contains( "anything" ) );
        EXPECT_EQ( set.find( "anything" ), nullptr );
    }

    TEST( FastHashSetTests, EdgeCases_EmptyStringKey )
    {
        FastHashSet<std::string> set;

        EXPECT_TRUE( set.insert( "" ) );
        EXPECT_TRUE( set.contains( "" ) );

        auto* found = set.find( "" );
        EXPECT_NE( found, nullptr );
        EXPECT_EQ( *found, "" );
    }

    TEST( FastHashSetTests, EdgeCases_DuplicateInserts )
    {
        FastHashSet<int> set;

        EXPECT_TRUE( set.insert( 42 ) );
        EXPECT_FALSE( set.insert( 42 ) );
        EXPECT_FALSE( set.insert( 42 ) );

        EXPECT_EQ( set.size(), 1 );
    }

    TEST( FastHashSetTests, EdgeCases_LargeNumberOfElements )
    {
        FastHashSet<int> set;

        const int numElements = 10000;
        for ( int i = 0; i < numElements; ++i )
        {
            EXPECT_TRUE( set.insert( i ) );
        }

        EXPECT_EQ( set.size(), numElements );

        // Verify all elements present
        for ( int i = 0; i < numElements; ++i )
        {
            EXPECT_TRUE( set.contains( i ) );
        }

        // Erase half
        for ( int i = 0; i < numElements / 2; ++i )
        {
            EXPECT_TRUE( set.erase( i ) );
        }

        EXPECT_EQ( set.size(), numElements / 2 );

        // Verify correct elements remain
        for ( int i = 0; i < numElements / 2; ++i )
        {
            EXPECT_FALSE( set.contains( i ) );
        }
        for ( int i = numElements / 2; i < numElements; ++i )
        {
            EXPECT_TRUE( set.contains( i ) );
        }
    }

    //=====================================================================
    // std::hash tests - Verify compatibility with standard library hashers
    //=====================================================================

    TEST( FastHashSetTests, StdHash_StringKeys )
    {
        FastHashSet<std::string, uint32_t, hashing::constants::FNV_OFFSET_BASIS_32, std::hash<std::string>> set;

        set.insert( "apple" );
        set.insert( "banana" );
        set.insert( "cherry" );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( "apple" ) );
        EXPECT_TRUE( set.contains( "banana" ) );
        EXPECT_TRUE( set.contains( "cherry" ) );
        EXPECT_FALSE( set.contains( "mango" ) );
    }

    TEST( FastHashSetTests, StdHash_IntegerKeys )
    {
        FastHashSet<int, uint32_t, hashing::constants::FNV_OFFSET_BASIS_32, std::hash<int>> set;

        set.insert( 1 );
        set.insert( 2 );
        set.insert( 3 );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( 1 ) );
        EXPECT_TRUE( set.contains( 2 ) );
        EXPECT_TRUE( set.contains( 3 ) );
    }

    TEST( FastHashSetTests, StdHash_EnumKeys )
    {
        enum class Priority
        {
            Low = 1,
            Medium = 2,
            High = 3
        };

        struct PriorityHash
        {
            size_t operator()( Priority p ) const
            {
                return std::hash<int>{}( static_cast<int>( p ) );
            }
        };

        FastHashSet<Priority, uint32_t, hashing::constants::FNV_OFFSET_BASIS_32, PriorityHash> set;

        set.insert( Priority::Low );
        set.insert( Priority::Medium );
        set.insert( Priority::High );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( Priority::Low ) );
        EXPECT_TRUE( set.contains( Priority::Medium ) );
        EXPECT_TRUE( set.contains( Priority::High ) );
    }

    //=====================================================================
    // Custom hasher tests - Verify custom hash functions work correctly
    //=====================================================================

    // Simple multiplicative hasher
    struct SimpleMultiplicativeHasher
    {
        uint32_t operator()( int key ) const { return static_cast<uint32_t>( key ) * 2654435761u; }
    };

    TEST( FastHashSetTests, CustomHasher_SimpleMultiplicative )
    {
        FastHashSet<int, uint32_t, hashing::constants::FNV_OFFSET_BASIS_32, SimpleMultiplicativeHasher> set;

        set.insert( 42 );
        set.insert( 100 );
        set.insert( 999 );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( 42 ) );
        EXPECT_TRUE( set.contains( 100 ) );
        EXPECT_TRUE( set.contains( 999 ) );
    }

    // Identity hasher (for testing - returns key as-is)
    struct Identity32Hasher
    {
        uint32_t operator()( int key ) const { return static_cast<uint32_t>( key ); }
    };

    TEST( FastHashSetTests, CustomHasher_Identity32 )
    {
        FastHashSet<int, uint32_t, hashing::constants::FNV_OFFSET_BASIS_32, Identity32Hasher> set;

        set.insert( 1 );
        set.insert( 2 );
        set.insert( 3 );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( 1 ) );
        EXPECT_TRUE( set.contains( 2 ) );
        EXPECT_TRUE( set.contains( 3 ) );
    }

    // XOR-shift hasher
    struct XorShiftHasher
    {
        uint32_t operator()( int key ) const
        {
            uint32_t x = static_cast<uint32_t>( key );
            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            return x;
        }
    };

    TEST( FastHashSetTests, CustomHasher_XorShift )
    {
        FastHashSet<int, uint32_t, hashing::constants::FNV_OFFSET_BASIS_32, XorShiftHasher> set;

        set.insert( 42 );
        set.insert( 100 );
        set.insert( 999 );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( 42 ) );
        EXPECT_TRUE( set.contains( 100 ) );
        EXPECT_TRUE( set.contains( 999 ) );
    }

    //=====================================================================
    // Hash collision tests (Robin Hood algorithm verification)
    //=====================================================================

    TEST( FastHashSetTests, HashCollisions_MultipleInserts )
    {
        FastHashSet<int> set;

        // Insert elements that may collide based on hash table size
        for ( int i = 0; i < 100; ++i )
        {
            EXPECT_TRUE( set.insert( i ) );
        }

        // Verify all are present
        for ( int i = 0; i < 100; ++i )
        {
            EXPECT_TRUE( set.contains( i ) );
        }
    }

    //=====================================================================
    // 64-bit hashing tests
    //=====================================================================

    TEST( FastHashSetTests, Hash64_StringKeys )
    {
        FastHashSet<std::string, uint64_t, hashing::constants::FNV_OFFSET_BASIS_64> set;

        set.insert( "hash64_test_alpha" );
        set.insert( "hash64_test_beta" );
        set.insert( "hash64_test_gamma" );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( "hash64_test_alpha" ) );
        EXPECT_TRUE( set.contains( "hash64_test_beta" ) );
        EXPECT_TRUE( set.contains( "hash64_test_gamma" ) );
    }

    TEST( FastHashSetTests, Hash64_IntegerKeys )
    {
        FastHashSet<uint64_t, uint64_t, hashing::constants::FNV_OFFSET_BASIS_64> set;

        set.insert( 0xFFFFFFFFFFFFFFFFULL );
        set.insert( 0x123456789ABCDEFULL );
        set.insert( 42ULL );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( 0xFFFFFFFFFFFFFFFFULL ) );
        EXPECT_TRUE( set.contains( 0x123456789ABCDEFULL ) );
        EXPECT_TRUE( set.contains( 42ULL ) );
    }
} // namespace nfx::containers::test

//=====================================================================
// Custom type tests - Hasher fallback to std::hash
//=====================================================================

namespace nfx::containers::test
{
    struct ComplexObject
    {
        int id;
        std::string name;
        double coefficient;
        std::vector<int> data;

        bool operator==( const ComplexObject& other ) const
        {
            return id == other.id && name == other.name && coefficient == other.coefficient && data == other.data;
        }
    };
} // namespace nfx::containers::test

// Specialize std::hash for ComplexObject so nfx::Hasher can use it as fallback
namespace std
{
    template <>
    struct hash<nfx::containers::test::ComplexObject>
    {
        size_t operator()( const nfx::containers::test::ComplexObject& obj ) const noexcept
        {
            size_t h1 = std::hash<int>{}( obj.id );
            size_t h2 = std::hash<std::string>{}( obj.name );
            size_t h3 = std::hash<double>{}( obj.coefficient );

            size_t h_data = 0;
            for ( int val : obj.data )
            {
                h_data ^= std::hash<int>{}( val ) + 0x9e3779b9 + ( h_data << 6 ) + ( h_data >> 2 );
            }

            return h1 ^ ( h2 << 1 ) ^ ( h3 << 2 ) ^ ( h_data << 3 );
        }
    };
} // namespace std

namespace nfx::containers::test
{
    TEST( FastHashSetTests, CustomType_ComplexObject )
    {
        FastHashSet<ComplexObject> set;

        ComplexObject obj1{ 1, "alpha", 1.5, { 1, 2, 3 } };
        ComplexObject obj2{ 2, "beta", 2.5, { 4, 5, 6 } };
        ComplexObject obj3{ 3, "gamma", 3.5, { 7, 8, 9 } };

        EXPECT_TRUE( set.insert( obj1 ) );
        EXPECT_TRUE( set.insert( obj2 ) );
        EXPECT_TRUE( set.insert( obj3 ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( obj1 ) );
        EXPECT_TRUE( set.contains( obj2 ) );
        EXPECT_TRUE( set.contains( obj3 ) );

        // Duplicate insert should fail
        EXPECT_FALSE( set.insert( obj1 ) );
        EXPECT_EQ( set.size(), 3 );
    }

    TEST( FastHashSetTests, CustomType_NestedComplexity )
    {
        using SimplerKey = std::pair<std::pair<int, std::string>, double>;

        FastHashSet<SimplerKey> set;

        SimplerKey key1{ { 1, "one" }, 1.0 };
        SimplerKey key2{ { 2, "two" }, 2.0 };
        SimplerKey key3{ { 3, "three" }, 3.0 };

        EXPECT_TRUE( set.insert( key1 ) );
        EXPECT_TRUE( set.insert( key2 ) );
        EXPECT_TRUE( set.insert( key3 ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( key1 ) );
        EXPECT_TRUE( set.contains( key2 ) );
        EXPECT_TRUE( set.contains( key3 ) );
    }

    TEST( FastHashSetTests, CustomType_TripleNestedPairs )
    {
        using Level1 = std::pair<int, int>;
        using Level2 = std::pair<Level1, std::string>;
        using Level3 = std::pair<Level2, double>;

        FastHashSet<Level3> set;

        Level3 key1{ { { 1, 2 }, "alpha" }, 1.5 };
        Level3 key2{ { { 3, 4 }, "beta" }, 2.5 };

        EXPECT_TRUE( set.insert( key1 ) );
        EXPECT_TRUE( set.insert( key2 ) );

        EXPECT_EQ( set.size(), 2 );
        EXPECT_TRUE( set.contains( key1 ) );
        EXPECT_TRUE( set.contains( key2 ) );
    }

    TEST( FastHashSetTests, CustomType_MixedPairTypes )
    {
        using InnerMix = std::pair<float, int>;
        using MixedKey = std::pair<std::string, InnerMix>;

        FastHashSet<MixedKey> set;

        MixedKey key1{ "alpha", { 1.5f, 10 } };
        MixedKey key2{ "beta", { 2.5f, 20 } };
        MixedKey key3{ "gamma", { 3.5f, 30 } };

        EXPECT_TRUE( set.insert( key1 ) );
        EXPECT_TRUE( set.insert( key2 ) );
        EXPECT_TRUE( set.insert( key3 ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( key1 ) );
        EXPECT_TRUE( set.contains( key2 ) );
        EXPECT_TRUE( set.contains( key3 ) );
    }
} // namespace nfx::containers::test

//=====================================================================
// Another custom type with different hashing strategy
//=====================================================================

namespace nfx::containers::test
{
    // RGB color with custom hash using bit-packing strategy
    struct RGBColor
    {
        uint8_t r, g, b;

        bool operator==( const RGBColor& other ) const
        {
            return r == other.r && g == other.g && b == other.b;
        }
    };
} // namespace nfx::containers::test

// Full specialization using bit-packing instead of field combining
namespace std
{
    template <>
    struct hash<nfx::containers::test::RGBColor>
    {
        size_t operator()( const nfx::containers::test::RGBColor& color ) const noexcept
        {
            // Pack RGB into 24-bit value (efficient bit-level hashing)
            return ( static_cast<size_t>( color.r ) << 16 ) | ( static_cast<size_t>( color.g ) << 8 ) | static_cast<size_t>( color.b );
        }
    };
} // namespace std

namespace nfx::containers::test
{
    TEST( FastHashSetTests, CustomType_RGBColor_BitPacking )
    {
        FastHashSet<RGBColor> set;

        RGBColor red{ 255, 0, 0 };
        RGBColor green{ 0, 255, 0 };
        RGBColor blue{ 0, 0, 255 };
        RGBColor white{ 255, 255, 255 };
        RGBColor black{ 0, 0, 0 };

        EXPECT_TRUE( set.insert( red ) );
        EXPECT_TRUE( set.insert( green ) );
        EXPECT_TRUE( set.insert( blue ) );
        EXPECT_TRUE( set.insert( white ) );
        EXPECT_TRUE( set.insert( black ) );

        EXPECT_EQ( set.size(), 5 );
        EXPECT_TRUE( set.contains( red ) );
        EXPECT_TRUE( set.contains( green ) );
        EXPECT_TRUE( set.contains( blue ) );
        EXPECT_TRUE( set.contains( white ) );
        EXPECT_TRUE( set.contains( black ) );

        // Duplicate
        EXPECT_FALSE( set.insert( red ) );
        EXPECT_EQ( set.size(), 5 );
    }
} // namespace nfx::containers::test

//=====================================================================
// Another custom type with std::hash specialization using different strategy
//=====================================================================

namespace nfx::containers::test
{
    // 3D Point that will use std::hash specialization with bit-packing
    struct Point3D
    {
        float x, y, z;

        bool operator==( const Point3D& other ) const
        {
            return x == other.x && y == other.y && z == other.z;
        }
    };
} // namespace nfx::containers::test

// Another std::hash specialization demonstrating different hashing strategy
namespace std
{
    template <>
    struct hash<nfx::containers::test::Point3D>
    {
        size_t operator()( const nfx::containers::test::Point3D& p ) const noexcept
        {
            // Use bit representation instead of float values
            const uint32_t* x_bits = reinterpret_cast<const uint32_t*>( &p.x );
            const uint32_t* y_bits = reinterpret_cast<const uint32_t*>( &p.y );
            const uint32_t* z_bits = reinterpret_cast<const uint32_t*>( &p.z );

            // Combine using XOR and bit rotation
            size_t hash = *x_bits;
            hash ^= ( *y_bits << 1 ) | ( *y_bits >> 31 );
            hash ^= ( *z_bits << 2 ) | ( *z_bits >> 30 );

            return hash;
        }
    };
} // namespace std

namespace nfx::containers::test
{
    TEST( FastHashSetTests, CustomType_Point3D_BitRepresentation )
    {
        FastHashSet<Point3D> set;

        Point3D p1{ 1.0f, 2.0f, 3.0f };
        Point3D p2{ 4.0f, 5.0f, 6.0f };
        Point3D p3{ 7.0f, 8.0f, 9.0f };

        EXPECT_TRUE( set.insert( p1 ) );
        EXPECT_TRUE( set.insert( p2 ) );
        EXPECT_TRUE( set.insert( p3 ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( p1 ) );
        EXPECT_TRUE( set.contains( p2 ) );
        EXPECT_TRUE( set.contains( p3 ) );
    }
} // namespace nfx::containers::test

//=====================================================================
// Custom type with custom hasher
//=====================================================================

namespace nfx::containers::test
{
    // Custom domain object - Employee ID
    struct EmployeeID
    {
        uint16_t department;      // 0-999 (limited range)
        uint32_t employee_number; // Globally unique

        bool operator==( const EmployeeID& other ) const
        {
            return department == other.department && employee_number == other.employee_number;
        }
    };

    // CUSTOM HASHER specifically designed for EmployeeID
    // Uses domain knowledge: department has limited range, employee_number is more distributed
    template <hashing::Hash32or64 HashType, HashType Seed = hashing::constants::FNV_OFFSET_BASIS_32>
    struct EmployeeIDHasher
    {
        HashType operator()( const EmployeeID& id ) const
        {
            // Combine department (low entropy) with employee_number (high entropy)
            // Use multiplicative hash for employee_number, shift for department
            constexpr HashType prime = sizeof( HashType ) == 4 ? 2654435761u : 14695981039346656037ull;

            HashType hash = static_cast<HashType>( id.employee_number ) * prime;
            hash ^= static_cast<HashType>( id.department ) << 16;

            return hash;
        }
    };

    TEST( FastHashSetTests, CustomType_CustomHasher_EmployeeID )
    {
        FastHashSet<EmployeeID, uint32_t, hashing::constants::FNV_OFFSET_BASIS_32, EmployeeIDHasher<uint32_t>> set;

        EmployeeID emp1{ 100, 12345 };
        EmployeeID emp2{ 200, 67890 };
        EmployeeID emp3{ 100, 54321 }; // Same department, different employee

        EXPECT_TRUE( set.insert( emp1 ) );
        EXPECT_TRUE( set.insert( emp2 ) );
        EXPECT_TRUE( set.insert( emp3 ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( emp1 ) );
        EXPECT_TRUE( set.contains( emp2 ) );
        EXPECT_TRUE( set.contains( emp3 ) );

        // Duplicate
        EXPECT_FALSE( set.insert( emp1 ) );
        EXPECT_EQ( set.size(), 3 );
    }

    TEST( FastHashSetTests, CustomType_CustomHasher_64bit )
    {
        FastHashSet<EmployeeID, uint64_t, hashing::constants::FNV_OFFSET_BASIS_64, EmployeeIDHasher<uint64_t>> set;

        EmployeeID emp1{ 999, 0xFFFFFFFF };
        EmployeeID emp2{ 1, 0x12345678 };

        EXPECT_TRUE( set.insert( emp1 ) );
        EXPECT_TRUE( set.insert( emp2 ) );

        EXPECT_EQ( set.size(), 2 );
        EXPECT_TRUE( set.contains( emp1 ) );
        EXPECT_TRUE( set.contains( emp2 ) );
    }

    // Bonus: Custom hasher with domain-specific optimization
    struct IPAddress
    {
        uint8_t octet1, octet2, octet3, octet4;

        bool operator==( const IPAddress& other ) const
        {
            return octet1 == other.octet1 && octet2 == other.octet2 && octet3 == other.octet3 && octet4 == other.octet4;
        }
    };

    // Custom hasher for IP addresses - treats as 32-bit integer
    template <hashing::Hash32or64 HashType, HashType Seed = hashing::constants::FNV_OFFSET_BASIS_32>
    struct IPAddressHasher
    {
        HashType operator()( const IPAddress& ip ) const
        {
            // Pack IP address into 32-bit value and hash
            uint32_t packed = ( static_cast<uint32_t>( ip.octet1 ) << 24 ) | ( static_cast<uint32_t>( ip.octet2 ) << 16 ) |
                              ( static_cast<uint32_t>( ip.octet3 ) << 8 ) | static_cast<uint32_t>( ip.octet4 );

            // Use multiplicative hash
            constexpr HashType prime = sizeof( HashType ) == 4 ? 2654435761u : 14695981039346656037ull;
            return static_cast<HashType>( packed ) * prime;
        }
    };

    TEST( FastHashSetTests, CustomType_CustomHasher_IPAddress )
    {
        FastHashSet<IPAddress, uint32_t, hashing::constants::FNV_OFFSET_BASIS_32, IPAddressHasher<uint32_t>> set;

        IPAddress ip1{ 192, 168, 1, 1 };
        IPAddress ip2{ 10, 0, 0, 1 };
        IPAddress ip3{ 172, 16, 0, 1 };

        EXPECT_TRUE( set.insert( ip1 ) );
        EXPECT_TRUE( set.insert( ip2 ) );
        EXPECT_TRUE( set.insert( ip3 ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( ip1 ) );
        EXPECT_TRUE( set.contains( ip2 ) );
        EXPECT_TRUE( set.contains( ip3 ) );
    }

    //=====================================================================
    // Emplace method tests
    //=====================================================================

    TEST( FastHashSetTests, Emplace_BasicUsage )
    {
        FastHashSet<std::string> set;

        // Emplace with constructor arguments
        EXPECT_TRUE( set.emplace( "apple" ) );
        EXPECT_TRUE( set.emplace( 5, 'x' ) ); // std::string(5, 'x') = "xxxxx"

        EXPECT_EQ( set.size(), 2 );
        EXPECT_TRUE( set.contains( "apple" ) );
        EXPECT_TRUE( set.contains( "xxxxx" ) );
    }

    TEST( FastHashSetTests, Emplace_NoDuplicates )
    {
        FastHashSet<std::string> set;

        // First emplace should succeed
        EXPECT_TRUE( set.emplace( "test" ) );
        EXPECT_EQ( set.size(), 1 );

        // Second emplace with same key should fail
        EXPECT_FALSE( set.emplace( "test" ) );
        EXPECT_EQ( set.size(), 1 );
    }

    TEST( FastHashSetTests, Emplace_PerfectForwarding )
    {
        FastHashSet<std::string> set;

        // Emplace with multiple constructor arguments
        EXPECT_TRUE( set.emplace( 3, 'a' ) );  // "aaa"
        EXPECT_TRUE( set.emplace( 5, 'b' ) );  // "bbbbb"
        EXPECT_TRUE( set.emplace( 10, 'c' ) ); // "cccccccccc"

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( "aaa" ) );
        EXPECT_TRUE( set.contains( "bbbbb" ) );
        EXPECT_TRUE( set.contains( "cccccccccc" ) );
    }

    TEST( FastHashSetTests, Emplace_WithResize )
    {
        FastHashSet<std::string> set( 4 ); // Small initial capacity

        // Emplace enough elements to trigger resize
        for ( int i = 0; i < 20; ++i )
        {
            EXPECT_TRUE( set.emplace( 3, static_cast<char>( 'A' + ( i % 26 ) ) ) );
        }

        EXPECT_EQ( set.size(), 20 );

        // Verify all elements exist
        for ( int i = 0; i < 20; ++i )
        {
            std::string expected( 3, static_cast<char>( 'A' + ( i % 26 ) ) );
            EXPECT_TRUE( set.contains( expected ) );
        }
    }

    TEST( FastHashSetTests, Emplace_CustomType )
    {
        struct Point
        {
            int x = 0;
            int y = 0;
            Point() = default;

            Point( int x_, int y_ )
                : x( x_ ),
                  y( y_ )
            {
            }

            bool operator==( const Point& other ) const { return x == other.x && y == other.y; }
        };

        struct PointHasher
        {
            uint32_t operator()( const Point& p ) const
            {
                // Simple hash combining x and y
                return static_cast<uint32_t>( p.x * 73856093 ) ^ static_cast<uint32_t>( p.y * 19349663 );
            }
        };

        FastHashSet<Point, uint32_t, hashing::constants::FNV_OFFSET_BASIS_32, PointHasher> set;

        // Emplace with constructor arguments
        EXPECT_TRUE( set.emplace( 10, 20 ) );
        EXPECT_TRUE( set.emplace( 30, 40 ) );
        EXPECT_TRUE( set.emplace( 50, 60 ) );

        EXPECT_EQ( set.size(), 3 );
        EXPECT_TRUE( set.contains( Point{ 10, 20 } ) );
        EXPECT_TRUE( set.contains( Point{ 30, 40 } ) );
        EXPECT_TRUE( set.contains( Point{ 50, 60 } ) );
    }

    //=====================================================================
    // Iterator-based erase tests
    //=====================================================================

    TEST( FastHashSetTests, EraseIterator_SingleElement )
    {
        FastHashSet<int> set;
        set.insert( 1 );
        set.insert( 2 );
        set.insert( 3 );

        auto it = set.begin();
        while ( it != set.end() && *it != 2 )
        {
            ++it;
        }

        ASSERT_NE( it, set.end() );
        auto nextIt = set.erase( static_cast<FastHashSet<int>::const_iterator>( it ) );
        (void)nextIt;

        EXPECT_EQ( set.size(), 2 );
        EXPECT_FALSE( set.contains( 2 ) );
        EXPECT_TRUE( set.contains( 1 ) );
        EXPECT_TRUE( set.contains( 3 ) );
    }

    TEST( FastHashSetTests, EraseIterator_Range )
    {
        FastHashSet<int> set;
        for ( int i = 0; i < 10; ++i )
        {
            set.insert( i );
        }

        size_t eraseCount = 0;
        auto it = set.begin();
        auto first = it;

        // Erase first 5 elements
        for ( int i = 0; i < 5 && it != set.end(); ++i, ++it )
        {
            ++eraseCount;
        }

        size_t originalSize = set.size();
        set.erase( first, it );

        EXPECT_EQ( set.size(), originalSize - eraseCount );
    }

    TEST( FastHashSetTests, EraseIterator_AllElements )
    {
        FastHashSet<int> set;
        for ( int i = 0; i < 20; ++i )
        {
            set.insert( i );
        }

        set.erase( set.begin(), set.end() );

        EXPECT_EQ( set.size(), 0 );
        EXPECT_TRUE( set.isEmpty() );
    }

    //=====================================================================
    // tryEmplace tests
    //=====================================================================

    TEST( FastHashSetTests, TryEmplace_NewKey )
    {
        FastHashSet<std::string> set;

        auto [it, inserted] = set.tryEmplace( "hello" );

        EXPECT_TRUE( inserted );
        EXPECT_EQ( *it, "hello" );
        EXPECT_EQ( set.size(), 1 );
    }

    TEST( FastHashSetTests, TryEmplace_ExistingKey )
    {
        FastHashSet<std::string> set;
        set.insert( "hello" );

        auto [it, inserted] = set.tryEmplace( "hello" );

        EXPECT_FALSE( inserted );
        EXPECT_EQ( *it, "hello" );
        EXPECT_EQ( set.size(), 1 );
    }

    TEST( FastHashSetTests, TryEmplace_ComplexType )
    {
        struct Point
        {
            int x, y;

            Point() = default;

            Point( int x_, int y_ ) : x( x_ ), y( y_ )
            {
            }

            bool operator==( const Point& other ) const
            {
                return x == other.x && y == other.y;
            }
        };

        struct PointHasher
        {
            uint32_t operator()( const Point& p ) const
            {
                return static_cast<uint32_t>( p.x * 31 + p.y );
            }
        };

        FastHashSet<Point, uint32_t, 0, PointHasher> set;

        auto [it, inserted] = set.tryEmplace( 10, 20 );

        EXPECT_TRUE( inserted );
        EXPECT_EQ( it->x, 10 );
        EXPECT_EQ( it->y, 20 );
        EXPECT_EQ( set.size(), 1 );
    }

    namespace
    {
        int g_constructCount = 0;

        struct ConstructionCounter
        {
            int value;

            ConstructionCounter() : value( 0 )
            {
                ++g_constructCount;
            }

            ConstructionCounter( int v ) : value( v )
            {
                ++g_constructCount;
            }

            bool operator==( const ConstructionCounter& other ) const
            {
                return value == other.value;
            }
        };

        struct CounterHasher
        {
            uint32_t operator()( const ConstructionCounter& c ) const
            {
                return static_cast<uint32_t>( c.value );
            }
        };
    } // namespace

    TEST( FastHashSetTests, TryEmplace_VsEmplace_PerformanceBehavior )
    {
        FastHashSet<ConstructionCounter, uint32_t, 0, CounterHasher> set;

        // First insert with tryEmplace
        g_constructCount = 0;
        set.tryEmplace( 100 );
        int firstCount = g_constructCount;
        EXPECT_EQ( firstCount, 1 ); // One construction

        // Try to insert again - key already constructed to check, but shouldn't be inserted
        g_constructCount = 0;
        set.tryEmplace( 100 );
        int secondCount = g_constructCount;
        // Note: For set, we must construct key to check equality
        EXPECT_EQ( secondCount, 1 ); // Key constructed but not inserted
        EXPECT_EQ( set.size(), 1 );  // Set size unchanged
    }
} // namespace nfx::containers::test
