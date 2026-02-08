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
 * @file Sample_FastHashMap.cpp
 * @brief Demonstrates FastHashMap usage with Robin Hood hashing
 * @details This sample shows how to use FastHashMap for high-performance key-value storage
 *          with bounded probe distances and heterogeneous lookup capabilities
 */

#include <nfx/Containers.h>

#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers FastHashMap ===\n\n";

    //=====================================================================
    // 1. Basic construction and insertion
    //=====================================================================
    {
        std::cout << "1. Basic construction and insertion\n";
        std::cout << "------------------------------------\n";

        FastHashMap<std::string, int> inventory;
        inventory.insertOrAssign( "apples", 50 );
        inventory.insertOrAssign( "bananas", 30 );
        inventory.insertOrAssign( "cherries", 75 );

        std::cout << "Size: " << inventory.size() << " items\n";
        std::cout << "Capacity: " << inventory.capacity() << " slots\n";
        std::cout << "Load factor: " << ( 100.0 * inventory.size() / inventory.capacity() ) << "%\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Initializer list construction
    //=====================================================================
    {
        std::cout << "2. Initializer list construction\n";
        std::cout << "---------------------------------\n";

        FastHashMap<std::string, int> prices = {
            { "apple", 150 },
            { "banana", 80 },
            { "cherry", 200 } };

        std::cout << "Created map with " << prices.size() << " items\n";
        std::cout << "Syntax: FastHashMap<K,V> map = {{k1,v1}, {k2,v2}, ...};\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 3. Lookup operations (pointer-based find)
    //=====================================================================
    {
        std::cout << "3. Lookup operations (pointer-based find)\n";
        std::cout << "------------------------------------------\n";

        FastHashMap<std::string, int> data;
        data.insertOrAssign( "found", 42 );

        if ( int* value = data.find( "found" ) )
        {
            std::cout << "find(\"found\"): " << *value << "\n";
        }

        if ( int* value = data.find( "missing" ) )
        {
            std::cout << "find(\"missing\"): " << *value << "\n";
        }
        else
        {
            std::cout << "find(\"missing\"): nullptr (not found)\n";
        }

        std::cout << "Note: Returns pointer for null-safe access without exceptions\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 4. Heterogeneous lookup (zero-copy)
    //=====================================================================
    {
        std::cout << "4. Heterogeneous lookup (zero-copy)\n";
        std::cout << "------------------------------------\n";

        FastHashMap<std::string, int> cache;
        cache.insertOrAssign( "key1", 100 );

        // string_view lookup - no temporary string allocation!
        std::string_view sv = "key1";
        if ( int* value = cache.find( sv ) )
        {
            std::cout << "string_view lookup: " << *value << " (zero allocations!)\n";
        }

        // const char* lookup - also zero-copy
        if ( int* value = cache.find( "key1" ) )
        {
            std::cout << "const char* lookup: " << *value << " (zero allocations!)\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Update and erase operations
    //=====================================================================
    {
        std::cout << "5. Update and erase operations\n";
        std::cout << "-------------------------------\n";

        FastHashMap<std::string, int> map;
        map.insertOrAssign( "counter", 10 );

        std::cout << "Initial: counter = " << *map.find( "counter" ) << "\n";

        map.insertOrAssign( "counter", 20 );
        std::cout << "After update: counter = " << *map.find( "counter" ) << "\n";

        bool erased = map.erase( "counter" );
        std::cout << "erase(\"counter\"): " << ( erased ? "success" : "not found" ) << "\n";
        std::cout << "Size after erase: " << map.size() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 6. Iteration
    //=====================================================================
    {
        std::cout << "6. Iteration\n";
        std::cout << "------------\n";

        FastHashMap<std::string, int> items = {
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
    // 7. Integer keys with 64-bit hashing
    //=====================================================================
    {
        std::cout << "7. Integer keys with 64-bit hashing\n";
        std::cout << "------------------------------------\n";

        FastHashMap<int, std::string, uint64_t> employees;
        employees.insertOrAssign( 1001, "Alice" );
        employees.insertOrAssign( 1002, "Bob" );
        employees.insertOrAssign( 2001, "Charlie" );

        std::cout << "Stored " << employees.size() << " employee records\n";
        if ( std::string* name = employees.find( 1002 ) )
        {
            std::cout << "Employee 1002: " << *name << "\n";
        }

        std::cout << "Note: uint64_t hash provides better distribution for integer keys\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 8. Custom capacity and reserve
    //=====================================================================
    {
        std::cout << "8. Custom capacity and reserve\n";
        std::cout << "-------------------------------\n";

        FastHashMap<int, int> preallocated( 128 );
        std::cout << "Initial capacity: " << preallocated.capacity() << " (user-specified)\n";

        FastHashMap<int, int> resized;
        std::cout << "Default capacity: " << resized.capacity() << "\n";
        resized.reserve( 256 );
        std::cout << "After reserve(256): " << resized.capacity() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 9. Robin Hood hashing efficiency
    //=====================================================================
    {
        std::cout << "9. Robin Hood hashing efficiency\n";
        std::cout << "---------------------------------\n";

        FastHashMap<int, int> robinHood;

        // Insert elements that may collide
        for ( int i = 0; i < 100; ++i )
        {
            robinHood.insertOrAssign( i * 13, i );
        }

        std::cout << "Inserted 100 elements with potential collisions\n";
        std::cout << "Size: " << robinHood.size() << "\n";
        std::cout << "Capacity: " << robinHood.capacity() << "\n";
        std::cout << "Load factor: " << ( 100.0 * robinHood.size() / robinHood.capacity() ) << "%\n";
        std::cout << "Note: Robin Hood ensures O(log n) worst-case probe distance\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 10. Equality comparison
    //=====================================================================
    {
        std::cout << "10. Equality comparison\n";
        std::cout << "-----------------------\n";

        FastHashMap<std::string, int> map1 = { { "a", 1 }, { "b", 2 } };
        FastHashMap<std::string, int> map2 = { { "a", 1 }, { "b", 2 } };
        FastHashMap<std::string, int> map3 = { { "a", 1 }, { "b", 3 } };

        std::cout << "map1 == map2: " << ( map1 == map2 ? "true" : "false" ) << "\n";
        std::cout << "map1 == map3: " << ( map1 == map3 ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 11. Custom hash seed
    //=====================================================================
    {
        std::cout << "11. Custom hash seed\n";
        std::cout << "--------------------\n";

        using CustomHasher = nfx::hashing::Hasher<uint32_t, 0xDEADBEEF>;
        FastHashMap<std::string, int, uint32_t, 0xDEADBEEF, CustomHasher> secureMap;

        secureMap.insertOrAssign( "token", 12345 );

        std::cout << "Created map with custom seed: 0xDEADBEEF\n";
        std::cout << "Use case: Domain separation for security contexts\n";
        std::cout << "Size: " << secureMap.size() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 12. Complex key types (pairs)
    //=====================================================================
    {
        std::cout << "12. Complex key types (pairs)\n";
        std::cout << "------------------------------\n";

        FastHashMap<std::pair<int, int>, std::string> grid;
        grid.insertOrAssign( { 0, 0 }, "origin" );
        grid.insertOrAssign( { 1, 0 }, "right" );
        grid.insertOrAssign( { 0, 1 }, "up" );

        auto coord = std::make_pair( 1, 0 );
        if ( std::string* direction = grid.find( coord ) )
        {
            std::cout << "grid[{1,0}] = " << *direction << "\n";
        }

        std::cout << "Grid cells: " << grid.size() << "\n";
        std::cout << "Note: Pairs, tuples, and custom types supported\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 13. Move semantics
    //=====================================================================
    {
        std::cout << "13. Move semantics\n";
        std::cout << "------------------\n";

        FastHashMap<std::string, std::vector<int>> dataMap;

        std::vector<int> largeVec = { 1, 2, 3, 4, 5 };
        std::cout << "Original vector size: " << largeVec.size() << "\n";

        dataMap.insertOrAssign( "data", std::move( largeVec ) );
        std::cout << "After move, original size: " << largeVec.size() << " (moved-from)\n";

        if ( std::vector<int>* stored = dataMap.find( "data" ) )
        {
            std::cout << "Stored vector size: " << stored->size() << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 14. Clear operation
    //=====================================================================
    {
        std::cout << "14. Clear operation\n";
        std::cout << "-------------------\n";

        FastHashMap<std::string, int> temp = { { "a", 1 }, { "b", 2 }, { "c", 3 } };
        std::cout << "Size before clear: " << temp.size() << "\n";

        temp.clear();
        std::cout << "Size after clear: " << temp.size() << "\n";
        std::cout << "isEmpty(): " << ( temp.isEmpty() ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 15. Large dataset performance
    //=====================================================================
    {
        std::cout << "15. Large dataset performance\n";
        std::cout << "------------------------------\n";

        FastHashMap<int, int, uint64_t> large;

        // Insert 10,000 elements
        for ( int i = 0; i < 10000; ++i )
        {
            large.insertOrAssign( i, i * i );
        }

        std::cout << "Inserted: 10,000 elements\n";
        std::cout << "Size: " << large.size() << "\n";
        std::cout << "Capacity: " << large.capacity() << "\n";
        std::cout << "Load factor: " << ( 100.0 * large.size() / large.capacity() ) << "%\n";

        // Verify lookup
        if ( int* result = large.find( 5000 ) )
        {
            std::cout << "Lookup 5000: " << *result << " (expected: 25000000)\n";
        }

        std::cout << "\n";
    }

    return 0;
}
