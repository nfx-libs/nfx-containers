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
 * @file Sample_StackHashMap.cpp
 * @brief Demonstrates StackHashMap usage with small buffer optimization
 * @details This sample shows how to use StackHashMap for zero-allocation small maps
 *          with automatic heap fallback for larger datasets
 */

#include <nfx/Containers.h>

#include <iostream>
#include <string>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers StackHashMap ===\n\n";

    //=====================================================================
    // 1. Stack storage - zero heap allocations
    //=====================================================================
    {
        std::cout << "1. Stack storage - zero heap allocations\n";
        std::cout << "-----------------------------------------\n";

        StackHashMap<std::string, int, 8> config;
        config["debug"] = 1;
        config["verbose"] = 0;
        config["max_threads"] = 4;

        std::cout << "Size: " << config.size() << " items\n";
        std::cout << "Stack capacity: " << config.stackCapacity() << " items\n";
        std::cout << "Storage: Stack (zero heap allocations!)\n";
        std::cout << "debug = " << config.at( "debug" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Initializer list construction
    //=====================================================================
    {
        std::cout << "2. Initializer list construction\n";
        std::cout << "---------------------------------\n";

        StackHashMap<std::string, int, 4> settings{
            { "width", 1920 },
            { "height", 1080 },
            { "fps", 60 } };

        std::cout << "Created with " << settings.size() << " items (on stack)\n";
        std::cout << "width = " << settings["width"] << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 3. Automatic transition to heap
    //=====================================================================
    {
        std::cout << "3. Automatic transition to heap\n";
        std::cout << "--------------------------------\n";

        StackHashMap<int, std::string, 3> map;

        std::cout << "Inserting items (capacity = 3):\n";
        map[1] = "one";
        std::cout << "  After insert 1: size = " << map.size() << " (stack)\n";

        map[2] = "two";
        std::cout << "  After insert 2: size = " << map.size() << " (stack)\n";

        map[3] = "three";
        std::cout << "  After insert 3: size = " << map.size() << " (stack)\n";

        map[4] = "four";
        std::cout << "  After insert 4: size = " << map.size() << " (heap - automatic transition!)\n";

        map[5] = "five";
        std::cout << "  After insert 5: size = " << map.size() << " (heap)\n";

        std::cout << "\nAll data preserved after transition:\n";
        std::cout << "  map[1] = " << map[1] << "\n";
        std::cout << "  map[2] = " << map[2] << "\n";
        std::cout << "  map[3] = " << map[3] << "\n";
        std::cout << "  map[4] = " << map[4] << "\n";
        std::cout << "  map[5] = " << map[5] << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 4. Lookup operations
    //=====================================================================
    {
        std::cout << "4. Lookup operations\n";
        std::cout << "--------------------\n";

        StackHashMap<std::string, int> cache{
            { "hits", 100 },
            { "misses", 20 } };

        std::cout << "contains(\"hits\"): " << ( cache.contains( "hits" ) ? "true" : "false" ) << "\n";
        std::cout << "contains(\"errors\"): " << ( cache.contains( "errors" ) ? "true" : "false" ) << "\n";

        std::cout << "count(\"hits\"): " << cache.count( "hits" ) << "\n";
        std::cout << "count(\"errors\"): " << cache.count( "errors" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Heterogeneous lookup (zero-copy)
    //=====================================================================
    {
        std::cout << "5. Heterogeneous lookup (zero-copy)\n";
        std::cout << "------------------------------------\n";

        StackHashMap<std::string, int> vars{
            { "x", 10 },
            { "y", 20 } };

        std::string_view key{ "x" };
        std::cout << "string_view lookup: contains(\"x\") = " << ( vars.contains( key ) ? "true" : "false" ) << "\n";
        std::cout << "Note: Zero allocations for string_view lookup!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 6. Insert and erase
    //=====================================================================
    {
        std::cout << "6. Insert and erase\n";
        std::cout << "-------------------\n";

        StackHashMap<int, std::string, 8> items;

        auto [ptr1, inserted1]{ items.insert( { 10, "ten" } ) };
        std::cout << "insert({10, \"ten\"}): inserted = " << ( inserted1 ? "true" : "false" ) << "\n";

        auto [ptr2, inserted2]{ items.insert( { 10, "duplicate" } ) };
        std::cout << "insert({10, \"duplicate\"}): inserted = " << ( inserted2 ? "true" : "false" ) << " (key exists)\n";

        std::cout << "Value for key 10: " << ptr1->second << "\n";

        size_t erased{ items.erase( 10 ) };
        std::cout << "erase(10): removed " << erased << " item(s)\n";
        std::cout << "Size after erase: " << items.size() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 7. Emplace construction
    //=====================================================================
    {
        std::cout << "7. Emplace construction\n";
        std::cout << "-----------------------\n";

        StackHashMap<std::string, std::string> map;

        auto [ptr, inserted]{ map.emplace( "key", "value" ) };
        std::cout << "emplace(\"key\", \"value\"): inserted = " << ( inserted ? "true" : "false" ) << "\n";
        std::cout << "Result: " << ptr->first << " -> " << ptr->second << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 8. Clear operation
    //=====================================================================
    {
        std::cout << "8. Clear operation\n";
        std::cout << "------------------\n";

        StackHashMap<int, int> data{ { 1, 10 }, { 2, 20 }, { 3, 30 } };
        std::cout << "Before clear: size = " << data.size() << "\n";

        data.clear();
        std::cout << "After clear: size = " << data.size() << "\n";
        std::cout << "isEmpty: " << ( data.isEmpty() ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 9. Extract operation
    //=====================================================================
    {
        std::cout << "9. Extract operation\n";
        std::cout << "--------------------\n";

        StackHashMap<std::string, int> stats{
            { "wins", 10 },
            { "losses", 5 },
            { "draws", 2 } };

        auto extracted{ stats.extract( "wins" ) };

        if ( extracted.has_value() )
        {
            std::cout << "Extracted: " << extracted.value().first << " = " << extracted.value().second << "\n";
        }

        std::cout << "After extract: size = " << stats.size() << "\n";
        std::cout << "contains(\"wins\"): " << ( stats.contains( "wins" ) ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 10. Merge operation
    //=====================================================================
    {
        std::cout << "10. Merge operation\n";
        std::cout << "-------------------\n";

        StackHashMap<std::string, int> config1{
            { "timeout", 30 },
            { "retries", 3 } };

        StackHashMap<std::string, int> config2{
            { "port", 8080 },
            { "retries", 5 } }; // Duplicate key

        config1.merge( config2 );

        std::cout << "After merge: size = " << config1.size() << "\n";
        std::cout << "  timeout: " << config1["timeout"] << "\n";
        std::cout << "  retries: " << config1["retries"] << " (original value preserved)\n";
        std::cout << "  port: " << config1["port"] << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 11. Use case: Small local cache
    //=====================================================================
    {
        std::cout << "11. Use case: Small local cache\n";
        std::cout << "--------------------------------\n";

        // Perfect for function-local caches with predictable small size
        StackHashMap<std::string, int, 4> recentScores{
            { "player1", 150 },
            { "player2", 200 },
            { "player3", 175 } };

        std::cout << "Recent scores cache (stack-allocated):\n";
        std::cout << "  player1: " << recentScores["player1"] << "\n";
        std::cout << "  player2: " << recentScores["player2"] << "\n";
        std::cout << "  player3: " << recentScores["player3"] << "\n";

        std::cout << "\nBenefit: Zero heap allocations for hot path!\n";
        std::cout << "\n";
    }

    std::cout << "=== Sample completed ===\n";
    return 0;
}
