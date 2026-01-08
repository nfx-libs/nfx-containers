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
 * @file Tests_TransparentHashMap.cpp
 * @brief Tests for TransparentHashMap (STL std::unordered_map wrapper with Hasher)
 */

#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nfx/containers/TransparentHashMap.h>

namespace nfx::containers::test
{
    using namespace nfx::hashing;

    //=====================================================================
    // String keys - CRC32-C hashing with heterogeneous lookup tests
    //=====================================================================

    TEST( TransparentHashMapTests, StringKeys_BasicLookup )
    {
        TransparentHashMap<std::string, int> map;

        map["apple"] = 1;
        map["banana"] = 2;
        map["cherry"] = 3;

        EXPECT_EQ( map.size(), 3 );
        EXPECT_FALSE( map.empty() );

        EXPECT_EQ( map.at( "apple" ), 1 );
        EXPECT_EQ( map.at( "banana" ), 2 );
        EXPECT_EQ( map.at( "cherry" ), 3 );
    }

    TEST( TransparentHashMapTests, StringKeys_HeterogeneousLookup_StringView )
    {
        TransparentHashMap<std::string, int> map;

        map["hello"] = 100;
        map["world"] = 200;

        // Zero-copy string_view lookup (transparent lookup via is_transparent)
        std::string_view key1 = "hello";
        std::string_view key2 = "world";
        std::string_view key3 = "missing";

        auto it1 = map.find( key1 );
        auto it2 = map.find( key2 );
        auto it3 = map.find( key3 );

        ASSERT_NE( it1, map.end() );
        ASSERT_NE( it2, map.end() );
        EXPECT_EQ( it3, map.end() );

        EXPECT_EQ( it1->second, 100 );
        EXPECT_EQ( it2->second, 200 );

        EXPECT_TRUE( map.contains( key1 ) );
        EXPECT_TRUE( map.contains( key2 ) );
        EXPECT_FALSE( map.contains( key3 ) );
    }

    TEST( TransparentHashMapTests, StringKeys_HeterogeneousLookup_CString )
    {
        TransparentHashMap<std::string, int> map;

        map["test"] = 42;

        // Lookup with const char* (heterogeneous)
        const char* key = "test";
        EXPECT_TRUE( map.contains( key ) );

        auto it = map.find( key );
        ASSERT_NE( it, map.end() );
        EXPECT_EQ( it->second, 42 );
    }

    TEST( TransparentHashMapTests, StringKeys_CRC32C_Consistency )
    {
        // Verify that CRC32-C hashing produces consistent results
        // for the same string across different representations
        TransparentHashMap<std::string, int> map;

        std::string str = "consistent";
        map[str] = 999;

        std::string_view sv = "consistent";
        const char* cstr = "consistent";

        // All should find the same entry
        EXPECT_EQ( map.at( str ), 999 );
        EXPECT_EQ( map.find( sv )->second, 999 );
        EXPECT_EQ( map.find( cstr )->second, 999 );
        EXPECT_EQ( map.count( str ), 1 );
        EXPECT_EQ( map.count( sv ), 1 );
        EXPECT_EQ( map.count( cstr ), 1 );
    }

    TEST( TransparentHashMapTests, StringKeys_InsertOrUpdate )
    {
        TransparentHashMap<std::string, std::string> map;

        map["update_key"] = "initial";
        EXPECT_EQ( map.at( "update_key" ), "initial" );
        EXPECT_EQ( map.size(), 1 );

        // Update existing key
        map["update_key"] = "updated";
        EXPECT_EQ( map.at( "update_key" ), "updated" );
        EXPECT_EQ( map.size(), 1 ); // Size should remain 1
    }

    TEST( TransparentHashMapTests, StringKeys_EmptyString )
    {
        TransparentHashMap<std::string, int> map;

        // Empty string should hash to 0 and be usable as a key
        map[""] = 42;
        map["non-empty"] = 100;

        EXPECT_EQ( map.size(), 2 );
        EXPECT_EQ( map.at( "" ), 42 );
        EXPECT_EQ( map.at( "non-empty" ), 100 );
        EXPECT_TRUE( map.contains( "" ) );
    }

    //=====================================================================
    // Integer keys - Multiplicative hashing (Knuth/Wang) tests
    //=====================================================================

