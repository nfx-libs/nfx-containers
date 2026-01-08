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
 * @file BM_TransparentHashSet.cpp
 * @brief TransparentHashSet benchmarks vs std::unordered_set
 * @details Objective comparison: TransparentHashSet vs std::unordered_set
 */

#include <benchmark/benchmark.h>

#include <nfx/containers/TransparentHashSet.h>

#include <algorithm>
#include <random>
#include <string>
#include <unordered_set>
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

        bool operator==( const ASTNode& other ) const
        {
            return scopePath == other.scopePath &&
                   nodeType == other.nodeType &&
                   line == other.line &&
                   column == other.column;
        }
    };

    struct ASTNodeHasher
    {
        using is_transparent = void;

        static constexpr uint64_t FNV_PRIME = 1099511628211ULL;
        static constexpr uint64_t FNV_OFFSET = 14695981039346656037ULL;

        size_t operator()( const ASTNode& node ) const noexcept
        {
            uint64_t hash = FNV_OFFSET;
            for ( char c : node.scopePath )
            {
                hash ^= static_cast<uint64_t>( c );
                hash *= FNV_PRIME;
            }
            hash ^= static_cast<uint64_t>( node.line );
            hash *= FNV_PRIME;
            hash ^= static_cast<uint64_t>( node.column );
            hash *= FNV_PRIME;
            return static_cast<size_t>( hash );
        }
    };

    //=====================================================================
    // Construction cost
    //=====================================================================

    static void BM_TransparentHashSet_Construction_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            nfx::containers::TransparentHashSet<std::string> set;
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
                set.insert( g_keys_1000[i] );
            }
            ::benchmark::DoNotOptimize( set );
        }
    }

    //=====================================================================
    // Basic Lookup performance
    //=====================================================================

    static void BM_TransparentHashSet_Lookup_1000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashSet<std::string> set;
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
                    sum++;
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
                    sum++;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Heterogeneous lookup (string_view)
    //=====================================================================

    static void BM_TransparentHashSet_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashSet<std::string> set;
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
                auto it = set.find( svKeys[i] );
                if ( it != set.end() )
                {
                    sum++;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_set_HeterogeneousLookup_1000( ::benchmark::State& state )
    {
        std::unordered_set<std::string, nfx::hashing::Hasher<>, std::equal_to<>> set;
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
                auto it = set.find( svKeys[i] );
                if ( it != set.end() )
                {
                    sum++;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Iteration performance
    //=====================================================================

    static void BM_TransparentHashSet_Iteration_1000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashSet<std::string> set;
        set.reserve( 1000 );
        for ( size_t i = 0; i < 1000; ++i )
        {
            set.insert( g_keys_1000[i] );
        }

        for ( auto _ : state )
        {
            size_t sum = 0;
            for ( const auto& key : set )
            {
                sum += key.size();
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
            size_t sum = 0;
            for ( const auto& key : set )
            {
                sum += key.size();
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    //=====================================================================
    // Insertion performance (incremental)
    //=====================================================================

    static void BM_TransparentHashSet_IncrementalInsert_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            nfx::containers::TransparentHashSet<std::string> set;
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

    static void BM_TransparentHashSet_MixedOps_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            nfx::containers::TransparentHashSet<std::string> set;

            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }

            int sum = 0;
            for ( size_t i = 0; i < 500; ++i )
            {
                if ( set.find( g_keys_1000[i] ) != set.end() )
                {
                    sum++;
                }
            }

            for ( size_t i = 0; i < 200; ++i )
            {
                set.erase( g_keys_1000[i] );
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

            for ( size_t i = 0; i < 1000; ++i )
            {
                set.insert( g_keys_1000[i] );
            }

            int sum = 0;
            for ( size_t i = 0; i < 500; ++i )
            {
                if ( set.find( g_keys_1000[i] ) != set.end() )
                {
                    sum++;
                }
            }

            for ( size_t i = 0; i < 200; ++i )
            {
                set.erase( g_keys_1000[i] );
            }

            ::benchmark::DoNotOptimize( sum );
            ::benchmark::DoNotOptimize( set );
        }
    }

    //=====================================================================
    // Erase performance
    //=====================================================================

    static void BM_TransparentHashSet_Erase_1000( ::benchmark::State& state )
    {
        for ( auto _ : state )
        {
            state.PauseTiming();
            nfx::containers::TransparentHashSet<std::string> set;
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

    static void BM_TransparentHashSet_Lookup_10000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashSet<std::string> set;
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
                    sum++;
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
                    sum++;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static std::vector<ASTNode> generateASTNodes( size_t count )
    {
        std::vector<ASTNode> nodes;
        nodes.reserve( count );

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

            nodes.emplace_back( scopePath, types[typeDist( gen )], lineDist( gen ), colDist( gen ) );
        }

        return nodes;
    }

    static const auto g_ast_nodes_1000 = generateASTNodes( 1000 );

    static void BM_TransparentHashSet_ComplexStruct_1000( ::benchmark::State& state )
    {
        nfx::containers::TransparentHashSet<ASTNode, ASTNodeHasher> set;
        set.reserve( 1000 );
        for ( const auto& node : g_ast_nodes_1000 )
        {
            set.insert( node );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& node : g_ast_nodes_1000 )
            {
                auto it = set.find( node );
                if ( it != set.end() )
                {
                    sum += it->line + it->column;
                }
            }
            ::benchmark::DoNotOptimize( sum );
        }
    }

    static void BM_std_unordered_set_ComplexStruct_1000( ::benchmark::State& state )
    {
        std::unordered_set<ASTNode, ASTNodeHasher> set;
        set.reserve( 1000 );
        for ( const auto& node : g_ast_nodes_1000 )
        {
            set.insert( node );
        }

        for ( auto _ : state )
        {
            int sum = 0;
            for ( const auto& node : g_ast_nodes_1000 )
            {
                auto it = set.find( node );
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
// Benchmark registrations
//=====================================================================

// Construction benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashSet_Construction_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Construction_1000 )->Repetitions( 3 );

// Lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashSet_Lookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Lookup_1000 )->Repetitions( 3 );

// Heterogeneous lookup benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashSet_HeterogeneousLookup_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_HeterogeneousLookup_1000 )->Repetitions( 3 );

// Iteration benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashSet_Iteration_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Iteration_1000 )->Repetitions( 3 );

// Incremental insert benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashSet_IncrementalInsert_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_IncrementalInsert_1000 )->Repetitions( 3 );

// Mixed operations benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashSet_MixedOps_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_MixedOps_1000 )->Repetitions( 3 );

// Erase benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashSet_Erase_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Erase_1000 )->Repetitions( 3 );

// Large dataset benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashSet_Lookup_10000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_Lookup_10000 )->Repetitions( 3 );

// Complex structure benchmarks
BENCHMARK( nfx::containers::benchmark::BM_TransparentHashSet_ComplexStruct_1000 )->Repetitions( 3 );
BENCHMARK( nfx::containers::benchmark::BM_std_unordered_set_ComplexStruct_1000 )->Repetitions( 3 );

BENCHMARK_MAIN();
