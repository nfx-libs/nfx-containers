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
 * @file Sample_TransparentHashMap.cpp
 * @brief Demonstrates TransparentHashMap usage with STL compatibility
 * @details This sample shows how to use TransparentHashMap as a drop-in std::unordered_map replacement
 *          with transparent lookup and heterogeneous key support
 */

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include <nfx/Containers.h>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers TransparentHashMap ===\n\n";

    //=====================================================================
    // 1. Basic construction and insertion
    //=====================================================================
    {
        std::cout << "1. Basic construction and insertion\n";
        std::cout << "------------------------------------\n";

        TransparentHashMap<std::string, int> inventory;
        inventory["apples"] = 50;
        inventory["bananas"] = 30;
        inventory["cherries"] = 75;

        std::cout << "Size: " << inventory.size() << " items\n";
        std::cout << "Bucket count: " << inventory.bucket_count() << "\n";
        std::cout << "Load factor: " << inventory.load_factor() << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Lookup operations (find, at, operator[])
    //=====================================================================
    {
        std::cout << "2. Lookup operations (find, at, operator[])\n";
        std::cout << "--------------------------------------------\n";

        TransparentHashMap<std::string, int> data;
        data["key1"] = 42;
        data["key2"] = 100;

        auto it = data.find( "key1" );
        if ( it != data.end() )
        {
            std::cout << "find(\"key1\"): " << it->second << "\n";
        }

        std::cout << "at(\"key2\"): " << data.at( "key2" ) << "\n";
        std::cout << "operator[\"key1\"]: " << data["key1"] << "\n";
        std::cout << "count(\"missing\"): " << data.count( "missing" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 3. Heterogeneous lookup (zero-copy)
    //=====================================================================
    {
        std::cout << "3. Heterogeneous lookup (zero-copy)\n";
        std::cout << "------------------------------------\n";

        TransparentHashMap<std::string, int> cache;
        cache["data.json"] = 100;
        cache["config.xml"] = 200;

        // string_view lookup - no temporary string allocation!
        std::string_view sv = "data.json";
        auto it = cache.find( sv );
        if ( it != cache.end() )
        {
            std::cout << "string_view lookup: " << it->second << " (zero allocations!)\n";
        }

        // const char* lookup - also zero-copy
        auto it2 = cache.find( "config.xml" );
        if ( it2 != cache.end() )
        {
            std::cout << "const char* lookup: " << it2->second << " (zero allocations!)\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 4. Insert and assignment operations
    //=====================================================================
    {
        std::cout << "4. Insert and assignment operations\n";
        std::cout << "------------------------------------\n";

        TransparentHashMap<std::string, int> map;

        auto [it1, inserted1] = map.insert( { "key1", 10 } );
        std::cout << "insert(\"key1\", 10): " << ( inserted1 ? "inserted" : "exists" ) << "\n";

        auto [it2, inserted2] = map.insert( { "key1", 999 } );
        std::cout << "insert(\"key1\", 999): " << ( inserted2 ? "inserted" : "exists" ) << "\n";
        std::cout << "Value unchanged: " << map["key1"] << "\n";

        auto [it3, inserted3] = map.insert_or_assign( "key1", 100 );
        std::cout << "insert_or_assign(\"key1\", 100): " << ( inserted3 ? "inserted" : "assigned" ) << "\n";
        std::cout << "Value updated: " << map["key1"] << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Erase operations
    //=====================================================================
    {
        std::cout << "5. Erase operations\n";
        std::cout << "-------------------\n";

        TransparentHashMap<std::string, int> map;
        map["a"] = 1;
        map["b"] = 2;
        map["c"] = 3;

        std::cout << "Size before erase: " << map.size() << "\n";

        size_t erased = map.erase( "b" );
        std::cout << "erase(\"b\"): " << erased << " elements removed\n";
        std::cout << "Size after erase: " << map.size() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 6. Iteration
    //=====================================================================
    {
        std::cout << "6. Iteration\n";
        std::cout << "------------\n";

        TransparentHashMap<std::string, int> items = {
            { "first", 1 },
            { "second", 2 },
            { "third", 3 } };

        std::cout << "Range-based for loop:\n";
        for ( const auto& [key, value] : items )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 7. Capacity management
    //=====================================================================
    {
        std::cout << "7. Capacity management\n";
        std::cout << "----------------------\n";

        TransparentHashMap<int, int> map;

        std::cout << "Bucket count before reserve: " << map.bucket_count() << "\n";
        map.reserve( 100 );
        std::cout << "Bucket count after reserve(100): " << map.bucket_count() << "\n";
        std::cout << "Max load factor: " << map.max_load_factor() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 8. Integer keys
    //=====================================================================
    {
        std::cout << "8. Integer keys\n";
        std::cout << "---------------\n";

        TransparentHashMap<int, std::string> employees;
        employees[1001] = "Alice";
        employees[1002] = "Bob";
        employees[2001] = "Charlie";

        std::cout << "Stored " << employees.size() << " employee records\n";
        std::cout << "Employee 1002: " << employees[1002] << "\n";
        std::cout << "Bucket count: " << employees.bucket_count() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 9. Equality comparison
    //=====================================================================
    {
        std::cout << "9. Equality comparison\n";
        std::cout << "----------------------\n";

        TransparentHashMap<std::string, int> map1 = { { "a", 1 }, { "b", 2 } };
        TransparentHashMap<std::string, int> map2 = { { "a", 1 }, { "b", 2 } };
        TransparentHashMap<std::string, int> map3 = { { "a", 1 }, { "b", 3 } };

        std::cout << "map1 == map2: " << ( map1 == map2 ? "true" : "false" ) << "\n";
        std::cout << "map1 == map3: " << ( map1 == map3 ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 10. Custom hash seed
    //=====================================================================
    {
        std::cout << "10. Custom hash seed\n";
        std::cout << "--------------------\n";

        using CustomHasher = nfx::hashing::Hasher<uint32_t, 0xCAFEBABE>;
        TransparentHashMap<std::string, int, CustomHasher> secureMap;

        secureMap["password_hash"] = 12345;
        secureMap["api_key_hash"] = 67890;

        std::cout << "Created map with custom seed: 0xCAFEBABE\n";
        std::cout << "Use case: Domain separation for security contexts\n";
        std::cout << "password_hash: " << secureMap["password_hash"] << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 11. Complex key types (pairs)
    //=====================================================================
    {
        std::cout << "11. Complex key types (pairs)\n";
        std::cout << "------------------------------\n";

        TransparentHashMap<std::pair<int, int>, std::string> grid;
        grid[{ 0, 0 }] = "origin";
        grid[{ 1, 0 }] = "east";
        grid[{ 0, 1 }] = "north";

        std::cout << "grid[{1,0}]: " << grid[{ 1, 0 }] << "\n";
        std::cout << "Grid cells: " << grid.size() << "\n";
        std::cout << "Bucket count: " << grid.bucket_count() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 12. Move semantics and emplace
    //=====================================================================
    {
        std::cout << "12. Move semantics and emplace\n";
        std::cout << "-------------------------------\n";

        TransparentHashMap<std::string, std::vector<int>> dataMap;

        std::vector<int> largeVec = { 1, 2, 3, 4, 5 };
        std::cout << "Original vector size: " << largeVec.size() << "\n";

        dataMap["data"] = std::move( largeVec );
        std::cout << "After move, original size: " << largeVec.size() << " (moved-from)\n";
        std::cout << "Stored vector size: " << dataMap["data"].size() << "\n";

        dataMap.emplace( "data2", std::vector<int>{ 6, 7, 8 } );
        std::cout << "Emplaced data2 size: " << dataMap["data2"].size() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 13. STL algorithm compatibility
    //=====================================================================
    {
        std::cout << "13. STL algorithm compatibility\n";
        std::cout << "--------------------------------\n";

        TransparentHashMap<std::string, int> numbers = {
            { "one", 1 },
            { "two", 2 },
            { "three", 3 },
            { "four", 4 },
            { "five", 5 } };

        size_t count = std::count_if( numbers.begin(), numbers.end(),
            []( const auto& pair ) { return pair.second > 2; } );
        std::cout << "Elements with value > 2: " << count << "\n";

        auto it = std::find_if( numbers.begin(), numbers.end(),
            []( const auto& pair ) { return pair.second == 3; } );
        if ( it != numbers.end() )
        {
            std::cout << "Found element with value 3: " << it->first << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 14. Large dataset
    //=====================================================================
    {
        std::cout << "14. Large dataset\n";
        std::cout << "-----------------\n";

        TransparentHashMap<int, int> large;

        for ( int i = 0; i < 10000; ++i )
        {
            large[i] = i * i;
        }

        std::cout << "Inserted: 10,000 elements\n";
        std::cout << "Size: " << large.size() << "\n";
        std::cout << "Bucket count: " << large.bucket_count() << "\n";
        std::cout << "Load factor: " << large.load_factor() << "\n";
        std::cout << "Lookup 5000: " << large[5000] << " (expected: 25000000)\n";

        std::cout << "\n";
    }

    return 0;
}