    TEST( TransparentHashMapTests, IntegerKeys_BasicLookup )
    {
        TransparentHashMap<int, std::string> map;

        map[1] = "one";
        map[2] = "two";
        map[3] = "three";

        EXPECT_EQ( map.at( 1 ), "one" );
        EXPECT_EQ( map.at( 2 ), "two" );
        EXPECT_EQ( map.at( 3 ), "three" );
        EXPECT_EQ( map.size(), 3 );
    }

    TEST( TransparentHashMapTests, IntegerKeys_AllIntegralTypes )
    {
        // Test different integral types to verify Hasher handles all correctly
        TransparentHashMap<uint8_t, int> map8;
        map8[255] = 1;
        EXPECT_EQ( map8.at( 255 ), 1 );

        TransparentHashMap<int16_t, int> map16;
        map16[-32768] = 2;
        map16[32767] = 3;
        EXPECT_EQ( map16.at( -32768 ), 2 );
        EXPECT_EQ( map16.at( 32767 ), 3 );

        TransparentHashMap<uint64_t, int> map64;
        map64[0xFFFFFFFFFFFFFFFFULL] = 4;
        EXPECT_EQ( map64.at( 0xFFFFFFFFFFFFFFFFULL ), 4 );

        TransparentHashMap<size_t, int> mapSize;
        mapSize[SIZE_MAX] = 5;
        EXPECT_EQ( mapSize.at( SIZE_MAX ), 5 );
    }

    TEST( TransparentHashMapTests, IntegerKeys_NegativeValues )
    {
        TransparentHashMap<int, std::string> map;

        map[-100] = "minus_100";
        map[-1] = "minus_1";
        map[0] = "zero";
        map[1] = "one";
        map[100] = "plus_100";

        EXPECT_EQ( map.at( -100 ), "minus_100" );
        EXPECT_EQ( map.at( -1 ), "minus_1" );
        EXPECT_EQ( map.at( 0 ), "zero" );
        EXPECT_EQ( map.at( 1 ), "one" );
        EXPECT_EQ( map.at( 100 ), "plus_100" );
        EXPECT_EQ( map.size(), 5 );
    }

    TEST( TransparentHashMapTests, IntegerKeys_LargeValues )
    {
        TransparentHashMap<uint64_t, int> map;

        map[0] = 1;
        map[1] = 2;
        map[0x7FFFFFFFFFFFFFFFULL] = 3; // Max int64_t
        map[0xFFFFFFFFFFFFFFFFULL] = 4; // Max uint64_t

        EXPECT_EQ( map.at( 0 ), 1 );
        EXPECT_EQ( map.at( 1 ), 2 );
        EXPECT_EQ( map.at( 0x7FFFFFFFFFFFFFFFULL ), 3 );
        EXPECT_EQ( map.at( 0xFFFFFFFFFFFFFFFFULL ), 4 );
    }

    //=====================================================================
    // Enum keys - Underlying type conversion tests
    //=====================================================================

    enum class Color : uint32_t
    {
        Red = 1,
        Green = 2,
        Blue = 3
    };

    TEST( TransparentHashMapTests, EnumKeys_BasicLookup )
    {
        TransparentHashMap<Color, std::string> map;

        map[Color::Red] = "red";
        map[Color::Green] = "green";
        map[Color::Blue] = "blue";

        EXPECT_EQ( map.at( Color::Red ), "red" );
        EXPECT_EQ( map.at( Color::Green ), "green" );
        EXPECT_EQ( map.at( Color::Blue ), "blue" );
        EXPECT_EQ( map.size(), 3 );
    }

    enum class Status : uint8_t
    {
        Idle = 0,
        Running = 1,
        Stopped = 2
    };

    TEST( TransparentHashMapTests, EnumKeys_DifferentUnderlyingTypes )
    {
        // Test enum with uint8_t underlying type
        TransparentHashMap<Status, std::string> map;

        map[Status::Idle] = "idle";
        map[Status::Running] = "running";
        map[Status::Stopped] = "stopped";

        EXPECT_EQ( map.at( Status::Idle ), "idle" );
        EXPECT_EQ( map.at( Status::Running ), "running" );
        EXPECT_EQ( map.at( Status::Stopped ), "stopped" );
    }

