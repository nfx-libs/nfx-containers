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
 * @file Sample_OrderedHashSet.cpp
 * @brief Demonstrates OrderedHashSet usage with insertion-order preservation
 * @details This sample shows how to use OrderedHashSet for high-performance key storage
 *          with guaranteed insertion-order iteration and heterogeneous lookup capabilities
 */

#include <nfx/Containers.h>

#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers OrderedHashSet ===\n\n";

    //=====================================================================
    // 1. Basic construction and insertion-order preservation
    //=====================================================================
    {
        std::cout << "1. Basic construction and insertion-order preservation\n";
        std::cout << "-------------------------------------------------------\n";

        OrderedHashSet<std::string> fruits;
        fruits.insert( "cherries" );
        fruits.insert( "apples" );
        fruits.insert( "bananas" );

        std::cout << "Insertion order: cherries, apples, bananas\n";
        std::cout << "Iteration order:\n";
        for ( const auto& fruit : fruits )
        {
            std::cout << "  " << fruit << "\n";
        }

        std::cout << "Size: " << fruits.size() << " items\n";
        std::cout << "Capacity: " << fruits.capacity() << " slots\n";
        std::cout << "Note: Iteration follows insertion order!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Initializer list construction (preserves list order)
    //=====================================================================
    {
        std::cout << "2. Initializer list construction (preserves list order)\n";
        std::cout << "--------------------------------------------------------\n";

        OrderedHashSet<std::string> colors = { "red", "green", "blue" };

        std::cout << "Created set with " << colors.size() << " items\n";
        std::cout << "Iteration order matches initializer list:\n";
        for ( const auto& color : colors )
        {
            std::cout << "  " << color << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 3. Lookup operations (pointer-based find, O(1) performance)
    //=====================================================================
    {
        std::cout << "3. Lookup operations (pointer-based find, O(1) performance)\n";
        std::cout << "------------------------------------------------------------\n";

        OrderedHashSet<std::string> data;
        data.insert( "found" );

        if ( const std::string* key = data.find( "found" ) )
        {
            std::cout << "find(\"found\"): " << *key << "\n";
        }

        if ( const std::string* key = data.find( "missing" ) )
        {
            std::cout << "find(\"missing\"): " << *key << "\n";
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

        OrderedHashSet<std::string> cache;
        cache.insert( "key1" );
        cache.insert( "key2" );

        // string_view lookup - no temporary string allocation!
        std::string_view sv = "key1";
        if ( cache.contains( sv ) )
        {
            std::cout << "string_view lookup: found (zero allocations!)\n";
        }

        // const char* lookup - also zero-copy
        if ( cache.contains( "key2" ) )
        {
            std::cout << "const char* lookup: found (zero allocations!)\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Duplicate insertion preserves original order
    //=====================================================================
    {
        std::cout << "5. Duplicate insertion preserves original order\n";
        std::cout << "------------------------------------------------\n";

        OrderedHashSet<std::string> set;
        set.insert( "first" );
        set.insert( "second" );
        set.insert( "third" );

        std::cout << "Initial state:\n";
        for ( const auto& key : set )
        {
            std::cout << "  " << key << "\n";
        }

        bool inserted = set.insert( "second" ); // Try to insert duplicate

        std::cout << "\nAfter insert(\"second\"): " << ( inserted ? "inserted" : "already exists" ) << "\n";
        std::cout << "Iteration order:\n";
        for ( const auto& key : set )
        {
            std::cout << "  " << key << "\n";
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

        OrderedHashSet<std::string> set;
        set.insert( "one" );
        set.insert( "two" );
        set.insert( "three" );
        set.insert( "four" );

        std::cout << "Before erase:\n";
        for ( const auto& key : set )
        {
            std::cout << "  " << key << "\n";
        }

        bool erased = set.erase( "two" );
        std::cout << "\nerase(\"two\"): " << ( erased ? "success" : "not found" ) << "\n";

        std::cout << "After erase (order preserved):\n";
        for ( const auto& key : set )
        {
            std::cout << "  " << key << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 7. Bidirectional iteration
    //=====================================================================
    {
        std::cout << "7. Bidirectional iteration\n";
        std::cout << "--------------------------\n";

        OrderedHashSet<std::string> items = { "first", "second", "third" };

        std::cout << "Forward iteration:\n";
        for ( auto it = items.begin(); it != items.end(); ++it )
        {
            std::cout << "  " << *it << "\n";
        }

        std::cout << "\nBackward iteration:\n";
        for ( auto it = --items.end(); it != items.begin(); --it )
        {
            std::cout << "  " << *it << "\n";
        }
        // Print the first element
        std::cout << "  " << *items.begin() << "\n";

        std::cout << "Note: Bidirectional iterators support ++, --, and range-based loops!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 8. Integer keys with 64-bit hashing
    //=====================================================================
    {
        std::cout << "8. Integer keys with 64-bit hashing\n";
        std::cout << "------------------------------------\n";

        OrderedHashSet<int, uint64_t> ids;
        ids.insert( 2001 );
        ids.insert( 1001 );
        ids.insert( 1002 );

        std::cout << "Insertion order: 2001, 1001, 1002\n";
        std::cout << "Iteration order:\n";
        for ( const auto& id : ids )
        {
            std::cout << "  " << id << "\n";
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

        OrderedHashSet<int> preallocated( 128 );
        std::cout << "Initial capacity: " << preallocated.capacity() << " (user-specified)\n";

        OrderedHashSet<int> resized;
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

        OrderedHashSet<std::string> set1 = { "a", "b" };
        OrderedHashSet<std::string> set2 = { "b", "a" };
        OrderedHashSet<std::string> set3 = { "a", "c" };

        std::cout << "set1 insertion order: a, b\n";
        std::cout << "set2 insertion order: b, a\n";
        std::cout << "set1 == set2: " << ( set1 == set2 ? "true" : "false" ) << " (content matches)\n";
        std::cout << "set1 == set3: " << ( set1 == set3 ? "true" : "false" ) << " (different keys)\n";

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
        OrderedHashSet<std::string, uint32_t, 0xDEADBEEF, CustomHasher> secureSet;

        secureSet.insert( "token1" );
        secureSet.insert( "token2" );

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

        OrderedHashSet<std::pair<int, int>> coordinates;
        coordinates.insert( { 0, 0 } );
        coordinates.insert( { 1, 0 } );
        coordinates.insert( { 0, 1 } );

        std::cout << "Insertion order: {0,0}, {1,0}, {0,1}\n";
        std::cout << "Iteration order:\n";
        for ( const auto& coord : coordinates )
        {
            std::cout << "  {" << coord.first << "," << coord.second << "}\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 13. Move semantics with emplace
    //=====================================================================
    {
        std::cout << "13. Move semantics with emplace\n";
        std::cout << "--------------------------------\n";

        OrderedHashSet<std::string> set;

        std::string movable = "movable_string";
        std::cout << "Original string: \"" << movable << "\"\n";

        bool inserted = set.emplace( std::move( movable ) );
        std::cout << "emplace(std::move(str)): " << ( inserted ? "inserted" : "already exists" ) << "\n";
        std::cout << "After move, original: \"" << movable << "\" (moved-from)\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 14. Clear operation
    //=====================================================================
    {
        std::cout << "14. Clear operation\n";
        std::cout << "-------------------\n";

        OrderedHashSet<std::string> temp = { "a", "b", "c" };
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

        OrderedHashSet<std::string> log;

        log.insert( "event1" );
        log.insert( "event2" );
        log.insert( "event3" );
        log.insert( "event2" ); // Duplicate - no change
        log.insert( "event4" );

        std::cout << "Operations: insert(event1), insert(event2), insert(event3),\n";
        std::cout << "            insert(event2), insert(event4)\n";
        std::cout << "Iteration order (duplicates don't change position):\n";
        for ( const auto& key : log )
        {
            std::cout << "  " << key << "\n";
        }

        log.erase( "event2" );
        std::cout << "\nAfter erase(event2):\n";
        for ( const auto& key : log )
        {
            std::cout << "  " << key << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 16. Large dataset with preserved order
    //=====================================================================
    {
        std::cout << "16. Large dataset with preserved order\n";
        std::cout << "---------------------------------------\n";

        OrderedHashSet<int, uint64_t> large;

        // Insert 1,000 elements
        for ( int i = 0; i < 1000; ++i )
        {
            large.insert( i );
        }

        std::cout << "Inserted: 1,000 elements\n";
        std::cout << "Size: " << large.size() << "\n";
        std::cout << "Capacity: " << large.capacity() << "\n";

        // Verify order (first 5 elements)
        std::cout << "First 5 elements:\n";
        int count = 0;
        for ( const auto& key : large )
        {
            if ( count++ < 5 )
            {
                std::cout << "  " << key << "\n";
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
    // 17. Use case: Unique tag tracking (ordered set foundation)
    //=====================================================================
    {
        std::cout << "17. Use case: Unique tag tracking (ordered set foundation)\n";
        std::cout << "-----------------------------------------------------------\n";

        OrderedHashSet<std::string> tags;

        // Simulate tag additions (insert in order)
        tags.insert( "priority:high" );
        tags.insert( "category:bug" );
        tags.insert( "status:open" );

        std::cout << "Tags (in addition order):\n";
        for ( const auto& tag : tags )
        {
            std::cout << "  " << tag << "\n";
        }

        std::cout << "\nNote: Perfect for ordered unique collections!\n";
        std::cout << "      Use cases: tag systems, unique event logs,\n";
        std::cout << "      ordered symbol tables, configuration keys.\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 17. Extract operation
    //=====================================================================
    {
        std::cout << "17. Extract operation\n";
        std::cout << "---------------------\n";

        OrderedHashSet<std::string> set = { "first", "second", "third", "fourth" };

        std::cout << "Original order: ";
        for ( const auto& key : set )
        {
            std::cout << key << " ";
        }
        std::cout << "\n";

        std::cout << "Set size before extract: " << set.size() << "\n";

        auto extracted = set.extract( "second" );

        if ( extracted )
        {
            std::cout << "Extracted: " << *extracted << "\n";
        }

        std::cout << "Set size after extract: " << set.size() << "\n";
        std::cout << "Order after extract: ";
        for ( const auto& key : set )
        {
            std::cout << key << " ";
        }
        std::cout << "\n";

        // Extract non-existent element
        auto notFound = set.extract( "nonexistent" );
        std::cout << "Extract 'nonexistent': " << ( notFound ? "found" : "not found" ) << "\n";

        // Heterogeneous lookup with string_view
        auto extracted2 = set.extract( std::string_view( "third" ) );
        if ( extracted2 )
        {
            std::cout << "Extracted with string_view: " << *extracted2 << "\n";
        }

        std::cout << "Note: Insertion order is preserved after extraction!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 18. Merge operation
    //=====================================================================
    {
        std::cout << "18. Merge operation\n";
        std::cout << "-------------------\n";

        OrderedHashSet<std::string> set1 = { "apple", "banana" };
        OrderedHashSet<std::string> set2 = { "cherry", "apple", "date" }; // "apple" is duplicate

        std::cout << "set1 order before merge: ";
        for ( const auto& key : set1 )
        {
            std::cout << key << " ";
        }
        std::cout << "\n";

        std::cout << "set2 order before merge: ";
        for ( const auto& key : set2 )
        {
            std::cout << key << " ";
        }
        std::cout << "\n";

        std::cout << "set1 size before merge: " << set1.size() << "\n";
        std::cout << "set2 size before merge: " << set2.size() << "\n";

        set1.merge( set2 );

        std::cout << "set1 size after merge: " << set1.size() << "\n";
        std::cout << "set2 size after merge: " << set2.size() << " (contains duplicates only)\n";

        std::cout << "\nset1 order after merge: ";
        for ( const auto& key : set1 )
        {
            std::cout << key << " ";
        }
        std::cout << "\n";

        std::cout << "\nset2 remaining contents:\n";
        for ( const auto& key : set2 )
        {
            std::cout << "  " << key << " (duplicate)\n";
        }

        std::cout << "Note: Duplicates remain in source set, unique elements are moved\n";
        std::cout << "      Insertion order is preserved in both sets!\n";
        std::cout << "\n";
    }

    return 0;
}
