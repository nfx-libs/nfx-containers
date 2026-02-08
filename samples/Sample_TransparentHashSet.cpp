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
 * @file Sample_TransparentHashSet.cpp
 * @brief Demonstrates TransparentHashSet usage with STL compatibility
 * @details This sample shows how to use TransparentHashSet as a drop-in std::unordered_set replacement
 *          with transparent lookup and heterogeneous key support
 */

#include <nfx/Containers.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers TransparentHashSet ===\n\n";

    //=====================================================================
    // 1. Basic construction and insertion
    //=====================================================================
    {
        std::cout << "1. Basic construction and insertion\n";
        std::cout << "------------------------------------\n";

        TransparentHashSet<std::string> fruits;
        fruits.insert( "apple" );
        fruits.insert( "banana" );
        fruits.insert( "cherry" );

        std::cout << "Size: " << fruits.size() << " items\n";
        std::cout << "Bucket count: " << fruits.bucket_count() << "\n";
        std::cout << "Load factor: " << fruits.load_factor() << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Duplicate insertion handling
    //=====================================================================
    {
        std::cout << "2. Duplicate insertion handling\n";
        std::cout << "--------------------------------\n";

        TransparentHashSet<std::string> set;
        auto [it1, inserted1] = set.insert( "apple" );
        auto [it2, inserted2] = set.insert( "apple" );

        std::cout << "First insert(\"apple\"): " << ( inserted1 ? "inserted" : "exists" ) << "\n";
        std::cout << "Second insert(\"apple\"): " << ( inserted2 ? "inserted" : "exists" ) << "\n";
        std::cout << "Size: " << set.size() << " (duplicates ignored)\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 3. Lookup operations (find, count, contains)
    //=====================================================================
    {
        std::cout << "3. Lookup operations (find, count, contains)\n";
        std::cout << "---------------------------------------------\n";

        TransparentHashSet<std::string> data;
        data.insert( "key1" );
        data.insert( "key2" );

        auto it = data.find( "key1" );
        if ( it != data.end() )
        {
            std::cout << "find(\"key1\"): " << *it << "\n";
        }

        std::cout << "count(\"key2\"): " << data.count( "key2" ) << "\n";
        std::cout << "contains(\"key1\"): " << ( data.contains( "key1" ) ? "true" : "false" ) << "\n";
        std::cout << "contains(\"missing\"): " << ( data.contains( "missing" ) ? "true" : "false" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 4. Heterogeneous lookup (zero-copy)
    //=====================================================================
    {
        std::cout << "4. Heterogeneous lookup (zero-copy)\n";
        std::cout << "------------------------------------\n";

        TransparentHashSet<std::string> cache;
        cache.insert( "data.json" );
        cache.insert( "config.xml" );

        // string_view lookup - no temporary string allocation!
        std::string_view sv = "data.json";
        if ( cache.contains( sv ) )
        {
            std::cout << "string_view lookup: found (zero allocations!)\n";
        }

        // const char* lookup - also zero-copy
        if ( cache.contains( "config.xml" ) )
        {
            std::cout << "const char* lookup: found (zero allocations!)\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Erase operations
    //=====================================================================
    {
        std::cout << "5. Erase operations\n";
        std::cout << "-------------------\n";

        TransparentHashSet<std::string> set;
        set.insert( "a" );
        set.insert( "b" );
        set.insert( "c" );

        std::cout << "Size before erase: " << set.size() << "\n";

        size_t erased = set.erase( "b" );
        std::cout << "erase(\"b\"): " << erased << " elements removed\n";
        std::cout << "Size after erase: " << set.size() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 6. Iteration
    //=====================================================================
    {
        std::cout << "6. Iteration\n";
        std::cout << "------------\n";

        TransparentHashSet<std::string> items = { "first", "second", "third" };

        std::cout << "Range-based for loop:\n";
        for ( const auto& item : items )
        {
            std::cout << "  " << item << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 7. Initializer list construction
    //=====================================================================
    {
        std::cout << "7. Initializer list construction\n";
        std::cout << "---------------------------------\n";

        TransparentHashSet<int> numbers = { 1, 2, 3, 4, 5, 5, 5 };

        std::cout << "Created from {1, 2, 3, 4, 5, 5, 5}\n";
        std::cout << "Size: " << numbers.size() << " (duplicates removed)\n";
        std::cout << "contains(3): " << ( numbers.contains( 3 ) ? "true" : "false" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 8. Capacity management
    //=====================================================================
    {
        std::cout << "8. Capacity management\n";
        std::cout << "----------------------\n";

        TransparentHashSet<int> set;

        std::cout << "Bucket count before reserve: " << set.bucket_count() << "\n";
        set.reserve( 100 );
        std::cout << "Bucket count after reserve(100): " << set.bucket_count() << "\n";
        std::cout << "Max load factor: " << set.max_load_factor() << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 9. Integer set
    //=====================================================================
    {
        std::cout << "9. Integer set\n";
        std::cout << "--------------\n";

        TransparentHashSet<int> ids;
        for ( int i = 1000; i <= 1010; ++i )
        {
            ids.insert( i );
        }

        std::cout << "Inserted IDs 1000-1010\n";
        std::cout << "Size: " << ids.size() << "\n";
        std::cout << "contains(1005): " << ( ids.contains( 1005 ) ? "true" : "false" ) << "\n";
        std::cout << "Bucket count: " << ids.bucket_count() << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 10. Equality comparison
    //=====================================================================
    {
        std::cout << "10. Equality comparison\n";
        std::cout << "-----------------------\n";

        TransparentHashSet<std::string> set1 = { "a", "b", "c" };
        TransparentHashSet<std::string> set2 = { "a", "b", "c" };
        TransparentHashSet<std::string> set3 = { "a", "b", "d" };

        std::cout << "set1 == set2: " << ( set1 == set2 ? "true" : "false" ) << "\n";
        std::cout << "set1 == set3: " << ( set1 == set3 ? "true" : "false" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 11. Custom hash seed
    //=====================================================================
    {
        std::cout << "11. Custom hash seed\n";
        std::cout << "--------------------\n";

        using CustomHasher = nfx::hashing::Hasher<uint32_t, 0xDEADBEEF>;
        TransparentHashSet<std::string, CustomHasher> secureSet;

        secureSet.insert( "token_1" );
        secureSet.insert( "token_2" );

        std::cout << "Created set with custom seed: 0xDEADBEEF\n";
        std::cout << "Use case: Domain separation for security contexts\n";
        std::cout << "Size: " << secureSet.size() << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 12. Complex key types (pairs)
    //=====================================================================
    {
        std::cout << "12. Complex key types (pairs)\n";
        std::cout << "------------------------------\n";

        TransparentHashSet<std::pair<int, int>> coords;
        coords.insert( { 0, 0 } );
        coords.insert( { 1, 0 } );
        coords.insert( { 0, 1 } );
        coords.insert( { 0, 0 } ); // Duplicate

        std::cout << "Inserted 4 coordinates (1 duplicate)\n";
        std::cout << "Size: " << coords.size() << " (unique coordinates)\n";
        std::cout << "contains({1,0}): " << ( coords.contains( { 1, 0 } ) ? "true" : "false" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 13. Move semantics and emplace
    //=====================================================================
    {
        std::cout << "13. Move semantics and emplace\n";
        std::cout << "-------------------------------\n";

        TransparentHashSet<std::string> set;

        std::string str = "long_string_value";
        std::cout << "Original string length: " << str.length() << "\n";

        set.insert( std::move( str ) );
        std::cout << "After move, original length: " << str.length() << " (moved-from)\n";
        std::cout << "Set size: " << set.size() << "\n";

        set.emplace( "another_string" );
        std::cout << "After emplace, set size: " << set.size() << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 14. STL algorithm compatibility
    //=====================================================================
    {
        std::cout << "14. STL algorithm compatibility\n";
        std::cout << "--------------------------------\n";

        TransparentHashSet<int> numbers = { 10, 20, 30, 40, 50 };

        size_t count = std::count_if( numbers.begin(), numbers.end(),
            []( int val ) { return val > 25; } );
        std::cout << "Elements > 25: " << count << "\n";

        auto it = std::find_if( numbers.begin(), numbers.end(),
            []( int val ) { return val == 30; } );
        if ( it != numbers.end() )
        {
            std::cout << "Found element with value 30: " << *it << "\n";
        }

        bool allPositive = std::all_of( numbers.begin(), numbers.end(),
            []( int val ) { return val > 0; } );
        std::cout << "All elements positive: " << ( allPositive ? "true" : "false" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 15. Clear operation
    //=====================================================================
    {
        std::cout << "15. Clear operation\n";
        std::cout << "-------------------\n";

        TransparentHashSet<std::string> set = { "a", "b", "c", "d", "e" };
        std::cout << "Size before clear: " << set.size() << "\n";
        std::cout << "empty(): " << ( set.empty() ? "true" : "false" ) << "\n";

        set.clear();
        std::cout << "Size after clear: " << set.size() << "\n";
        std::cout << "empty(): " << ( set.empty() ? "true" : "false" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 16. Large dataset
    //=====================================================================
    {
        std::cout << "16. Large dataset\n";
        std::cout << "-----------------\n";

        TransparentHashSet<int> large;

        for ( int i = 0; i < 10000; ++i )
        {
            large.insert( i );
        }

        std::cout << "Inserted: 10,000 elements\n";
        std::cout << "Size: " << large.size() << "\n";
        std::cout << "Bucket count: " << large.bucket_count() << "\n";
        std::cout << "Load factor: " << large.load_factor() << "\n";
        std::cout << "contains(5000): " << ( large.contains( 5000 ) ? "true" : "false" ) << "\n";
        std::cout << "contains(99999): " << ( large.contains( 99999 ) ? "true" : "false" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 11. Extract operation (node handle)
    //=====================================================================
    {
        std::cout << "11. Extract operation (node handle)\n";
        std::cout << "------------------------------------------\n";

        TransparentHashSet<std::string> set = { "apple", "banana", "cherry", "date" };

        std::cout << "Original set size: " << set.size() << "\n";
        std::cout << "Contains 'banana': " << ( set.contains( "banana" ) ? "true" : "false" ) << "\n";

        // Extract an element (returns node_type handle)
        // Note: extract() only supports exact key type, not heterogeneous lookup
        // (this is a limitation of std::unordered_set::extract in C++17)
        auto node = set.extract( "banana" );

        if ( !node.empty() )
        {
            std::cout << "Extracted value: " << node.value() << "\n";
        }

        std::cout << "Set size after extract: " << set.size() << "\n";
        std::cout << "Contains 'banana': " << ( set.contains( "banana" ) ? "true" : "false" ) << "\n";

        // Re-insert the extracted node
        if ( !node.empty() )
        {
            set.insert( std::move( node ) );
            std::cout << "Re-inserted extracted value\n";
            std::cout << "Size after re-insert: " << set.size() << "\n";
        }

        // Extract non-existent element
        auto empty_node = set.extract( "grape" );
        std::cout << "Extract 'grape' (non-existent): " << ( empty_node.empty() ? "empty" : "found" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 12. Merge operation
    //=====================================================================
    {
        std::cout << "12. Merge operation\n";
        std::cout << "---------------------------\n";

        TransparentHashSet<std::string> set1 = { "apple", "banana", "cherry" };
        TransparentHashSet<std::string> set2 = { "date", "elderberry", "banana" };

        std::cout << "Set1 size: " << set1.size() << "\n";
        std::cout << "Set2 size: " << set2.size() << "\n";

        // Merge set2 into set1 (duplicates remain in set2)
        set1.merge( set2 );

        std::cout << "\nAfter merge:\n";
        std::cout << "Set1 size: " << set1.size() << "\n";
        std::cout << "Set2 size: " << set2.size() << "\n";

        std::cout << "\nSet1 elements: ";
        for ( const auto& v : set1 )
        {
            std::cout << v << " ";
        }
        std::cout << "\n";

        std::cout << "Set2 elements (duplicates): ";
        for ( const auto& v : set2 )
        {
            std::cout << v << " ";
        }
        std::cout << "\n";

        std::cout << "\n";
    }

    return 0;
}
