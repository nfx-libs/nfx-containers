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
 * @file Sample_PerfectHashMap.cpp
 * @brief Demonstrates PerfectHashMap usage with CHD perfect hashing
 * @details This sample shows how to use PerfectHashMap for immutable key-value storage
 *          with guaranteed O(1) lookup and minimal memory overhead
 */

#include <iostream>
#include <string>
#include <vector>

#include <nfx/Containers.h>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers PerfectHashMap ===\n\n";

    //=====================================================================
    // 1. Basic construction from data
    //=====================================================================
    {
        std::cout << "1. Basic construction from data\n";
        std::cout << "--------------------------------\n";

        std::vector<std::pair<std::string, int>> data = {
            { "apple", 5 },
            { "banana", 3 },
            { "cherry", 8 },
            { "date", 12 } };

        PerfectHashMap<std::string, int> fruits( std::move( data ) );

        std::cout << "Created map with " << fruits.count() << " items\n";
        std::cout << "Table size: " << fruits.size() << " slots\n";
        std::cout << "Note: Immutable after construction\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Lookup operations (contains, find, at)
    //=====================================================================
    {
        std::cout << "2. Lookup operations (contains, find, at)\n";
        std::cout << "------------------------------------------\n";

        std::vector<std::pair<std::string, int>> data = {
            { "red", 1 },
            { "green", 2 },
            { "blue", 3 } };

        PerfectHashMap<std::string, int> colors( std::move( data ) );

        std::cout << "contains(\"green\"): " << ( colors.contains( "green" ) ? "true" : "false" ) << "\n";
        std::cout << "contains(\"yellow\"): " << ( colors.contains( "yellow" ) ? "true" : "false" ) << "\n";

        if ( const int* value = colors.find( "blue" ) )
        {
            std::cout << "find(\"blue\"): " << *value << "\n";
        }

        std::cout << "at(\"red\"): " << colors.at( "red" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 3. Heterogeneous lookup (zero-copy)
    //=====================================================================
    {
        std::cout << "3. Heterogeneous lookup (zero-copy)\n";
        std::cout << "------------------------------------\n";

        std::vector<std::pair<std::string, int>> data = {
            { "config.json", 100 },
            { "data.bin", 200 } };

        PerfectHashMap<std::string, int> files( std::move( data ) );

        // string_view lookup - no temporary string allocation!
        std::string_view sv = "config.json";
        if ( const int* value = files.find( sv ) )
        {
            std::cout << "string_view lookup: " << *value << " (zero allocations!)\n";
        }

        // const char* lookup - also zero-copy
        if ( const int* value = files.find( "data.bin" ) )
        {
            std::cout << "const char* lookup: " << *value << " (zero allocations!)\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 4. Iteration
    //=====================================================================
    {
        std::cout << "4. Iteration\n";
        std::cout << "------------\n";

        std::vector<std::pair<std::string, int>> data = {
            { "first", 1 },
            { "second", 2 },
            { "third", 3 } };

        PerfectHashMap<std::string, int> items( std::move( data ) );

        std::cout << "Range-based for loop:\n";
        for ( const auto& [key, value] : items )
        {
            std::cout << "  " << key << " -> " << value << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Integer keys with 64-bit hashing
    //=====================================================================
    {
        std::cout << "5. Integer keys with 64-bit hashing\n";
        std::cout << "------------------------------------\n";

        std::vector<std::pair<int, std::string>> data = {
            { 1, "one" },
            { 2, "two" },
            { 42, "answer" },
            { 100, "century" } };

        PerfectHashMap<int, std::string, uint64_t> numbers( std::move( data ) );

        std::cout << "numbers[42]: " << numbers.at( 42 ) << "\n";
        std::cout << "Table size: " << numbers.size() << " slots for " << numbers.count() << " items\n";
        std::cout << "Note: uint64_t hash provides better distribution\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 6. Empty map
    //=====================================================================
    {
        std::cout << "6. Empty map\n";
        std::cout << "------------\n";

        PerfectHashMap<std::string, int> empty;

        std::cout << "isEmpty(): " << ( empty.isEmpty() ? "true" : "false" ) << "\n";
        std::cout << "count(): " << empty.count() << "\n";
        std::cout << "size(): " << empty.size() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 7. Equality comparison
    //=====================================================================
    {
        std::cout << "7. Equality comparison\n";
        std::cout << "----------------------\n";

        std::vector<std::pair<std::string, int>> data1 = { { "a", 1 }, { "b", 2 } };
        std::vector<std::pair<std::string, int>> data2 = { { "a", 1 }, { "b", 2 } };
        std::vector<std::pair<std::string, int>> data3 = { { "a", 1 }, { "b", 3 } };

        PerfectHashMap<std::string, int> map1( std::move( data1 ) );
        PerfectHashMap<std::string, int> map2( std::move( data2 ) );
        PerfectHashMap<std::string, int> map3( std::move( data3 ) );

        std::cout << "map1 == map2: " << ( map1 == map2 ? "true" : "false" ) << "\n";
        std::cout << "map1 == map3: " << ( map1 == map3 ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 8. Duplicate key detection
    //=====================================================================
    {
        std::cout << "8. Duplicate key detection\n";
        std::cout << "--------------------------\n";

        try
        {
            std::vector<std::pair<std::string, int>> badData = {
                { "key1", 1 },
                { "key2", 2 },
                { "key1", 3 } // Duplicate!
            };
            PerfectHashMap<std::string, int> badMap( std::move( badData ) );
        }
        catch ( const std::invalid_argument& e )
        {
            std::cout << "Caught exception: " << e.what() << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 9. Complex key types (pairs)
    //=====================================================================
    {
        std::cout << "9. Complex key types (pairs)\n";
        std::cout << "-----------------------------\n";

        std::vector<std::pair<std::pair<int, int>, std::string>> data = {
            { { 0, 0 }, "origin" },
            { { 1, 0 }, "east" },
            { { 0, 1 }, "north" } };

        PerfectHashMap<std::pair<int, int>, std::string> grid( std::move( data ) );

        auto coord = std::make_pair( 1, 0 );
        std::cout << "grid[{1,0}]: " << grid.at( coord ) << "\n";
        std::cout << "Table size: " << grid.size() << " slots\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 10. Custom hash seed
    //=====================================================================
    {
        std::cout << "10. Custom hash seed\n";
        std::cout << "--------------------\n";

        using CustomHasher = nfx::hashing::Hasher<uint32_t, 0xDEADBEEF>;
        std::vector<std::pair<std::string, std::string>> data = {
            { "token", "abc123" },
            { "session", "xyz789" } };

        PerfectHashMap<std::string, std::string, uint32_t, 0xDEADBEEF, CustomHasher> secureMap( std::move( data ) );

        std::cout << "Created map with custom seed: 0xDEADBEEF\n";
        std::cout << "Use case: Domain separation for security contexts\n";
        std::cout << "secureMap[\"token\"]: " << secureMap.at( "token" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 11. Custom struct with custom hasher
    //=====================================================================
    {
        std::cout << "11. Custom struct with custom hasher\n";
        std::cout << "-------------------------------------\n";

        struct Point3D
        {
            int x, y, z;

            bool operator==( const Point3D& other ) const
            {
                return x == other.x && y == other.y && z == other.z;
            }
        };

        struct Point3DHasher
        {
            using is_transparent = void;

            uint32_t operator()( const Point3D& p ) const noexcept
            {
                uint32_t hash = 0x811c9dc5; // FNV offset basis
                hash ^= static_cast<uint32_t>( p.x );
                hash *= 0x01000193; // FNV prime
                hash ^= static_cast<uint32_t>( p.y );
                hash *= 0x01000193;
                hash ^= static_cast<uint32_t>( p.z );
                hash *= 0x01000193;
                return hash;
            }
        };

        std::vector<std::pair<Point3D, std::string>> data = {
            { { 0, 0, 0 }, "origin" },
            { { 1, 0, 0 }, "x-axis" },
            { { 0, 1, 0 }, "y-axis" },
            { { 1, 1, 1 }, "diagonal" } };

        PerfectHashMap<Point3D, std::string, uint32_t, 0x811c9dc5, Point3DHasher, std::equal_to<>> point3DMap( std::move( data ) );

        Point3D lookup = { 1, 1, 1 };
        std::cout << "point3DMap[{1,1,1}]: " << point3DMap.at( lookup ) << "\n";
        std::cout << "Using custom Point3DHasher with FNV-1a mixing\n";
        std::cout << "Table size: " << point3DMap.size() << " slots\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 12. Large dataset with perfect hashing
    //=====================================================================
    {
        std::cout << "12. Large dataset with perfect hashing\n";
        std::cout << "---------------------------------------\n";

        std::vector<std::pair<int, int>> data;
        for ( int i = 0; i < 10000; ++i )
        {
            data.emplace_back( i, i * i ); // key = i, value = i²
        }

        PerfectHashMap<int, int, uint64_t> large( std::move( data ) );

        std::cout << "Created map with " << large.count() << " items\n";
        std::cout << "Table size: " << large.size() << " slots\n";
        std::cout << "Load factor: " << ( 100.0 * large.count() / large.size() ) << "%\n";

        // Verify lookup
        std::cout << "large[5000]: " << large.at( 5000 ) << " (expected: 25000000)\n";
        std::cout << "Note: O(1) guaranteed lookup with CHD algorithm\n";

        std::cout << "\n";
    }

    return 0;
}
