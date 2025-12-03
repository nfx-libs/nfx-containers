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
 * @file TESTS_FastHashMap.cpp
 * @brief Tests for FastHashMap (Robin Hood hashing)
 */

#include <gtest/gtest.h>

#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <nfx/containers/FastHashMap.h>

namespace nfx::containers::test
{
	using namespace nfx::hashing;

	//=====================================================================
	// Constructor tests - initializer_list
	//=====================================================================

	TEST( FastHashMapTests, InitializerListConstructor_Basic )
	{
		FastHashMap<std::string, int> map = { { "apple", 1 }, { "banana", 2 }, { "cherry", 3 } };

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

	TEST( FastHashMapTests, InitializerListConstructor_Empty )
	{
		FastHashMap<std::string, int> map = {};

		EXPECT_EQ( map.size(), 0 );
		EXPECT_TRUE( map.isEmpty() );
	}

	TEST( FastHashMapTests, InitializerListConstructor_SingleElement )
	{
		FastHashMap<std::string, int> map = { { "single", 42 } };

		EXPECT_EQ( map.size(), 1 );
		auto* value = map.find( "single" );
		ASSERT_NE( value, nullptr );
		EXPECT_EQ( *value, 42 );
	}

	TEST( FastHashMapTests, InitializerListConstructor_DuplicateKeys )
	{
		// Last value wins with duplicate keys
		FastHashMap<std::string, int> map = { { "key", 1 }, { "key", 2 }, { "key", 3 } };

		EXPECT_EQ( map.size(), 1 );
		auto* value = map.find( "key" );
		ASSERT_NE( value, nullptr );
		EXPECT_EQ( *value, 3 ); // Last value should win
	}

	TEST( FastHashMapTests, InitializerListConstructor_IntKeys )
	{
		FastHashMap<int, std::string> map = { { 1, "one" }, { 2, "two" }, { 3, "three" } };

		EXPECT_EQ( map.size(), 3 );

		auto* value1 = map.find( 1 );
		auto* value2 = map.find( 2 );
		auto* value3 = map.find( 3 );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "one" );
		EXPECT_EQ( *value2, "two" );
		EXPECT_EQ( *value3, "three" );
	}

	//=====================================================================
	// Range constructor tests
	//=====================================================================

	TEST( FastHashMapTests, RangeConstructor_FromVector )
	{
		std::vector<std::pair<std::string, int>> data = {
			{ "one", 1 },
			{ "two", 2 },
			{ "three", 3 },
			{ "four", 4 } };

		FastHashMap<std::string, int> map( data.begin(), data.end() );

		EXPECT_EQ( map.size(), 4 );
		EXPECT_EQ( *map.find( "one" ), 1 );
		EXPECT_EQ( *map.find( "two" ), 2 );
		EXPECT_EQ( *map.find( "three" ), 3 );
		EXPECT_EQ( *map.find( "four" ), 4 );
	}

	TEST( FastHashMapTests, RangeConstructor_FromAnotherMap )
	{
		FastHashMap<int, std::string> original;
		original.insert( 10, "ten" );
		original.insert( 20, "twenty" );
		original.insert( 30, "thirty" );

		FastHashMap<int, std::string> copy( original.begin(), original.end() );

		EXPECT_EQ( copy.size(), 3 );
		EXPECT_EQ( *copy.find( 10 ), "ten" );
		EXPECT_EQ( *copy.find( 20 ), "twenty" );
		EXPECT_EQ( *copy.find( 30 ), "thirty" );
	}

	TEST( FastHashMapTests, RangeConstructor_EmptyRange )
	{
		std::vector<std::pair<int, int>> data;
		FastHashMap<int, int> map( data.begin(), data.end() );

		EXPECT_EQ( map.size(), 0 );
		EXPECT_TRUE( map.isEmpty() );
	}

	//=====================================================================
	// String keys - Heterogeneous lookup tests
	//=====================================================================

