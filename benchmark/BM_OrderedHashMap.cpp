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
 * @file BM_OrderedHashMap.cpp
 * @brief OrderedHashMap benchmarks vs std::unordered_map
 * @details Objective comparison: OrderedHashMap (insertion-order) vs std::unordered_map (no order)
 */

#include <benchmark/benchmark.h>

#include <nfx/Containers.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_map>
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
    // Complex structure with custom hasher
    //=====================================================================

    struct ASTNode
    {
        std::string scopePath;
        std::string nodeType;
        int line;
        int column;
        std::vector<std::string> attributes;

        ASTNode() = default;

        ASTNode( std::string path, std::string type, int l, int c )
            : scopePath{ std::move( path ) },
              nodeType{ std::move( type ) },
              line{ l },
              column{ c }
        {
        }
    };

    // Custom hasher for scope paths
    struct ScopePathHasher
    {
        using is_transparent = void;

        static constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        static constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;

        size_t operator()( const std::string& scopePath ) const noexcept
        {
            uint64_t hash = FNV_OFFSET;
            for ( char c : scopePath )
            {
                hash ^= static_cast<uint64_t>( c );
                hash *= FNV_PRIME;
            }
            return hash;
        }

        size_t operator()( std::string_view scopePath ) const noexcept
        {
            uint64_t hash = FNV_OFFSET;
            for ( char c : scopePath )
            {
                hash ^= static_cast<uint64_t>( c );
                hash *= FNV_PRIME;
            }
            return hash;
        }
    };

    static std::vector<std::pair<std::string, ASTNode>> generateASTData( size_t count )
    {
        std::vector<std::pair<std::string, ASTNode>> data;
        data.reserve( count );

        std::mt19937 gen( 42 );
        std::uniform_int_distribution<> lineDist( 1, 10000 );
        std::uniform_int_distribution<> colDist( 1, 120 );

        const std::vector<std::string> namespaces = { "std", "nfx", "internal", "detail", "impl" };
        const std::vector<std::string> classes = { "HashMap", "Vector", "String", "Container", "Iterator" };
        const std::vector<std::string> methods = { "insert", "find", "erase", "size", "begin", "end" };
        const std::vector<std::string> types = { "FunctionDecl", "VarDecl", "ClassDecl", "MethodDecl" };

        std::uniform_int_distribution<size_t> nsDist( 0, namespaces.size() - 1 );
        std::uniform_int_distribution<size_t> classDist( 0, classes.size() - 1 );
        std::uniform_int_distribution<size_t> methodDist( 0, methods.size() - 1 );
        std::uniform_int_distribution<size_t> typeDist( 0, types.size() - 1 );

        for ( size_t i = 0; i < count; ++i )
        {
            std::string scopePath = namespaces[nsDist( gen )] + "::" +
                                    classes[classDist( gen )] + "::" +
                                    methods[methodDist( gen )];
            data.emplace_back( scopePath, ASTNode{ scopePath, types[typeDist( gen )], lineDist( gen ), colDist( gen ) } );
        }

        return data;
    }

    static const auto g_ast_data_1000 = generateASTData( 1000 );

    //=====================================================================
    // Construction cost
    //=====================================================================

    static void BM_OrderedHashMap_Construction_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            OrderedHashMap<std::string, int> map;
            for ( size_t i = 0; i < 1000; ++i )
            {
                map.insertOrAssign( g_keys_1000[i], static_cast<int>( i ) );
            }
            ::benchmark::DoNotOptimize( map );
            ::benchmark::ClobberMemory();
        }
    }

    static void BM_std_unordered_map_Construction_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_map<std::string, int> map;
            for ( size_t i = 0; i < 1000; ++i )
            {
                map[g_keys_1000[i]] = static_cast<int>( i );
            }
            ::benchmark::DoNotOptimize( map );
            ::benchmark::ClobberMemory();
        }
    }

    //=====================================================================
    // Basic Lookup performance
    //=====================================================================

    static void BM_OrderedHashMap_Lookup_1000( ::benchmark::State& state )
    {
        OrderedHashMap<std::string, int> map;
        for ( size_t i = 0; i < 1000; ++i )
        {
            map.insertOrAssign( g_keys_1000[i], static_cast<int>( i ) );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( int* value = map.find( g_keys_1000[i] ) )
                {
                    sum += *value;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_Lookup_1000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, int> map;
        for ( size_t i = 0; i < 1000; ++i )
        {
            map[g_keys_1000[i]] = static_cast<int>( i );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = map.find( g_keys_1000[i] );
                if ( it != map.end() )
                {
                    sum += it->second;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Heterogeneous lookup (string_view)
    //=====================================================================

    static void BM_OrderedHashMap_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        OrderedHashMap<std::string, int> map;
        for ( size_t i = 0; i < 1000; ++i )
        {
            map.insertOrAssign( g_keys_1000[i], static_cast<int>( i ) );
        }

        std::vector<std::string_view> svKeys;
        svKeys.reserve( 1000 );
        for ( const auto& key : g_keys_1000 )
        {
            svKeys.emplace_back( key );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& key : svKeys )
            {
                if ( int* value = map.find( key ) )
                {
                    sum += *value;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, int, nfx::hashing::Hasher<>, std::equal_to<>> map;
        for ( size_t i = 0; i < 1000; ++i )
        {
            map[g_keys_1000[i]] = static_cast<int>( i );
        }

        std::vector<std::string_view> svKeys;
        svKeys.reserve( 1000 );
        for ( const auto& key : g_keys_1000 )
        {
            svKeys.emplace_back( key );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& key : svKeys )
            {
                auto it = map.find( key );
                if ( it != map.end() )
                {
                    sum += it->second;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Iteration performance (OrderedHashMap's key feature!)
    //=====================================================================

    static void BM_OrderedHashMap_Iteration_1000( ::benchmark::State& state )
    {
        OrderedHashMap<std::string, int> map;
        for ( size_t i = 0; i < 1000; ++i )
        {
            map.insertOrAssign( g_keys_1000[i], static_cast<int>( i ) );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& [key, value] : map )
            {
                sum += value;
                ::benchmark::DoNotOptimize( key.data() );
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_Iteration_1000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, int> map;
        for ( size_t i = 0; i < 1000; ++i )
        {
            map[g_keys_1000[i]] = static_cast<int>( i );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& [key, value] : map )
            {
                sum += value;
                ::benchmark::DoNotOptimize( key.data() );
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Backward iteration (OrderedHashMap unique feature!)
    //=====================================================================

    static void BM_OrderedHashMap_BackwardIteration_1000( ::benchmark::State& state )
    {
        OrderedHashMap<std::string, int> map;
        for ( size_t i = 0; i < 1000; ++i )
        {
            map.insertOrAssign( g_keys_1000[i], static_cast<int>( i ) );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( auto it = --map.end(); it != map.begin(); --it )
            {
                sum += it->second;
                ::benchmark::DoNotOptimize( it->first.data() );
            }
            sum += map.begin()->second;
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Insertion performance (incremental)
    //=====================================================================

    static void BM_OrderedHashMap_IncrementalInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            OrderedHashMap<std::string, int> map;
            for ( size_t i = 0; i < 1000; ++i )
            {
                map.insertOrAssign( g_keys_1000[i], static_cast<int>( i ) );
            }
            ::benchmark::DoNotOptimize( map );
        }
    }

    static void BM_std_unordered_map_IncrementalInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_map<std::string, int> map;
            for ( size_t i = 0; i < 1000; ++i )
            {
                map[g_keys_1000[i]] = static_cast<int>( i );
            }
            ::benchmark::DoNotOptimize( map );
        }
    }

    //=====================================================================
    // Mixed Operations (insert + lookup)
    //=====================================================================

    static void BM_OrderedHashMap_MixedOps_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            OrderedHashMap<std::string, int> map;

            // Insert half
            for ( size_t i = 0; i < 500; ++i )
            {
                map.insertOrAssign( g_keys_1000[i], static_cast<int>( i ) );
            }

            // Mixed operations
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( i < 500 )
                {
                    if ( int* value = map.find( g_keys_1000[i] ) )
                    {
                        sum += *value;
                    }
                }
                else
                {
                    map.insertOrAssign( g_keys_1000[i], static_cast<int>( i ) );
                }
            }
            ::benchmark::DoNotOptimize( sum );
            ::benchmark::DoNotOptimize( map );
        }
    }

    static void BM_std_unordered_map_MixedOps_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_map<std::string, int> map;

            // Insert half
            for ( size_t i = 0; i < 500; ++i )
            {
                map[g_keys_1000[i]] = static_cast<int>( i );
            }

            // Mixed operations
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( i < 500 )
                {
                    auto it = map.find( g_keys_1000[i] );
                    if ( it != map.end() )
                    {
                        sum += it->second;
                    }
                }
                else
                {
                    map[g_keys_1000[i]] = static_cast<int>( i );
                }
            }
            ::benchmark::DoNotOptimize( sum );
            ::benchmark::DoNotOptimize( map );
        }
    }

    //=====================================================================
    // Erase performance
    //=====================================================================

    static void BM_OrderedHashMap_Erase_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            OrderedHashMap<std::string, int> map;
            for ( size_t i = 0; i < 1000; ++i )
            {
                map.insertOrAssign( g_keys_1000[i], static_cast<int>( i ) );
            }
            state.ResumeTiming();

            for ( size_t i = 0; i < 500; ++i )
            {
                map.erase( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( map );
        }
    }

    static void BM_std_unordered_map_Erase_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            std::unordered_map<std::string, int> map;
            for ( size_t i = 0; i < 1000; ++i )
            {
                map[g_keys_1000[i]] = static_cast<int>( i );
            }
            state.ResumeTiming();

            for ( size_t i = 0; i < 500; ++i )
            {
                map.erase( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( map );
        }
    }

    //=====================================================================
    // Large dataset
    //=====================================================================

    static void BM_OrderedHashMap_Lookup_10000( ::benchmark::State& state )
    {
        OrderedHashMap<std::string, int> map;
        for ( size_t i = 0; i < 10000; ++i )
        {
            map.insertOrAssign( g_keys_10000[i], static_cast<int>( i ) );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 10000; ++i )
            {
                if ( int* value = map.find( g_keys_10000[i] ) )
                {
                    sum += *value;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_Lookup_10000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, int> map;
        for ( size_t i = 0; i < 10000; ++i )
        {
            map[g_keys_10000[i]] = static_cast<int>( i );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 10000; ++i )
            {
                auto it = map.find( g_keys_10000[i] );
                if ( it != map.end() )
                {
                    sum += it->second;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Large dataset (100K elements)
    //=====================================================================

    static void BM_OrderedHashMap_Lookup_100000( ::benchmark::State& state )
    {
        OrderedHashMap<std::string, int> map;
        for ( size_t i = 0; i < 100000; ++i )
        {
            map.insertOrAssign( g_keys_100000[i], static_cast<int>( i ) );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 100000; i += 100 )
            {
                if ( int* value = map.find( g_keys_100000[i] ) )
                {
                    sum += *value;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_Lookup_100000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, int> map;
        for ( size_t i = 0; i < 100000; ++i )
        {
            map[g_keys_100000[i]] = static_cast<int>( i );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 100000; i += 100 )
            {
                auto it = map.find( g_keys_100000[i] );
                if ( it != map.end() )
                {
                    sum += it->second;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Complex structure with custom hasher
    //=====================================================================

    static void BM_OrderedHashMap_ComplexStruct_1000( ::benchmark::State& state )
    {
        OrderedHashMap<std::string, ASTNode> map;
        for ( const auto& [key, value] : g_ast_data_1000 )
        {
            map.insertOrAssign( key, value );
        }

        std::vector<std::string_view> svKeys;
        svKeys.reserve( 1000 );
        for ( const auto& [key, _] : g_ast_data_1000 )
        {
            svKeys.emplace_back( key );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& key : svKeys )
            {
                if ( ASTNode* node = map.find( key ) )
                {
                    sum += node->line + node->column;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_ComplexStruct_1000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, ASTNode, ScopePathHasher, std::equal_to<>> map;
        for ( const auto& [key, value] : g_ast_data_1000 )
        {
            map[key] = value;
        }

        std::vector<std::string_view> svKeys;
        svKeys.reserve( 1000 );
        for ( const auto& [key, _] : g_ast_data_1000 )
        {
            svKeys.emplace_back( key );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& key : svKeys )
            {
                auto it = map.find( key );
                if ( it != map.end() )
                {
                    sum += it->second.line + it->second.column;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }
} // namespace nfx::containers::benchmark

//=====================================================================
// Benchmark Registration
//=====================================================================

// Construction benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_Construction_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Construction_1000 )->Repetitions( 3 );

// Lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_Lookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Lookup_1000 )->Repetitions( 3 );

// Heterogeneous lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_HeterogeneousLookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_HeterogeneousLookup_1000 )->Repetitions( 3 );

// Iteration benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_Iteration_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Iteration_1000 )->Repetitions( 3 );

// Backward iteration (OrderedHashMap unique feature!)
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_BackwardIteration_1000 )->Repetitions( 3 );

// Incremental insert benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_IncrementalInsert_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_IncrementalInsert_1000 )->Repetitions( 3 );

// Mixed operations benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_MixedOps_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_MixedOps_1000 )->Repetitions( 3 );

// Erase benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_Erase_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Erase_1000 )->Repetitions( 3 );

// Large dataset benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_Lookup_10000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Lookup_10000 )->Repetitions( 3 );

// Very large dataset benchmarks (100K)
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_Lookup_100000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Lookup_100000 )->Repetitions( 3 );

// Complex structure benchmarks
BENCHMARK( nfx::containers::benchmark::BM_OrderedHashMap_ComplexStruct_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_ComplexStruct_1000 )->Repetitions( 3 );

BENCHMARK_MAIN();
