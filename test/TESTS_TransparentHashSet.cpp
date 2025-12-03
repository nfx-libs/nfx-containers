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
 * @file TESTS_TransparentHashSet.cpp
 * @brief Tests for TransparentHashSet (STL std::unordered_set wrapper with Hasher)
 */

#include <gtest/gtest.h>

#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nfx/containers/TransparentHashSet.h>

namespace nfx::containers::test
{
	using namespace nfx::hashing;

	//=====================================================================
	// String keys - CRC32-C hashing with heterogeneous lookup tests
	//=====================================================================

	TEST( TransparentHashSetTests, StringKeys_BasicInsert )
	{
		TransparentHashSet<std::string> set;

		auto [it1, inserted1] = set.insert( "apple" );
		auto [it2, inserted2] = set.insert( "banana" );
		auto [it3, inserted3] = set.insert( "cherry" );

		EXPECT_TRUE( inserted1 );
		EXPECT_TRUE( inserted2 );
		EXPECT_TRUE( inserted3 );
		EXPECT_EQ( set.size(), 3 );
		EXPECT_FALSE( set.empty() );

		// Duplicate insertion should fail
		auto [it4, inserted4] = set.insert( "apple" );
		EXPECT_FALSE( inserted4 );
		EXPECT_EQ( set.size(), 3 );
		EXPECT_EQ( *it4, "apple" );
	}

	TEST( TransparentHashSetTests, StringKeys_HeterogeneousLookup_StringView )
	{
		TransparentHashSet<std::string> set{ "hello", "world", "test" };

		// Zero-copy string_view lookup (transparent lookup via is_transparent)
		std::string_view key1 = "hello";
		std::string_view key2 = "world";
		std::string_view key3 = "missing";

		auto it1 = set.find( key1 );
		auto it2 = set.find( key2 );
		auto it3 = set.find( key3 );

		ASSERT_NE( it1, set.end() );
		ASSERT_NE( it2, set.end() );
		EXPECT_EQ( it3, set.end() );

		EXPECT_EQ( *it1, "hello" );
		EXPECT_EQ( *it2, "world" );

		EXPECT_TRUE( set.contains( key1 ) );
		EXPECT_TRUE( set.contains( key2 ) );
		EXPECT_FALSE( set.contains( key3 ) );
	}

	TEST( TransparentHashSetTests, StringKeys_HeterogeneousLookup_CString )
	{
		TransparentHashSet<std::string> set{ "test", "data" };

		// Lookup with const char* (heterogeneous)
		const char* key = "test";
		EXPECT_TRUE( set.contains( key ) );

		auto it = set.find( key );
		ASSERT_NE( it, set.end() );
		EXPECT_EQ( *it, "test" );
	}

	TEST( TransparentHashSetTests, StringKeys_CRC32C_Consistency )
	{
		// Verify that CRC32-C hashing produces consistent results
		// for the same string across different representations
		TransparentHashSet<std::string> set;

		std::string str = "consistent";
		set.insert( str );

		std::string_view sv = "consistent";
		const char* cstr = "consistent";

		// All should find the same entry
		EXPECT_NE( set.find( str ), set.end() );
		EXPECT_NE( set.find( sv ), set.end() );
		EXPECT_NE( set.find( cstr ), set.end() );
		EXPECT_EQ( set.count( str ), 1 );
		EXPECT_EQ( set.count( sv ), 1 );
		EXPECT_EQ( set.count( cstr ), 1 );
	}

	TEST( TransparentHashSetTests, StringKeys_EmptyString )
	{
		TransparentHashSet<std::string> set;

		// Empty string should hash to 0 and be usable as a key
		set.insert( "" );
		set.insert( "non-empty" );

		EXPECT_EQ( set.size(), 2 );
		EXPECT_TRUE( set.contains( "" ) );
		EXPECT_TRUE( set.contains( "non-empty" ) );
	}

