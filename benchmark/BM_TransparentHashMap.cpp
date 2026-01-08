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
 * @file BM_TransparentHashMap.cpp
 * @brief TransparentHashMap benchmarks vs std::unordered_map
 * @details Objective comparison: TransparentHashMap vs std::unordered_map
 */

#include <benchmark/benchmark.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_map>
#include <random>
#include <vector>

#include <nfx/containers/TransparentHashMap.h>

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
            const size_t len = static_cast<size_t>( lengthDist( gen ) );
            std::string key;
            key.reserve( len );
            for ( size_t j = 0; j < len; ++j )
            {
                key.push_back( static_cast<char>( charDist( gen ) ) );
            }
            keys.push_back( std::move( key ) );
        }

        return keys;
    }

    static const auto g_keys_100 = generateStringKeys( 100 );
    static const auto g_keys_1000 = generateStringKeys( 1000 );
    static const auto g_keys_10000 = generateStringKeys( 10000 );

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
            return static_cast<size_t>( hash );
        }

        size_t operator()( std::string_view scopePath ) const noexcept
        {
            uint64_t hash = FNV_OFFSET;
            for ( char c : scopePath )
            {
                hash ^= static_cast<uint64_t>( c );
                hash *= FNV_PRIME;
            }
            return static_cast<size_t>( hash );
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
                                    methods[methodDist( gen )] + "_" + std::to_string( i );

            ASTNode node( scopePath, types[typeDist( gen )], lineDist( gen ), colDist( gen ) );
            data.emplace_back( scopePath, std::move( node ) );
        }

        return data;
    }

    static const auto g_ast_data_1000 = generateASTData( 1000 );

    //=====================================================================
    // Construction cost
    //=====================================================================

    static void BM_TransparentHashMap_Construction_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            nfx::containers::TransparentHashMap<std::string, int> map;
            map.reserve( 1000 );
            for ( size_t i = 0; i < 1000; ++i )
            {
                map.emplace( g_keys_1000[i], static_cast<int>( i ) );
            }
            ::benchmark::DoNotOptimize( map );
        }
    }

    static void BM_std_unordered_map_Construction_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_map<std::string, int> map;
            map.reserve( 1000 );
            for ( size_t i = 0; i < 1000; ++i )
            {
                map.emplace( g_keys_1000[i], static_cast<int>( i ) );
            }
            ::benchmark::DoNotOptimize( map );
        }
    }

    //=====================================================================
    // Basic Lookup performance
    //=====================================================================

    static void BM_TransparentHashMap_Lookup_1000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashMap<std::string, int> map;
        map.reserve( 1000 );
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

    static void BM_std_unordered_map_Lookup_1000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, int> map;
        map.reserve( 1000 );
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

    static void BM_TransparentHashMap_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashMap<std::string, int> map;
        map.reserve( 1000 );
        for ( size_t i = 0; i < 1000; ++i )
        {
            map[g_keys_1000[i]] = static_cast<int>( i );
        }

        std::vector<std::string_view> svKeys;
        svKeys.reserve( 1000 );
        for ( const auto& key : g_keys_1000 )
        {
            svKeys.push_back( key );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = map.find( svKeys[i] );
                if ( it != map.end() )
                {
                    sum += it->second;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, int> map;
        map.reserve( 1000 );
        for ( size_t i = 0; i < 1000; ++i )
        {
            map[g_keys_1000[i]] = static_cast<int>( i );
        }

        std::vector<std::string_view> svKeys;
        svKeys.reserve( 1000 );
        for ( const auto& key : g_keys_1000 )
        {
            svKeys.push_back( key );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = map.find( std::string( svKeys[i] ) );
                if ( it != map.end() )
                {
                    sum += it->second;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Iteration performance
    //=====================================================================

    static void BM_TransparentHashMap_Iteration_1000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashMap<std::string, int> map;
        map.reserve( 1000 );
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
                ::benchmark::DoNotOptimize( key.size() );
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_Iteration_1000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, int> map;
        map.reserve( 1000 );
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
                ::benchmark::DoNotOptimize( key.size() );
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Insertion performance (incremental)
    //=====================================================================

    static void BM_TransparentHashMap_IncrementalInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            nfx::containers::TransparentHashMap<std::string, int> map;
            state.ResumeTiming();

            for ( size_t i = 0; i < 1000; ++i )
            {
                map[g_keys_1000[i]] = static_cast<int>( i );
            }

            ::benchmark::DoNotOptimize( map );
        }
    }

    static void BM_std_unordered_map_IncrementalInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            std::unordered_map<std::string, int> map;
            state.ResumeTiming();

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

    static void BM_TransparentHashMap_MixedOps_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            nfx::containers::TransparentHashMap<std::string, int> map;

            // Insert first 500
            for ( size_t i = 0; i < 500; ++i )
            {
                map[g_keys_1000[i]] = static_cast<int>( i );
            }

            // Lookup and insert interleaved
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

    static void BM_std_unordered_map_MixedOps_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_map<std::string, int> map;

            for ( size_t i = 0; i < 500; ++i )
            {
                map[g_keys_1000[i]] = static_cast<int>( i );
            }

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

    static void BM_TransparentHashMap_Erase_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            nfx::containers::TransparentHashMap<std::string, int> map;
            for ( size_t i = 0; i < 1000; ++i )
            {
                map[g_keys_1000[i]] = static_cast<int>( i );
            }
            state.ResumeTiming();

            for ( size_t i = 0; i < 1000; ++i )
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

            for ( size_t i = 0; i < 1000; ++i )
            {
                map.erase( g_keys_1000[i] );
            }

            ::benchmark::DoNotOptimize( map );
        }
    }

    //=====================================================================
    // Large dataset
    //=====================================================================

    static void BM_TransparentHashMap_Lookup_10000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashMap<std::string, int> map;
        map.reserve( 10000 );
        for ( size_t i = 0; i < 10000; ++i )
        {
            map[g_keys_10000[i]] = static_cast<int>( i );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = map.find( g_keys_10000[i * 10] );
                if ( it != map.end() )
                {
                    sum += it->second;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_Lookup_10000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, int> map;
        map.reserve( 10000 );
        for ( size_t i = 0; i < 10000; ++i )
        {
            map[g_keys_10000[i]] = static_cast<int>( i );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = map.find( g_keys_10000[i * 10] );
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

    static void BM_TransparentHashMap_ComplexStruct_1000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashMap<std::string, ASTNode, ScopePathHasher, std::equal_to<>> map;
        map.reserve( 1000 );
        for ( const auto& [key, value] : g_ast_data_1000 )
        {
            map.emplace( key, value );
        }

        std::vector<std::string_view> lookupKeys;
        lookupKeys.reserve( 1000 );
        for ( const auto& [key, _] : g_ast_data_1000 )
        {
            lookupKeys.push_back( key );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = map.find( lookupKeys[i] );
                if ( it != map.end() )
                {
                    sum += it->second.line + it->second.column;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_map_ComplexStruct_1000( ::benchmark::State& state )
    {
        std::unordered_map<std::string, ASTNode, ScopePathHasher, std::equal_to<>> map;
        map.reserve( 1000 );
        for ( const auto& [key, value] : g_ast_data_1000 )
        {
            map.emplace( key, value );
        }

        std::vector<std::string_view> lookupKeys;
        lookupKeys.reserve( 1000 );
        for ( const auto& [key, _] : g_ast_data_1000 )
        {
            lookupKeys.push_back( key );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = map.find( lookupKeys[i] );
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
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashMap_Construction_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Construction_1000 )->Repetitions( 3 );

// Lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashMap_Lookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Lookup_1000 )->Repetitions( 3 );

// Heterogeneous lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashMap_HeterogeneousLookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_HeterogeneousLookup_1000 )->Repetitions( 3 );

// Iteration benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashMap_Iteration_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Iteration_1000 )->Repetitions( 3 );

// Incremental insert benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashMap_IncrementalInsert_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_IncrementalInsert_1000 )->Repetitions( 3 );

// Mixed operations benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashMap_MixedOps_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_MixedOps_1000 )->Repetitions( 3 );

// Erase benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashMap_Erase_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Erase_1000 )->Repetitions( 3 );

// Large dataset benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashMap_Lookup_10000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_Lookup_10000 )->Repetitions( 3 );

// Complex structure benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashMap_ComplexStruct_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_map_ComplexStruct_1000 )->Repetitions( 3 );

BENCHMARK_MAIN();