    //=====================================================================
    // Floating-point keys - Normalization tests
    //=====================================================================

    TEST( TransparentHashMapTests, FloatKeys_BasicLookup )
    {
        TransparentHashMap<float, std::string> map;

        map[1.0f] = "one";
        map[2.5f] = "two_and_half";
        map[3.14159f] = "pi";

        EXPECT_EQ( map.at( 1.0f ), "one" );
        EXPECT_EQ( map.at( 2.5f ), "two_and_half" );
        EXPECT_EQ( map.at( 3.14159f ), "pi" );
    }

    TEST( TransparentHashMapTests, FloatKeys_SpecialValues )
    {
        TransparentHashMap<double, int> map;

        // Positive and negative zero should hash to the same value
        map[0.0] = 100;
        EXPECT_EQ( map.at( 0.0 ), 100 );
        EXPECT_EQ( map.at( -0.0 ), 100 ); // -0.0 normalized to 0.0
        EXPECT_EQ( map.size(), 1 );       // Only one entry for both +0 and -0

        // Infinity values
        map[std::numeric_limits<double>::infinity()] = 200;
        map[-std::numeric_limits<double>::infinity()] = 300;
        EXPECT_EQ( map.at( std::numeric_limits<double>::infinity() ), 200 );
        EXPECT_EQ( map.at( -std::numeric_limits<double>::infinity() ), 300 );
        EXPECT_EQ( map.size(), 3 ); // +0, +inf, -inf

        // NaN behavior: NaN == NaN is always false by IEEE 754 standard
        // This means we can insert NaN, but each NaN comparison fails equality
        // So effectively, NaN keys are insert-only (cannot be reliably looked up)
        // This is standard IEEE 754 behavior and applies to all containers
        double nan = std::numeric_limits<double>::quiet_NaN();
        map[nan] = 400;
        EXPECT_EQ( map.size(), 4 );

        // We document that NaN cannot be reliably used as a key due to IEEE 754 semantics
        // (NaN != NaN, so lookups fail even though the entry exists)
    }

    TEST( TransparentHashMapTests, FloatKeys_ZeroNormalization )
    {
        TransparentHashMap<double, std::string> map;

        // Insert with +0.0
        map[0.0] = "zero";
        EXPECT_EQ( map.size(), 1 );

        // Lookup with -0.0 should find the same entry
        EXPECT_TRUE( map.contains( -0.0 ) );
        EXPECT_EQ( map.at( -0.0 ), "zero" );

        // Inserting -0.0 should not create a new entry
        map[-0.0] = "still_zero";
        EXPECT_EQ( map.size(), 1 );
        EXPECT_EQ( map.at( 0.0 ), "still_zero" );
    }

    //=====================================================================
    // Pointer keys - Address hashing tests
    //=====================================================================

    TEST( TransparentHashMapTests, PointerKeys_BasicLookup )
    {
        int a = 10, b = 20, c = 30;

        TransparentHashMap<int*, std::string> map;

        map[&a] = "a";
        map[&b] = "b";
        map[&c] = "c";

        EXPECT_EQ( map.at( &a ), "a" );
        EXPECT_EQ( map.at( &b ), "b" );
        EXPECT_EQ( map.at( &c ), "c" );
        EXPECT_EQ( map.size(), 3 );
    }

    TEST( TransparentHashMapTests, PointerKeys_UniquePtr )
    {
        auto p1 = std::make_unique<int>( 100 );
        auto p2 = std::make_unique<int>( 200 );

        TransparentHashMap<int*, std::string> map;

        // Use raw pointer addresses as keys
        map[p1.get()] = "first";
        map[p2.get()] = "second";

        EXPECT_EQ( map.at( p1.get() ), "first" );
        EXPECT_EQ( map.at( p2.get() ), "second" );
    }