	TEST( FastHashMapTests, StringKeys_BasicLookup )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "apple", 1 );
		map.insertOrAssign( "banana", 2 );
		map.insertOrAssign( "cherry", 3 );

		EXPECT_EQ( map.size(), 3 );
		EXPECT_FALSE( map.isEmpty() );

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( "apple" );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( "banana" );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( "cherry" );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 1 );
		EXPECT_EQ( *value2, 2 );
		EXPECT_EQ( *value3, 3 );
	}

	TEST( FastHashMapTests, StringKeys_HeterogeneousLookup_StringView )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "hello", 100 );
		map.insertOrAssign( "world", 200 );

		// Zero-copy string_view lookup
		std::string_view key1 = "hello";
		std::string_view key2 = "world";
		std::string_view key3 = "missing";

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( key1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( key2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( key3 );

		EXPECT_EQ( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		EXPECT_EQ( value3, nullptr );

		EXPECT_EQ( *value1, 100 );
		EXPECT_EQ( *value2, 200 );
	}

	TEST( FastHashMapTests, StringKeys_HeterogeneousLookup_CString )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "test", 42 );

		const char* key = "test";
		int* value = map.find( key );

		EXPECT_NE( value, nullptr );
		ASSERT_NE( value, nullptr );
		EXPECT_EQ( *value, 42 );
	}

	TEST( FastHashMapTests, StringKeys_InsertOrAssignUpdate )
	{
		FastHashMap<std::string, std::string> map;

		map.insertOrAssign( "update_key", "initial" );
		std::string* value1 = map.find( "update_key" );

		EXPECT_NE( value1, nullptr );
		ASSERT_NE( value1, nullptr );
		EXPECT_EQ( *value1, "initial" );
		EXPECT_EQ( map.size(), 1 );

		// Update
		map.insertOrAssign( "update_key", "updated" );
		std::string* value2 = map.find( "update_key" );

		EXPECT_NE( value2, nullptr );
		ASSERT_NE( value2, nullptr );
		EXPECT_EQ( *value2, "updated" );
		EXPECT_EQ( map.size(), 1 );
	}

	//=====================================================================
	// operator[] - Subscript access tests
	//=====================================================================

	TEST( FastHashMapTests, OperatorSubscript_InsertIfMissing )
	{
		FastHashMap<std::string, int> map;

		// Insert via operator[]
		map["key1"] = 100;
		map["key2"] = 200;
		map["key3"] = 300;

		EXPECT_EQ( map.size(), 3 );
		EXPECT_EQ( map["key1"], 100 );
		EXPECT_EQ( map["key2"], 200 );
		EXPECT_EQ( map["key3"], 300 );
	}

	TEST( FastHashMapTests, OperatorSubscript_DefaultConstruction )
	{
		FastHashMap<std::string, int> map;

		// Access non-existent key - should insert default value (0 for int)
		int& value = map["new_key"];
		EXPECT_EQ( value, 0 );
		EXPECT_EQ( map.size(), 1 );

		// Modify the value
		value = 42;
		EXPECT_EQ( map["new_key"], 42 );
	}

	TEST( FastHashMapTests, OperatorSubscript_UpdateExisting )
	{
		FastHashMap<std::string, std::string> map;

		map["key"] = "initial";
		EXPECT_EQ( map["key"], "initial" );
		EXPECT_EQ( map.size(), 1 );

		// Update via operator[]
		map["key"] = "updated";
		EXPECT_EQ( map["key"], "updated" );
		EXPECT_EQ( map.size(), 1 ); // Size unchanged
	}

	TEST( FastHashMapTests, OperatorSubscript_RvalueKey )
	{
		FastHashMap<std::string, int> map;

		// Use rvalue reference for key
		map[std::string( "rvalue_key" )] = 999;
		EXPECT_EQ( map.size(), 1 );
		EXPECT_EQ( map["rvalue_key"], 999 );
	}

	TEST( FastHashMapTests, OperatorSubscript_ComplexValue )
	{
		FastHashMap<std::string, std::vector<int>> map;

		// Default constructs empty vector
		map["vec1"].push_back( 1 );
		map["vec1"].push_back( 2 );
		map["vec1"].push_back( 3 );

		EXPECT_EQ( map["vec1"].size(), 3 );
		EXPECT_EQ( map["vec1"][0], 1 );
		EXPECT_EQ( map["vec1"][1], 2 );
		EXPECT_EQ( map["vec1"][2], 3 );
	}

	//=====================================================================
	// at() - Checked element access tests
	//=====================================================================

	TEST( FastHashMapTests, At_BasicAccess )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "key1", 100 );
		map.insertOrAssign( "key2", 200 );

		EXPECT_EQ( map.at( "key1" ), 100 );
		EXPECT_EQ( map.at( "key2" ), 200 );
	}

	TEST( FastHashMapTests, At_ThrowsOutOfRange )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "existing", 42 );

		EXPECT_THROW( map.at( "non_existent" ), std::out_of_range );
		EXPECT_THROW( map.at( "missing_key" ), std::out_of_range );
	}

	TEST( FastHashMapTests, At_ConstVersion )
	{
		FastHashMap<std::string, int> map;
		map.insertOrAssign( "const_key", 777 );

		const auto& const_map = map;
		EXPECT_EQ( const_map.at( "const_key" ), 777 );
		EXPECT_THROW( const_map.at( "missing" ), std::out_of_range );
	}

	TEST( FastHashMapTests, At_HeterogeneousLookup )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "hetero_key", 555 );

		// string_view lookup
		std::string_view sv = "hetero_key";
		EXPECT_EQ( map.at( sv ), 555 );

		// const char* lookup
		EXPECT_EQ( map.at( "hetero_key" ), 555 );

		// Non-existent with string_view
		std::string_view missing_sv = "missing";
		EXPECT_THROW( map.at( missing_sv ), std::out_of_range );
	}

	TEST( FastHashMapTests, At_ModifyValue )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "modify", 100 );

		int& value = map.at( "modify" );
		value = 999;

		EXPECT_EQ( map.at( "modify" ), 999 );
	}

	//=====================================================================
	// Integer keys - Multiplicative hashing tests
	//=====================================================================

	TEST( FastHashMapTests, IntegerKeys_BasicLookup )
	{
		FastHashMap<int, std::string> map;

		map.insertOrAssign( 1, "one" );
		map.insertOrAssign( 2, "two" );
		map.insertOrAssign( 3, "three" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( 1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( 3 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "one" );
		EXPECT_EQ( *value2, "two" );
		EXPECT_EQ( *value3, "three" );
	}

	TEST( FastHashMapTests, IntegerKeys_LargeValues )
	{
		FastHashMap<uint64_t, std::string> map;

		map.insertOrAssign( 0xFFFFFFFFFFFFFFFFULL, "max" );
		map.insertOrAssign( 0x0000000000000001ULL, "min" );
		map.insertOrAssign( 0x8000000000000000ULL, "mid" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( 0xFFFFFFFFFFFFFFFFULL );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 0x0000000000000001ULL );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( 0x8000000000000000ULL );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "max" );
		EXPECT_EQ( *value2, "min" );
		EXPECT_EQ( *value3, "mid" );
	}

	TEST( FastHashMapTests, IntegerKeys_NegativeValues )
	{
		FastHashMap<int, std::string> map;

		map.insertOrAssign( -100, "negative_hundred" );
		map.insertOrAssign( -1, "negative_one" );
		map.insertOrAssign( 0, "zero" );
		map.insertOrAssign( 1, "one" );
		map.insertOrAssign( 100, "hundred" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;
		std::string* value4 = nullptr;
		std::string* value5 = nullptr;

		value1 = map.find( -100 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( -1 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( 0 );

		EXPECT_NE( value3, nullptr );
		value4 = map.find( 1 );

		EXPECT_NE( value4, nullptr );
		value5 = map.find( 100 );

		EXPECT_NE( value5, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );
		ASSERT_NE( value4, nullptr );
		ASSERT_NE( value5, nullptr );

		EXPECT_EQ( *value1, "negative_hundred" );
		EXPECT_EQ( *value2, "negative_one" );
		EXPECT_EQ( *value3, "zero" );
		EXPECT_EQ( *value4, "one" );
		EXPECT_EQ( *value5, "hundred" );
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

	TEST( FastHashMapTests, EnumKeys_BasicLookup )
	{
		FastHashMap<Color, std::string> map;

		map.insertOrAssign( Color::Red, "red" );
		map.insertOrAssign( Color::Green, "green" );
		map.insertOrAssign( Color::Blue, "blue" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( Color::Red );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( Color::Green );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( Color::Blue );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "red" );
		EXPECT_EQ( *value2, "green" );
		EXPECT_EQ( *value3, "blue" );
	}

	enum class Status : uint8_t
	{
		Idle = 0,
		Running = 1,
		Stopped = 2
	};

	TEST( FastHashMapTests, EnumKeys_DifferentUnderlyingType )
	{
		FastHashMap<Status, int> map;

		map.insertOrAssign( Status::Idle, 0 );
		map.insertOrAssign( Status::Running, 1 );
		map.insertOrAssign( Status::Stopped, 2 );

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( Status::Idle );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( Status::Running );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( Status::Stopped );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 0 );
		EXPECT_EQ( *value2, 1 );
		EXPECT_EQ( *value3, 2 );
	}

	//=====================================================================
	// Floating-point keys - Normalization tests
	//=====================================================================

	TEST( FastHashMapTests, FloatKeys_BasicLookup )
	{
		FastHashMap<double, std::string> map;

		map.insertOrAssign( 3.14159, "pi" );
		map.insertOrAssign( 2.71828, "e" );
		map.insertOrAssign( 1.41421, "sqrt2" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( 3.14159 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 2.71828 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( 1.41421 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "pi" );
		EXPECT_EQ( *value2, "e" );
		EXPECT_EQ( *value3, "sqrt2" );
	}

	TEST( FastHashMapTests, FloatKeys_SpecialValues )
	{
		FastHashMap<double, std::string> map;

		map.insertOrAssign( 0.0, "zero" );
		map.insertOrAssign( 1.0, "one" );
		map.insertOrAssign( -1.0, "negative_one" );
		map.insertOrAssign( std::numeric_limits<double>::infinity(), "infinity" );
		map.insertOrAssign( -std::numeric_limits<double>::infinity(), "negative_infinity" );
		map.insertOrAssign( std::numeric_limits<double>::min(), "min_normal" );
		map.insertOrAssign( std::numeric_limits<double>::max(), "max" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;
		std::string* value4 = nullptr;
		std::string* value5 = nullptr;
		std::string* value6 = nullptr;
		std::string* value7 = nullptr;

		value1 = map.find( 0.0 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 1.0 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( -1.0 );

		EXPECT_NE( value3, nullptr );
		value4 = map.find( std::numeric_limits<double>::infinity() );

		EXPECT_NE( value4, nullptr );
		value5 = map.find( -std::numeric_limits<double>::infinity() );

		EXPECT_NE( value5, nullptr );
		value6 = map.find( std::numeric_limits<double>::min() );

		EXPECT_NE( value6, nullptr );
		value7 = map.find( std::numeric_limits<double>::max() );

		EXPECT_NE( value7, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );
		ASSERT_NE( value4, nullptr );
		ASSERT_NE( value5, nullptr );
		ASSERT_NE( value6, nullptr );
		ASSERT_NE( value7, nullptr );

		EXPECT_EQ( *value1, "zero" );
		EXPECT_EQ( *value2, "one" );
		EXPECT_EQ( *value3, "negative_one" );
		EXPECT_EQ( *value4, "infinity" );
		EXPECT_EQ( *value5, "negative_infinity" );
		EXPECT_EQ( *value6, "min_normal" );
		EXPECT_EQ( *value7, "max" );
	}

	//=====================================================================
	// Pointer keys - Address hashing tests
	//=====================================================================

	TEST( FastHashMapTests, PointerKeys_BasicLookup )
	{
		int a = 1, b = 2, c = 3;

		FastHashMap<int*, std::string> map;

		map.insertOrAssign( &a, "a" );
		map.insertOrAssign( &b, "b" );
		map.insertOrAssign( &c, "c" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;
		std::string* value4 = nullptr;

		value1 = map.find( &a );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( &b );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( &c );

		EXPECT_NE( value3, nullptr );
		value4 = map.find( nullptr );

		EXPECT_EQ( value4, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );
		EXPECT_EQ( value4, nullptr );

		EXPECT_EQ( *value1, "a" );
		EXPECT_EQ( *value2, "b" );
		EXPECT_EQ( *value3, "c" );
	}

	//=====================================================================
	// Pair keys - Hash combining tests
	//=====================================================================

	TEST( FastHashMapTests, PairKeys_BasicLookup )
	{
		using Key = std::pair<int, int>;

		FastHashMap<Key, std::string> map;

		map.insertOrAssign( { 1, 2 }, "first" );
		map.insertOrAssign( { 3, 4 }, "second" );
		map.insertOrAssign( { 5, 6 }, "third" );

		Key key1{ 1, 2 };
		Key key2{ 3, 4 };
		Key key3{ 5, 6 };

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( key1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( key2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( key3 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "first" );
		EXPECT_EQ( *value2, "second" );
		EXPECT_EQ( *value3, "third" );
	}

	TEST( FastHashMapTests, PairKeys_StringPair )
	{
		using Key = std::pair<std::string, std::string>;

		FastHashMap<Key, int> map;

		map.insertOrAssign( { "foo", "bar" }, 1 );
		map.insertOrAssign( { "hello", "world" }, 2 );

		Key key1{ "foo", "bar" };
		Key key2{ "hello", "world" };
		Key key3{ "missing", "key" };

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( key1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( key2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( key3 );

		EXPECT_EQ( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		EXPECT_EQ( value3, nullptr );

		EXPECT_EQ( *value1, 1 );
		EXPECT_EQ( *value2, 2 );
	}

	//=====================================================================
	// Erase operations
	//=====================================================================

	TEST( FastHashMapTests, Erase_BasicOperation )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "erase1", 1 );
		map.insertOrAssign( "erase2", 2 );
		map.insertOrAssign( "erase3", 3 );

		EXPECT_EQ( map.size(), 3 );

		bool erased = map.erase( "erase2" );
		EXPECT_TRUE( erased );
		EXPECT_EQ( map.size(), 2 );

		int* value = map.find( "erase2" );

		EXPECT_EQ( value, nullptr );
		EXPECT_EQ( value, nullptr );

		int* value1 = nullptr;
		int* value3 = nullptr;
		value1 = map.find( "erase1" );

		EXPECT_NE( value1, nullptr );
		value3 = map.find( "erase3" );

		EXPECT_NE( value3, nullptr );
		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value3, nullptr );
		EXPECT_EQ( *value1, 1 );
		EXPECT_EQ( *value3, 3 );
	}

	TEST( FastHashMapTests, Erase_NonExistent )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "existing", 100 );

		bool erased = map.erase( "non_existent" );
		EXPECT_FALSE( erased );
		EXPECT_EQ( map.size(), 1 );

		int* value = map.find( "existing" );

		EXPECT_NE( value, nullptr );
		ASSERT_NE( value, nullptr );
		EXPECT_EQ( *value, 100 );
	}

	TEST( FastHashMapTests, Erase_HeterogeneousKey )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "hetero_erase", 999 );

		std::string_view svKey{ "hetero_erase" };
		bool erased = map.erase( svKey );
		EXPECT_TRUE( erased );
		EXPECT_EQ( map.size(), 0 );
	}

	//=====================================================================
	// Capacity and memory management
	//=====================================================================

	TEST( FastHashMapTests, Capacity_DefaultConstruction )
	{
		FastHashMap<std::string, int> map;

		EXPECT_TRUE( map.isEmpty() );
		EXPECT_EQ( map.size(), 0 );
		EXPECT_GE( map.capacity(), 32 ); // INITIAL_CAPACITY
	}

	TEST( FastHashMapTests, Capacity_SpecificConstruction )
	{
		FastHashMap<std::string, int> map( 64 );

		EXPECT_TRUE( map.isEmpty() );
		EXPECT_EQ( map.size(), 0 );
		EXPECT_GE( map.capacity(), 64 );
	}

	TEST( FastHashMapTests, Capacity_Reserve )
	{
		FastHashMap<std::string, int> map;

		size_t initialCapacity = map.capacity();

		map.reserve( 128 );
		EXPECT_GE( map.capacity(), 128 );
		EXPECT_GE( map.capacity(), initialCapacity );
		EXPECT_EQ( map.size(), 0 );
	}

	TEST( FastHashMapTests, Capacity_AutomaticResize )
	{
		FastHashMap<std::string, int> map;

		size_t initialCapacity = map.capacity();

		// Trigger resize (75% load factor)
		size_t itemsToInsert = ( initialCapacity * 75 ) / 100 + 5;

		for ( size_t i = 0; i < itemsToInsert; ++i )
		{
			map.insertOrAssign( "key_" + std::to_string( i ), static_cast<int>( i ) );
		}

		EXPECT_GT( map.capacity(), initialCapacity );
		EXPECT_EQ( map.size(), itemsToInsert );

		// Verify all accessible
		for ( size_t i = 0; i < itemsToInsert; ++i )
		{
			int* value = map.find( "key_" + std::to_string( i ) );

			EXPECT_NE( value, nullptr );
			ASSERT_NE( value, nullptr );
			EXPECT_EQ( *value, static_cast<int>( i ) );
		}
	}

	//=====================================================================
	// Edge cases and stress tests
	//=====================================================================

	TEST( FastHashMapTests, EdgeCase_EmptyStringKey )
	{
		FastHashMap<std::string, std::string> map;

		map.insertOrAssign( "", "empty_value" );
		map.insertOrAssign( "nonempty", "nonempty_value" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;

		value1 = map.find( "" );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( "nonempty" );

		EXPECT_NE( value2, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );

		EXPECT_EQ( *value1, "empty_value" );
		EXPECT_EQ( *value2, "nonempty_value" );

		// Test with empty string_view
		std::string_view emptySv;
		std::string* value3 = map.find( emptySv );

		EXPECT_NE( value3, nullptr );
		ASSERT_NE( value3, nullptr );
		EXPECT_EQ( *value3, "empty_value" );
	}

	TEST( FastHashMapTests, EdgeCase_VeryLongStrings )
	{
		FastHashMap<std::string, int> map;

		std::string long1( 1000, 'a' );
		std::string long2( 1000, 'b' );
		std::string long3( 1000, 'c' );

		map.insertOrAssign( long1, 1 );
		map.insertOrAssign( long2, 2 );
		map.insertOrAssign( long3, 3 );

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( long1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( long2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( long3 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 1 );
		EXPECT_EQ( *value2, 2 );
		EXPECT_EQ( *value3, 3 );
	}

	TEST( FastHashMapTests, EdgeCase_UnicodeStrings )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "Hello", 1 );
		map.insertOrAssign( "Привет", 2 );	   // Russian
		map.insertOrAssign( "你好", 3 );	   // Chinese
		map.insertOrAssign( "こんにちは", 4 ); // Japanese
		map.insertOrAssign( "🚀🎉💻", 5 );	   // Emojis

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;
		int* value4 = nullptr;
		int* value5 = nullptr;

		value1 = map.find( "Hello" );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( "Привет" );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( "你好" );

		EXPECT_NE( value3, nullptr );
		value4 = map.find( "こんにちは" );

		EXPECT_NE( value4, nullptr );
		value5 = map.find( "🚀🎉💻" );

		EXPECT_NE( value5, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );
		ASSERT_NE( value4, nullptr );
		ASSERT_NE( value5, nullptr );

		EXPECT_EQ( *value1, 1 );
		EXPECT_EQ( *value2, 2 );
		EXPECT_EQ( *value3, 3 );
		EXPECT_EQ( *value4, 4 );
		EXPECT_EQ( *value5, 5 );
	}

	TEST( FastHashMapTests, EdgeCase_SpecialCharacters )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "key with spaces", 1 );
		map.insertOrAssign( "key\twith\ttabs", 2 );
		map.insertOrAssign( "key\nwith\nnewlines", 3 );
		map.insertOrAssign( "key\"with\"quotes", 4 );
		map.insertOrAssign( "key\\with\\backslashes", 5 );

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;
		int* value4 = nullptr;
		int* value5 = nullptr;

		value1 = map.find( "key with spaces" );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( "key\twith\ttabs" );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( "key\nwith\nnewlines" );

		EXPECT_NE( value3, nullptr );
		value4 = map.find( "key\"with\"quotes" );

		EXPECT_NE( value4, nullptr );
		value5 = map.find( "key\\with\\backslashes" );

		EXPECT_NE( value5, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );
		ASSERT_NE( value4, nullptr );
		ASSERT_NE( value5, nullptr );

		EXPECT_EQ( *value1, 1 );
		EXPECT_EQ( *value2, 2 );
		EXPECT_EQ( *value3, 3 );
		EXPECT_EQ( *value4, 4 );
		EXPECT_EQ( *value5, 5 );
	}

	TEST( FastHashMapTests, EdgeCase_PowerOfTwoKeys )
	{
		FastHashMap<int, std::string> map;

		map.insertOrAssign( 0, "zero" );
		map.insertOrAssign( 1, "one" );
		map.insertOrAssign( 2, "two" );
		map.insertOrAssign( 4, "four" );
		map.insertOrAssign( 8, "eight" );
		map.insertOrAssign( 16, "sixteen" );
		map.insertOrAssign( 32, "thirty_two" );
		map.insertOrAssign( 64, "sixty_four" );
		map.insertOrAssign( 128, "one_twenty_eight" );

		std::string* value0 = nullptr;
		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value4 = nullptr;
		std::string* value8 = nullptr;
		std::string* value16 = nullptr;
		std::string* value32 = nullptr;
		std::string* value64 = nullptr;
		std::string* value128 = nullptr;

		value0 = map.find( 0 );

		EXPECT_NE( value0, nullptr );
		value1 = map.find( 1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 2 );

		EXPECT_NE( value2, nullptr );
		value4 = map.find( 4 );

		EXPECT_NE( value4, nullptr );
		value8 = map.find( 8 );

		EXPECT_NE( value8, nullptr );
		value16 = map.find( 16 );

		EXPECT_NE( value16, nullptr );
		value32 = map.find( 32 );

		EXPECT_NE( value32, nullptr );
		value64 = map.find( 64 );

		EXPECT_NE( value64, nullptr );
		value128 = map.find( 128 );

		EXPECT_NE( value128, nullptr );

		ASSERT_NE( value0, nullptr );
		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value4, nullptr );
		ASSERT_NE( value8, nullptr );
		ASSERT_NE( value16, nullptr );
		ASSERT_NE( value32, nullptr );
		ASSERT_NE( value64, nullptr );
		ASSERT_NE( value128, nullptr );

		EXPECT_EQ( *value0, "zero" );
		EXPECT_EQ( *value1, "one" );
		EXPECT_EQ( *value2, "two" );
		EXPECT_EQ( *value4, "four" );
		EXPECT_EQ( *value8, "eight" );
		EXPECT_EQ( *value16, "sixteen" );
		EXPECT_EQ( *value32, "thirty_two" );
		EXPECT_EQ( *value64, "sixty_four" );
		EXPECT_EQ( *value128, "one_twenty_eight" );
	}

	TEST( FastHashMapTests, Stress_ManyElements )
	{
		FastHashMap<int, int> map;

		constexpr size_t numItems = 1000;
		for ( size_t i = 0; i < numItems; ++i )
		{
			map.insertOrAssign( static_cast<int>( i ), static_cast<int>( i * 10 ) );
		}

		EXPECT_EQ( map.size(), numItems );

		for ( size_t i = 0; i < numItems; ++i )
		{
			int* value = map.find( static_cast<int>( i ) );

			EXPECT_NE( value, nullptr );
			ASSERT_NE( value, nullptr );
			EXPECT_EQ( *value, static_cast<int>( i * 10 ) );
		}

		// Verify non-existent keys
		int* valueMissing1 = nullptr;
		int* valueMissing2 = nullptr;
		int* valueMissing3 = nullptr;

		valueMissing1 = map.find( -1 );

		EXPECT_EQ( valueMissing1, nullptr );
		valueMissing2 = map.find( 1000 );

		EXPECT_EQ( valueMissing2, nullptr );
		valueMissing3 = map.find( 10000 );

		EXPECT_EQ( valueMissing3, nullptr );

		EXPECT_EQ( valueMissing1, nullptr );
		EXPECT_EQ( valueMissing2, nullptr );
		EXPECT_EQ( valueMissing3, nullptr );
	}

	TEST( FastHashMapTests, Stress_InsertErasePattern )
	{
		FastHashMap<std::string, int> map;

		// Insert 100 items
		for ( int i = 0; i < 100; ++i )
		{
			map.insertOrAssign( "pattern_" + std::to_string( i ), i );
		}
		EXPECT_EQ( map.size(), 100 );

		// Erase every other item
		for ( int i = 0; i < 100; i += 2 )
		{
			bool erased = map.erase( "pattern_" + std::to_string( i ) );
			EXPECT_TRUE( erased );
		}
		EXPECT_EQ( map.size(), 50 );

		// Insert new items
		for ( int i = 100; i < 150; ++i )
		{
			map.insertOrAssign( "pattern_" + std::to_string( i ), i );
		}
		EXPECT_EQ( map.size(), 100 );

		// Verify remaining odd numbers
		for ( int i = 1; i < 100; i += 2 )
		{
			int* value = map.find( "pattern_" + std::to_string( i ) );

			EXPECT_NE( value, nullptr );
			ASSERT_NE( value, nullptr );
			EXPECT_EQ( *value, i );
		}

		// Verify new items
		for ( int i = 100; i < 150; ++i )
		{
			int* value = map.find( "pattern_" + std::to_string( i ) );

			EXPECT_NE( value, nullptr );
			ASSERT_NE( value, nullptr );
			EXPECT_EQ( *value, i );
		}
	}

	TEST( FastHashMapTests, Stress_LargePrimeNumbers )
	{
		FastHashMap<uint64_t, std::string> map;

		map.insertOrAssign( 2147483647ULL, "mersenne_31" );				// 2^31 - 1
		map.insertOrAssign( 4294967291ULL, "large_prime_1" );			// Largest prime < 2^32
		map.insertOrAssign( 18446744073709551557ULL, "large_prime_2" ); // Large prime < 2^64

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( 2147483647ULL );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 4294967291ULL );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( 18446744073709551557ULL );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "mersenne_31" );
		EXPECT_EQ( *value2, "large_prime_1" );
		EXPECT_EQ( *value3, "large_prime_2" );
	}

	//=====================================================================
	// Clear operation
	//=====================================================================

	TEST( FastHashMapTests, Clear_BasicOperation )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "one", 1 );
		map.insertOrAssign( "two", 2 );
		map.insertOrAssign( "three", 3 );

		EXPECT_EQ( map.size(), 3 );
		EXPECT_FALSE( map.isEmpty() );

		map.clear();

		EXPECT_EQ( map.size(), 0 );
		EXPECT_TRUE( map.isEmpty() );
		EXPECT_EQ( map.find( "one" ), nullptr );
		EXPECT_EQ( map.find( "two" ), nullptr );
		EXPECT_EQ( map.find( "three" ), nullptr );

		// Can insert after clear
		map.insertOrAssign( "new", 99 );
		EXPECT_EQ( map.size(), 1 );
		EXPECT_NE( map.find( "new" ), nullptr );
		EXPECT_EQ( *map.find( "new" ), 99 );
	}

	//=====================================================================
	// Comparison operator tests
	//=====================================================================

	TEST( FastHashMapTests, ComparisonOperators_Equal )
	{
		FastHashMap<std::string, int> map1;
		FastHashMap<std::string, int> map2;

		map1.insertOrAssign( "a", 1 );
		map1.insertOrAssign( "b", 2 );

		map2.insertOrAssign( "a", 1 );
		map2.insertOrAssign( "b", 2 );

		EXPECT_TRUE( map1 == map2 );
		EXPECT_FALSE( map1 != map2 );
	}

	TEST( FastHashMapTests, ComparisonOperators_NotEqual_DifferentValues )
	{
		FastHashMap<std::string, int> map1;
		FastHashMap<std::string, int> map2;

		map1.insertOrAssign( "a", 1 );
		map1.insertOrAssign( "b", 2 );

		map2.insertOrAssign( "a", 1 );
		map2.insertOrAssign( "b", 999 ); // Different value

		EXPECT_FALSE( map1 == map2 );
		EXPECT_TRUE( map1 != map2 );
	}

	TEST( FastHashMapTests, ComparisonOperators_NotEqual_DifferentSize )
	{
		FastHashMap<std::string, int> map1;
		FastHashMap<std::string, int> map2;

		map1.insertOrAssign( "a", 1 );
		map1.insertOrAssign( "b", 2 );

		map2.insertOrAssign( "a", 1 );

		EXPECT_FALSE( map1 == map2 );
		EXPECT_TRUE( map1 != map2 );
	}

	//=====================================================================
	// swap() - Container swap tests
	//=====================================================================

	TEST( FastHashMapTests, Swap_BasicOperation )
	{
		FastHashMap<std::string, int> map1;
		FastHashMap<std::string, int> map2;

		map1.insertOrAssign( "a", 1 );
		map1.insertOrAssign( "b", 2 );

		map2.insertOrAssign( "x", 10 );
		map2.insertOrAssign( "y", 20 );
		map2.insertOrAssign( "z", 30 );

		EXPECT_EQ( map1.size(), 2 );
		EXPECT_EQ( map2.size(), 3 );

		map1.swap( map2 );

		// Sizes swapped
		EXPECT_EQ( map1.size(), 3 );
		EXPECT_EQ( map2.size(), 2 );

		// Contents swapped
		EXPECT_EQ( map1.at( "x" ), 10 );
		EXPECT_EQ( map1.at( "y" ), 20 );
		EXPECT_EQ( map1.at( "z" ), 30 );

		EXPECT_EQ( map2.at( "a" ), 1 );
		EXPECT_EQ( map2.at( "b" ), 2 );
	}

	TEST( FastHashMapTests, Swap_EmptyWithFilled )
	{
		FastHashMap<std::string, int> empty_map;
		FastHashMap<std::string, int> filled_map;

		filled_map.insertOrAssign( "key1", 100 );
		filled_map.insertOrAssign( "key2", 200 );

		EXPECT_EQ( empty_map.size(), 0 );
		EXPECT_EQ( filled_map.size(), 2 );

		empty_map.swap( filled_map );

		EXPECT_EQ( empty_map.size(), 2 );
		EXPECT_EQ( filled_map.size(), 0 );

		EXPECT_EQ( empty_map.at( "key1" ), 100 );
		EXPECT_EQ( empty_map.at( "key2" ), 200 );
	}

	TEST( FastHashMapTests, Swap_BothEmpty )
	{
		FastHashMap<std::string, int> map1;
		FastHashMap<std::string, int> map2;

		EXPECT_EQ( map1.size(), 0 );
		EXPECT_EQ( map2.size(), 0 );

		map1.swap( map2 );

		EXPECT_EQ( map1.size(), 0 );
		EXPECT_EQ( map2.size(), 0 );
	}

	TEST( FastHashMapTests, Swap_DifferentCapacities )
	{
		FastHashMap<std::string, int> small_map( 32 );
		FastHashMap<std::string, int> large_map( 128 );

		small_map.insertOrAssign( "small", 1 );
		large_map.insertOrAssign( "large", 999 );

		size_t small_capacity = small_map.capacity();
		size_t large_capacity = large_map.capacity();

		small_map.swap( large_map );

		// Capacities swapped
		EXPECT_EQ( small_map.capacity(), large_capacity );
		EXPECT_EQ( large_map.capacity(), small_capacity );

		// Contents swapped
		EXPECT_EQ( small_map.at( "large" ), 999 );
		EXPECT_EQ( large_map.at( "small" ), 1 );
	}

	TEST( FastHashMapTests, Swap_StdSwapCompatibility )
	{
		FastHashMap<std::string, int> map1;
		FastHashMap<std::string, int> map2;

		map1.insertOrAssign( "first", 1 );
		map2.insertOrAssign( "second", 2 );

		// Use std::swap
		std::swap( map1, map2 );

		EXPECT_EQ( map1.at( "second" ), 2 );
		EXPECT_EQ( map2.at( "first" ), 1 );
	}

	//=====================================================================
	// Value type tests
	//=====================================================================

	TEST( FastHashMapTests, ValueTypes_ComplexValues )
	{
		FastHashMap<std::string, std::vector<int>> map;

		map.insertOrAssign( "vector1", std::vector<int>{ 1, 2, 3 } );
		map.insertOrAssign( "vector2", std::vector<int>{ 4, 5, 6, 7 } );

		std::vector<int>* value1 = nullptr;
		std::vector<int>* value2 = nullptr;

		value1 = map.find( "vector1" );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( "vector2" );

		EXPECT_NE( value2, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );

		EXPECT_EQ( value1->size(), 3 );
		EXPECT_EQ( value2->size(), 4 );
		EXPECT_EQ( ( *value1 )[0], 1 );
		EXPECT_EQ( ( *value2 )[3], 7 );
	}

	TEST( FastHashMapTests, ValueTypes_MoveSemantics )
	{
		FastHashMap<std::string, std::unique_ptr<int>> map;

		map.insertOrAssign( "unique1", std::make_unique<int>( 42 ) );
		map.insertOrAssign( "unique2", std::make_unique<int>( 84 ) );

		std::unique_ptr<int>* value1 = nullptr;
		std::unique_ptr<int>* value2 = nullptr;

		value1 = map.find( "unique1" );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( "unique2" );

		EXPECT_NE( value2, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value1->get(), nullptr );
		ASSERT_NE( value2->get(), nullptr );

		EXPECT_EQ( **value1, 42 );
		EXPECT_EQ( **value2, 84 );
	}

	//=====================================================================
	// std::hash tests - Verify compatibility with standard library hashers
	//=====================================================================

	TEST( FastHashMapTests, StdHash_StringKeys )
	{
		// Using std::hash<std::string> instead of nfx::hashing::Hasher
		FastHashMap<std::string, int, uint32_t, 0, std::hash<std::string>> map;

		map.insertOrAssign( "apple", 100 );
		map.insertOrAssign( "banana", 200 );
		map.insertOrAssign( "cherry", 300 );

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( "apple" );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( "banana" );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( "cherry" );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 100 );
		EXPECT_EQ( *value2, 200 );
		EXPECT_EQ( *value3, 300 );
	}

	TEST( FastHashMapTests, StdHash_IntegerKeys )
	{
		FastHashMap<int, std::string, uint32_t, 0, std::hash<int>> map;

		map.insertOrAssign( 42, "answer" );
		map.insertOrAssign( 100, "century" );
		map.insertOrAssign( -1, "negative" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( 42 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 100 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( -1 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "answer" );
		EXPECT_EQ( *value2, "century" );
		EXPECT_EQ( *value3, "negative" );
	}

	TEST( FastHashMapTests, StdHash_EnumKeys )
	{
		enum class Status
		{
			Active,
			Inactive,
			Pending
		};

		FastHashMap<Status, std::string, uint32_t, 0, std::hash<Status>> map;

		map.insertOrAssign( Status::Active, "running" );
		map.insertOrAssign( Status::Inactive, "stopped" );
		map.insertOrAssign( Status::Pending, "waiting" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( Status::Active );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( Status::Inactive );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( Status::Pending );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "running" );
		EXPECT_EQ( *value2, "stopped" );
		EXPECT_EQ( *value3, "waiting" );
	}

	TEST( FastHashMapTests, StdHash_FloatKeys )
	{
		FastHashMap<double, std::string, uint32_t, 0, std::hash<double>> map;

		map.insertOrAssign( 3.14, "pi" );
		map.insertOrAssign( 2.71, "e" );
		map.insertOrAssign( 1.41, "sqrt2" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( 3.14 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 2.71 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( 1.41 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "pi" );
		EXPECT_EQ( *value2, "e" );
		EXPECT_EQ( *value3, "sqrt2" );
	}

	TEST( FastHashMapTests, StdHash_PointerKeys )
	{
		int a = 1, b = 2, c = 3;
		FastHashMap<int*, std::string, uint32_t, 0, std::hash<int*>> map;

		map.insertOrAssign( &a, "first" );
		map.insertOrAssign( &b, "second" );
		map.insertOrAssign( &c, "third" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( &a );
		EXPECT_NE( value1, nullptr );
		value2 = map.find( &b );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( &c );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "first" );
		EXPECT_EQ( *value2, "second" );
		EXPECT_EQ( *value3, "third" );
	}

	// Custom hash functor for pairs
	template <typename T1, typename T2>
	struct PairHash
	{
		std::size_t operator()( const std::pair<T1, T2>& p ) const
		{
			std::size_t h1 = std::hash<T1>{}( p.first );
			std::size_t h2 = std::hash<T2>{}( p.second );
			return h1 ^ ( h2 << 1 );
		}
	};

	TEST( FastHashMapTests, StdHash_PairKeys )
	{
		using KeyType = std::pair<int, int>;
		FastHashMap<KeyType, std::string, uint32_t, 0, PairHash<int, int>> map;

		map.insertOrAssign( { 1, 2 }, "pair_1_2" );
		map.insertOrAssign( { 3, 4 }, "pair_3_4" );
		map.insertOrAssign( { 5, 6 }, "pair_5_6" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( KeyType{ 1, 2 } );
		EXPECT_NE( value1, nullptr );
		value2 = map.find( KeyType{ 3, 4 } );
		EXPECT_NE( value2, nullptr );
		value3 = map.find( KeyType{ 5, 6 } );
		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "pair_1_2" );
		EXPECT_EQ( *value2, "pair_3_4" );
		EXPECT_EQ( *value3, "pair_5_6" );
	}

	//=====================================================================
	// Custom hasher tests - Verify custom hash functions work correctly
	//=====================================================================

	// Simple multiplicative hasher
	struct SimpleMultiplicativeHasher
	{
		uint32_t operator()( uint32_t key ) const
		{
			return key * 2654435761u; // Knuth's multiplicative hash
		}
	};

	TEST( FastHashMapTests, CustomHasher_SimpleMultiplicative )
	{
		FastHashMap<uint32_t, std::string, uint32_t, 0, SimpleMultiplicativeHasher> map;

		map.insertOrAssign( 1, "one" );
		map.insertOrAssign( 100, "hundred" );
		map.insertOrAssign( 1000, "thousand" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( 1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 100 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( 1000 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "one" );
		EXPECT_EQ( *value2, "hundred" );
		EXPECT_EQ( *value3, "thousand" );
	}

	// Identity hasher (for testing - returns key as-is)
	struct Identity32Hasher
	{
		uint32_t operator()( uint32_t key ) const
		{
			return key;
		}
	};

	TEST( FastHashMapTests, CustomHasher_Identity32 )
	{
		FastHashMap<uint32_t, int, uint32_t, 0, Identity32Hasher> map;

		map.insertOrAssign( 10, 100 );
		map.insertOrAssign( 20, 200 );
		map.insertOrAssign( 30, 300 );

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( 10 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 20 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( 30 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 100 );
		EXPECT_EQ( *value2, 200 );
		EXPECT_EQ( *value3, 300 );
	}

	// XOR-shift hasher
	struct XorShiftHasher
	{
		uint32_t operator()( uint32_t key ) const
		{
			key ^= key << 13;
			key ^= key >> 17;
			key ^= key << 5;
			return key;
		}
	};

	TEST( FastHashMapTests, CustomHasher_XorShift )
	{
		FastHashMap<uint32_t, std::string, uint32_t, 0, XorShiftHasher> map;

		map.insertOrAssign( 42, "answer" );
		map.insertOrAssign( 1337, "leet" );
		map.insertOrAssign( 9001, "over_9000" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( 42 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( 1337 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( 9001 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "answer" );
		EXPECT_EQ( *value2, "leet" );
		EXPECT_EQ( *value3, "over_9000" );
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
	TEST( FastHashMapTests, CustomType_ComplexObject )
	{
		ComplexObject obj1{ 1, "alpha", 3.14159, { 10, 20, 30 } };
		ComplexObject obj2{ 2, "beta", 2.71828, { 40, 50 } };
		ComplexObject obj3{ 3, "gamma", 1.41421, { 60, 70, 80, 90 } };

		FastHashMap<ComplexObject, std::string> map;
		map.insertOrAssign( obj1, "first_object" );
		map.insertOrAssign( obj2, "second_object" );
		map.insertOrAssign( obj3, "third_object" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( obj1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( obj2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( obj3 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "first_object" );
		EXPECT_EQ( *value2, "second_object" );
		EXPECT_EQ( *value3, "third_object" );
		EXPECT_EQ( map.size(), 3 );

		ComplexObject obj4{ 999, "nonexistent", 0.0, {} };
		std::string* value4 = map.find( obj4 );

		EXPECT_EQ( value4, nullptr );
		EXPECT_EQ( value4, nullptr );
	}

	TEST( FastHashMapTests, CustomType_NestedComplexity )
	{
		// Test deeply nested pair types that Hasher handles natively
		using SimplerKey = std::pair<std::pair<int, std::string>, double>;

		SimplerKey key1{ { 1, "one" }, 1.1 };
		SimplerKey key2{ { 2, "two" }, 2.2 };
		SimplerKey key3{ { 3, "three" }, 3.3 };

		FastHashMap<SimplerKey, int> map;
		map.insertOrAssign( key1, 100 );
		map.insertOrAssign( key2, 200 );
		map.insertOrAssign( key3, 300 );

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( key1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( key2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( key3 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 100 );
		EXPECT_EQ( *value2, 200 );
		EXPECT_EQ( *value3, 300 );

		SimplerKey key4{ { 99, "missing" }, 9.9 };
		int* value4 = map.find( key4 );

		EXPECT_EQ( value4, nullptr );
	}

	TEST( FastHashMapTests, CustomType_TripleNestedPairs )
	{
		// Test triple-nested pairs: pair<pair<pair<int, int>, string>, double>
		using Level1 = std::pair<int, int>;
		using Level2 = std::pair<Level1, std::string>;
		using Level3 = std::pair<Level2, double>;

		Level3 key1{ { { 1, 10 }, "a" }, 1.5 };
		Level3 key2{ { { 2, 20 }, "b" }, 2.5 };
		Level3 key3{ { { 3, 30 }, "c" }, 3.5 };

		FastHashMap<Level3, std::string> map;
		map.insertOrAssign( key1, "level1" );
		map.insertOrAssign( key2, "level2" );
		map.insertOrAssign( key3, "level3" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;

		value1 = map.find( key1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( key2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( key3 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, "level1" );
		EXPECT_EQ( *value2, "level2" );
		EXPECT_EQ( *value3, "level3" );
		EXPECT_EQ( map.size(), 3 );
	}

	TEST( FastHashMapTests, CustomType_MixedPairTypes )
	{
		// Test pair with different fundamental types: pair<string, pair<float, int>>
		using InnerMix = std::pair<float, int>;
		using MixedKey = std::pair<std::string, InnerMix>;

		MixedKey key1{ "test", { 1.5f, 10 } };
		MixedKey key2{ "demo", { 2.5f, 20 } };
		MixedKey key3{ "sample", { 3.5f, 30 } };

		FastHashMap<MixedKey, int> map;
		map.insertOrAssign( key1, 111 );
		map.insertOrAssign( key2, 222 );
		map.insertOrAssign( key3, 333 );

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( key1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( key2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( key3 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 111 );
		EXPECT_EQ( *value2, 222 );
		EXPECT_EQ( *value3, 333 );

		MixedKey key4{ "missing", { 9.9f, 99 } };
		int* value4 = map.find( key4 );

		EXPECT_EQ( value4, nullptr );
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
	TEST( FastHashMapTests, CustomType_RGBColor_BitPacking )
	{
		RGBColor red{ 255, 0, 0 };
		RGBColor green{ 0, 255, 0 };
		RGBColor blue{ 0, 0, 255 };
		RGBColor white{ 255, 255, 255 };
		RGBColor black{ 0, 0, 0 };

		FastHashMap<RGBColor, std::string> map;
		map.insertOrAssign( red, "red" );
		map.insertOrAssign( green, "green" );
		map.insertOrAssign( blue, "blue" );
		map.insertOrAssign( white, "white" );
		map.insertOrAssign( black, "black" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;
		std::string* value4 = nullptr;
		std::string* value5 = nullptr;

		value1 = map.find( red );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( green );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( blue );

		EXPECT_NE( value3, nullptr );
		value4 = map.find( white );

		EXPECT_NE( value4, nullptr );
		value5 = map.find( black );

		EXPECT_NE( value5, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );
		ASSERT_NE( value4, nullptr );
		ASSERT_NE( value5, nullptr );

		EXPECT_EQ( *value1, "red" );
		EXPECT_EQ( *value2, "green" );
		EXPECT_EQ( *value3, "blue" );
		EXPECT_EQ( *value4, "white" );
		EXPECT_EQ( *value5, "black" );
		EXPECT_EQ( map.size(), 5 );

		RGBColor purple{ 128, 0, 128 };
		std::string* value6 = map.find( purple );

		EXPECT_EQ( value6, nullptr );
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
	TEST( FastHashMapTests, CustomType_Point3D_BitRepresentation )
	{
		Point3D p1{ 1.0f, 2.0f, 3.0f };
		Point3D p2{ 4.0f, 5.0f, 6.0f };
		Point3D p3{ 7.0f, 8.0f, 9.0f };
		Point3D p4{ 0.0f, 0.0f, 0.0f };

		FastHashMap<Point3D, std::string> map;
		map.insertOrAssign( p1, "point_one" );
		map.insertOrAssign( p2, "point_two" );
		map.insertOrAssign( p3, "point_three" );
		map.insertOrAssign( p4, "origin" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;
		std::string* value4 = nullptr;

		value1 = map.find( p1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( p2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( p3 );

		EXPECT_NE( value3, nullptr );
		value4 = map.find( p4 );

		EXPECT_NE( value4, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );
		ASSERT_NE( value4, nullptr );

		EXPECT_EQ( *value1, "point_one" );
		EXPECT_EQ( *value2, "point_two" );
		EXPECT_EQ( *value3, "point_three" );
		EXPECT_EQ( *value4, "origin" );
		EXPECT_EQ( map.size(), 4 );

		Point3D p5{ 99.0f, 99.0f, 99.0f };
		std::string* value5 = map.find( p5 );

		EXPECT_EQ( value5, nullptr );
		EXPECT_EQ( value5, nullptr );
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
		uint16_t department;	  // 0-65535
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

	TEST( FastHashMapTests, CustomType_CustomHasher_EmployeeID )
	{
		EmployeeID emp1{ 10, 12345 }; // Department 10, Employee #12345
		EmployeeID emp2{ 10, 67890 }; // Same department, different employee
		EmployeeID emp3{ 20, 12345 }; // Different department, same employee number
		EmployeeID emp4{ 50, 99999 }; // Department 50, Employee #99999
		EmployeeID emp5{ 100, 1 };	  // Department 100, Employee #1

		// Template params: <Key, Value, HashType, Seed, CustomHasher>
		FastHashMap<EmployeeID, std::string, uint32_t, 0x811c9dc5, EmployeeIDHasher<uint32_t, 0x811c9dc5>> map;

		map.insertOrAssign( emp1, "Alice Johnson" );
		map.insertOrAssign( emp2, "Bob Smith" );
		map.insertOrAssign( emp3, "Charlie Brown" );
		map.insertOrAssign( emp4, "Diana Prince" );
		map.insertOrAssign( emp5, "Eve Wilson" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;
		std::string* value4 = nullptr;
		std::string* value5 = nullptr;

		value1 = map.find( emp1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( emp2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( emp3 );

		EXPECT_NE( value3, nullptr );
		value4 = map.find( emp4 );

		EXPECT_NE( value4, nullptr );
		value5 = map.find( emp5 );

		EXPECT_NE( value5, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );
		ASSERT_NE( value4, nullptr );
		ASSERT_NE( value5, nullptr );

		EXPECT_EQ( *value1, "Alice Johnson" );
		EXPECT_EQ( *value2, "Bob Smith" );
		EXPECT_EQ( *value3, "Charlie Brown" );
		EXPECT_EQ( *value4, "Diana Prince" );
		EXPECT_EQ( *value5, "Eve Wilson" );
		EXPECT_EQ( map.size(), 5 );

		EmployeeID emp_missing{ 999, 999999 };
		std::string* value_missing = map.find( emp_missing );

		EXPECT_EQ( value_missing, nullptr );
	}

	TEST( FastHashMapTests, CustomType_CustomHasher_64bit )
	{
		// Same custom type, but with 64-bit hasher variant
		EmployeeID emp1{ 1, 1000 };
		EmployeeID emp2{ 2, 2000 };
		EmployeeID emp3{ 3, 3000 };

		FastHashMap<EmployeeID, int, uint64_t, 0xcbf29ce484222325ull,
			EmployeeIDHasher<uint64_t, 0xcbf29ce484222325ull>>
			map;

		map.insertOrAssign( emp1, 100 );
		map.insertOrAssign( emp2, 200 );
		map.insertOrAssign( emp3, 300 );

		int* value1 = nullptr;
		int* value2 = nullptr;
		int* value3 = nullptr;

		value1 = map.find( emp1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( emp2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( emp3 );

		EXPECT_NE( value3, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 100 );
		EXPECT_EQ( *value2, 200 );
		EXPECT_EQ( *value3, 300 );
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

	TEST( FastHashMapTests, CustomType_CustomHasher_IPAddress )
	{
		IPAddress ip1{ 192, 168, 1, 1 };   // 192.168.1.1
		IPAddress ip2{ 192, 168, 1, 100 }; // 192.168.1.100
		IPAddress ip3{ 10, 0, 0, 1 };	   // 10.0.0.1
		IPAddress ip4{ 172, 16, 0, 1 };	   // 172.16.0.1
		IPAddress ip5{ 8, 8, 8, 8 };	   // 8.8.8.8 (Google DNS)

		// Custom IP address hasher with FastHashMap
		FastHashMap<IPAddress, std::string, uint32_t, 0x12345678,
			IPAddressHasher<uint32_t, 0x12345678>>
			map;

		map.insertOrAssign( ip1, "router" );
		map.insertOrAssign( ip2, "workstation" );
		map.insertOrAssign( ip3, "gateway" );
		map.insertOrAssign( ip4, "server" );
		map.insertOrAssign( ip5, "dns" );

		std::string* value1 = nullptr;
		std::string* value2 = nullptr;
		std::string* value3 = nullptr;
		std::string* value4 = nullptr;
		std::string* value5 = nullptr;

		value1 = map.find( ip1 );

		EXPECT_NE( value1, nullptr );
		value2 = map.find( ip2 );

		EXPECT_NE( value2, nullptr );
		value3 = map.find( ip3 );

		EXPECT_NE( value3, nullptr );
		value4 = map.find( ip4 );

		EXPECT_NE( value4, nullptr );
		value5 = map.find( ip5 );

		EXPECT_NE( value5, nullptr );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );
		ASSERT_NE( value4, nullptr );
		ASSERT_NE( value5, nullptr );

		EXPECT_EQ( *value1, "router" );
		EXPECT_EQ( *value2, "workstation" );
		EXPECT_EQ( *value3, "gateway" );
		EXPECT_EQ( *value4, "server" );
		EXPECT_EQ( *value5, "dns" );
		EXPECT_EQ( map.size(), 5 );

		IPAddress ip_missing{ 255, 255, 255, 255 };
		std::string* value_missing = map.find( ip_missing );

		EXPECT_EQ( value_missing, nullptr );
	}

	//=====================================================================
	// Emplace tests
	//=====================================================================

	TEST( FastHashMapTests, Emplace_BasicTypes )
	{
		FastHashMap<std::string, int> map;

		// Emplace with const key
		map.emplace( "one", 1 );
		map.emplace( "two", 2 );
		map.emplace( "three", 3 );

		EXPECT_EQ( map.size(), 3 );

		int* value1 = map.find( "one" );
		int* value2 = map.find( "two" );
		int* value3 = map.find( "three" );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 1 );
		EXPECT_EQ( *value2, 2 );
		EXPECT_EQ( *value3, 3 );
	}

	TEST( FastHashMapTests, Emplace_RvalueKey )
	{
		FastHashMap<std::string, int> map;

		// Emplace with rvalue key (moved)
		std::string key1 = "alpha";
		std::string key2 = "beta";
		std::string key3 = "gamma";

		map.emplace( std::move( key1 ), 10 );
		map.emplace( std::move( key2 ), 20 );
		map.emplace( std::move( key3 ), 30 );

		EXPECT_EQ( map.size(), 3 );

		int* value1 = map.find( "alpha" );
		int* value2 = map.find( "beta" );
		int* value3 = map.find( "gamma" );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( *value1, 10 );
		EXPECT_EQ( *value2, 20 );
		EXPECT_EQ( *value3, 30 );
	}

	TEST( FastHashMapTests, Emplace_ComplexValue )
	{
		// Test emplacing complex values constructed in-place
		struct ComplexValue
		{
			std::string name;
			int id;
			double score;

			ComplexValue() = default; // Need default constructor for Bucket

			ComplexValue( const std::string& n, int i, double s ) : name{ n },
																	id{ i },
																	score{ s }
			{
			}
		};

		FastHashMap<std::string, ComplexValue> map;

		// Emplace constructs ComplexValue in-place (no temporary object)
		map.emplace( "first", "Alice", 101, 95.5 );
		map.emplace( "second", "Bob", 102, 87.3 );
		map.emplace( "third", "Charlie", 103, 92.1 );

		EXPECT_EQ( map.size(), 3 );

		ComplexValue* value1 = map.find( "first" );
		ComplexValue* value2 = map.find( "second" );
		ComplexValue* value3 = map.find( "third" );

		ASSERT_NE( value1, nullptr );
		ASSERT_NE( value2, nullptr );
		ASSERT_NE( value3, nullptr );

		EXPECT_EQ( value1->name, "Alice" );
		EXPECT_EQ( value1->id, 101 );
		EXPECT_DOUBLE_EQ( value1->score, 95.5 );

		EXPECT_EQ( value2->name, "Bob" );
		EXPECT_EQ( value2->id, 102 );
		EXPECT_DOUBLE_EQ( value2->score, 87.3 );

		EXPECT_EQ( value3->name, "Charlie" );
		EXPECT_EQ( value3->id, 103 );
		EXPECT_DOUBLE_EQ( value3->score, 92.1 );
	}

	TEST( FastHashMapTests, Emplace_UpdateExisting )
	{
		FastHashMap<std::string, int> map;

		// Initial insert
		map.emplace( "key", 100 );
		EXPECT_EQ( map.size(), 1 );

		int* value = map.find( "key" );
		ASSERT_NE( value, nullptr );
		EXPECT_EQ( *value, 100 );

		// Update with emplace (should replace value)
		map.emplace( "key", 200 );
		EXPECT_EQ( map.size(), 1 ); // Size should not change

		value = map.find( "key" );
		ASSERT_NE( value, nullptr );
		EXPECT_EQ( *value, 200 );
	}

	TEST( FastHashMapTests, Emplace_VectorValue )
	{
		FastHashMap<int, std::vector<int>> map;

		// Emplace vector with initializer list
		map.emplace( 1, std::initializer_list<int>{ 1, 2, 3 } );
		map.emplace( 2, std::initializer_list<int>{ 4, 5, 6, 7 } );
		map.emplace( 3, std::initializer_list<int>{ 8, 9 } );

		EXPECT_EQ( map.size(), 3 );

		std::vector<int>* vec1 = map.find( 1 );
		std::vector<int>* vec2 = map.find( 2 );
		std::vector<int>* vec3 = map.find( 3 );

		ASSERT_NE( vec1, nullptr );
		ASSERT_NE( vec2, nullptr );
		ASSERT_NE( vec3, nullptr );

		EXPECT_EQ( vec1->size(), 3 );
		EXPECT_EQ( ( *vec1 )[0], 1 );
		EXPECT_EQ( ( *vec1 )[2], 3 );

		EXPECT_EQ( vec2->size(), 4 );
		EXPECT_EQ( ( *vec2 )[0], 4 );
		EXPECT_EQ( ( *vec2 )[3], 7 );

		EXPECT_EQ( vec3->size(), 2 );
		EXPECT_EQ( ( *vec3 )[0], 8 );
		EXPECT_EQ( ( *vec3 )[1], 9 );
	}

	TEST( FastHashMapTests, Emplace_StringConstruction )
	{
		FastHashMap<int, std::string> map;

		// Emplace string with constructor arguments (count, char)
		map.emplace( 1, 5, 'a' ); // std::string(5, 'a') -> "aaaaa"
		map.emplace( 2, 3, 'b' ); // std::string(3, 'b') -> "bbb"
		map.emplace( 3, 7, 'x' ); // std::string(7, 'x') -> "xxxxxxx"

		EXPECT_EQ( map.size(), 3 );

		std::string* str1 = map.find( 1 );
		std::string* str2 = map.find( 2 );
		std::string* str3 = map.find( 3 );

		ASSERT_NE( str1, nullptr );
		ASSERT_NE( str2, nullptr );
		ASSERT_NE( str3, nullptr );

		EXPECT_EQ( *str1, "aaaaa" );
		EXPECT_EQ( *str2, "bbb" );
		EXPECT_EQ( *str3, "xxxxxxx" );
	}

	TEST( FastHashMapTests, Emplace_WithResize )
	{
		FastHashMap<int, std::string> map;

		// Insert enough elements to trigger resize
		for ( int i = 0; i < 100; ++i )
		{
			map.emplace( i, 10, static_cast<char>( 'A' + ( i % 26 ) ) );
		}

		EXPECT_EQ( map.size(), 100 );

		// Verify all elements are accessible
		for ( int i = 0; i < 100; ++i )
		{
			std::string* value = map.find( i );
			ASSERT_NE( value, nullptr );
			EXPECT_EQ( value->size(), 10 );
			EXPECT_EQ( ( *value )[0], static_cast<char>( 'A' + ( i % 26 ) ) );
		}
	}

	//=====================================================================
	// Contains method tests
	//=====================================================================

	TEST( FastHashMapTests, Contains_BasicUsage )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "apple", 1 );
		map.insertOrAssign( "banana", 2 );

		EXPECT_TRUE( map.contains( "apple" ) );
		EXPECT_TRUE( map.contains( "banana" ) );
		EXPECT_FALSE( map.contains( "cherry" ) );
		EXPECT_FALSE( map.contains( "nonexistent" ) );
	}

	TEST( FastHashMapTests, Contains_HeterogeneousLookup )
	{
		FastHashMap<std::string, int> map;

		map.insertOrAssign( "test", 42 );

		// Heterogeneous lookup with string_view
		EXPECT_TRUE( map.contains( std::string_view{ "test" } ) );
		EXPECT_FALSE( map.contains( std::string_view{ "missing" } ) );
	}

	TEST( FastHashMapTests, Contains_AfterErase )
	{
		FastHashMap<int, std::string> map;

		map.insertOrAssign( 1, "one" );
		map.insertOrAssign( 2, "two" );
		map.insertOrAssign( 3, "three" );

		EXPECT_TRUE( map.contains( 2 ) );
		map.erase( 2 );
		EXPECT_FALSE( map.contains( 2 ) );
		EXPECT_TRUE( map.contains( 1 ) );
		EXPECT_TRUE( map.contains( 3 ) );
	}

	//=====================================================================
	// Insert method tests (no overwrite)
	//=====================================================================

	TEST( FastHashMapTests, Insert_BasicUsage )
	{
		FastHashMap<std::string, int> map;

		// First insert should succeed
		EXPECT_TRUE( map.insert( "apple", 1 ) );
		EXPECT_EQ( map.size(), 1 );
		EXPECT_EQ( *map.find( "apple" ), 1 );

		// Second insert with same key should fail (no overwrite)
		EXPECT_FALSE( map.insert( "apple", 999 ) );
		EXPECT_EQ( map.size(), 1 );
		EXPECT_EQ( *map.find( "apple" ), 1 ); // Value unchanged
	}

	TEST( FastHashMapTests, Insert_MoveSemantics )
	{
		FastHashMap<std::string, std::string> map;

		std::string key = "key";
		std::string value = "value";

		// Insert with move semantics
		EXPECT_TRUE( map.insert( std::move( key ), std::move( value ) ) );
		EXPECT_EQ( map.size(), 1 );
		EXPECT_EQ( *map.find( "key" ), "value" );

		// Try to insert again (should fail)
		EXPECT_FALSE( map.insert( "key", "new_value" ) );
		EXPECT_EQ( *map.find( "key" ), "value" ); // Original value preserved
	}

	TEST( FastHashMapTests, Insert_DifferentFromInsertOrAssign )
	{
		FastHashMap<int, std::string> map;

		// insert() doesn't overwrite
		map.insert( 1, "first" );
		bool inserted = map.insert( 1, "second" );
		EXPECT_FALSE( inserted );
		EXPECT_EQ( *map.find( 1 ), "first" );

		// insertOrAssign() does overwrite
		map.insertOrAssign( 1, "updated" );
		EXPECT_EQ( *map.find( 1 ), "updated" );
	}

	TEST( FastHashMapTests, Insert_WithResize )
	{
		FastHashMap<int, std::string> map( 4 ); // Small initial capacity

		// Insert enough to trigger resize
		for ( int i = 0; i < 20; ++i )
		{
			EXPECT_TRUE( map.insert( i, std::to_string( i ) ) );
		}

		EXPECT_EQ( map.size(), 20 );

		// Verify all keys exist
		for ( int i = 0; i < 20; ++i )
		{
			EXPECT_TRUE( map.contains( i ) );
			EXPECT_EQ( *map.find( i ), std::to_string( i ) );
		}

		// Try to insert duplicates (should all fail)
		for ( int i = 0; i < 20; ++i )
		{
			EXPECT_FALSE( map.insert( i, "duplicate" ) );
		}

		EXPECT_EQ( map.size(), 20 ); // Size unchanged
	}

	//=====================================================================
	// Iterator-based erase tests
	//=====================================================================

	TEST( FastHashMapTests, EraseIterator_SingleElement )
	{
		FastHashMap<int, std::string> map;
		map.insert( 1, "one" );
		map.insert( 2, "two" );
		map.insert( 3, "three" );

		auto it = map.begin();
		while ( it != map.end() && it->first != 2 )
		{
			++it;
		}

		ASSERT_NE( it, map.end() );
		auto nextIt = map.erase( static_cast<FastHashMap<int, std::string>::ConstIterator>( it ) );
		(void)nextIt;

		EXPECT_EQ( map.size(), 2 );
		EXPECT_EQ( map.find( 2 ), nullptr );
		EXPECT_NE( map.find( 1 ), nullptr );
		EXPECT_NE( map.find( 3 ), nullptr );
	}

	TEST( FastHashMapTests, EraseIterator_Range )
	{
		FastHashMap<int, std::string> map;
		for ( int i = 0; i < 10; ++i )
		{
			map.insert( i, std::to_string( i ) );
		}

		size_t eraseCount = 0;
		auto it = map.begin();
		auto first = it;

		// Find range to erase (first 5 elements)
		for ( int i = 0; i < 5 && it != map.end(); ++i, ++it )
		{
			++eraseCount;
		}

		size_t originalSize = map.size();
		map.erase( first, it );

		EXPECT_EQ( map.size(), originalSize - eraseCount );
	}

	TEST( FastHashMapTests, EraseIterator_AllElements )
	{
		FastHashMap<int, int> map;
		for ( int i = 0; i < 20; ++i )
		{
			map.insert( i, i * 10 );
		}

		map.erase( map.begin(), map.end() );

		EXPECT_EQ( map.size(), 0 );
		EXPECT_TRUE( map.isEmpty() );
	}

	//=====================================================================
	// tryEmplace tests
	//=====================================================================

	TEST( FastHashMapTests, TryEmplace_NewKey )
	{
		FastHashMap<std::string, std::string> map;

		auto [it, inserted] = map.tryEmplace( "hello", "world" );

		EXPECT_TRUE( inserted );
		EXPECT_EQ( it->first, "hello" );
		EXPECT_EQ( it->second, "world" );
		EXPECT_EQ( map.size(), 1 );
	}

	TEST( FastHashMapTests, TryEmplace_ExistingKey )
	{
		FastHashMap<std::string, std::string> map;
		map.insert( "hello", "world" );

		auto [it, inserted] = map.tryEmplace( "hello", "universe" );

		EXPECT_FALSE( inserted );
		EXPECT_EQ( it->first, "hello" );
		EXPECT_EQ( it->second, "world" ); // Original value unchanged
		EXPECT_EQ( map.size(), 1 );
	}

	TEST( FastHashMapTests, TryEmplace_MoveKey )
	{
		FastHashMap<std::string, int> map;

		std::string key = "movable";
		auto [it, inserted] = map.tryEmplace( std::move( key ), 42 );

		EXPECT_TRUE( inserted );
		EXPECT_EQ( it->first, "movable" );
		EXPECT_EQ( it->second, 42 );
		EXPECT_EQ( map.size(), 1 );
	}

	TEST( FastHashMapTests, TryEmplace_MultipleArgs )
	{
		struct ComplexValue
		{
			std::string s;
			int i;
			double d;

			ComplexValue() = default;

			ComplexValue( std::string str, int num, double dbl )
				: s( std::move( str ) ), i( num ), d( dbl )
			{
			}
		};

		FastHashMap<int, ComplexValue> map;
		auto [it, inserted] = map.tryEmplace( 1, "test", 42, 3.14 );

		EXPECT_TRUE( inserted );
		EXPECT_EQ( it->second.s, "test" );
		EXPECT_EQ( it->second.i, 42 );
		EXPECT_DOUBLE_EQ( it->second.d, 3.14 );
	}

	TEST( FastHashMapTests, TryEmplace_VsEmplace_PerformanceBehavior )
	{
		static int constructCount = 0;

		struct ConstructionCounter
		{
			ConstructionCounter()
			{
				++constructCount;
			}

			ConstructionCounter( int )
			{
				++constructCount;
			}
		};

		FastHashMap<int, ConstructionCounter> map;

		// First insert with tryEmplace
		constructCount = 0;
		map.tryEmplace( 1, 100 );
		int firstCount = constructCount;
		EXPECT_EQ( firstCount, 1 ); // One construction

		// Try to insert again (should NOT construct)
		constructCount = 0;
		map.tryEmplace( 1, 200 );
		int secondCount = constructCount;
		EXPECT_EQ( secondCount, 0 ); // No construction - key exists!
	}

	//=====================================================================
	// Move semantics - constructor and assignment
	//=====================================================================

	TEST( FastHashMapTests, MoveConstructor_TransfersOwnership )
	{
		FastHashMap<std::string, int> map1;
		map1.insertOrAssign( "alpha", 1 );
		map1.insertOrAssign( "beta", 2 );
		map1.insertOrAssign( "gamma", 3 );

		FastHashMap<std::string, int> map2( std::move( map1 ) );

		EXPECT_EQ( map2.size(), 3 );
		EXPECT_EQ( *map2.find( "alpha" ), 1 );
		EXPECT_EQ( *map2.find( "beta" ), 2 );
		EXPECT_EQ( *map2.find( "gamma" ), 3 );

		// Verify source is moved-from (should be empty)
		EXPECT_EQ( map1.size(), 0 );
		EXPECT_TRUE( map1.isEmpty() );
	}

	TEST( FastHashMapTests, MoveAssignment_TransfersOwnership )
	{
		FastHashMap<std::string, int> map1;
		map1.insertOrAssign( "alpha", 1 );
		map1.insertOrAssign( "beta", 2 );

		FastHashMap<std::string, int> map2;
		map2.insertOrAssign( "old", 99 );

		map2 = std::move( map1 );

		EXPECT_EQ( map2.size(), 2 );
		EXPECT_EQ( *map2.find( "alpha" ), 1 );
		EXPECT_EQ( *map2.find( "beta" ), 2 );
		EXPECT_EQ( map2.find( "old" ), nullptr );

		// Verify source is moved-from (should be empty)
		EXPECT_EQ( map1.size(), 0 );
		EXPECT_TRUE( map1.isEmpty() );
	}
} // namespace nfx::containers::test
