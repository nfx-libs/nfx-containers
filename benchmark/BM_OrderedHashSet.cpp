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
 * @file BM_OrderedHashSet.cpp
 * @brief OrderedHashSet benchmarks vs std::unordered_set
 * @details Objective comparison: OrderedHashSet (insertion-order) vs std::unordered_set (no order)
 */

#include <benchmark/benchmark.h>

#include <nfx/Containers.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_set>
#include <random>
#include <vector>

namespace nfx::containers::benchmark
{
    //=====================================================================
    // Test data generation
    //=====================================================================

    static std::vector<std::string> generateStringKeys( size_t count, size_t minLen = 8, size_t maxLen = 32 )
    {
        std::vector<std::string> keys;
        keys.reserve( count );

        std::mt19937 gen( 42 );
        std::uniform_int_distribution<> lengthDist( static_cast<int>( minLen ), static_cast<int>( maxLen ) );
        std::uniform_int_distribution<> charDist( 'a', 'z' );

        for ( size_t i = 0; i < count; ++i )
        {
            size_t len = static_cast<size_t>( lengthDist( gen ) );
            std::string key;
            key.reserve( len );
            for ( size_t j = 0; j < len; ++j )
            {
                key += static_cast<char>( charDist( gen ) );
            }
            keys.push_back( std::move( key ) );
        }

        return keys;
    }

    static const auto g_keys_100 = generateStringKeys( 100 );
    static const auto g_keys_1000 = generateStringKeys( 1000 );
    static const auto g_keys_10000 = generateStringKeys( 10000 );
    static const auto g_keys_100000 = generateStringKeys( 100000 );

    //=====================================================================
    // Integer keys generation
    //=====================================================================

    static std::vector<int> generateIntegerKeys( size_t count )
    {
        std::vector<int> keys;
        keys.reserve( count );

        std::mt19937 gen( 42 );
        std::uniform_int_distribution<int> dist( 0, static_cast<int>( count * 10 ) );

        for ( size_t i = 0; i < count; ++i )
        {
            keys.push_back( dist( gen ) );
        }

        return keys;
    }

    static const auto g_int_keys_1000 = generateIntegerKeys( 1000 );
    static const auto g_int_keys_10000 = generateIntegerKeys( 10000 );

    //=====================================================================
    // Construction cost
    //=====================================================================

