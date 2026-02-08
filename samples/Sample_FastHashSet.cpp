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
 * @file Sample_FastHashSet.cpp
 * @brief Demonstrates FastHashSet usage with Robin Hood hashing
 * @details This sample shows how to use FastHashSet for high-performance unique value storage
 *          with bounded probe distances and heterogeneous lookup capabilities
 */

#include <nfx/Containers.h>

#include <iostream>
#include <string>
#include <string_view>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers FastHashSet ===\n\n";

    //=====================================================================
    // 1. Basic construction and insertion
    //=====================================================================
    {
        std::cout << "1. Basic construction and insertion\n";
        std::cout << "------------------------------------\n";

        FastHashSet<std::string> usernames;

        bool inserted1 = usernames.insert( "alice" );
        bool inserted2 = usernames.insert( "bob" );
        bool inserted3 = usernames.insert( "alice" ); // Duplicate

        std::cout << "insert(\"alice\"): " << ( inserted1 ? "true" : "false" ) << "\n";
        std::cout << "insert(\"bob\"): " << ( inserted2 ? "true" : "false" ) << "\n";
        std::cout << "insert(\"alice\") again: " << ( inserted3 ? "true" : "false (duplicate)" ) << "\n";
        std::cout << "Size: " << usernames.size() << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Initializer list construction
    //=====================================================================
    {
        std::cout << "2. Initializer list construction\n";
        std::cout << "---------------------------------\n";

        FastHashSet<std::string> tags = { "cpp", "programming", "performance", "containers" };

        std::cout << "Created set with " << tags.size() << " tags\n";
        std::cout << "Syntax: FastHashSet<T> set = {val1, val2, ...};\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 3. Membership testing (contains and find)
    //=====================================================================
    {
        std::cout << "3. Membership testing (contains and find)\n";
        std::cout << "------------------------------------------\n";

        FastHashSet<std::string> cache;
        cache.insert( "data.json" );
        cache.insert( "config.xml" );

        if ( cache.contains( "data.json" ) )
        {
            std::cout << "contains(\"data.json\"): true\n";
        }

        if ( const std::string* found = cache.find( "config.xml" ) )
        {
            std::cout << "find(\"config.xml\"): " << *found << "\n";
        }

        if ( !cache.contains( "missing.txt" ) )
        {
            std::cout << "contains(\"missing.txt\"): false\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 4. Heterogeneous lookup (zero-copy)
    //=====================================================================
    {
        std::cout << "4. Heterogeneous lookup (zero-copy)\n";
        std::cout << "------------------------------------\n";

        FastHashSet<std::string> files;
        files.insert( "readme.md" );
        files.insert( "license.txt" );

        // string_view lookup - no temporary string allocation!
        std::string_view sv = "readme.md";
        if ( files.contains( sv ) )
        {
            std::cout << "string_view lookup: found (zero allocations!)\n";
        }

        // const char* lookup - also zero-copy
        if ( files.contains( "license.txt" ) )
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

        FastHashSet<std::string> items;
        items.insert( "item1" );
        items.insert( "item2" );
        items.insert( "item3" );

        std::cout << "Size before erase: " << items.size() << "\n";

        bool erased = items.erase( "item2" );
        std::cout << "erase(\"item2\"): " << ( erased ? "success" : "not found" ) << "\n";
        std::cout << "Size after erase: " << items.size() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 6. Iteration
    //=====================================================================
    {
        std::cout << "6. Iteration\n";
        std::cout << "------------\n";

        FastHashSet<std::string> colors = { "red", "green", "blue", "yellow" };

        std::cout << "Range-based for loop:\n";
        for ( const auto& color : colors )
        {
            std::cout << "  " << color << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 7. Integer keys
    //=====================================================================
    {
        std::cout << "7. Integer keys\n";
        std::cout << "---------------\n";

        FastHashSet<int, uint32_t> primes = { 2, 3, 5, 7, 11, 13 };

        std::cout << "Prime numbers: ";
        for ( const auto& prime : primes )
        {
            std::cout << prime << " ";
        }
        std::cout << "\n";

        std::cout << "contains(4): " << ( primes.contains( 4 ) ? "true" : "false" ) << "\n";
        std::cout << "contains(7): " << ( primes.contains( 7 ) ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 8. Custom capacity and reserve
    //=====================================================================
    {
        std::cout << "8. Custom capacity and reserve\n";
        std::cout << "-------------------------------\n";

        FastHashSet<int, uint32_t> preallocated( 256 );
        std::cout << "Initial capacity: " << preallocated.capacity() << " (user-specified)\n";

        FastHashSet<int, uint32_t> resized;
        std::cout << "Default capacity: " << resized.capacity() << "\n";
        resized.reserve( 512 );
        std::cout << "After reserve(512): " << resized.capacity() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 9. Robin Hood hashing efficiency
    //=====================================================================
    {
        std::cout << "9. Robin Hood hashing efficiency\n";
        std::cout << "---------------------------------\n";

        FastHashSet<int, uint32_t> numbers;

        // Insert elements that may collide
        for ( int i = 0; i < 1000; ++i )
        {
            numbers.insert( i );
        }

        std::cout << "Inserted 1000 elements\n";
        std::cout << "Size: " << numbers.size() << "\n";
        std::cout << "Capacity: " << numbers.capacity() << "\n";
        std::cout << "Load factor: " << ( 100.0 * numbers.size() / numbers.capacity() ) << "%\n";
        std::cout << "Note: Robin Hood ensures O(log n) worst-case probe distance\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 10. Equality comparison
    //=====================================================================
    {
        std::cout << "10. Equality comparison\n";
        std::cout << "-----------------------\n";

        FastHashSet<int, uint32_t> set1 = { 1, 2, 3 };
        FastHashSet<int, uint32_t> set2 = { 1, 2, 3 };
        FastHashSet<int, uint32_t> set3 = { 1, 2, 4 };

        std::cout << "set1 == set2: " << ( set1 == set2 ? "true" : "false" ) << "\n";
        std::cout << "set1 == set3: " << ( set1 == set3 ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 11. Set intersection
    //=====================================================================
    {
        std::cout << "11. Set intersection\n";
        std::cout << "--------------------\n";

        FastHashSet<int, uint32_t> setA = { 1, 2, 3, 4, 5 };
        FastHashSet<int, uint32_t> setB = { 3, 4, 5, 6, 7 };

        FastHashSet<int, uint32_t> intersection;
        for ( const auto& elem : setA )
        {
            if ( setB.contains( elem ) )
            {
                intersection.insert( elem );
            }
        }

        std::cout << "setA ∩ setB: ";
        for ( const auto& elem : intersection )
        {
            std::cout << elem << " ";
        }
        std::cout << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 12. Set difference
    //=====================================================================
    {
        std::cout << "12. Set difference\n";
        std::cout << "------------------\n";

        FastHashSet<int, uint32_t> setA = { 1, 2, 3, 4, 5 };
        FastHashSet<int, uint32_t> setB = { 3, 4, 5, 6, 7 };

        FastHashSet<int, uint32_t> difference;
        for ( const auto& elem : setA )
        {
            if ( !setB.contains( elem ) )
            {
                difference.insert( elem );
            }
        }

        std::cout << "setA - setB: ";
        for ( const auto& elem : difference )
        {
            std::cout << elem << " ";
        }
        std::cout << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 13. Move semantics
    //=====================================================================
    {
        std::cout << "13. Move semantics\n";
        std::cout << "------------------\n";

        FastHashSet<std::string> original = { "one", "two", "three" };
        std::cout << "Original size: " << original.size() << "\n";

        FastHashSet<std::string> moved( std::move( original ) );
        std::cout << "Moved size: " << moved.size() << "\n";
        std::cout << "Original size after move: " << original.size() << " (moved-from)\n";

        std::string temp = "temp_value";
        moved.insert( std::move( temp ) );
        std::cout << "Temp string after move: \"" << temp << "\" (moved-from)\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 14. Clear operation
    //=====================================================================
    {
        std::cout << "14. Clear operation\n";
        std::cout << "-------------------\n";

        FastHashSet<std::string> temp = { "a", "b", "c" };
        std::cout << "Size before clear: " << temp.size() << "\n";

        temp.clear();
        std::cout << "Size after clear: " << temp.size() << "\n";
        std::cout << "isEmpty(): " << ( temp.isEmpty() ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 15. 64-bit hashing for large datasets
    //=====================================================================
    {
        std::cout << "14. 64-bit hashing for large datasets\n";
        std::cout << "--------------------------------------\n";

        FastHashSet<uint64_t, uint64_t> largeIds;

        // Insert large 64-bit identifiers
        for ( uint64_t i = 0; i < 10000; ++i )
        {
            largeIds.insert( i * 1000000000ULL );
        }

        std::cout << "Inserted: 10,000 large IDs\n";
        std::cout << "Size: " << largeIds.size() << "\n";
        std::cout << "Capacity: " << largeIds.capacity() << "\n";

        uint64_t searchId = 5000000000000ULL;
        if ( largeIds.contains( searchId ) )
        {
            std::cout << "Found ID: " << searchId << "\n";
        }
        else
        {
            std::cout << "ID not found: " << searchId << "\n";
        }

        std::cout << "\n";
    }

    return 0;
}
