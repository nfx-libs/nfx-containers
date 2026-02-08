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
 * @file Sample_StackHashSet.cpp
 * @brief Demonstrates StackHashSet usage with small buffer optimization
 * @details This sample shows how to use StackHashSet for zero-allocation small sets
 *          with automatic heap fallback for larger datasets
 */

#include <nfx/Containers.h>

#include <iostream>
#include <string>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers StackHashSet ===\n\n";

    //=====================================================================
    // 1. Stack storage - zero heap allocations
    //=====================================================================
    {
        std::cout << "1. Stack storage - zero heap allocations\n";
        std::cout << "-----------------------------------------\n";

        StackHashSet<std::string, 8> flags;
        flags.insert( "debug" );
        flags.insert( "verbose" );
        flags.insert( "trace" );

        std::cout << "Size: " << flags.size() << " items\n";
        std::cout << "Stack capacity: " << flags.stackCapacity() << " items\n";
        std::cout << "Storage: Stack (zero heap allocations!)\n";
        std::cout << "contains(\"debug\"): " << ( flags.contains( "debug" ) ? "true" : "false" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Initializer list construction
    //=====================================================================
    {
        std::cout << "2. Initializer list construction\n";
        std::cout << "---------------------------------\n";

        StackHashSet<std::string, 4> tags{ "cpp", "performance", "containers" };

        std::cout << "Created with " << tags.size() << " items (on stack)\n";
        std::cout << "contains(\"cpp\"): " << ( tags.contains( "cpp" ) ? "true" : "false" ) << "\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 3. Automatic transition to heap
    //=====================================================================
    {
        std::cout << "3. Automatic transition to heap\n";
        std::cout << "--------------------------------\n";

        StackHashSet<int, 3> set;

        std::cout << "Inserting items (capacity = 3):\n";
        set.insert( 1 );
        std::cout << "  After insert 1: size = " << set.size() << " (stack)\n";

        set.insert( 2 );
        std::cout << "  After insert 2: size = " << set.size() << " (stack)\n";

        set.insert( 3 );
        std::cout << "  After insert 3: size = " << set.size() << " (stack)\n";

        set.insert( 4 );
        std::cout << "  After insert 4: size = " << set.size() << " (heap - automatic transition!)\n";

        set.insert( 5 );
        std::cout << "  After insert 5: size = " << set.size() << " (heap)\n";

        std::cout << "\nAll data preserved after transition:\n";
        for ( int i{ 1 }; i <= 5; ++i )
        {
            std::cout << "  contains(" << i << "): " << ( set.contains( i ) ? "true" : "false" ) << "\n";
        }
        std::cout << "\n";
    }

    //=====================================================================
    // 4. Lookup operations
    //=====================================================================
    {
        std::cout << "4. Lookup operations\n";
        std::cout << "--------------------\n";

        StackHashSet<std::string> categories{ "urgent", "review", "approved" };

        std::cout << "contains(\"urgent\"): " << ( categories.contains( "urgent" ) ? "true" : "false" ) << "\n";
        std::cout << "contains(\"rejected\"): " << ( categories.contains( "rejected" ) ? "true" : "false" ) << "\n";

        // Using find() to get pointer to key
        if ( const std::string * found{ categories.find( "urgent" ) } )
        {
            std::cout << "find(\"urgent\") = \"" << *found << "\"\n";
        }

        const std::string* notFound{ categories.find( "rejected" ) };
        std::cout << "find(\"rejected\") = " << ( notFound ? "found" : "nullptr" ) << "\n";

        // Using at() with exception handling
        try
        {
            const std::string& key{ categories.at( "approved" ) };
            std::cout << "at(\"approved\") = \"" << key << "\"\n";
        }
        catch ( const std::out_of_range& )
        {
            std::cout << "at(\"approved\") threw exception\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Heterogeneous lookup (zero-copy)
    //=====================================================================
    {
        std::cout << "5. Heterogeneous lookup (zero-copy)\n";
        std::cout << "------------------------------------\n";

        StackHashSet<std::string> keywords{ "const", "static", "inline" };

        std::string_view key{ "const" };
        std::cout << "string_view lookup: contains(\"const\") = " << ( keywords.contains( key ) ? "true" : "false" ) << "\n";
        std::cout << "Note: Zero allocations for string_view lookup!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 6. Insert and erase
    //=====================================================================
    {
        std::cout << "6. Insert and erase\n";
        std::cout << "-------------------\n";

        StackHashSet<int, 8> numbers;

        auto [ptr1, inserted1]{ numbers.insert( 10 ) };
        std::cout << "insert(10): inserted = " << ( inserted1 ? "true" : "false" ) << "\n";

        auto [ptr2, inserted2]{ numbers.insert( 10 ) };
        std::cout << "insert(10) again: inserted = " << ( inserted2 ? "true" : "false" ) << " (duplicate)\n";

        std::cout << "Value: " << *ptr1 << "\n";

        size_t erased{ numbers.erase( 10 ) };
        std::cout << "erase(10): removed " << erased << " item(s)\n";
        std::cout << "Size after erase: " << numbers.size() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 7. Emplace construction
    //=====================================================================
    {
        std::cout << "7. Emplace construction\n";
        std::cout << "-----------------------\n";

        StackHashSet<std::string> set;

        auto [ptr, inserted]{ set.emplace( "emplaced" ) };
        std::cout << "emplace(\"emplaced\"): inserted = " << ( inserted ? "true" : "false" ) << "\n";
        std::cout << "Result: " << *ptr << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 8. Clear operation
    //=====================================================================
    {
        std::cout << "8. Clear operation\n";
        std::cout << "------------------\n";

        StackHashSet<int> data{ 1, 2, 3 };
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

        StackHashSet<std::string> priorities{ "high", "medium", "low" };

        auto extracted{ priorities.extract( "high" ) };

        if ( extracted.has_value() )
        {
            std::cout << "Extracted: " << extracted.value() << "\n";
        }

        std::cout << "After extract: size = " << priorities.size() << "\n";
        std::cout << "contains(\"high\"): " << ( priorities.contains( "high" ) ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 10. Merge operation
    //=====================================================================
    {
        std::cout << "10. Merge operation\n";
        std::cout << "-------------------\n";

        StackHashSet<std::string> features1{ "async", "coroutines" };
        StackHashSet<std::string> features2{ "modules", "async" }; // Duplicate

        features1.merge( features2 );

        std::cout << "After merge: size = " << features1.size() << "\n";
        std::cout << "  contains(\"async\"): " << ( features1.contains( "async" ) ? "true" : "false" ) << "\n";
        std::cout << "  contains(\"coroutines\"): " << ( features1.contains( "coroutines" ) ? "true" : "false" ) << "\n";
        std::cout << "  contains(\"modules\"): " << ( features1.contains( "modules" ) ? "true" : "false" ) << "\n";
        std::cout << "Note: Duplicates are automatically ignored!\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 11. Use case: Feature flags
    //=====================================================================
    {
        std::cout << "11. Use case: Feature flags\n";
        std::cout << "----------------------------\n";

        // Perfect for local feature flags with predictable small size
        StackHashSet<std::string, 4> enabledFeatures{
            "dark_mode",
            "notifications",
            "auto_save" };

        std::cout << "Enabled features (stack-allocated):\n";
        std::cout << "  dark_mode: " << ( enabledFeatures.contains( "dark_mode" ) ? "ON" : "OFF" ) << "\n";
        std::cout << "  notifications: " << ( enabledFeatures.contains( "notifications" ) ? "ON" : "OFF" ) << "\n";
        std::cout << "  auto_save: " << ( enabledFeatures.contains( "auto_save" ) ? "ON" : "OFF" ) << "\n";

        std::cout << "\nBenefit: Zero heap allocations for hot path!\n";
        std::cout << "\n";
    }

    return 0;
}