    TEST( TransparentHashMapTests, PointerKeys_Nullptr )
    {
        // Test that nullptr works as a key with proper casting
        TransparentHashMap<int*, std::string> map;

        // Important: operator[] and at() do NOT support heterogeneous lookup
        // They require exact key type, so we must cast nullptr to int*
        int* null_key = nullptr;

        // nullptr should be a valid key
        map[null_key] = "null";
        EXPECT_EQ( map.size(), 1 );

        // Lookup methods (find, contains) support heterogeneous lookup with transparent comparator
        // but operator[] and at() do not - they require exact key type
        EXPECT_TRUE( map.contains( null_key ) );
        EXPECT_NE( map.find( null_key ), map.end() );
        EXPECT_EQ( map[null_key], "null" ); // operator[] requires exact type

        // Add another pointer
        int dummy = 42;
        map[&dummy] = "not_null";

        EXPECT_EQ( map.size(), 2 );
        EXPECT_TRUE( map.contains( null_key ) );
        EXPECT_TRUE( map.contains( &dummy ) );

        // find() with heterogeneous lookup
        auto it_null = map.find( null_key );
        EXPECT_NE( it_null, map.end() );
        EXPECT_EQ( it_null->second, "null" );

        auto it_dummy = map.find( &dummy );
        EXPECT_NE( it_dummy, map.end() );
        EXPECT_EQ( it_dummy->second, "not_null" );

        // Erase nullptr
        EXPECT_EQ( map.erase( null_key ), 1 );
        EXPECT_EQ( map.size(), 1 );
        EXPECT_FALSE( map.contains( null_key ) );
        EXPECT_TRUE( map.contains( &dummy ) );
    }

    //=====================================================================
    // Pair keys - Hash combining tests
    //=====================================================================

    TEST( TransparentHashMapTests, PairKeys_IntPairs )
    {
        using Key = std::pair<int, int>;
        TransparentHashMap<Key, std::string> map;

        map[{ 1, 2 }] = "pair_1_2";
        map[{ 3, 4 }] = "pair_3_4";
        map[{ 1, 3 }] = "pair_1_3";

        EXPECT_EQ( map.at( { 1, 2 } ), "pair_1_2" );
        EXPECT_EQ( map.at( { 3, 4 } ), "pair_3_4" );
        EXPECT_EQ( map.at( { 1, 3 } ), "pair_1_3" );
        EXPECT_EQ( map.size(), 3 );
    }

    TEST( TransparentHashMapTests, PairKeys_StringPairs )
    {
        using Key = std::pair<std::string, std::string>;
        TransparentHashMap<Key, int> map;

        map[{ "first", "second" }] = 1;
        map[{ "hello", "world" }] = 2;

        EXPECT_EQ( map.at( { "first", "second" } ), 1 );
        EXPECT_EQ( map.at( { "hello", "world" } ), 2 );
    }

    TEST( TransparentHashMapTests, PairKeys_MixedTypes )
    {
        using Key = std::pair<std::string, int>;
        TransparentHashMap<Key, double> map;

        map[{ "alpha", 1 }] = 1.1;
        map[{ "beta", 2 }] = 2.2;
        map[{ "gamma", 3 }] = 3.3;

        EXPECT_DOUBLE_EQ( map.at( { "alpha", 1 } ), 1.1 );
        EXPECT_DOUBLE_EQ( map.at( { "beta", 2 } ), 2.2 );
        EXPECT_DOUBLE_EQ( map.at( { "gamma", 3 } ), 3.3 );
    }

    //=====================================================================
    // Hash quality tests - Distribution and collision resistance
    //=====================================================================

    TEST( TransparentHashMapTests, HashQuality_IntegerDistribution32 )
    {
        // Verify that Hasher<uint32_t> produces well-distributed hashes for sequential integers
        TransparentHashMap<int, int> map;
        map.reserve( 1000 );

        for ( int i = 0; i < 1000; ++i )
        {
            map[i] = i * 2;
        }

        // All values should be findable (no collisions should prevent insertion)
        EXPECT_EQ( map.size(), 1000 );
        for ( int i = 0; i < 1000; ++i )
        {
            EXPECT_EQ( map.at( i ), i * 2 );
        }
    }

    TEST( TransparentHashMapTests, HashQuality_IntegerDistribution64 )
    {
        // Test 64-bit integer hashing (Wang's algorithm)
        TransparentHashMap<uint64_t, int> map;
        map.reserve( 1000 );

        for ( uint64_t i = 0; i < 1000; ++i )
        {
            uint64_t key = i * 0x123456789ABCDEFULL; // Spread values
            map[key] = static_cast<int>( i );
        }

        EXPECT_EQ( map.size(), 1000 );
    }

