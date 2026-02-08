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
 * @file Sample_StackVector.cpp
 * @brief Demonstrates StackVector usage with stack storage optimization
 * @details This sample shows how to use StackVector for small-size-optimized vector storage
 *          with automatic stack/heap transition and zero allocations for small sizes
 */

#include <nfx/Containers.h>

#include <iostream>
#include <string>
#include <vector>

int main()
{
    using namespace nfx::containers;

    std::cout << "=== nfx-containers StackVector ===\n\n";

    //=====================================================================
    // 1. Basic construction and stack storage
    //=====================================================================
    {
        std::cout << "1. Basic construction and stack storage\n";
        std::cout << "----------------------------------------\n";

        StackVector<int, 8> numbers; // Up to 8 ints on stack
        numbers.push_back( 10 );
        numbers.push_back( 20 );
        numbers.push_back( 30 );

        std::cout << "Size: " << numbers.size() << " items\n";
        std::cout << "Capacity: " << numbers.capacity() << " slots\n";
        std::cout << "Note: No heap allocations for <= 8 elements!\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 2. Initializer list construction
    //=====================================================================
    {
        std::cout << "2. Initializer list construction\n";
        std::cout << "---------------------------------\n";

        StackVector<std::string, 4> words = { "hello", "world", "from", "stack" };

        std::cout << "Created vector with " << words.size() << " strings\n";
        std::cout << "Syntax: StackVector<T, N> vec = {val1, val2, ...};\n";
        std::cout << "Storage: Stack (no heap allocations)\n";
        std::cout << "\n";
    }

    //=====================================================================
    // 3. Element access
    //=====================================================================
    {
        std::cout << "3. Element access\n";
        std::cout << "-----------------\n";

        StackVector<int, 4> data = { 100, 200, 300 };

        std::cout << "operator[0]: " << data[0] << "\n";
        std::cout << "operator[1]: " << data[1] << "\n";
        std::cout << "at(2): " << data.at( 2 ) << "\n";
        std::cout << "front(): " << data.front() << "\n";
        std::cout << "back(): " << data.back() << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 4. Automatic heap transition
    //=====================================================================
    {
        std::cout << "4. Automatic heap transition\n";
        std::cout << "-----------------------------\n";

        StackVector<int, 4> vec; // Stack capacity: 4
        std::cout << "Initial capacity (stack): " << vec.capacity() << "\n";

        for ( int i = 0; i < 4; ++i )
        {
            vec.push_back( i );
        }
        std::cout << "After 4 pushes - size: " << vec.size() << ", capacity: " << vec.capacity() << " (still stack)\n";

        vec.push_back( 999 ); // Triggers heap transition
        std::cout << "After 5th push - size: " << vec.size() << ", capacity: " << vec.capacity() << " (now heap)\n";
        std::cout << "Note: Automatic transition from stack to heap storage!\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 5. Iteration
    //=====================================================================
    {
        std::cout << "5. Iteration\n";
        std::cout << "------------\n";

        StackVector<std::string, 8> items = { "first", "second", "third" };

        std::cout << "Range-based for loop:\n";
        for ( const auto& item : items )
        {
            std::cout << "  " << item << "\n";
        }

        std::cout << "\nIterator-based loop:\n";
        for ( auto it = items.begin(); it != items.end(); ++it )
        {
            std::cout << "  " << *it << "\n";
        }

        std::cout << "\n";
    }

    //=====================================================================
    // 6. Emplace operations
    //=====================================================================
    {
        std::cout << "6. Emplace operations\n";
        std::cout << "---------------------\n";

        StackVector<std::string, 4> words;

        words.emplace_back( "constructed" );
        words.emplace_back( "in" );
        words.emplace_back( "place" );

        std::cout << "emplace_back() constructed " << words.size() << " strings in-place\n";
        std::cout << "Values: ";
        for ( const auto& w : words )
        {
            std::cout << w << " ";
        }
        std::cout << "\n";
        std::cout << "Note: No temporary objects created!\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 7. Resize and reserve operations
    //=====================================================================
    {
        std::cout << "7. Resize and reserve operations\n";
        std::cout << "---------------------------------\n";

        StackVector<int, 4> vec;
        std::cout << "Initial - size: " << vec.size() << ", capacity: " << vec.capacity() << "\n";

        vec.resize( 3, 42 );
        std::cout << "After resize(3, 42) - size: " << vec.size() << ", values: ";
        for ( int v : vec )
        {
            std::cout << v << " ";
        }
        std::cout << "\n";

        vec.reserve( 10 );
        std::cout << "After reserve(10) - size: " << vec.size() << ", capacity: " << vec.capacity() << "\n";
        std::cout << "Note: reserve() triggered heap allocation\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 8. Clear and pop operations
    //=====================================================================
    {
        std::cout << "8. Clear and pop operations\n";
        std::cout << "---------------------------\n";

        StackVector<int, 4> vec = { 1, 2, 3, 4 };
        std::cout << "Size before pop_back(): " << vec.size() << "\n";

        vec.pop_back();
        std::cout << "Size after pop_back(): " << vec.size() << "\n";
        std::cout << "Last element: " << vec.back() << "\n";

        vec.clear();
        std::cout << "Size after clear(): " << vec.size() << "\n";
        std::cout << "isEmpty(): " << ( vec.isEmpty() ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 9. Copy and move semantics
    //=====================================================================
    {
        std::cout << "9. Copy and move semantics\n";
        std::cout << "--------------------------\n";

        StackVector<int, 4> vec1 = { 10, 20, 30 };
        StackVector<int, 4> vec2 = vec1; // Copy

        std::cout << "vec1 == vec2: " << ( vec1 == vec2 ? "true" : "false" ) << "\n";

        StackVector<int, 4> vec3 = std::move( vec1 ); // Move
        std::cout << "After move - vec3 size: " << vec3.size() << "\n";
        std::cout << "vec2 == vec3: " << ( vec2 == vec3 ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 10. Custom stack capacity
    //=====================================================================
    {
        std::cout << "10. Custom stack capacity\n";
        std::cout << "-------------------------\n";

        StackVector<int, 16> large; // 16 ints on stack
        StackVector<int, 2> tiny;   // Only 2 ints on stack
        StackVector<int> medium;    // Default: 8 on stack

        std::cout << "StackVector<int, 16> capacity: " << large.capacity() << "\n";
        std::cout << "StackVector<int, 2> capacity: " << tiny.capacity() << "\n";
        std::cout << "StackVector<int> capacity: " << medium.capacity() << " (default)\n";
        std::cout << "Note: Choose N based on typical use case!\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 11. Direct data access
    //=====================================================================
    {
        std::cout << "11. Direct data access\n";
        std::cout << "----------------------\n";

        StackVector<int, 4> vec = { 1, 2, 3, 4 };

        int* ptr = vec.data();
        std::cout << "data() pointer: " << static_cast<void*>( ptr ) << "\n";
        std::cout << "First element via pointer: " << *ptr << "\n";
        std::cout << "Second element via pointer: " << *( ptr + 1 ) << "\n";
        std::cout << "Note: Compatible with C APIs expecting T*\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 12. Equality comparison
    //=====================================================================
    {
        std::cout << "12. Equality comparison\n";
        std::cout << "-----------------------\n";

        StackVector<int, 4> vec1 = { 1, 2, 3 };
        StackVector<int, 4> vec2 = { 1, 2, 3 };
        StackVector<int, 4> vec3 = { 1, 2, 4 };

        std::cout << "vec1 == vec2: " << ( vec1 == vec2 ? "true" : "false" ) << "\n";
        std::cout << "vec1 == vec3: " << ( vec1 == vec3 ? "true" : "false" ) << "\n";
        std::cout << "vec1 != vec3: " << ( vec1 != vec3 ? "true" : "false" ) << "\n";

        std::cout << "\n";
    }

    //=====================================================================
    // 13. Mixed stack/heap scenarios
    //=====================================================================
    {
        std::cout << "13. Mixed stack/heap scenarios\n";
        std::cout << "-------------------------------\n";

        StackVector<std::string, 3> words;

        // Start on stack
        words.push_back( "stack" );
        words.push_back( "storage" );
        std::cout << "After 2 pushes - capacity: " << words.capacity() << " (stack)\n";

        // Force heap transition
        words.push_back( "more" );
        words.push_back( "data" );
        std::cout << "After 4 pushes - capacity: " << words.capacity() << " (heap)\n";

        // Clear doesn't revert to stack
        words.clear();
        std::cout << "After clear() - capacity: " << words.capacity() << " (still heap)\n";
        std::cout << "Note: Once on heap, stays on heap (design choice for performance)\n";

        std::cout << "\n";
    }

    return 0;
}
