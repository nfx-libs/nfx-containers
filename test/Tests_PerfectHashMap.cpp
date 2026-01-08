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
 * @file Tests_PerfectHashMap.cpp
 * @brief Tests for PerfectHashMap (Compress-Hash-Displace perfect hash map)
 */

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nfx/containers/PerfectHashMap.h>

namespace nfx::containers::test
{
    using namespace nfx::hashing;

    //=====================================================================
    // String keys - Heterogeneous lookup tests
    //=====================================================================

    TEST( PerfectHashMapTests, StringKeys_BasicLookup )
    {
        std::vector<std::pair<std::string, int>> data{
            { "apple", 1 },
            { "banana", 2 },
            { "cherry", 3 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        EXPECT_EQ( map.at( "apple" ), 1 );
        EXPECT_EQ( map.at( "banana" ), 2 );
        EXPECT_EQ( map.at( "cherry" ), 3 );
        EXPECT_EQ( map.count(), 3 );
    }

    TEST( PerfectHashMapTests, StringKeys_HeterogeneousLookup_StringView )
    {
        std::vector<std::pair<std::string, int>> data{
            { "hello", 100 },
            { "world", 200 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        // Zero-copy string_view lookup (no temporary string allocation)
        std::string_view key1 = "hello";
        std::string_view key2 = "world";
        std::string_view key3 = "missing";

        EXPECT_EQ( map.at( key1 ), 100 );
        EXPECT_EQ( map.at( key2 ), 200 );
        EXPECT_TRUE( map.contains( key1 ) );
        EXPECT_TRUE( map.contains( key2 ) );
        EXPECT_FALSE( map.contains( key3 ) );
    }

    TEST( PerfectHashMapTests, StringKeys_HeterogeneousLookup_CString )
    {
        std::vector<std::pair<std::string, int>> data{
            { "test", 42 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        // Lookup with const char* (heterogeneous)
        const char* key = "test";
        EXPECT_TRUE( map.contains( key ) );
        EXPECT_EQ( map.at( key ), 42 );
    }

    TEST( PerfectHashMapTests, StringKeys_Find )
    {
        std::vector<std::pair<std::string, int>> data{
            { "key1", 10 },
            { "key2", 20 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        // Success case
        auto result1 = map.find( "key1" );
        ASSERT_NE( result1, nullptr );
        EXPECT_EQ( *result1, 10 );

        // Heterogeneous lookup with string_view
        std::string_view sv = "key2";
        auto result2 = map.find( sv );
        ASSERT_NE( result2, nullptr );
        EXPECT_EQ( *result2, 20 );

        // Missing key
        auto result3 = map.find( "missing" );
        EXPECT_EQ( result3, nullptr );
    }

    TEST( PerfectHashMapTests, StringKeys_Iterator )
    {
        std::vector<std::pair<std::string, int>> data{
            { "a", 1 },
            { "b", 2 },
            { "c", 3 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        size_t count = 0;
        int sum = 0;

        for ( const auto& [key, value] : map )
        {
            ++count;
            sum += value;
            EXPECT_FALSE( key.empty() );
            EXPECT_GT( value, 0 );
        }

        EXPECT_EQ( count, 3 );
        EXPECT_EQ( sum, 6 ); // 1 + 2 + 3
    }

    //=====================================================================
    // Integer keys - Multiplicative hashing tests
    //=====================================================================

    TEST( PerfectHashMapTests, IntegerKeys_BasicLookup )
    {
        std::vector<std::pair<int, std::string>> data{
            { 1, "one" },
            { 2, "two" },
            { 3, "three" } };

        PerfectHashMap<int, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( 1 ), "one" );
        EXPECT_EQ( map.at( 2 ), "two" );
        EXPECT_EQ( map.at( 3 ), "three" );
    }

    TEST( PerfectHashMapTests, IntegerKeys_Contains )
    {
        std::vector<std::pair<int, int>> data{
            { 42, 100 },
            { 99, 200 } };

        PerfectHashMap<int, int> map( std::move( data ) );

        EXPECT_TRUE( map.contains( 42 ) );
        EXPECT_TRUE( map.contains( 99 ) );
        EXPECT_FALSE( map.contains( 0 ) );
        EXPECT_FALSE( map.contains( 50 ) );
    }

    TEST( PerfectHashMapTests, IntegerKeys_LargeValues )
    {
        std::vector<std::pair<uint64_t, std::string>> data{
            { 0xFFFFFFFFFFFFFFFFULL, "max" },
            { 0x0000000000000001ULL, "min" },
            { 0x8000000000000000ULL, "mid" } };

        PerfectHashMap<uint64_t, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( 0xFFFFFFFFFFFFFFFFULL ), "max" );
        EXPECT_EQ( map.at( 0x0000000000000001ULL ), "min" );
        EXPECT_EQ( map.at( 0x8000000000000000ULL ), "mid" );
    }

    //=====================================================================
    // Enum keys - Underlying type conversion tests
    //=====================================================================

    enum class Color
    {
        Red = 1,
        Green = 2,
        Blue = 3
    };

    TEST( PerfectHashMapTests, EnumKeys_BasicLookup )
    {
        std::vector<std::pair<Color, std::string>> data{
            { Color::Red, "red" },
            { Color::Green, "green" },
            { Color::Blue, "blue" } };

        PerfectHashMap<Color, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( Color::Red ), "red" );
        EXPECT_EQ( map.at( Color::Green ), "green" );
        EXPECT_EQ( map.at( Color::Blue ), "blue" );
    }

    enum class Status : uint8_t
    {
        Idle = 0,
        Running = 1,
        Stopped = 2
    };

    TEST( PerfectHashMapTests, EnumKeys_DifferentUnderlyingType )
    {
        std::vector<std::pair<Status, int>> data{
            { Status::Idle, 0 },
            { Status::Running, 1 },
            { Status::Stopped, 2 } };

        PerfectHashMap<Status, int> map( std::move( data ) );

        EXPECT_TRUE( map.contains( Status::Idle ) );
        EXPECT_TRUE( map.contains( Status::Running ) );
        EXPECT_TRUE( map.contains( Status::Stopped ) );
    }

    //=====================================================================
    // Floating-point keys - Normalization tests
    //=====================================================================

    TEST( PerfectHashMapTests, FloatKeys_BasicLookup )
    {
        std::vector<std::pair<double, std::string>> data{
            { 3.14159, "pi" },
            { 2.71828, "e" },
            { 1.41421, "sqrt2" } };

        PerfectHashMap<double, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( 3.14159 ), "pi" );
        EXPECT_EQ( map.at( 2.71828 ), "e" );
        EXPECT_EQ( map.at( 1.41421 ), "sqrt2" );
    }

    //=====================================================================
    // Pointer keys - Address hashing tests
    //=====================================================================

    TEST( PerfectHashMapTests, PointerKeys_BasicLookup )
    {
        int a = 1, b = 2, c = 3;

        std::vector<std::pair<int*, std::string>> data{
            { &a, "a" },
            { &b, "b" },
            { &c, "c" } };

        PerfectHashMap<int*, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( &a ), "a" );
        EXPECT_EQ( map.at( &b ), "b" );
        EXPECT_EQ( map.at( &c ), "c" );
        EXPECT_TRUE( map.contains( &a ) );
        EXPECT_FALSE( map.contains( nullptr ) );
    }

    //=====================================================================
    // Pair keys - Hash combining tests
    //=====================================================================

    TEST( PerfectHashMapTests, PairKeys_BasicLookup )
    {
        using Key = std::pair<int, int>;

        std::vector<std::pair<Key, std::string>> data{
            { { 1, 2 }, "first" },
            { { 3, 4 }, "second" },
            { { 5, 6 }, "third" } };

        PerfectHashMap<Key, std::string> map( std::move( data ) );

        Key key1{ 1, 2 };
        Key key2{ 3, 4 };
        Key key3{ 5, 6 };

        EXPECT_EQ( map.at( key1 ), "first" );
        EXPECT_EQ( map.at( key2 ), "second" );
        EXPECT_EQ( map.at( key3 ), "third" );
    }

    TEST( PerfectHashMapTests, PairKeys_StringPair )
    {
        using Key = std::pair<std::string, std::string>;

        std::vector<std::pair<Key, int>> data{
            { { "foo", "bar" }, 1 },
            { { "hello", "world" }, 2 } };

        PerfectHashMap<Key, int> map( std::move( data ) );

        Key key1{ "foo", "bar" };
        Key key2{ "hello", "world" };
        Key key3{ "missing", "key" };

        EXPECT_EQ( map.at( key1 ), 1 );
        EXPECT_EQ( map.at( key2 ), 2 );
        EXPECT_TRUE( map.contains( key1 ) );
        EXPECT_FALSE( map.contains( key3 ) );
    }

    //=====================================================================
    // Custom seed tests
    //=====================================================================

    TEST( PerfectHashMapTests, CustomSeed_32bit )
    {
        std::vector<std::pair<std::string, int>> data{
            { "cat", 1 },
            { "dog", 2 } };

        // Using custom seed instead of default FNV_OFFSET_BASIS_32
        PerfectHashMap<std::string, int, uint32_t, 0xABCDEF01> map( std::move( data ) );

        EXPECT_EQ( map.at( "cat" ), 1 );
        EXPECT_EQ( map.at( "dog" ), 2 );
    }

    TEST( PerfectHashMapTests, CustomSeed_64bit )
    {
        std::vector<std::pair<std::string, int>> data{
            { "test", 42 } };

        // Using 64-bit hash with custom seed
        PerfectHashMap<std::string, int, uint64_t, 0x123456789ABCDEF0ULL> map( std::move( data ) );

        EXPECT_EQ( map.at( "test" ), 42 );
        EXPECT_TRUE( map.contains( "test" ) );
    }

    //=====================================================================
    // 64-bit hashing tests
    //=====================================================================

    TEST( PerfectHashMapTests, Hash64_StringKeys )
    {
        std::vector<std::pair<std::string, int>> data{
            { "alpha", 1 },
            { "beta", 2 },
            { "gamma", 3 } };

        // Explicit 64-bit hash type with default FNV_OFFSET_BASIS_64 seed
        PerfectHashMap<std::string, int, uint64_t> map( std::move( data ) );

        EXPECT_EQ( map.at( "alpha" ), 1 );
        EXPECT_EQ( map.at( "beta" ), 2 );
        EXPECT_EQ( map.at( "gamma" ), 3 );

        // Heterogeneous lookup with string_view
        std::string_view sv = "alpha";
        EXPECT_TRUE( map.contains( sv ) );
    }

    TEST( PerfectHashMapTests, Hash64_IntegerKeys )
    {
        std::vector<std::pair<uint64_t, std::string>> data{
            { 1000000000000ULL, "trillion" },
            { 2000000000000ULL, "two_trillion" } };

        PerfectHashMap<uint64_t, std::string, uint64_t> map( std::move( data ) );

        EXPECT_EQ( map.at( 1000000000000ULL ), "trillion" );
        EXPECT_EQ( map.at( 2000000000000ULL ), "two_trillion" );
    }

    //=====================================================================
    // Empty map tests
    //=====================================================================

    TEST( PerfectHashMapTests, EmptyMap )
    {
        std::vector<std::pair<std::string, int>> data;
        PerfectHashMap<std::string, int> map( std::move( data ) );

        EXPECT_TRUE( map.isEmpty() );
        EXPECT_EQ( map.size(), 0 );
        EXPECT_EQ( map.count(), 0 );
        EXPECT_EQ( map.begin(), map.end() );
    }

    TEST( PerfectHashMapTests, EmptyMap_Lookups )
    {
        std::vector<std::pair<int, int>> data;
        PerfectHashMap<int, int> map( std::move( data ) );

        EXPECT_FALSE( map.contains( 0 ) );
        EXPECT_FALSE( map.contains( 42 ) );
        EXPECT_EQ( map.find( 0 ), nullptr );
    }

    //=====================================================================
    // Hasher API tests
    //=====================================================================

    TEST( PerfectHashMapTests, HasherAPI_HashFunction )
    {
        std::vector<std::pair<std::string, int>> data{
            { "test", 1 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        auto hasher = map.hash_function();

        // Verify hasher is functional
        uint32_t hash1 = hasher( "test" );
        uint32_t hash2 = hasher( std::string_view{ "test" } );
        uint32_t hash3 = hasher( std::string{ "test" } );

        // All should produce same hash
        EXPECT_EQ( hash1, hash2 );
        EXPECT_EQ( hash2, hash3 );
    }

    TEST( PerfectHashMapTests, HasherAPI_KeyEqual )
    {
        std::vector<std::pair<std::string, int>> data{
            { "key", 42 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        auto key_eq = map.key_eq();

        // Verify key equality comparator
        EXPECT_TRUE( key_eq( std::string{ "key" }, std::string{ "key" } ) );
        EXPECT_FALSE( key_eq( std::string{ "key" }, std::string{ "other" } ) );
    }

    //=====================================================================
    // Capacity and memory management
    //=====================================================================

    TEST( PerfectHashMapTests, SizeAndCount )
    {
        std::vector<std::pair<int, int>> data{
            { 1, 10 },
            { 2, 20 },
            { 3, 30 },
            { 4, 40 },
            { 5, 50 } };

        PerfectHashMap<int, int> map( std::move( data ) );

        EXPECT_EQ( map.count(), 5 );
        EXPECT_FALSE( map.isEmpty() );
    }

    //=====================================================================
    // Iterator tests
    //=====================================================================

    TEST( PerfectHashMapTests, Iterator_BeginEnd )
    {
        std::vector<std::pair<std::string, int>> data{
            { "x", 1 },
            { "y", 2 },
            { "z", 3 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        auto it = map.begin();
        auto end = map.end();

        EXPECT_NE( it, end );

        size_t count = 0;
        while ( it != end )
        {
            ++count;
            ++it;
        }

        EXPECT_EQ( count, 3 );
    }

    TEST( PerfectHashMapTests, Iterator_ConstIterator )
    {
        std::vector<std::pair<std::string, int>> data{
            { "a", 1 },
            { "b", 2 } };

        const PerfectHashMap<std::string, int> map( std::move( data ) );

        size_t count = 0;
        for ( auto it = map.cbegin(); it != map.cend(); ++it )
        {
            ++count;
            EXPECT_FALSE( it->first.empty() );
            EXPECT_GT( it->second, 0 );
        }

        EXPECT_EQ( count, 2 );
    }

    //=====================================================================
    // Comparison operator tests
    //=====================================================================

    TEST( PerfectHashMapTests, ComparisonOperators_Equal )
    {
        std::vector<std::pair<std::string, int>> data1{
            { "a", 1 },
            { "b", 2 } };

        std::vector<std::pair<std::string, int>> data2{
            { "a", 1 },
            { "b", 2 } };

        PerfectHashMap<std::string, int> map1( std::move( data1 ) );
        PerfectHashMap<std::string, int> map2( std::move( data2 ) );

        EXPECT_TRUE( map1 == map2 );
        EXPECT_FALSE( map1 != map2 );
    }

    TEST( PerfectHashMapTests, ComparisonOperators_NotEqual_DifferentValues )
    {
        std::vector<std::pair<std::string, int>> data1{
            { "a", 1 },
            { "b", 2 } };

        std::vector<std::pair<std::string, int>> data2{
            { "a", 1 },
            { "b", 999 } // Different value
        };

        PerfectHashMap<std::string, int> map1( std::move( data1 ) );
        PerfectHashMap<std::string, int> map2( std::move( data2 ) );

        EXPECT_FALSE( map1 == map2 );
        EXPECT_TRUE( map1 != map2 );
    }

    TEST( PerfectHashMapTests, ComparisonOperators_NotEqual_DifferentSize )
    {
        std::vector<std::pair<std::string, int>> data1{
            { "a", 1 },
            { "b", 2 } };

        std::vector<std::pair<std::string, int>> data2{
            { "a", 1 } };

        PerfectHashMap<std::string, int> map1( std::move( data1 ) );
        PerfectHashMap<std::string, int> map2( std::move( data2 ) );

        EXPECT_FALSE( map1 == map2 );
        EXPECT_TRUE( map1 != map2 );
    }

    //=====================================================================
    // Edge cases and stress tests
    //=====================================================================

    TEST( PerfectHashMapTests, EdgeCase_SingleElement )
    {
        std::vector<std::pair<std::string, int>> data{
            { "only", 42 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        EXPECT_EQ( map.count(), 1 );
        EXPECT_EQ( map.at( "only" ), 42 );
        EXPECT_TRUE( map.contains( "only" ) );
        EXPECT_FALSE( map.isEmpty() );
    }

    TEST( PerfectHashMapTests, EdgeCase_EmptyStringKey )
    {
        std::vector<std::pair<std::string, int>> data{
            { "", 100 },
            { "nonempty", 200 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        EXPECT_EQ( map.at( "" ), 100 );
        EXPECT_EQ( map.at( "nonempty" ), 200 );
        EXPECT_TRUE( map.contains( "" ) );

        // Heterogeneous lookup with empty string_view
        std::string_view empty_sv;
        EXPECT_TRUE( map.contains( empty_sv ) );
        EXPECT_EQ( map.at( empty_sv ), 100 );
    }

    TEST( PerfectHashMapTests, EdgeCase_VeryLongStrings )
    {
        std::string long1( 1000, 'a' );
        std::string long2( 1000, 'b' );
        std::string long3( 1000, 'c' );

        std::vector<std::pair<std::string, int>> data{
            { long1, 1 },
            { long2, 2 },
            { long3, 3 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        EXPECT_EQ( map.at( long1 ), 1 );
        EXPECT_EQ( map.at( long2 ), 2 );
        EXPECT_EQ( map.at( long3 ), 3 );
        EXPECT_EQ( map.count(), 3 );
    }

    TEST( PerfectHashMapTests, EdgeCase_UnicodeStrings )
    {
        std::vector<std::pair<std::string, int>> data{
            { "Hello", 1 },
            { "Привет", 2 },     // Russian
            { "你好", 3 },       // Chinese
            { "こんにちは", 4 }, // Japanese
            { "🚀🎉💻", 5 } };   // Emojis

        PerfectHashMap<std::string, int> map( std::move( data ) );

        EXPECT_EQ( map.at( "Hello" ), 1 );
        EXPECT_EQ( map.at( "Привет" ), 2 );
        EXPECT_EQ( map.at( "你好" ), 3 );
        EXPECT_EQ( map.at( "こんにちは" ), 4 );
        EXPECT_EQ( map.at( "🚀🎉💻" ), 5 );
        EXPECT_EQ( map.count(), 5 );
    }

    TEST( PerfectHashMapTests, EdgeCase_SimilarStrings )
    {
        // Strings differing by single character
        std::vector<std::pair<std::string, int>> data{
            { "test1", 1 },
            { "test2", 2 },
            { "test3", 3 },
            { "test4", 4 },
            { "test5", 5 } };

        PerfectHashMap<std::string, int> map( std::move( data ) );

        EXPECT_EQ( map.at( "test1" ), 1 );
        EXPECT_EQ( map.at( "test2" ), 2 );
        EXPECT_EQ( map.at( "test3" ), 3 );
        EXPECT_EQ( map.at( "test4" ), 4 );
        EXPECT_EQ( map.at( "test5" ), 5 );
    }

    TEST( PerfectHashMapTests, EdgeCase_PowerOfTwoKeys )
    {
        // Keys designed to potentially collide in power-of-2 hash tables
        std::vector<std::pair<int, std::string>> data{
            { 0, "zero" },
            { 1, "one" },
            { 2, "two" },
            { 4, "four" },
            { 8, "eight" },
            { 16, "sixteen" },
            { 32, "thirty_two" },
            { 64, "sixty_four" },
            { 128, "one_twenty_eight" } };

        PerfectHashMap<int, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( 0 ), "zero" );
        EXPECT_EQ( map.at( 1 ), "one" );
        EXPECT_EQ( map.at( 2 ), "two" );
        EXPECT_EQ( map.at( 4 ), "four" );
        EXPECT_EQ( map.at( 8 ), "eight" );
        EXPECT_EQ( map.at( 16 ), "sixteen" );
        EXPECT_EQ( map.at( 32 ), "thirty_two" );
        EXPECT_EQ( map.at( 64 ), "sixty_four" );
        EXPECT_EQ( map.at( 128 ), "one_twenty_eight" );
    }

    TEST( PerfectHashMapTests, EdgeCase_NegativeIntegerKeys )
    {
        std::vector<std::pair<int, std::string>> data{
            { -100, "negative_hundred" },
            { -1, "negative_one" },
            { 0, "zero" },
            { 1, "one" },
            { 100, "hundred" } };

        PerfectHashMap<int, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( -100 ), "negative_hundred" );
        EXPECT_EQ( map.at( -1 ), "negative_one" );
        EXPECT_EQ( map.at( 0 ), "zero" );
        EXPECT_EQ( map.at( 1 ), "one" );
        EXPECT_EQ( map.at( 100 ), "hundred" );
    }

    TEST( PerfectHashMapTests, EdgeCase_SpecialFloatingPoints )
    {
        // Note: +0.0 and -0.0 are normalized to same value by Hasher, so don't include both
        std::vector<std::pair<double, std::string>> data{
            { 0.0, "zero" },
            { 1.0, "one" },
            { -1.0, "negative_one" },
            { std::numeric_limits<double>::infinity(), "infinity" },
            { -std::numeric_limits<double>::infinity(), "negative_infinity" },
            { std::numeric_limits<double>::min(), "min_normal" },
            { std::numeric_limits<double>::max(), "max" } };

        PerfectHashMap<double, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( 0.0 ), "zero" );
        EXPECT_EQ( map.at( 1.0 ), "one" );
        EXPECT_EQ( map.at( -1.0 ), "negative_one" );
        EXPECT_TRUE( map.contains( std::numeric_limits<double>::infinity() ) );
        EXPECT_TRUE( map.contains( -std::numeric_limits<double>::infinity() ) );
        EXPECT_TRUE( map.contains( std::numeric_limits<double>::min() ) );
        EXPECT_TRUE( map.contains( std::numeric_limits<double>::max() ) );
        EXPECT_EQ( map.count(), 7 );
    }

    TEST( PerfectHashMapTests, Stress_ManyElements )
    {
        // Test with 100 elements
        std::vector<std::pair<int, int>> data;
        for ( int i = 0; i < 100; ++i )
        {
            data.emplace_back( i, i * 10 );
        }

        PerfectHashMap<int, int> map( std::move( data ) );

        EXPECT_EQ( map.count(), 100 );

        // Verify all elements are accessible
        for ( int i = 0; i < 100; ++i )
        {
            EXPECT_TRUE( map.contains( i ) );
            EXPECT_EQ( map.at( i ), i * 10 );
        }

        // Verify non-existent keys
        EXPECT_FALSE( map.contains( -1 ) );
        EXPECT_FALSE( map.contains( 100 ) );
        EXPECT_FALSE( map.contains( 1000 ) );
    }

    TEST( PerfectHashMapTests, Stress_LargePrimeNumbers )
    {
        // Test with large prime numbers (harder to hash well)
        std::vector<std::pair<uint64_t, std::string>> data{
            { 2147483647ULL, "mersenne_31" },               // 2^31 - 1
            { 4294967291ULL, "large_prime_1" },             // Largest prime < 2^32
            { 18446744073709551557ULL, "large_prime_2" } }; // Large prime < 2^64

        PerfectHashMap<uint64_t, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( 2147483647ULL ), "mersenne_31" );
        EXPECT_EQ( map.at( 4294967291ULL ), "large_prime_1" );
        EXPECT_EQ( map.at( 18446744073709551557ULL ), "large_prime_2" );
    }

    TEST( PerfectHashMapTests, Stress_AllASCIICharacters )
    {
        // Test with all printable ASCII characters as single-char strings
        std::vector<std::pair<std::string, int>> data;
        for ( int c = 32; c < 127; ++c ) // Printable ASCII
        {
            data.emplace_back( std::string( 1, static_cast<char>( c ) ), c );
        }

        PerfectHashMap<std::string, int> map( std::move( data ) );

        EXPECT_EQ( map.count(), 95 ); // 127 - 32 = 95 printable chars

        // Verify all characters are accessible
        for ( int c = 32; c < 127; ++c )
        {
            std::string key( 1, static_cast<char>( c ) );
            EXPECT_TRUE( map.contains( key ) );
            EXPECT_EQ( map.at( key ), c );
        }
    }

    TEST( PerfectHashMapTests, EdgeCase_DuplicateKeys )
    {
        // Verify that duplicate keys are detected and throw exception
        std::vector<std::pair<std::string, int>> data{
            { "apple", 1 },
            { "banana", 2 },
            { "apple", 3 } // DUPLICATE!
        };

        EXPECT_THROW( ( PerfectHashMap<std::string, int>( std::move( data ) ) ), std::invalid_argument );

        // Test with integer keys
        std::vector<std::pair<int, std::string>> data2{
            { 1, "one" },
            { 2, "two" },
            { 1, "uno" } // DUPLICATE!
        };

        EXPECT_THROW( ( PerfectHashMap<int, std::string>( std::move( data2 ) ) ), std::invalid_argument );
    }

    //=====================================================================
    // std::hash tests - Verify compatibility with standard library hashers
    //=====================================================================

    TEST( PerfectHashMapTests, StdHash_StringKeys )
    {
        // PerfectHashMap with std::hash<std::string> instead of nfx::Hasher
        std::vector<std::pair<std::string, int>> data{
            { "apple", 100 },
            { "banana", 200 },
            { "cherry", 300 } };

        PerfectHashMap<std::string, int, uint32_t, 0, std::hash<std::string>> map( std::move( data ) );

        EXPECT_EQ( map.at( "apple" ), 100 );
        EXPECT_EQ( map.at( "banana" ), 200 );
        EXPECT_EQ( map.at( "cherry" ), 300 );
        EXPECT_TRUE( map.contains( "apple" ) );
        EXPECT_FALSE( map.contains( "missing" ) );
    }

    TEST( PerfectHashMapTests, StdHash_IntegerKeys )
    {
        std::vector<std::pair<int, std::string>> data{
            { 42, "answer" },
            { 100, "century" },
            { -1, "negative" } };

        PerfectHashMap<int, std::string, uint32_t, 0, std::hash<int>> map( std::move( data ) );

        EXPECT_EQ( map.at( 42 ), "answer" );
        EXPECT_EQ( map.at( 100 ), "century" );
        EXPECT_EQ( map.at( -1 ), "negative" );
    }

    //=====================================================================
    // Custom hasher tests - Verify custom hash functions work correctly
    //=====================================================================

    // Simple multiplicative hasher
    struct SimpleMultiplicativeHasher
    {
        uint32_t operator()( int key ) const { return static_cast<uint32_t>( key ) * 2654435761u; }
    };

    TEST( PerfectHashMapTests, CustomHasher_SimpleMultiplicative )
    {
        std::vector<std::pair<int, std::string>> data{
            { 42, "answer" },
            { 100, "century" },
            { 999, "large" } };

        PerfectHashMap<int, std::string, uint32_t, 0, SimpleMultiplicativeHasher> map( std::move( data ) );

        EXPECT_EQ( map.at( 42 ), "answer" );
        EXPECT_EQ( map.at( 100 ), "century" );
        EXPECT_EQ( map.at( 999 ), "large" );
        EXPECT_TRUE( map.contains( 42 ) );
        EXPECT_FALSE( map.contains( 0 ) );
    }

    // Identity hasher (for testing - returns key as-is)
    struct Identity32Hasher
    {
        uint32_t operator()( int key ) const { return static_cast<uint32_t>( key ); }
    };

    TEST( PerfectHashMapTests, CustomHasher_Identity32 )
    {
        std::vector<std::pair<int, std::string>> data{
            { 1, "one" },
            { 2, "two" },
            { 3, "three" } };

        PerfectHashMap<int, std::string, uint32_t, 0, Identity32Hasher> map( std::move( data ) );

        EXPECT_EQ( map.at( 1 ), "one" );
        EXPECT_EQ( map.at( 2 ), "two" );
        EXPECT_EQ( map.at( 3 ), "three" );
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

    TEST( PerfectHashMapTests, CustomHasher_XorShift )
    {
        std::vector<std::pair<int, std::string>> data{
            { 42, "answer" },
            { 100, "century" },
            { 999, "large" } };

        PerfectHashMap<int, std::string, uint32_t, 0, XorShiftHasher> map( std::move( data ) );

        EXPECT_EQ( map.at( 42 ), "answer" );
        EXPECT_EQ( map.at( 100 ), "century" );
        EXPECT_EQ( map.at( 999 ), "large" );
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

        bool operator==( const ComplexObject& other ) const noexcept
        {
            return id == other.id &&
                   name == other.name &&
                   coefficient == other.coefficient &&
                   data == other.data;
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
            // Use nfx::Hasher for individual components
            using nfx::hashing::Hasher;
            using nfx::hashing::combine;

            Hasher<uint32_t> hasher;

            // Hash each field and combine
            uint32_t h = hasher( obj.id );
            h = combine( h, hasher( obj.name ) );
            h = combine( h, hasher( obj.coefficient ) );

            // Hash vector elements
            for ( const auto& value : obj.data )
            {
                h = combine( h, hasher( value ) );
            }

            return h;
        }
    };
} // namespace std

namespace nfx::containers::test
{
    TEST( PerfectHashMapTests, CustomType_ComplexObject )
    {
        ComplexObject obj1{ 1, "alpha", 3.14159, { 10, 20, 30 } };
        ComplexObject obj2{ 2, "beta", 2.71828, { 40, 50 } };
        ComplexObject obj3{ 3, "gamma", 1.41421, { 60, 70, 80, 90 } };

        std::vector<std::pair<ComplexObject, std::string>> data;
        data.emplace_back( obj1, "first_object" );
        data.emplace_back( obj2, "second_object" );
        data.emplace_back( obj3, "third_object" );

        // PerfectHashMap uses Hasher<HashType, Seed> which falls back to std::hash<ComplexObject>
        PerfectHashMap<ComplexObject, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( obj1 ), "first_object" );
        EXPECT_EQ( map.at( obj2 ), "second_object" );
        EXPECT_EQ( map.at( obj3 ), "third_object" );
        EXPECT_EQ( map.count(), 3 );

        EXPECT_TRUE( map.contains( obj1 ) );
        EXPECT_TRUE( map.contains( obj2 ) );
        EXPECT_TRUE( map.contains( obj3 ) );

        ComplexObject obj4{ 999, "nonexistent", 0.0, {} };
        EXPECT_FALSE( map.contains( obj4 ) );
        EXPECT_EQ( map.find( obj4 ), nullptr );
    }

    TEST( PerfectHashMapTests, CustomType_NestedComplexity )
    {
        using SimplerKey = std::pair<std::pair<int, std::string>, double>;

        SimplerKey key1{ { 1, "one" }, 1.1 };
        SimplerKey key2{ { 2, "two" }, 2.2 };
        SimplerKey key3{ { 3, "three" }, 3.3 };

        std::vector<std::pair<SimplerKey, int>> data;
        data.emplace_back( key1, 100 );
        data.emplace_back( key2, 200 );
        data.emplace_back( key3, 300 );

        // Hasher natively supports std::pair recursively
        PerfectHashMap<SimplerKey, int> map( std::move( data ) );

        EXPECT_EQ( map.at( key1 ), 100 );
        EXPECT_EQ( map.at( key2 ), 200 );
        EXPECT_EQ( map.at( key3 ), 300 );

        SimplerKey key4{ { 99, "missing" }, 9.9 };
        EXPECT_FALSE( map.contains( key4 ) );
    }

    TEST( PerfectHashMapTests, CustomType_TripleNestedPairs )
    {
        // Test triple-nested pairs: pair<pair<pair<int, int>, string>, double>
        using Level1 = std::pair<int, int>;
        using Level2 = std::pair<Level1, std::string>;
        using Level3 = std::pair<Level2, double>;

        Level3 key1{ { { 1, 10 }, "a" }, 1.5 };
        Level3 key2{ { { 2, 20 }, "b" }, 2.5 };
        Level3 key3{ { { 3, 30 }, "c" }, 3.5 };

        std::vector<std::pair<Level3, std::string>> data;
        data.emplace_back( key1, "level1" );
        data.emplace_back( key2, "level2" );
        data.emplace_back( key3, "level3" );

        PerfectHashMap<Level3, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( key1 ), "level1" );
        EXPECT_EQ( map.at( key2 ), "level2" );
        EXPECT_EQ( map.at( key3 ), "level3" );
        EXPECT_EQ( map.count(), 3 );
    }

    TEST( PerfectHashMapTests, CustomType_MixedPairTypes )
    {
        // Test pair with different fundamental types: pair<string, pair<float, int>>
        using InnerMix = std::pair<float, int>;
        using MixedKey = std::pair<std::string, InnerMix>;

        MixedKey key1{ "test", { 1.5f, 10 } };
        MixedKey key2{ "demo", { 2.5f, 20 } };
        MixedKey key3{ "sample", { 3.5f, 30 } };

        std::vector<std::pair<MixedKey, int>> data;
        data.emplace_back( key1, 111 );
        data.emplace_back( key2, 222 );
        data.emplace_back( key3, 333 );

        PerfectHashMap<MixedKey, int> map( std::move( data ) );

        EXPECT_EQ( map.at( key1 ), 111 );
        EXPECT_EQ( map.at( key2 ), 222 );
        EXPECT_EQ( map.at( key3 ), 333 );

        MixedKey key4{ "missing", { 9.9f, 99 } };
        EXPECT_FALSE( map.contains( key4 ) );
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

        bool operator==( const RGBColor& other ) const noexcept
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
            // Pack RGB into single 32-bit value: 0x00RRGGBB
            // Then hash the packed value using nfx::Hasher
            uint32_t packed = ( static_cast<uint32_t>( color.r ) << 16 ) |
                              ( static_cast<uint32_t>( color.g ) << 8 ) |
                              static_cast<uint32_t>( color.b );

            nfx::hashing::Hasher<uint32_t> hasher;
            return hasher( packed );
        }
    };
} // namespace std

namespace nfx::containers::test
{
    TEST( PerfectHashMapTests, CustomType_RGBColor_BitPacking )
    {
        RGBColor red{ 255, 0, 0 };
        RGBColor green{ 0, 255, 0 };
        RGBColor blue{ 0, 0, 255 };
        RGBColor white{ 255, 255, 255 };
        RGBColor black{ 0, 0, 0 };

        std::vector<std::pair<RGBColor, std::string>> data;
        data.emplace_back( red, "red" );
        data.emplace_back( green, "green" );
        data.emplace_back( blue, "blue" );
        data.emplace_back( white, "white" );
        data.emplace_back( black, "black" );

        // Uses std::hash<RGBColor> via Hasher fallback
        PerfectHashMap<RGBColor, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( red ), "red" );
        EXPECT_EQ( map.at( green ), "green" );
        EXPECT_EQ( map.at( blue ), "blue" );
        EXPECT_EQ( map.at( white ), "white" );
        EXPECT_EQ( map.at( black ), "black" );
        EXPECT_EQ( map.count(), 5 );

        RGBColor purple{ 128, 0, 128 };
        EXPECT_FALSE( map.contains( purple ) );
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

        bool operator==( const Point3D& other ) const noexcept
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
        size_t operator()( const nfx::containers::test::Point3D& point ) const noexcept
        {
            // Alternative strategy: use bit representation of floats
            // Cast each float to uint32, then combine using nfx::Hasher
            using nfx::hashing::Hasher;
            using nfx::hashing::combine;

            Hasher<uint32_t> hasher;

            // Treat float bits as uint32 for hashing
            uint32_t x_bits;
            uint32_t y_bits;
            uint32_t z_bits;
            std::memcpy( &x_bits, &point.x, sizeof( float ) );
            std::memcpy( &y_bits, &point.y, sizeof( float ) );
            std::memcpy( &z_bits, &point.z, sizeof( float ) );

            uint32_t h = hasher( x_bits );
            h = combine( h, hasher( y_bits ) );
            h = combine( h, hasher( z_bits ) );

            return h;
        }
    };
} // namespace std

namespace nfx::containers::test
{
    TEST( PerfectHashMapTests, CustomType_Point3D_BitRepresentation )
    {
        Point3D p1{ 1.0f, 2.0f, 3.0f };
        Point3D p2{ 4.0f, 5.0f, 6.0f };
        Point3D p3{ 7.0f, 8.0f, 9.0f };
        Point3D p4{ 0.0f, 0.0f, 0.0f };

        std::vector<std::pair<Point3D, std::string>> data;
        data.emplace_back( p1, "point_one" );
        data.emplace_back( p2, "point_two" );
        data.emplace_back( p3, "point_three" );
        data.emplace_back( p4, "origin" );

        // Uses std::hash<Point3D> via Hasher fallback mechanism
        PerfectHashMap<Point3D, std::string> map( std::move( data ) );

        EXPECT_EQ( map.at( p1 ), "point_one" );
        EXPECT_EQ( map.at( p2 ), "point_two" );
        EXPECT_EQ( map.at( p3 ), "point_three" );
        EXPECT_EQ( map.at( p4 ), "origin" );
        EXPECT_EQ( map.count(), 4 );

        Point3D p5{ 99.0f, 99.0f, 99.0f };
        EXPECT_FALSE( map.contains( p5 ) );
        EXPECT_EQ( map.find( p5 ), nullptr );
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
        uint16_t department;      // 0-65535
        uint32_t employee_number; // 0-4294967295

        bool operator==( const EmployeeID& other ) const noexcept
        {
            return department == other.department &&
                   employee_number == other.employee_number;
        }
    };

    // CUSTOM HASHER specifically designed for EmployeeID
    // Uses domain knowledge: department has limited range, employee_number is more distributed
    template <hashing::Hash32or64 HashType, HashType Seed = hashing::constants::FNV_OFFSET_BASIS_32>
    struct EmployeeIDHasher
    {
        using result_type = HashType;

        HashType operator()( const EmployeeID& id ) const noexcept
        {
            // Custom hashing strategy optimized for EmployeeID structure:
            // 1. Department is limited range (16 bits) - use as high-order bits
            // 2. Employee number has full 32-bit distribution - use multiplicative hash

            // Combine department and employee_number using bit rotation and XOR
            uint64_t combined = ( static_cast<uint64_t>( id.department ) << 32 ) |
                                static_cast<uint64_t>( id.employee_number );

            // Apply FNV-1a-like mixing with custom seed
            HashType hash = Seed;

            // Mix in 8 bytes of combined value
            for ( int i = 0; i < 8; ++i )
            {
                uint8_t byte = static_cast<uint8_t>( ( combined >> ( i * 8 ) ) & 0xFF );
                hash ^= byte;

                if constexpr ( sizeof( HashType ) == 4 )
                {
                    hash *= 16777619u; // FNV prime 32-bit
                }
                else
                {
                    hash *= 1099511628211ull; // FNV prime 64-bit
                }
            }

            return hash;
        }
    };

    TEST( PerfectHashMapTests, CustomType_CustomHasher_EmployeeID )
    {
        EmployeeID emp1{ 10, 12345 }; // Department 10, Employee #12345
        EmployeeID emp2{ 10, 67890 }; // Same department, different employee
        EmployeeID emp3{ 20, 12345 }; // Different department, same employee number
        EmployeeID emp4{ 50, 99999 }; // Department 50, Employee #99999
        EmployeeID emp5{ 100, 1 };    // Department 100, Employee #1

        std::vector<std::pair<EmployeeID, std::string>> data;
        data.emplace_back( emp1, "Alice Johnson" );
        data.emplace_back( emp2, "Bob Smith" );
        data.emplace_back( emp3, "Charlie Brown" );
        data.emplace_back( emp4, "Diana Prince" );
        data.emplace_back( emp5, "Eve Wilson" );

        // Template params: <Key, Value, HashType, Seed, CustomHasher>
        PerfectHashMap<EmployeeID, std::string, uint32_t, 0x811c9dc5, EmployeeIDHasher<uint32_t, 0x811c9dc5>> map( std::move( data ) );

        EXPECT_EQ( map.at( emp1 ), "Alice Johnson" );
        EXPECT_EQ( map.at( emp2 ), "Bob Smith" );
        EXPECT_EQ( map.at( emp3 ), "Charlie Brown" );
        EXPECT_EQ( map.at( emp4 ), "Diana Prince" );
        EXPECT_EQ( map.at( emp5 ), "Eve Wilson" );
        EXPECT_EQ( map.count(), 5 );

        EXPECT_TRUE( map.contains( emp1 ) );
        EXPECT_TRUE( map.contains( emp2 ) );
        EXPECT_TRUE( map.contains( emp3 ) );
        EXPECT_TRUE( map.contains( emp4 ) );
        EXPECT_TRUE( map.contains( emp5 ) );

        EmployeeID emp_missing{ 999, 999999 };
        EXPECT_FALSE( map.contains( emp_missing ) );
        EXPECT_EQ( map.find( emp_missing ), nullptr );

        // Verify hash_function() returns our custom hasher
        auto hasher = map.hash_function();
        uint32_t hash1 = hasher( emp1 );
        uint32_t hash2 = hasher( emp2 );

        // Different employees should produce different hashes
        EXPECT_NE( hash1, hash2 );
    }

    TEST( PerfectHashMapTests, CustomType_CustomHasher_64bit )
    {
        // Same custom type, but with 64-bit hasher variant
        EmployeeID emp1{ 1, 1000 };
        EmployeeID emp2{ 2, 2000 };
        EmployeeID emp3{ 3, 3000 };

        std::vector<std::pair<EmployeeID, int>> data;
        data.emplace_back( emp1, 100 );
        data.emplace_back( emp2, 200 );
        data.emplace_back( emp3, 300 );

        // Custom hasher with 64-bit hash type and custom seed
        PerfectHashMap<EmployeeID, int, uint64_t, 0xcbf29ce484222325ull,
            EmployeeIDHasher<uint64_t, 0xcbf29ce484222325ull>>
            map( std::move( data ) );

        EXPECT_EQ( map.at( emp1 ), 100 );
        EXPECT_EQ( map.at( emp2 ), 200 );
        EXPECT_EQ( map.at( emp3 ), 300 );

        auto hasher = map.hash_function();
        uint64_t hash1 = hasher( emp1 );
        uint64_t hash2 = hasher( emp2 );

        EXPECT_NE( hash1, hash2 );
    }

    // Bonus: Custom hasher with domain-specific optimization
    struct IPAddress
    {
        uint8_t octet1, octet2, octet3, octet4;

        bool operator==( const IPAddress& other ) const noexcept
        {
            return octet1 == other.octet1 && octet2 == other.octet2 &&
                   octet3 == other.octet3 && octet4 == other.octet4;
        }
    };

    // Custom hasher for IP addresses - treats as 32-bit integer
    template <hashing::Hash32or64 HashType, HashType Seed = hashing::constants::FNV_OFFSET_BASIS_32>
    struct IPAddressHasher
    {
        using result_type = HashType;

        HashType operator()( const IPAddress& ip ) const noexcept
        {
            // Pack IP octets into 32-bit value: octet1.octet2.octet3.octet4
            uint32_t packed = ( static_cast<uint32_t>( ip.octet1 ) << 24 ) |
                              ( static_cast<uint32_t>( ip.octet2 ) << 16 ) |
                              ( static_cast<uint32_t>( ip.octet3 ) << 8 ) |
                              static_cast<uint32_t>( ip.octet4 );

            // Use multiplicative hashing (Knuth's method)
            // Golden ratio multiplier for good distribution
            constexpr uint64_t GOLDEN_RATIO = 0x9e3779b97f4a7c15ull;

            if constexpr ( sizeof( HashType ) == 4 )
            {
                // 32-bit: multiply and XOR with seed
                uint64_t hash64 = static_cast<uint64_t>( packed ) * static_cast<uint32_t>( GOLDEN_RATIO );
                return static_cast<uint32_t>( hash64 >> 32 ) ^ static_cast<uint32_t>( Seed );
            }
            else
            {
                // 64-bit: full multiplication with seed mixing
                uint64_t hash = static_cast<uint64_t>( packed ) * GOLDEN_RATIO;
                return hash ^ Seed;
            }
        }
    };

    TEST( PerfectHashMapTests, CustomType_CustomHasher_IPAddress )
    {
        IPAddress ip1{ 192, 168, 1, 1 };   // 192.168.1.1
        IPAddress ip2{ 192, 168, 1, 100 }; // 192.168.1.100
        IPAddress ip3{ 10, 0, 0, 1 };      // 10.0.0.1
        IPAddress ip4{ 172, 16, 0, 1 };    // 172.16.0.1
        IPAddress ip5{ 8, 8, 8, 8 };       // 8.8.8.8 (Google DNS)

        std::vector<std::pair<IPAddress, std::string>> data;
        data.emplace_back( ip1, "router" );
        data.emplace_back( ip2, "workstation" );
        data.emplace_back( ip3, "gateway" );
        data.emplace_back( ip4, "server" );
        data.emplace_back( ip5, "dns" );

        // Custom IP address hasher with PerfectHashMap
        PerfectHashMap<IPAddress, std::string, uint32_t, 0x12345678,
            IPAddressHasher<uint32_t, 0x12345678>>
            map( std::move( data ) );

        EXPECT_EQ( map.at( ip1 ), "router" );
        EXPECT_EQ( map.at( ip2 ), "workstation" );
        EXPECT_EQ( map.at( ip3 ), "gateway" );
        EXPECT_EQ( map.at( ip4 ), "server" );
        EXPECT_EQ( map.at( ip5 ), "dns" );
        EXPECT_EQ( map.count(), 5 );

        IPAddress ip_missing{ 255, 255, 255, 255 };
        EXPECT_FALSE( map.contains( ip_missing ) );
    }
} // namespace nfx::containers::test