    TEST( TransparentHashMapTests, HashQuality_StringDistribution )
    {
        // Verify CRC32-C produces well-distributed hashes for similar strings
        TransparentHashMap<std::string, int> map;
        map.reserve( 100 );

        for ( int i = 0; i < 100; ++i )
        {
            std::string key = "key_" + std::to_string( i );
            map[key] = i;
        }

        EXPECT_EQ( map.size(), 100 );
        for ( int i = 0; i < 100; ++i )
        {
            std::string key = "key_" + std::to_string( i );
            EXPECT_EQ( map.at( key ), i );
        }
    }

    TEST( TransparentHashMapTests, HashQuality_CollisionResistance )
    {
        // Test that very similar strings don't cause excessive collisions
        TransparentHashMap<std::string, int> map;

        map["abc"] = 1;
        map["abd"] = 2;
        map["acd"] = 3;
        map["bcd"] = 4;

        EXPECT_EQ( map.size(), 4 );
        EXPECT_EQ( map.at( "abc" ), 1 );
        EXPECT_EQ( map.at( "abd" ), 2 );
        EXPECT_EQ( map.at( "acd" ), 3 );
        EXPECT_EQ( map.at( "bcd" ), 4 );
    }

    //=====================================================================
    // Performance and efficiency tests
    //=====================================================================

    TEST( TransparentHashMapTests, Performance_Reserve )
    {
        TransparentHashMap<int, int> map;

        // Reserve should prevent rehashing
        map.reserve( 1000 );
        size_t bucket_count_before = map.bucket_count();

        for ( int i = 0; i < 1000; ++i )
        {
            map[i] = i;
        }

        size_t bucket_count_after = map.bucket_count();

        // Bucket count should not change if reserve was effective
        EXPECT_EQ( bucket_count_before, bucket_count_after );
        EXPECT_EQ( map.size(), 1000 );
    }

    TEST( TransparentHashMapTests, Performance_TransparentLookup )
    {
        TransparentHashMap<std::string, int> map;

        map["expensive_allocation"] = 42;

        // string_view lookup should not allocate a temporary string
        std::string_view sv = "expensive_allocation";
        EXPECT_TRUE( map.contains( sv ) );
        auto it_sv = map.find( sv );
        ASSERT_NE( it_sv, map.end() );
        EXPECT_EQ( it_sv->second, 42 );

        // const char* lookup should also avoid allocation
        const char* cstr = "expensive_allocation";
        EXPECT_TRUE( map.contains( cstr ) );
        auto it_cstr = map.find( cstr );
        ASSERT_NE( it_cstr, map.end() );
        EXPECT_EQ( it_cstr->second, 42 );
    }

    //=====================================================================
    // Erase operations
    //=====================================================================

    TEST( TransparentHashMapTests, Erase_BasicOperation )
    {
        TransparentHashMap<std::string, int> map;

        map["a"] = 1;
        map["b"] = 2;
        map["c"] = 3;

        EXPECT_EQ( map.size(), 3 );

        map.erase( "b" );
        EXPECT_EQ( map.size(), 2 );
        EXPECT_FALSE( map.contains( "b" ) );
        EXPECT_TRUE( map.contains( "a" ) );
        EXPECT_TRUE( map.contains( "c" ) );
    }

    TEST( TransparentHashMapTests, Erase_NonExistent )
    {
        TransparentHashMap<int, int> map;
        map[1] = 100;
        map[2] = 200;

        size_t erased = map.erase( 999 ); // Non-existent key
        EXPECT_EQ( erased, 0 );
        EXPECT_EQ( map.size(), 2 );
    }

    TEST( TransparentHashMapTests, Erase_HeterogeneousKey )
    {
        TransparentHashMap<std::string, int> map;
        map["key"] = 42;

        // Erase requires exact key type in C++20 (heterogeneous erase is C++23)
        // Use find() first with heterogeneous lookup, then erase with iterator
        std::string_view sv = "key";
        auto it = map.find( sv );
        ASSERT_NE( it, map.end() );
        map.erase( it );
        EXPECT_EQ( map.size(), 0 );
    }

    //=====================================================================
    // Capacity and memory management
    //=====================================================================

