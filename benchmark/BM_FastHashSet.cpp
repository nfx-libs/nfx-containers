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
 * @file BM_FastHashSet.cpp
 * @brief FastHashSet benchmarks vs std::unordered_set
 * @details Objective comparison: FastHashSet vs std::unordered_set
 */

#include <benchmark/benchmark.h>

#include <algorithm>
#include <string>
#include <string_view>
#include <unordered_set>
#include <random>
#include <vector>

#include <nfx/containers/FastHashSet.h>

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
    static const auto g_keys_100000 = generateStringKeys( 100000 );

    //=====================================================================
    // Complex structure with custom hasher
    //=====================================================================

    struct ScopePath
    {
        std::string path;
        std::string nodeType;
        int line;
        int column;
        std::vector<std::string> attributes;

        ScopePath() = default;

        ScopePath( std::string p, std::string type, int l, int c )
            : path{ std::move( p ) },
              nodeType{ std::move( type ) },
              line{ l },
              column{ c }
        {
        }

        bool operator==( const ScopePath& other ) const
        {
            return path == other.path;
        }
    };

    // Custom hasher for scope paths
    struct ScopePathHasher
    {
        using is_transparent = void;

        static constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        static constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;

        uint64_t operator()( const ScopePath& sp ) const noexcept
        {
            uint64_t hash = FNV_OFFSET;
            for ( char c : sp.path )
            {
                hash ^= static_cast<uint64_t>( c );
                hash *= FNV_PRIME;
            }
            return hash;
        }

        uint64_t operator()( const std::string& scopePath ) const noexcept
        {
            uint64_t hash = FNV_OFFSET;
            for ( char c : scopePath )
            {
                hash ^= static_cast<uint64_t>( c );
                hash *= FNV_PRIME;
            }
            return hash;
        }

        uint64_t operator()( std::string_view scopePath ) const noexcept
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

    struct ScopePathEqual
    {
        using is_transparent = void;

        bool operator()( const ScopePath& lhs, const ScopePath& rhs ) const noexcept
        {
            return lhs.path == rhs.path;
        }

        bool operator()( const ScopePath& lhs, const std::string& rhs ) const noexcept
        {
            return lhs.path == rhs;
        }

        bool operator()( const ScopePath& lhs, std::string_view rhs ) const noexcept
        {
            return lhs.path == rhs;
        }

        bool operator()( const std::string& lhs, const ScopePath& rhs ) const noexcept
        {
            return lhs == rhs.path;
        }

        bool operator()( std::string_view lhs, const ScopePath& rhs ) const noexcept
        {
            return lhs == rhs.path;
        }

        bool operator()( std::string_view lhs, std::string_view rhs ) const noexcept
        {
            return lhs == rhs;
        }
    };

    static std::vector<ScopePath> generateScopePathData( size_t count )
    {
        std::vector<ScopePath> data;
        data.reserve( count );

        std::mt19937 gen( 42 );
        std::uniform_int_distribution<> lineDist( 1, 10000 );
        std::uniform_int_distribution<> colDist( 1, 120 );

        const std::vector<std::string> namespaces = { "std", "nfx", "internal", "detail", "impl" };
        const std::vector<std::string> classes = { "FastHashSet", "Vector", "String", "Container", "Iterator" };
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

            ScopePath sp( scopePath, types[typeDist( gen )], lineDist( gen ), colDist( gen ) );
            data.push_back( std::move( sp ) );
        }

        return data;
    }

    static const auto g_scope_path_data_1000 = generateScopePathData( 1000 );

    //=====================================================================
    // Construction cost
    //=====================================================================

    static void BM_FastHashSet_Construction_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            nfx::containers::FastHashSet<std::string> set;
            set.reserve( 1000 );
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
        }
    }

    static void BM_std_unordered_set_Construction_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_set<std::string> set;
            set.reserve( 1000 );
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.emplace( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
        }
    }

    //=====================================================================
    // Basic Lookup performance
    //=====================================================================

    static void BM_FastHashSet_Lookup_1000( ::benchmark::State& state )
    {
        nfx::containers::FastHashSet<std::string> set;
        set.reserve( 1000 );
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( const auto* key = set.find( g_keys_1000[i] ) )
                {
                    sum += static_cast<int>( key->length() );
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_set_Lookup_1000( ::benchmark::State& state )
    {
        std::unordered_set<std::string> set;
        set.reserve( 1000 );
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = set.find( g_keys_1000[i] );
                if ( it != set.end() )
                {
                    sum += static_cast<int>( it->length() );
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Heterogeneous lookup (string_view)
    //=====================================================================

    static void BM_FastHashSet_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        nfx::containers::FastHashSet<std::string> set;
        set.reserve( 1000 );
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
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
                if ( const auto* key = set.find( svKeys[i] ) )
                {
                    sum += static_cast<int>( key->length() );
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_set_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        std::unordered_set<std::string> set;
        set.reserve( 1000 );
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
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
                auto it = set.find( std::string( svKeys[i] ) );
                if ( it != set.end() )
                {
                    sum += static_cast<int>( it->length() );
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Iteration performance
    //=====================================================================

    static void BM_FastHashSet_Iteration_1000( ::benchmark::State& state )
    {
        nfx::containers::FastHashSet<std::string> set;
        set.reserve( 1000 );
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& key : set )
            {
                sum += static_cast<int>( key.size() );
                ::benchmark::DoNotOptimize( key.size() );
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_set_Iteration_1000( ::benchmark::State& state )
    {
        std::unordered_set<std::string> set;
        set.reserve( 1000 );
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& key : set )
            {
                sum += static_cast<int>( key.size() );
                ::benchmark::DoNotOptimize( key.size() );
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Insertion performance (incremental)
    //=====================================================================

    static void BM_FastHashSet_IncrementalInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            nfx::containers::FastHashSet<std::string> set;
            state.ResumeTiming();

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
            state.PauseTiming();
            std::unordered_set<std::string> set;
            state.ResumeTiming();

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

    static void BM_FastHashSet_MixedOps_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            nfx::containers::FastHashSet<std::string> set;

            // Insert first 500
            for ( size_t i = 0; i < 500; ++i )
            {
                set.insert( g_keys_1000[i] );
            }

            // Lookup and insert interleaved
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( i < 500 )
                {
                    if ( const auto* key = set.find( g_keys_1000[i] ) )
                    {
                        sum += static_cast<int>( key->length() );
                    }
                }
                else
                {
                    set.insert( g_keys_1000[i] );
                }
            }

            ::benchmark::DoNotOptimize( sum );
            ::benchmark::DoNotOptimize( set );
        }
    }

    static void BM_std_unordered_set_MixedOps_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            std::unordered_set<std::string> set;

            for ( size_t i = 0; i < 500; ++i )
            {
                set.insert( g_keys_1000[i] );
            }

            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( i < 500 )
                {
                    auto it = set.find( g_keys_1000[i] );
                    if ( it != set.end() )
                    {
                        sum += static_cast<int>( it->length() );
                    }
                }
                else
                {
                    set.insert( g_keys_1000[i] );
                }
            }

            ::benchmark::DoNotOptimize( sum );
            ::benchmark::DoNotOptimize( set );
        }
    }

    //=====================================================================
    // Erase performance
    //=====================================================================

    static void BM_FastHashSet_Erase_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            nfx::containers::FastHashSet<std::string> set;
            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }
            state.ResumeTiming();

            for ( size_t i = 0; i < 1000; ++i )
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

            for ( size_t i = 0; i < 1000; ++i )
            {
                set.erase( g_keys_1000[i] );
            }

            ::benchmark::DoNotOptimize( set );
        }
    }

    //=====================================================================
    // Large dataset
    //=====================================================================

    static void BM_FastHashSet_Lookup_10000( ::benchmark::State& state )
    {
        nfx::containers::FastHashSet<std::string> set;
        set.reserve( 10000 );
        for ( size_t i = 0; i < 10000; ++i )
        {
            set.insert( g_keys_10000[i] );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( const auto* key = set.find( g_keys_10000[i * 10] ) )
                {
                    sum += static_cast<int>( key->length() );
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_set_Lookup_10000( ::benchmark::State& state )
    {
        std::unordered_set<std::string> set;
        set.reserve( 10000 );
        for ( size_t i = 0; i < 10000; ++i )
        {
            set.insert( g_keys_10000[i] );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = set.find( g_keys_10000[i * 10] );
                if ( it != set.end() )
                {
                    sum += static_cast<int>( it->length() );
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Large dataset (100K elements)
    //=====================================================================

    static void BM_FastHashSet_Lookup_100000( ::benchmark::State& state )
    {
        nfx::containers::FastHashSet<std::string> set;
        set.reserve( 100000 );
        for ( size_t i = 0; i < 100000; ++i )
        {
            set.insert( g_keys_100000[i] );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 10000; ++i )
            {
                if ( const auto* key = set.find( g_keys_100000[i * 10] ) )
                {
                    sum += static_cast<int>( key->length() );
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_set_Lookup_100000( ::benchmark::State& state )
    {
        std::unordered_set<std::string> set;
        set.reserve( 100000 );
        for ( size_t i = 0; i < 100000; ++i )
        {
            set.insert( g_keys_100000[i] );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 10000; ++i )
            {
                auto it = set.find( g_keys_100000[i * 10] );
                if ( it != set.end() )
                {
                    sum += static_cast<int>( it->length() );
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Complex structure with custom hasher
    //=====================================================================

    static void BM_FastHashSet_ComplexStruct_1000( ::benchmark::State& state )
    {
        nfx::containers::FastHashSet<ScopePath, uint64_t, 14695981039346656037ULL, ScopePathHasher, ScopePathEqual> set;
        set.reserve( 1000 );
        for ( const auto& sp : g_scope_path_data_1000 )
        {
            set.insert( sp );
        }

        std::vector<std::string_view> lookupKeys;
        lookupKeys.reserve( 1000 );
        for ( const auto& sp : g_scope_path_data_1000 )
        {
            lookupKeys.push_back( sp.path );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                if ( const auto* sp = set.find( lookupKeys[i] ) )
                {
                    sum += sp->line + sp->column;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_set_ComplexStruct_1000( ::benchmark::State& state )
    {
        std::unordered_set<ScopePath, ScopePathHasher, ScopePathEqual> set;
        set.reserve( 1000 );
        for ( const auto& sp : g_scope_path_data_1000 )
        {
            set.emplace( sp );
        }

        std::vector<std::string_view> lookupKeys;
        lookupKeys.reserve( 1000 );
        for ( const auto& sp : g_scope_path_data_1000 )
        {
            lookupKeys.push_back( sp.path );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( size_t i = 0; i < 1000; ++i )
            {
                auto it = set.find( lookupKeys[i] );
                if ( it != set.end() )
                {
                    sum += it->line + it->column;
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
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_Construction_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Construction_1000 )->Repetitions( 3 );

// Lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_Lookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Lookup_1000 )->Repetitions( 3 );

// Heterogeneous lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_HeterogeneousLookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_HeterogeneousLookup_1000 )->Repetitions( 3 );

// Iteration benchmarks
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_Iteration_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Iteration_1000 )->Repetitions( 3 );

// Incremental insert benchmarks
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_IncrementalInsert_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_IncrementalInsert_1000 )->Repetitions( 3 );

// Mixed operations benchmarks
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_MixedOps_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_MixedOps_1000 )->Repetitions( 3 );

// Erase benchmarks
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_Erase_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Erase_1000 )->Repetitions( 3 );

// Large dataset benchmarks
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_Lookup_10000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Lookup_10000 )->Repetitions( 3 );

// Very large dataset benchmarks (100K)
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_Lookup_100000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Lookup_100000 )->Repetitions( 3 );

// Complex structure benchmarks
BENCHMARK( nfx::containers::benchmark::BM_FastHashSet_ComplexStruct_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_ComplexStruct_1000 )->Repetitions( 3 );

BENCHMARK_MAIN();