	TEST( TransparentHashSetTests, StringKeys_InitializerList )
	{
		TransparentHashSet<std::string> set{ "one", "two", "three", "four" };

		EXPECT_EQ( set.size(), 4 );
		EXPECT_TRUE( set.contains( "one" ) );
		EXPECT_TRUE( set.contains( "two" ) );
		EXPECT_TRUE( set.contains( "three" ) );
		EXPECT_TRUE( set.contains( "four" ) );
		EXPECT_FALSE( set.contains( "five" ) );
	}

	//=====================================================================
	// Integer keys - Multiplicative hashing (Knuth/Wang) tests
	//=====================================================================

	TEST( TransparentHashSetTests, IntegerKeys_BasicInsert )
	{
		TransparentHashSet<int> set;

		set.insert( 1 );
		set.insert( 2 );
		set.insert( 3 );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( 1 ) );
		EXPECT_TRUE( set.contains( 2 ) );
		EXPECT_TRUE( set.contains( 3 ) );
		EXPECT_FALSE( set.contains( 4 ) );
	}

	TEST( TransparentHashSetTests, IntegerKeys_AllIntegralTypes )
	{
		// Test different integral types to verify Hasher handles all correctly
		TransparentHashSet<uint8_t> set8{ 0, 127, 255 };
		EXPECT_EQ( set8.size(), 3 );
		EXPECT_TRUE( set8.contains( 255 ) );

		TransparentHashSet<int16_t> set16{ -32768, 0, 32767 };
		EXPECT_EQ( set16.size(), 3 );
		EXPECT_TRUE( set16.contains( -32768 ) );
		EXPECT_TRUE( set16.contains( 32767 ) );

		TransparentHashSet<uint64_t> set64{ 0, 0xFFFFFFFFFFFFFFFFULL };
		EXPECT_EQ( set64.size(), 2 );
		EXPECT_TRUE( set64.contains( 0xFFFFFFFFFFFFFFFFULL ) );

		TransparentHashSet<size_t> setSize{ 0, SIZE_MAX };
		EXPECT_EQ( setSize.size(), 2 );
		EXPECT_TRUE( setSize.contains( SIZE_MAX ) );
	}

	TEST( TransparentHashSetTests, IntegerKeys_NegativeValues )
	{
		TransparentHashSet<int> set{ -100, -1, 0, 1, 100 };

		EXPECT_EQ( set.size(), 5 );
		EXPECT_TRUE( set.contains( -100 ) );
		EXPECT_TRUE( set.contains( -1 ) );
		EXPECT_TRUE( set.contains( 0 ) );
		EXPECT_TRUE( set.contains( 1 ) );
		EXPECT_TRUE( set.contains( 100 ) );
	}

	TEST( TransparentHashSetTests, IntegerKeys_LargeValues )
	{
		TransparentHashSet<uint64_t> set;

		set.insert( 0 );
		set.insert( 1 );
		set.insert( 0x7FFFFFFFFFFFFFFFULL ); // Max int64_t
		set.insert( 0xFFFFFFFFFFFFFFFFULL ); // Max uint64_t

		EXPECT_EQ( set.size(), 4 );
		EXPECT_TRUE( set.contains( 0 ) );
		EXPECT_TRUE( set.contains( 1 ) );
		EXPECT_TRUE( set.contains( 0x7FFFFFFFFFFFFFFFULL ) );
		EXPECT_TRUE( set.contains( 0xFFFFFFFFFFFFFFFFULL ) );
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

	TEST( TransparentHashSetTests, EnumKeys_BasicInsert )
	{
		TransparentHashSet<Color> set;

		set.insert( Color::Red );
		set.insert( Color::Green );
		set.insert( Color::Blue );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( Color::Red ) );
		EXPECT_TRUE( set.contains( Color::Green ) );
		EXPECT_TRUE( set.contains( Color::Blue ) );

		// Duplicate
		auto [it, inserted] = set.insert( Color::Red );
		EXPECT_FALSE( inserted );
		EXPECT_EQ( set.size(), 3 );
	}

	enum class Status : uint8_t
	{
		Idle = 0,
		Running = 1,
		Stopped = 2
	};

	TEST( TransparentHashSetTests, EnumKeys_DifferentUnderlyingTypes )
	{
		TransparentHashSet<Status> set;

		set.insert( Status::Idle );
		set.insert( Status::Running );
		set.insert( Status::Stopped );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( Status::Idle ) );
		EXPECT_TRUE( set.contains( Status::Running ) );
		EXPECT_TRUE( set.contains( Status::Stopped ) );
	}

	//=====================================================================
	// Floating-point keys - Normalization tests
	//=====================================================================

	TEST( TransparentHashSetTests, FloatKeys_BasicInsert )
	{
		TransparentHashSet<double> set;

		set.insert( 1.5 );
		set.insert( 2.5 );
		set.insert( 3.5 );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( 1.5 ) );
		EXPECT_TRUE( set.contains( 2.5 ) );
		EXPECT_TRUE( set.contains( 3.5 ) );
	}

	TEST( TransparentHashSetTests, FloatKeys_SpecialValues )
	{
		TransparentHashSet<double> set;

		// Positive and negative zero should hash to the same value
		set.insert( 0.0 );
		EXPECT_EQ( set.size(), 1 );
		EXPECT_TRUE( set.contains( 0.0 ) );
		EXPECT_TRUE( set.contains( -0.0 ) ); // -0.0 normalized to 0.0

		auto [it, inserted] = set.insert( -0.0 );
		EXPECT_FALSE( inserted ); // Should not insert duplicate
		EXPECT_EQ( set.size(), 1 );

		// Infinity values
		set.insert( std::numeric_limits<double>::infinity() );
		set.insert( -std::numeric_limits<double>::infinity() );
		EXPECT_EQ( set.size(), 3 ); // +0, +inf, -inf
		EXPECT_TRUE( set.contains( std::numeric_limits<double>::infinity() ) );
		EXPECT_TRUE( set.contains( -std::numeric_limits<double>::infinity() ) );

		// NaN behavior: NaN == NaN is always false by IEEE 754 standard
		// This means we can insert NaN, but each NaN comparison fails equality
		// So effectively, NaN keys are insert-only (cannot be reliably looked up)
		double nan = std::numeric_limits<double>::quiet_NaN();
		set.insert( nan );
		EXPECT_EQ( set.size(), 4 );

		// We document that NaN cannot be reliably used as a key due to IEEE 754 semantics
	}

	TEST( TransparentHashSetTests, FloatKeys_ZeroNormalization )
	{
		TransparentHashSet<float> set;

		// Both +0.0f and -0.0f should be treated as identical
		set.insert( 0.0f );
		set.insert( -0.0f );

		EXPECT_EQ( set.size(), 1 ); // Only one zero entry
		EXPECT_TRUE( set.contains( 0.0f ) );
		EXPECT_TRUE( set.contains( -0.0f ) );
	}

	//=====================================================================
	// Pointer keys - Address hashing tests
	//=====================================================================

	TEST( TransparentHashSetTests, PointerKeys_BasicInsert )
	{
		TransparentHashSet<int*> set;

		int a = 1, b = 2, c = 3;
		set.insert( &a );
		set.insert( &b );
		set.insert( &c );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( &a ) );
		EXPECT_TRUE( set.contains( &b ) );
		EXPECT_TRUE( set.contains( &c ) );

		int d = 4;
		EXPECT_FALSE( set.contains( &d ) );
	}

	TEST( TransparentHashSetTests, PointerKeys_UniquePtr )
	{
		std::vector<std::unique_ptr<int>> ptrs;
		ptrs.push_back( std::make_unique<int>( 1 ) );
		ptrs.push_back( std::make_unique<int>( 2 ) );
		ptrs.push_back( std::make_unique<int>( 3 ) );

		TransparentHashSet<int*> set;
		set.insert( ptrs[0].get() );
		set.insert( ptrs[1].get() );
		set.insert( ptrs[2].get() );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( ptrs[0].get() ) );
		EXPECT_TRUE( set.contains( ptrs[1].get() ) );
		EXPECT_TRUE( set.contains( ptrs[2].get() ) );
	}

	TEST( TransparentHashSetTests, PointerKeys_Nullptr )
	{
		// Test that nullptr works as a key with proper casting
		TransparentHashSet<int*> set;

		// Important: Use typed pointer variable for proper type matching
		int* null_key = nullptr;

		// nullptr should be a valid key
		auto [it, inserted] = set.insert( null_key );
		EXPECT_TRUE( inserted );
		EXPECT_EQ( set.size(), 1 );

		// Lookup methods (find, contains) support heterogeneous lookup
		EXPECT_TRUE( set.contains( null_key ) );
		EXPECT_NE( set.find( null_key ), set.end() );

		// Duplicate insert should fail
		auto [it2, inserted2] = set.insert( null_key );
		EXPECT_FALSE( inserted2 );
		EXPECT_EQ( set.size(), 1 );

		// Add another pointer
		int dummy = 42;
		set.insert( &dummy );

		EXPECT_EQ( set.size(), 2 );
		EXPECT_TRUE( set.contains( null_key ) );
		EXPECT_TRUE( set.contains( &dummy ) );

		// Erase nullptr
		EXPECT_EQ( set.erase( null_key ), 1 );
		EXPECT_EQ( set.size(), 1 );
		EXPECT_FALSE( set.contains( null_key ) );
		EXPECT_TRUE( set.contains( &dummy ) );
	}

	//=====================================================================
	// Pair keys - Hash combining tests
	//=====================================================================

	TEST( TransparentHashSetTests, PairKeys_IntPairs )
	{
		using Key = std::pair<int, int>;
		TransparentHashSet<Key> set;

		set.insert( { 1, 2 } );
		set.insert( { 3, 4 } );
		set.insert( { 5, 6 } );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( Key{ 1, 2 } ) );
		EXPECT_TRUE( set.contains( Key{ 3, 4 } ) );
		EXPECT_TRUE( set.contains( Key{ 5, 6 } ) );
		EXPECT_FALSE( set.contains( Key{ 7, 8 } ) );
	}

	TEST( TransparentHashSetTests, PairKeys_StringPairs )
	{
		using Key = std::pair<std::string, std::string>;
		TransparentHashSet<Key> set;

		set.insert( { "hello", "world" } );
		set.insert( { "foo", "bar" } );

		EXPECT_EQ( set.size(), 2 );
		EXPECT_TRUE( set.contains( Key{ "hello", "world" } ) );
		EXPECT_TRUE( set.contains( Key{ "foo", "bar" } ) );
	}

	TEST( TransparentHashSetTests, PairKeys_MixedTypes )
	{
		using Key = std::pair<std::string, int>;
		TransparentHashSet<Key> set;

		set.insert( { "one", 1 } );
		set.insert( { "two", 2 } );
		set.insert( { "three", 3 } );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( Key{ "one", 1 } ) );
		EXPECT_TRUE( set.contains( Key{ "two", 2 } ) );
		EXPECT_FALSE( set.contains( Key{ "one", 2 } ) );
	}

	//=====================================================================
	// Hash quality tests - Distribution and collision resistance
	//=====================================================================

	TEST( TransparentHashSetTests, HashQuality_IntegerDistribution32 )
	{
		TransparentHashSet<uint32_t> set;

		// Insert sequential values - should distribute well
		for ( uint32_t i = 0; i < 1000; ++i )
		{
			set.insert( i );
		}

		EXPECT_EQ( set.size(), 1000 );

		// All should be findable
		for ( uint32_t i = 0; i < 1000; ++i )
		{
			EXPECT_TRUE( set.contains( i ) );
		}
	}

	TEST( TransparentHashSetTests, HashQuality_IntegerDistribution64 )
	{
		TransparentHashSet<uint64_t, Hasher<uint64_t>> set;

		for ( uint64_t i = 0; i < 500; ++i )
		{
			set.insert( i );
			set.insert( i << 32 ); // High bits
		}

		// Note: i=0 produces same value for both (0 and 0<<32 are both 0)
		// So we expect 999 unique values, not 1000
		EXPECT_EQ( set.size(), 999 );
	}

	TEST( TransparentHashSetTests, HashQuality_StringDistribution )
	{
		TransparentHashSet<std::string> set;

		// Insert various strings
		for ( int i = 0; i < 100; ++i )
		{
			set.insert( "prefix_" + std::to_string( i ) );
			set.insert( std::to_string( i ) + "_suffix" );
			set.insert( "mid_" + std::to_string( i ) + "_fix" );
		}

		EXPECT_EQ( set.size(), 300 );
	}

	TEST( TransparentHashSetTests, HashQuality_CollisionResistance )
	{
		TransparentHashSet<int> set;

		// Similar values that should hash differently
		set.insert( 12345 );
		set.insert( 12346 );
		set.insert( 12344 );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( 12345 ) );
		EXPECT_TRUE( set.contains( 12346 ) );
		EXPECT_TRUE( set.contains( 12344 ) );
	}

	//=====================================================================
	// Performance and efficiency tests
	//=====================================================================

	TEST( TransparentHashSetTests, Performance_Reserve )
	{
		TransparentHashSet<int> set;

		// Reserve space
		set.reserve( 1000 );
		size_t initial_buckets = set.bucket_count();

		// Insert elements
		for ( int i = 0; i < 1000; ++i )
		{
			set.insert( i );
		}

		// Should not have rehashed
		EXPECT_EQ( set.bucket_count(), initial_buckets );
		EXPECT_EQ( set.size(), 1000 );
	}

	TEST( TransparentHashSetTests, Performance_TransparentLookup )
	{
		TransparentHashSet<std::string> set;

		for ( int i = 0; i < 100; ++i )
		{
			set.insert( "key_" + std::to_string( i ) );
		}

		// Transparent lookup with string_view (no temporary string allocation)
		std::string_view sv = "key_50";
		EXPECT_TRUE( set.contains( sv ) );

		auto it = set.find( sv );
		ASSERT_NE( it, set.end() );
		EXPECT_EQ( *it, "key_50" );
	}

	//=====================================================================
	// Erase operations
	//=====================================================================

	TEST( TransparentHashSetTests, Erase_BasicOperation )
	{
		TransparentHashSet<std::string> set{ "one", "two", "three" };

		EXPECT_EQ( set.size(), 3 );

		size_t erased = set.erase( "two" );
		EXPECT_EQ( erased, 1 );
		EXPECT_EQ( set.size(), 2 );
		EXPECT_FALSE( set.contains( "two" ) );
		EXPECT_TRUE( set.contains( "one" ) );
		EXPECT_TRUE( set.contains( "three" ) );
	}

	TEST( TransparentHashSetTests, Erase_NonExistent )
	{
		TransparentHashSet<int> set{ 1, 2, 3 };

		size_t erased = set.erase( 999 );
		EXPECT_EQ( erased, 0 );
		EXPECT_EQ( set.size(), 3 );
	}

	TEST( TransparentHashSetTests, Erase_HeterogeneousKey )
	{
		TransparentHashSet<std::string> set{ "hello", "world" };

		// Note: erase() does NOT support heterogeneous lookup in std::unordered_set
		// We need to use find() with heterogeneous lookup, then erase by iterator
		std::string_view sv = "hello";
		auto it = set.find( sv );
		ASSERT_NE( it, set.end() );
		set.erase( it );

		EXPECT_EQ( set.size(), 1 );
		EXPECT_FALSE( set.contains( "hello" ) );
		EXPECT_TRUE( set.contains( "world" ) );
	}

	//=====================================================================
	// Capacity and memory management
	//=====================================================================

	TEST( TransparentHashSetTests, Capacity_DefaultConstruction )
	{
		TransparentHashSet<int> set;
		EXPECT_TRUE( set.empty() );
		EXPECT_EQ( set.size(), 0 );
	}

	TEST( TransparentHashSetTests, Capacity_Reserve )
	{
		TransparentHashSet<int> set;
		set.reserve( 100 );
		EXPECT_GE( set.bucket_count(), 100 );
	}

	TEST( TransparentHashSetTests, Capacity_LoadFactor )
	{
		TransparentHashSet<int> set;

		for ( int i = 0; i < 100; ++i )
		{
			set.insert( i );
		}

		float load_factor = set.load_factor();
		EXPECT_GT( load_factor, 0.0f );
		EXPECT_LE( load_factor, set.max_load_factor() );
	}

	//=====================================================================
	// Iterator tests
	//=====================================================================

	TEST( TransparentHashSetTests, Iterator_BasicIteration )
	{
		TransparentHashSet<int> set{ 1, 2, 3, 4, 5 };

		int count = 0;
		int sum = 0;
		for ( int value : set )
		{
			++count;
			sum += value;
		}

		EXPECT_EQ( count, 5 );
		EXPECT_EQ( sum, 15 ); // 1+2+3+4+5
	}

	TEST( TransparentHashSetTests, Iterator_EmptySet )
	{
		TransparentHashSet<int> set;

		int count = 0;
		for ( [[maybe_unused]] int value : set )
		{
			++count;
		}

		EXPECT_EQ( count, 0 );
		EXPECT_EQ( set.begin(), set.end() );
	}

	//=====================================================================
	// Edge cases and stress tests
	//=====================================================================

	TEST( TransparentHashSetTests, EdgeCase_MaxSizeTypes )
	{
		TransparentHashSet<uint8_t> set8{ 0, 127, 255 };
		EXPECT_EQ( set8.size(), 3 );

		TransparentHashSet<int8_t> setS8{ -128, 0, 127 };
		EXPECT_EQ( setS8.size(), 3 );
	}

	TEST( TransparentHashSetTests, EdgeCase_LongStrings )
	{
		TransparentHashSet<std::string> set;

		std::string long1( 1000, 'a' );
		std::string long2( 1000, 'b' );
		std::string long3 = long1; // Same as long1

		set.insert( long1 );
		set.insert( long2 );
		set.insert( long3 );

		EXPECT_EQ( set.size(), 2 ); // long1 and long3 are duplicates
		EXPECT_TRUE( set.contains( long1 ) );
		EXPECT_TRUE( set.contains( long2 ) );
	}

	TEST( TransparentHashSetTests, EdgeCase_UnicodeStrings )
	{
		TransparentHashSet<std::string> set;

		set.insert( "Hello" );
		set.insert( "Привет" ); // Russian
		set.insert( "你好" );	// Chinese
		set.insert( "مرحبا" );	// Arabic
		set.insert( "🌍🌎🌏" ); // Emojis

		EXPECT_EQ( set.size(), 5 );
		EXPECT_TRUE( set.contains( "Привет" ) );
		EXPECT_TRUE( set.contains( "🌍🌎🌏" ) );
	}

	TEST( TransparentHashSetTests, EdgeCase_PowerOfTwoKeys )
	{
		TransparentHashSet<uint32_t> set;

		// Powers of 2 can be challenging for hash tables
		for ( int i = 0; i < 20; ++i )
		{
			set.insert( 1U << i );
		}

		EXPECT_EQ( set.size(), 20 );
		EXPECT_TRUE( set.contains( 1 ) );
		EXPECT_TRUE( set.contains( 1024 ) );
		EXPECT_TRUE( set.contains( 524288 ) );
	}

	TEST( TransparentHashSetTests, Stress_ManyElements )
	{
		TransparentHashSet<int> set;

		// Insert many elements
		for ( int i = 0; i < 10000; ++i )
		{
			set.insert( i );
		}

		EXPECT_EQ( set.size(), 10000 );

		// Verify all are present
		for ( int i = 0; i < 10000; ++i )
		{
			EXPECT_TRUE( set.contains( i ) );
		}
	}

	TEST( TransparentHashSetTests, Stress_InsertErasePattern )
	{
		TransparentHashSet<int> set;

		// Pattern: insert, erase some, insert more
		for ( int i = 0; i < 100; ++i )
		{
			set.insert( i );
		}
		EXPECT_EQ( set.size(), 100 );

		// Erase even numbers
		for ( int i = 0; i < 100; i += 2 )
		{
			size_t erased = set.erase( i );
			EXPECT_EQ( erased, 1 );
		}
		EXPECT_EQ( set.size(), 50 );

		// Insert new values
		for ( int i = 100; i < 200; ++i )
		{
			set.insert( i );
		}
		EXPECT_EQ( set.size(), 150 );

		// Verify odd numbers from first batch are still there
		for ( int i = 1; i < 100; i += 2 )
		{
			EXPECT_TRUE( set.contains( i ) );
		}

		// Verify new values are there
		for ( int i = 100; i < 200; ++i )
		{
			EXPECT_TRUE( set.contains( i ) );
		}
	}

	//=====================================================================
	// Custom hasher tests
	//=====================================================================

	// Simple custom hasher for testing
	struct SimpleHasher
	{
		using is_transparent = void;

		uint32_t operator()( int key ) const noexcept
		{
			return static_cast<uint32_t>( key * 2654435761U );
		}

		uint32_t operator()( const std::string& key ) const noexcept
		{
			uint32_t hash = 0;
			for ( char c : key )
			{
				hash = hash * 31 + static_cast<uint32_t>( c );
			}
			return hash;
		}

		uint32_t operator()( std::string_view key ) const noexcept
		{
			uint32_t hash = 0;
			for ( char c : key )
			{
				hash = hash * 31 + static_cast<uint32_t>( c );
			}
			return hash;
		}
	};

	TEST( TransparentHashSetTests, CustomHasher_Simple )
	{
		TransparentHashSet<int, SimpleHasher> set;

		set.insert( 1 );
		set.insert( 2 );
		set.insert( 3 );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( 1 ) );
		EXPECT_TRUE( set.contains( 2 ) );
		EXPECT_TRUE( set.contains( 3 ) );
	}

	//=====================================================================
	// 64-bit hash tests - Dual-stream CRC32-C verification
	//=====================================================================

	TEST( TransparentHashSetTests, Hash64Bit_IntegerKeys )
	{
		TransparentHashSet<uint64_t, Hasher<uint64_t>> set;

		set.insert( 0 );
		set.insert( 1 );
		set.insert( 0xFFFFFFFFFFFFFFFFULL );

		EXPECT_EQ( set.size(), 3 );
		EXPECT_TRUE( set.contains( 0 ) );
		EXPECT_TRUE( set.contains( 1 ) );
		EXPECT_TRUE( set.contains( 0xFFFFFFFFFFFFFFFFULL ) );
	}

	TEST( TransparentHashSetTests, Hash64Bit_StringKeys )
	{
		TransparentHashSet<std::string, Hasher<uint64_t>> set;

		set.insert( "hash64_test1" );
		set.insert( "hash64_test2" );
		set.insert( "hash64_test3" );

		EXPECT_EQ( set.size(), 3 );

		// Transparent lookup with string_view
		std::string_view sv = "hash64_test2";
		EXPECT_TRUE( set.contains( sv ) );
	}

	TEST( TransparentHashSetTests, Hash64Bit_MixedTypes )
	{
		using Key = std::pair<std::string, uint64_t>;
		TransparentHashSet<Key, Hasher<uint64_t>> set;

		set.insert( { "key1", 100 } );
		set.insert( { "key2", 200 } );

		EXPECT_EQ( set.size(), 2 );
		EXPECT_TRUE( set.contains( Key{ "key1", 100 } ) );
	}

	//=====================================================================
	// Clear operation
	//=====================================================================

	TEST( TransparentHashSetTests, Clear_BasicOperation )
	{
		TransparentHashSet<std::string> set{ "one", "two", "three" };

		EXPECT_EQ( set.size(), 3 );
		EXPECT_FALSE( set.empty() );

		set.clear();

		EXPECT_EQ( set.size(), 0 );
		EXPECT_TRUE( set.empty() );
		EXPECT_FALSE( set.contains( "one" ) );

		// Can still insert after clear
		set.insert( "new" );
		EXPECT_EQ( set.size(), 1 );
		EXPECT_TRUE( set.contains( "new" ) );
	}

	//=====================================================================
	// Emplace operations
	//=====================================================================

	TEST( TransparentHashSetTests, Emplace_BasicOperation )
	{
		TransparentHashSet<std::string> set;

		auto [it1, inserted1] = set.emplace( "constructed" );
		EXPECT_TRUE( inserted1 );
		EXPECT_EQ( *it1, "constructed" );
		EXPECT_EQ( set.size(), 1 );

		// Duplicate
		auto [it2, inserted2] = set.emplace( "constructed" );
		EXPECT_FALSE( inserted2 );
		EXPECT_EQ( set.size(), 1 );
	}

	TEST( TransparentHashSetTests, Emplace_Pairs )
	{
		using Key = std::pair<int, std::string>;
		TransparentHashSet<Key> set;

		auto [it, inserted] = set.emplace( 1, "one" );
		EXPECT_TRUE( inserted );
		EXPECT_EQ( it->first, 1 );
		EXPECT_EQ( it->second, "one" );
	}

	//=====================================================================
	// Value type tests
	//=====================================================================

	TEST( TransparentHashSetTests, ValueTypes_MoveSemantics )
	{
		TransparentHashSet<std::string> set;

		std::string value = "movable";
		auto [it, inserted] = set.insert( std::move( value ) );

		EXPECT_TRUE( inserted );
		EXPECT_EQ( *it, "movable" );

		// Original string should be moved-from (empty or in moved-from state)
		EXPECT_TRUE( value.empty() || value == "movable" );
	}

	TEST( TransparentHashSetTests, MoveConstructor_TransfersOwnership )
	{
		TransparentHashSet<std::string> set1;
		set1.insert( "alpha" );
		set1.insert( "beta" );
		set1.insert( "gamma" );

		TransparentHashSet<std::string> set2( std::move( set1 ) );

		EXPECT_EQ( set2.size(), 3 );
		EXPECT_TRUE( set2.contains( "alpha" ) );
		EXPECT_TRUE( set2.contains( "beta" ) );
		EXPECT_TRUE( set2.contains( "gamma" ) );

		// Verify source is moved-from (should be empty)
		EXPECT_EQ( set1.size(), 0 );
		EXPECT_TRUE( set1.empty() );
	}

	TEST( TransparentHashSetTests, MoveAssignment_TransfersOwnership )
	{
		TransparentHashSet<std::string> set1;
		set1.insert( "alpha" );
		set1.insert( "beta" );

		TransparentHashSet<std::string> set2;
		set2.insert( "old" );

		set2 = std::move( set1 );

		EXPECT_EQ( set2.size(), 2 );
		EXPECT_TRUE( set2.contains( "alpha" ) );
		EXPECT_TRUE( set2.contains( "beta" ) );
		EXPECT_FALSE( set2.contains( "old" ) );

		// Verify source is moved-from (should be empty)
		EXPECT_EQ( set1.size(), 0 );
		EXPECT_TRUE( set1.empty() );
	}
} // namespace nfx::containers::test