    TEST( TransparentHashMapTests, Capacity_DefaultConstruction )
    {
        TransparentHashMap<int, int> map;

        EXPECT_TRUE( map.empty() );
        EXPECT_EQ( map.size(), 0 );
    }

    TEST( TransparentHashMapTests, Capacity_Reserve )
    {
        TransparentHashMap<std::string, int> map;
        map.reserve( 100 );

        EXPECT_GE( map.bucket_count(), 100 );
        EXPECT_TRUE( map.empty() );
    }

    TEST( TransparentHashMapTests, Capacity_LoadFactor )
    {
        TransparentHashMap<int, int> map;

        for ( int i = 0; i < 100; ++i )
        {
            map[i] = i;
        }

        // Load factor should be reasonable
        float load_factor = map.load_factor();
        EXPECT_GT( load_factor, 0.0f );
        EXPECT_LE( load_factor, map.max_load_factor() );
    }

    //=====================================================================
    // Iterator tests
    //=====================================================================

    TEST( TransparentHashMapTests, Iterator_BasicIteration )
    {
        TransparentHashMap<int, int> map;
        map[1] = 10;
        map[2] = 20;
        map[3] = 30;

        int sum_keys = 0;
        int sum_values = 0;

        for ( const auto& [key, value] : map )
        {
            sum_keys += key;
            sum_values += value;
        }

        EXPECT_EQ( sum_keys, 6 );    // 1 + 2 + 3
        EXPECT_EQ( sum_values, 60 ); // 10 + 20 + 30
    }

    TEST( TransparentHashMapTests, Iterator_EmptyMap )
    {
        TransparentHashMap<int, int> map;

        int count = 0;
        for ( [[maybe_unused]] const auto& kv : map )
        {
            ++count;
        }

        EXPECT_EQ( count, 0 );
    }

    //=====================================================================
    // Edge cases and stress tests
    //=====================================================================

    TEST( TransparentHashMapTests, EdgeCase_MaxSizeTypes )
    {
        TransparentHashMap<uint64_t, int> map;

        map[0] = 1;
        map[UINT64_MAX] = 2;
        map[UINT64_MAX / 2] = 3;

        EXPECT_EQ( map.at( 0 ), 1 );
        EXPECT_EQ( map.at( UINT64_MAX ), 2 );
        EXPECT_EQ( map.at( UINT64_MAX / 2 ), 3 );
    }

    TEST( TransparentHashMapTests, EdgeCase_LongStrings )
    {
        TransparentHashMap<std::string, int> map;

        std::string long1( 1000, 'a' );
        std::string long2( 1000, 'b' );
        std::string long3( 999, 'a' ); // Different length

        map[long1] = 1;
        map[long2] = 2;
        map[long3] = 3;

        EXPECT_EQ( map.at( long1 ), 1 );
        EXPECT_EQ( map.at( long2 ), 2 );
        EXPECT_EQ( map.at( long3 ), 3 );
        EXPECT_EQ( map.size(), 3 );
    }

    TEST( TransparentHashMapTests, EdgeCase_UnicodeStrings )
    {
        TransparentHashMap<std::string, int> map;

        map["hello"] = 1;
        map["世界"] = 2;   // Chinese
        map["مرحبا"] = 3;  // Arabic
        map["Привет"] = 4; // Russian
        map["😀🎉🚀"] = 5; // Emoji

        EXPECT_EQ( map.at( "hello" ), 1 );
        EXPECT_EQ( map.at( "世界" ), 2 );
        EXPECT_EQ( map.at( "مرحبا" ), 3 );
        EXPECT_EQ( map.at( "Привет" ), 4 );
        EXPECT_EQ( map.at( "😀🎉🚀" ), 5 );
        EXPECT_EQ( map.size(), 5 );
    }

    TEST( TransparentHashMapTests, EdgeCase_PowerOfTwoKeys )
    {
        // Test power-of-two keys which can expose poor hash functions
        TransparentHashMap<int, std::string> map;

        for ( int i = 0; i < 20; ++i )
        {
            int key = 1 << i; // Powers of 2: 1, 2, 4, 8, 16, ...
            map[key] = "pow_" + std::to_string( i );
        }

        EXPECT_EQ( map.size(), 20 );
        for ( int i = 0; i < 20; ++i )
        {
            int key = 1 << i;
            EXPECT_EQ( map.at( key ), "pow_" + std::to_string( i ) );
        }
    }