    static void BM_OrderedHashSet_Construction_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            OrderedHashSet<std::string> set;
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
            ::benchmark::ClobberMemory();
        }
    }

    static void BM_std_unordered_set_Construction_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_set<std::string> set;
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
            ::benchmark::ClobberMemory();
        }
    }

    //=====================================================================
    // Basic Lookup performance
    //=====================================================================

    static void BM_OrderedHashSet_Lookup_1000( ::benchmark::State& state )
    {
        OrderedHashSet<std::string> set;
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( set.contains( g_keys_1000[i] ) )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    static void BM_std_unordered_set_Lookup_1000( ::benchmark::State& state )
    {
        std::unordered_set<std::string> set;
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( set.find( g_keys_1000[i] ) != set.end() )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    //=====================================================================
    // Heterogeneous lookup (string_view)
    //=====================================================================

    static void BM_OrderedHashSet_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        OrderedHashSet<std::string> set;
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        std::vector<std::string_view> svKeys;
        svKeys.reserve( 1000 );
        for ( const auto& key : g_keys_1000 )
        {
            svKeys.emplace_back( key );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( const auto& key : svKeys )
            {
                if ( set.contains( key ) )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    static void BM_std_unordered_set_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        std::unordered_set<std::string, nfx::hashing::Hasher<>, std::equal_to<>> set;
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        std::vector<std::string_view> svKeys;
        svKeys.reserve( 1000 );
        for ( const auto& key : g_keys_1000 )
        {
            svKeys.emplace_back( key );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( const auto& key : svKeys )
            {
                if ( set.find( key ) != set.end() )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    //=====================================================================
    // Iteration performance (OrderedHashSet's key feature!)
    //=====================================================================

    static void BM_OrderedHashSet_Iteration_1000( ::benchmark::State& state )
    {
        OrderedHashSet<std::string> set;
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            size_t count = 0;
            for ( const auto& key : set )
            {
                ++count;
                ::benchmark::DoNotOptimize( key.data() );
            }
            ::benchmark::DoNotOptimize( count );
        }
    }

    static void BM_std_unordered_set_Iteration_1000( ::benchmark::State& state )
    {
        std::unordered_set<std::string> set;
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            size_t count = 0;
            for ( const auto& key : set )
            {
                ++count;
                ::benchmark::DoNotOptimize( key.data() );
            }
            ::benchmark::DoNotOptimize( count );
        }
    }

    //=====================================================================
    // Backward iteration (OrderedHashSet unique feature!)
    //=====================================================================

    static void BM_OrderedHashSet_BackwardIteration_1000( ::benchmark::State& state )
    {
        OrderedHashSet<std::string> set;
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            size_t count = 0;
            for ( auto it = --set.end(); it != set.begin(); --it )
            {
                ++count;
                ::benchmark::DoNotOptimize( it->data() );
            }
            ++count;
            ::benchmark::DoNotOptimize( count );
        }
    }

    //=====================================================================
    // Insertion performance (incremental)
    //=====================================================================

    static void BM_OrderedHashSet_IncrementalInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            OrderedHashSet<std::string> set;
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
        }
    }

    static void BM_std_unordered_set_IncrementalInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_set<std::string> set;
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
        }
    }

    //=====================================================================
    // Mixed Operations (insert + lookup)
    //=====================================================================

    static void BM_OrderedHashSet_MixedOps_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            OrderedHashSet<std::string> set;

            // Insert half
            for ( size_t i = 0; i < 500; ++i )
            {
                set.insert( g_keys_1000[i] );
            }

            // Mixed operations
            size_t found = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( i < 500 )
                {
                    if ( set.contains( g_keys_1000[i] ) )
                    {
                        ++found;
                    }
                }
                else
                {
                    set.insert( g_keys_1000[i] );
                }
            }
            ::benchmark::DoNotOptimize( found );
            ::benchmark::DoNotOptimize( set );
        }
    }

    static void BM_std_unordered_set_MixedOps_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_set<std::string> set;

            // Insert half
            for ( size_t i = 0; i < 500; ++i )
            {
                set.insert( g_keys_1000[i] );
            }

            // Mixed operations
            size_t found = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( i < 500 )
                {
                    if ( set.find( g_keys_1000[i] ) != set.end() )
                    {
                        ++found;
                    }
                }
                else
                {
                    set.insert( g_keys_1000[i] );
                }
            }
            ::benchmark::DoNotOptimize( found );
            ::benchmark::DoNotOptimize( set );
        }
    }

    //=====================================================================
    // Erase performance
    //=====================================================================

    static void BM_OrderedHashSet_Erase_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            OrderedHashSet<std::string> set;
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            state.ResumeTiming();

            for ( size_t i = 0; i < 500; ++i )
            {
                set.erase( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
        }
    }

    static void BM_std_unordered_set_Erase_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            std::unordered_set<std::string> set;
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            state.ResumeTiming();

            for ( size_t i = 0; i < 500; ++i )
            {
                set.erase( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
        }
    }

    //=====================================================================
    // Large dataset (10K)
    //=====================================================================

    static void BM_OrderedHashSet_Lookup_10000( ::benchmark::State& state )
    {
        OrderedHashSet<std::string> set;
        for ( size_t i = 0; i < 10000; ++i )
        {
            set.insert( g_keys_10000[i] );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( size_t i = 0; i < 10000; ++i )
            {
                if ( set.contains( g_keys_10000[i] ) )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    static void BM_std_unordered_set_Lookup_10000( ::benchmark::State& state )
    {
        std::unordered_set<std::string> set;
        for ( size_t i = 0; i < 10000; ++i )
        {
            set.insert( g_keys_10000[i] );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( size_t i = 0; i < 10000; ++i )
            {
                if ( set.find( g_keys_10000[i] ) != set.end() )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    //=====================================================================
    // Large dataset (100K elements)
    //=====================================================================

    static void BM_OrderedHashSet_Lookup_100000( ::benchmark::State& state )
    {
        OrderedHashSet<std::string> set;
        for ( size_t i = 0; i < 100000; ++i )
        {
            set.insert( g_keys_100000[i] );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( size_t i = 0; i < 100000; i += 100 )
            {
                if ( set.contains( g_keys_100000[i] ) )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    static void BM_std_unordered_set_Lookup_100000( ::benchmark::State& state )
    {
        std::unordered_set<std::string> set;
        for ( size_t i = 0; i < 100000; ++i )
        {
            set.insert( g_keys_100000[i] );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( size_t i = 0; i < 100000; i += 100 )
            {
                if ( set.find( g_keys_100000[i] ) != set.end() )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    //=====================================================================
    // Integer keys
    //=====================================================================

    static void BM_OrderedHashSet_IntegerKeys_1000( ::benchmark::State& state )
    {
        OrderedHashSet<int, uint64_t> set;
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_int_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( set.contains( g_int_keys_1000[i] ) )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    static void BM_std_unordered_set_IntegerKeys_1000( ::benchmark::State& state )
    {
        std::unordered_set<int> set;
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_int_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            size_t found = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( set.find( g_int_keys_1000[i] ) != set.end() )
                {
                    ++found;
                }
            }
            ::benchmark::DoNotOptimize( found );
        }
    }

    //=====================================================================
    // Duplicate insertion (no-op for sets)
    //=====================================================================

    static void BM_OrderedHashSet_DuplicateInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            OrderedHashSet<std::string> set;
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            state.ResumeTiming();

            // Try to insert all again (should be no-ops)
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
        }
    }

    static void BM_std_unordered_set_DuplicateInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            std::unordered_set<std::string> set;
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            state.ResumeTiming();

            // Try to insert all again (should be no-ops)
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
        }
    }
} // namespace nfx::containers::benchmark

//=====================================================================
// Benchmark Registration
//=====================================================================

// Construction benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_Construction_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Construction_1000 )->Repetitions( 3 );

// Lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_Lookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Lookup_1000 )->Repetitions( 3 );

// Heterogeneous lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_HeterogeneousLookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_HeterogeneousLookup_1000 )->Repetitions( 3 );

// Iteration benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_Iteration_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Iteration_1000 )->Repetitions( 3 );

// Backward iteration (OrderedHashSet unique feature!)
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_BackwardIteration_1000 )->Repetitions( 3 );

// Incremental insert benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_IncrementalInsert_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_IncrementalInsert_1000 )->Repetitions( 3 );

// Mixed operations benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_MixedOps_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_MixedOps_1000 )->Repetitions( 3 );

// Erase benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_Erase_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Erase_1000 )->Repetitions( 3 );

// Large dataset benchmarks (10K)
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_Lookup_10000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Lookup_10000 )->Repetitions( 3 );

// Very large dataset benchmarks (100K)
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_Lookup_100000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Lookup_100000 )->Repetitions( 3 );

// Integer key benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_IntegerKeys_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_IntegerKeys_1000 )->Repetitions( 3 );

// Duplicate insertion benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashSet_DuplicateInsert_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_DuplicateInsert_1000 )->Repetitions( 3 );

BENCHMARK_MAIN();
