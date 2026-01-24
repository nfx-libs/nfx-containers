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
 * @file Sample_OrderedHashMap.cpp
 * @brief Demonstrates OrderedHashMap usage with insertion-order preservation
 * @details This sample shows how to use OrderedHashMap for high-performance key-value storage
 *          with guaranteed insertion-order iteration and heterogeneous lookup capabilities
 */

#include <nfx/Containers.h>

#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers OrderedHashMap ===\n\n";

    //=====================================================================
    // 1. Basic construction and insertion-order preservation
    //=====================================================================
    {
        std::cout << "1. Basic construction and insertion-order preservation\n";
        std::cout << "-------------------------------------------------------\n";

        OrderedHashMap<std::string, int> inventory;
        inventory.insertOrAssign( "cherries", 75 );
        inventory.insertOrAssign( "apples", 50 );
        inventory.insertOrAssign( "bananas", 30 );

        std::cout << "Insertion order: cherries, apples, bananas\n";
        std::cout << "Iteration order:\n";
        for ( const auto& [key, value] : inventory )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        std::cout << "Size: " << inventory.size() << " items\n";
        std::cout << "Capacity: " << inventory.capacity() << " slots\n";
        std::cout << "Note: Iteration follows insertion order!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Initializer list construction (preserves list order)
    //=====================================================================
    {
        std::cout << "2. Initializer list construction (preserves list order)\n";
        std::cout << "--------------------------------------------------------\n";

        OrderedHashMap<std::string, int> prices = {
            { "apple", 150 },
            { "banana", 80 },
            { "cherry", 200 } };

        std::cout << "Created map with " << prices.size() << " items\n";
        std::cout << "Iteration order matches initializer list:\n";
        for ( const auto& [key, value] : prices )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 3. Lookup operations (pointer-based find, O(1) performance)
    //=====================================================================
    {
        std::cout << "3. Lookup operations (pointer-based find, O(1) performance)\n";
        std::cout << "------------------------------------------------------------\n";

        OrderedHashMap<std::string, int> data;
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

        std::cout << "Note: O(1) hash table lookup, no iteration needed!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 4. Heterogeneous lookup (zero-copy)
    //=====================================================================
    {
        std::cout << "4. Heterogeneous lookup (zero-copy)\n";
        std::cout << "------------------------------------\n";

        OrderedHashMap<std::string, int> cache;
        cache.insertOrAssign( "key1", 100 );
        cache.insertOrAssign( "key2", 200 );

        // string_view lookup - no temporary string allocation!
        std::string_view sv = "key1";
        if ( int* value = cache.find( sv ) )
        {
            std::cout << "string_view lookup: " << *value << " (zero allocations!)\n";
        }

        // const char* lookup - also zero-copy
        if ( int* value = cache.find( "key2" ) )
        {
            std::cout << "const char* lookup: " << *value << " (zero allocations!)\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Update preserves insertion order
    //=====================================================================
    {
        std::cout << "5. Update preserves insertion order\n";
        std::cout << "------------------------------------\n";

        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "first", 1 );
        map.insertOrAssign( "second", 2 );
        map.insertOrAssign( "third", 3 );

        std::cout << "Initial state:\n";
        for ( const auto& [key, value] : map )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        map.insertOrAssign( "second", 999 ); // Update existing key

        std::cout << "\nAfter updating 'second' to 999:\n";
        for ( const auto& [key, value] : map )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        std::cout << "Note: 'second' stays in its original position!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 6. Erase operations maintain order
    //=====================================================================
    {
        std::cout << "6. Erase operations maintain order\n";
        std::cout << "-----------------------------------\n";

        OrderedHashMap<std::string, int> map;
        map.insertOrAssign( "one", 1 );
        map.insertOrAssign( "two", 2 );
        map.insertOrAssign( "three", 3 );
        map.insertOrAssign( "four", 4 );

        std::cout << "Before erase:\n";
        for ( const auto& [key, value] : map )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        bool erased = map.erase( "two" );
        std::cout << "\nerase(\"two\"): " << ( erased ? "success" : "not found" ) << "\n";

        std::cout << "After erase (order preserved):\n";
        for ( const auto& [key, value] : map )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 7. Bidirectional iteration
    //=====================================================================
    {
        std::cout << "7. Bidirectional iteration\n";
        std::cout << "--------------------------\n";

        OrderedHashMap<std::string, int> items = {
            { "first", 1 },
            { "second", 2 },
            { "third", 3 } };

        std::cout << "Forward iteration:\n";
        for ( auto it = items.begin(); it != items.end(); ++it )
        {
            std::cout << "  " << it->first << " -> " << it->second << "\n";
        }

        std::cout << "\nBackward iteration:\n";
        for ( auto it = --items.end(); it != items.begin(); --it )
        {
            std::cout << "  " << it->first << " -> " << it->second << "\n";
        }
        // Print the first element
        std::cout << "  " << items.begin()->first << " -> " << items.begin()->second << "\n";

        std::cout << "Note: Bidirectional iterators support ++, --, and range-based loops!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 8. Integer keys with 64-bit hashing
    //=====================================================================
    {
        std::cout << "8. Integer keys with 64-bit hashing\n";
        std::cout << "------------------------------------\n";

        OrderedHashMap<int, std::string, uint64_t> employees;
        employees.insertOrAssign( 2001, "Charlie" );
        employees.insertOrAssign( 1001, "Alice" );
        employees.insertOrAssign( 1002, "Bob" );

        std::cout << "Insertion order: 2001, 1001, 1002\n";
        std::cout << "Iteration order:\n";
        for ( const auto& [id, name] : employees )
        {
            std::cout << "  " << id << " -> " << name << "\n";
        }

        std::cout << "Note: Order is by insertion, not by key value!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 9. Custom capacity and reserve
    //=====================================================================
    {
        std::cout << "9. Custom capacity and reserve\n";
        std::cout << "-------------------------------\n";

        OrderedHashMap<int, int> preallocated( 128 );
        std::cout << "Initial capacity: " << preallocated.capacity() << " (user-specified)\n";

        OrderedHashMap<int, int> resized;
        std::cout << "Default capacity: " << resized.capacity() << "\n";
        resized.reserve( 256 );
        std::cout << "After reserve(256): " << resized.capacity() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 10. Equality comparison (order-independent)
    //=====================================================================
    {
        std::cout << "10. Equality comparison (order-independent)\n";
        std::cout << "--------------------------------------------\n";

        OrderedHashMap<std::string, int> map1 = { { "a", 1 }, { "b", 2 } };
        OrderedHashMap<std::string, int> map2 = { { "b", 2 }, { "a", 1 } };
        OrderedHashMap<std::string, int> map3 = { { "a", 1 }, { "b", 3 } };

        std::cout << "map1 insertion order: a, b\n";
        std::cout << "map2 insertion order: b, a\n";
        std::cout << "map1 == map2: " << ( map1 == map2 ? "true" : "false" ) << " (content matches)\n";
        std::cout << "map1 == map3: " << ( map1 == map3 ? "true" : "false" ) << " (different values)\n";

        std::cout << "Note: Equality is based on content, not insertion order!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 11. Custom hash seed
    //=====================================================================
    {
        std::cout << "11. Custom hash seed\n";
        std::cout << "--------------------\n";

        using CustomHasher = nfx::hashing::Hasher<uint32_t, 0xDEADBEEF>;
        OrderedHashMap<std::string, int, uint32_t, 0xDEADBEEF, CustomHasher> secureMap;

        secureMap.insertOrAssign( "token", 12345 );
        secureMap.insertOrAssign( "session", 67890 );

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

        OrderedHashMap<std::pair<int, int>, std::string> grid;
        grid.insertOrAssign( { 0, 0 }, "origin" );
        grid.insertOrAssign( { 1, 0 }, "right" );
        grid.insertOrAssign( { 0, 1 }, "up" );

        std::cout << "Insertion order: {0,0}, {1,0}, {0,1}\n";
        std::cout << "Iteration order:\n";
        for ( const auto& [coord, name] : grid )
        {
            std::cout << "  {" << coord.first << "," << coord.second << "} -> " << name << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 13. Move semantics
    //=====================================================================
    {
        std::cout << "13. Move semantics\n";
        std::cout << "------------------\n";

        OrderedHashMap<std::string, std::vector<int>> dataMap;

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

        OrderedHashMap<std::string, int> temp = { { "a", 1 }, { "b", 2 }, { "c", 3 } };
        std::cout << "Size before clear: " << temp.size() << "\n";

        temp.clear();
        std::cout << "Size after clear: " << temp.size() << "\n";
        std::cout << "isEmpty(): " << ( temp.isEmpty() ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 15. Insertion order with mixed operations
    //=====================================================================
    {
        std::cout << "15. Insertion order with mixed operations\n";
        std::cout << "------------------------------------------\n";

        OrderedHashMap<std::string, int> log;

        log.insertOrAssign( "event1", 100 );
        log.insertOrAssign( "event2", 200 );
        log.insertOrAssign( "event3", 300 );
        log.insertOrAssign( "event2", 999 ); // Update
        log.insertOrAssign( "event4", 400 );

        std::cout << "Operations: insert(event1), insert(event2), insert(event3),\n";
        std::cout << "            update(event2), insert(event4)\n";
        std::cout << "Iteration order (updates don't change position):\n";
        for ( const auto& [key, value] : log )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        log.erase( "event2" );
        std::cout << "\nAfter erase(event2):\n";
        for ( const auto& [key, value] : log )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 16. Large dataset with preserved order
    //=====================================================================
    {
        std::cout << "16. Large dataset with preserved order\n";
        std::cout << "---------------------------------------\n";

        OrderedHashMap<int, int, uint64_t> large;

        // Insert 1,000 elements
        for ( int i = 0; i < 1000; ++i )
        {
            large.insertOrAssign( i, i * i );
        }

        std::cout << "Inserted: 1,000 elements\n";
        std::cout << "Size: " << large.size() << "\n";
        std::cout << "Capacity: " << large.capacity() << "\n";

        // Verify order (first 5 and last 5)
        std::cout << "First 5 elements:\n";
        int count = 0;
        for ( const auto& [key, value] : large )
        {
            if ( count++ < 5 )
            {
                std::cout << "  " << key << " -> " << value << "\n";
            }
            else
            {
                break;
            }
        }

        std::cout << "Note: Insertion order is preserved even with 1,000+ elements!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 17. Use case: LRU Cache simulation (ordered map foundation)
    //=====================================================================
    {
        std::cout << "17. Use case: LRU Cache simulation (ordered map foundation)\n";
        std::cout << "------------------------------------------------------------\n";

        OrderedHashMap<std::string, int> cache;

        // Simulate cache accesses (insert in access order)
        cache.insertOrAssign( "page1", 1 );
        cache.insertOrAssign( "page2", 2 );
        cache.insertOrAssign( "page3", 3 );

        std::cout << "Cache state (access order):\n";
        for ( const auto& [key, value] : cache )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        std::cout << "\nNote: Perfect for LRU/MRU cache implementations!\n";
        std::cout << "      Combined with manual re-insertion on access,\n";
        std::cout << "      you can maintain access-order semantics.\n";
        std::cout << "\n";
    }

    return 0;
}