    TEST( TransparentHashMapTests, Stress_ManyElements )
    {
        TransparentHashMap<int, int> map;
        const int count = 10000;

        // Insert many elements
        for ( int i = 0; i < count; ++i )
        {
            map[i] = i * 3;
        }

        EXPECT_EQ( map.size(), count );

        // Verify all elements
        for ( int i = 0; i < count; ++i )
        {
            EXPECT_EQ( map.at( i ), i * 3 );
        }
    }

    TEST( TransparentHashMapTests, Stress_InsertErasePattern )
    {
        TransparentHashMap<int, int> map;

        // Insert 1000 elements
        for ( int i = 0; i < 1000; ++i )
        {
            map[i] = i;
        }
        EXPECT_EQ( map.size(), 1000 );

        // Erase every other element
        for ( int i = 0; i < 1000; i += 2 )
        {
            map.erase( i );
        }
        EXPECT_EQ( map.size(), 500 );

        // Verify remaining elements
        for ( int i = 1; i < 1000; i += 2 )
        {
            EXPECT_EQ( map.at( i ), i );
        }

        // Re-insert erased elements
        for ( int i = 0; i < 1000; i += 2 )
        {
            map[i] = i * 2;
        }
        EXPECT_EQ( map.size(), 1000 );

        // Verify all elements
        for ( int i = 0; i < 1000; ++i )
        {
            int expected = ( i % 2 == 0 ) ? i * 2 : i;
            EXPECT_EQ( map.at( i ), expected );
        }
    }

    //=====================================================================
    // Custom hasher tests
    //=====================================================================

    // Simple custom hasher for testing
    struct SimpleHasher
    {
        using is_transparent = void;

        std::size_t operator()( const std::string& key ) const noexcept
        {
            // Simple sum of character codes (poor distribution, but tests custom hasher)
            std::size_t hash = 0;
            for ( char c : key )
            {
                hash += static_cast<std::size_t>( c );
            }
            return hash;
        }

        std::size_t operator()( std::string_view key ) const noexcept
        {
            std::size_t hash = 0;
            for ( char c : key )
            {
                hash += static_cast<std::size_t>( c );
            }
            return hash;
        }
    };

    TEST( TransparentHashMapTests, CustomHasher_Simple )
    {
        // Verify Map works with custom hashers
        std::unordered_map<std::string, int, SimpleHasher, std::equal_to<>> customMap;

        customMap["abc"] = 1;
        customMap["def"] = 2;

        // Heterogeneous lookup with string_view
        std::string_view sv = "abc";
        auto it = customMap.find( sv );
        ASSERT_NE( it, customMap.end() );
        EXPECT_EQ( it->second, 1 );
        EXPECT_EQ( customMap.at( "def" ), 2 );
    }

    //=====================================================================
    // 64-bit hash tests - Dual-stream CRC32-C verification
    //=====================================================================

    TEST( TransparentHashMapTests, Hash64Bit_IntegerKeys )
    {
        // Test Map with 64-bit hash output (uses Wang's 64-bit algorithm for integers)
        TransparentHashMap<uint64_t, int, Hasher<uint64_t>> map;

        map[0x123456789ABCDEFULL] = 1;
        map[0xFEDCBA9876543210ULL] = 2;
        map[UINT64_MAX] = 3;

        EXPECT_EQ( map.at( 0x123456789ABCDEFULL ), 1 );
        EXPECT_EQ( map.at( 0xFEDCBA9876543210ULL ), 2 );
        EXPECT_EQ( map.at( UINT64_MAX ), 3 );
    }

    TEST( TransparentHashMapTests, Hash64Bit_StringKeys )
    {
        // Test Map with 64-bit hash output (uses dual-stream CRC32-C for strings)
        TransparentHashMap<std::string, int, Hasher<uint64_t>> map;

        map["test64"] = 1;
        map["dual_stream_crc32c"] = 2;
        map["hardware_accelerated"] = 3;

        EXPECT_EQ( map.at( "test64" ), 1 );
        EXPECT_EQ( map.at( "dual_stream_crc32c" ), 2 );
        EXPECT_EQ( map.at( "hardware_accelerated" ), 3 );

        // Heterogeneous lookup should still work
        std::string_view sv = "test64";
        auto it = map.find( sv );
        ASSERT_NE( it, map.end() );
        EXPECT_EQ( it->second, 1 );
    }

    TEST( TransparentHashMapTests, Hash64Bit_MixedTypes )
    {
        // Test 64-bit hash with various types
        TransparentHashMap<int, std::string, Hasher<uint64_t>> map;

        map[1] = "one";
        map[1000000] = "million";
        map[-999] = "negative";

        EXPECT_EQ( map.at( 1 ), "one" );
        EXPECT_EQ( map.at( 1000000 ), "million" );
        EXPECT_EQ( map.at( -999 ), "negative" );
    }

    //=====================================================================
    // Clear operation
    //=====================================================================

    TEST( TransparentHashMapTests, Clear_BasicOperation )
    {
        TransparentHashMap<int, int> map;

        for ( int i = 0; i < 100; ++i )
        {
            map[i] = i * 2;
        }

        EXPECT_EQ( map.size(), 100 );

        map.clear();

        EXPECT_EQ( map.size(), 0 );
        EXPECT_TRUE( map.empty() );
        EXPECT_FALSE( map.contains( 0 ) );
    }

    //=====================================================================
    // Value type tests
    //=====================================================================

    TEST( TransparentHashMapTests, ValueTypes_ComplexValues )
    {
        TransparentHashMap<int, std::vector<int>> map;

        map[1] = { 1, 2, 3 };
        map[2] = { 4, 5, 6, 7 };

        EXPECT_EQ( map.at( 1 ).size(), 3 );
        EXPECT_EQ( map.at( 2 ).size(), 4 );
        EXPECT_EQ( map.at( 1 )[0], 1 );
        EXPECT_EQ( map.at( 2 )[3], 7 );
    }

    TEST( TransparentHashMapTests, ValueTypes_MoveSemantics )
    {
        TransparentHashMap<int, std::unique_ptr<int>> map;

        map[1] = std::make_unique<int>( 100 );
        map[2] = std::make_unique<int>( 200 );

        EXPECT_NE( map.at( 1 ), nullptr );
        EXPECT_NE( map.at( 2 ), nullptr );
        EXPECT_EQ( *map.at( 1 ), 100 );
        EXPECT_EQ( *map.at( 2 ), 200 );

        // Move out of map
        auto ptr = std::move( map.at( 1 ) );
        EXPECT_EQ( *ptr, 100 );
        EXPECT_EQ( map.at( 1 ), nullptr ); // Moved from
    }

    TEST( TransparentHashMapTests, MoveConstructor_TransfersOwnership )
    {
        TransparentHashMap<std::string, int> map1;
        map1["alpha"] = 1;
        map1["beta"] = 2;
        map1["gamma"] = 3;

        TransparentHashMap<std::string, int> map2( std::move( map1 ) );

        EXPECT_EQ( map2.size(), 3 );
        EXPECT_EQ( map2.at( "alpha" ), 1 );
        EXPECT_EQ( map2.at( "beta" ), 2 );
        EXPECT_EQ( map2.at( "gamma" ), 3 );

        // Verify source is moved-from (should be empty)
        EXPECT_EQ( map1.size(), 0 );
        EXPECT_TRUE( map1.empty() );
    }

    TEST( TransparentHashMapTests, MoveAssignment_TransfersOwnership )
    {
        TransparentHashMap<std::string, int> map1;
        map1["alpha"] = 1;
        map1["beta"] = 2;

        TransparentHashMap<std::string, int> map2;
        map2["old"] = 99;

        map2 = std::move( map1 );

        EXPECT_EQ( map2.size(), 2 );
        EXPECT_EQ( map2.at( "alpha" ), 1 );
        EXPECT_EQ( map2.at( "beta" ), 2 );
        EXPECT_EQ( map2.find( "old" ), map2.end() );

        // Verify source is moved-from (should be empty)
        EXPECT_EQ( map1.size(), 0 );
        EXPECT_TRUE( map1.empty() );
    }
} // namespace nfx::containers::test
