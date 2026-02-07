# nfx-containers

<!-- Project Info -->

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square)](https://github.com/nfx-libs/nfx-containers/blob/main/LICENSE.txt) [![GitHub release (latest by date)](https://img.shields.io/github/v/release/nfx-libs/nfx-containers?style=flat-square)](https://github.com/nfx-libs/nfx-containers/releases) [![GitHub tag (latest by date)](https://img.shields.io/github/tag/nfx-libs/nfx-containers?style=flat-square)](https://github.com/nfx-libs/nfx-containers/tags)<br/>

![C++20](https://img.shields.io/badge/C%2B%2B-20-blue?style=flat-square) ![CMake](https://img.shields.io/badge/CMake-3.20%2B-green.svg?style=flat-square) ![Cross Platform](https://img.shields.io/badge/Platform-Linux_Windows-lightgrey?style=flat-square)

<!-- CI/CD Status -->

[![Linux GCC](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-containers/build-linux-gcc.yml?branch=main&label=Linux%20GCC&style=flat-square)](https://github.com/nfx-libs/nfx-containers/actions/workflows/build-linux-gcc.yml) [![Linux Clang](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-containers/build-linux-clang.yml?branch=main&label=Linux%20Clang&style=flat-square)](https://github.com/nfx-libs/nfx-containers/actions/workflows/build-linux-clang.yml) [![Windows MinGW](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-containers/build-windows-mingw.yml?branch=main&label=Windows%20MinGW&style=flat-square)](https://github.com/nfx-libs/nfx-containers/actions/workflows/build-windows-mingw.yml) [![Windows MSVC](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-containers/build-windows-msvc.yml?branch=main&label=Windows%20MSVC&style=flat-square)](https://github.com/nfx-libs/nfx-containers/actions/workflows/build-windows-msvc.yml) [![CodeQL](https://img.shields.io/github/actions/workflow/status/nfx-libs/nfx-containers/codeql.yml?branch=main&label=CodeQL&style=flat-square)](https://github.com/nfx-libs/nfx-containers/actions/workflows/codeql.yml)

> A modern C++20 header-only library providing high-performance hash containers (maps and sets) with Robin Hood hashing and perfect hashing (CHD algorithm)

## Overview

nfx-containers is a modern C++20 header-only library providing high-performance associative containers optimized for speed and memory efficiency.
It offers Robin Hood hash maps and sets for general use, perfect hash maps using the CHD (Compress, Hash and Displace) algorithm for static data, and specialized string containers with zero-copy heterogeneous lookup capabilities.

## Key Features

### 📦 High-Performance Hash Containers

- **Robin Hood Hashing**: Open addressing with backward shift deletion for optimal performance in maps and sets
- **Perfect Hashing (CHD)**: Compile-time perfect hash maps for static data with O(1) guaranteed lookups
- **Cache-Optimized Layout**: Contiguous memory storage for excellent cache locality
- **Heterogeneous Lookup**: Zero-copy lookups with compatible types (string_view, const char\*, integers, enums, etc.)
- **Template Support**: Generic key-value and key-only storage with customizable hash functors

### 📊 Real-World Applications

- **Configuration Management**: Fast key-value lookups for application settings
- **Symbol Tables**: Compiler and interpreter symbol resolution
- **Caching Systems**: High-performance in-memory caches with predictable performance
- **Static Data Structures**: Perfect hash maps for compile-time known datasets
- **String Indexing**: Optimized string-to-value mappings without allocation overhead

### 🎯 Configurable Hashing

- **Flexible Hash Types**: Configurable 32-bit or 64-bit hash values via template parameters
- **Custom Hash Functions**: Support for custom hash functors and seeds
- **Type-Safe**: Compile-time type checking for key types and hash functions
- **Hardware-Accelerated**: Leverages nfx-hashing library with SSE4.2 CRC32 support when available

### ✅ Container Types

- **PerfectHashMap**: Perfect hash map using CHD algorithm for immutable datasets
- **FastHashMap**: Robin Hood hash map with superior performance over `std::unordered_map`
- **FastHashSet**: Robin Hood hash set with superior performance over `std::unordered_set`
- **OrderedHashMap**: Insertion-order preserving hash map with bidirectional iterators
- **OrderedHashSet**: Insertion-order preserving hash set with bidirectional iterators
- **TransparentHashMap**: Enhanced `std::unordered_map` with heterogeneous lookup
- **TransparentHashSet**: Enhanced `std::unordered_set` with heterogeneous lookup
- **StackVector**: Small vector optimization with stack storage and automatic heap fallback

### 🌐 Heterogeneous Lookup Optimization

- **Zero-Copy Lookups**: Query with compatible types (`const char*`, `std::string_view`, integers, enums) without temporary allocations
- **Universal Hash Support**: Optimized hashing for strings, integers, floats, pairs, and custom types
- **Extensible Design**: Custom hashers can enable heterogeneous lookup for any logically equivalent types
- **Full Compatibility**: Drop-in replacements for standard library containers

### 🔧 Advanced Features

- **Iterator Support**: Full STL-compatible iterator interface with range-based for loops
- **Exception Safety**: Strong exception guarantees with RAII principles
- **Move Semantics**: Full move constructor and assignment support
- **Const Correctness**: Properly const-qualified member functions
- **SSE4.2 Acceleration**: Hardware-accelerated CRC32 hashing when available

### ⚡ Performance Optimized

- Header-only library with minimal runtime dependencies (requires nfx-hashing)
- Robin Hood hashing with backward shift deletion
- Perfect hashing with O(1) guaranteed lookup for static data
- Zero-copy heterogeneous lookup for compatible key types
- Hardware-accelerated hashing with SSE4.2 CRC32 instructions
- Cache-friendly contiguous memory layout

### 🌍 Cross-Platform Support

- **Platforms**: Linux, Windows
- **Architecture**: x86-64 (x86 SIMD features: SSE4.2, AVX, AVX2)
- **Compilers**: GCC 14+, Clang 18+, MSVC 2022+
- Thread-safe operations
- Consistent behavior across platforms
- CI/CD testing on multiple compilers

## Quick Start

### Requirements

- C++20 compatible compiler:
  - **GCC 14+** (14.2.0 tested)
  - **Clang 18+** (19.1.7 tested)
  - **MSVC 2022+** (19.44+ tested)
- CMake 3.20 or higher

### CMake Integration

```cmake
# Development options
option(NFX_CONTAINERS_BUILD_TESTS         "Build tests"                       OFF)
option(NFX_CONTAINERS_BUILD_SAMPLES       "Build samples"                     OFF)
option(NFX_CONTAINERS_BUILD_BENCHMARKS    "Build benchmarks"                  OFF)
option(NFX_CONTAINERS_BUILD_DOCUMENTATION "Build Doxygen documentation"       OFF)

# Installation
option(NFX_CONTAINERS_INSTALL_PROJECT     "Install project"                   OFF)

# Packaging
option(NFX_CONTAINERS_PACKAGE_SOURCE      "Enable source package generation"  OFF)
option(NFX_CONTAINERS_PACKAGE_ARCHIVE     "Enable TGZ/ZIP package generation" OFF)
option(NFX_CONTAINERS_PACKAGE_DEB         "Enable DEB package generation"     OFF)
option(NFX_CONTAINERS_PACKAGE_RPM         "Enable RPM package generation"     OFF)
option(NFX_CONTAINERS_PACKAGE_WIX         "Enable WiX MSI installer"          OFF)
```

### Using in Your Project

#### Option 1: Using FetchContent (Recommended)

```cmake
include(FetchContent)
FetchContent_Declare(
  nfx-containers
  GIT_REPOSITORY https://github.com/nfx-libs/nfx-containers.git
  GIT_TAG        main  # or use specific version tag like "0.1.0"
)
FetchContent_MakeAvailable(nfx-containers)

# Link with header-only interface library
target_link_libraries(your_target PRIVATE nfx-containers::nfx-containers)
```

#### Option 2: As a Git Submodule

```bash
# Add as submodule
git submodule add https://github.com/nfx-libs/nfx-containers.git third-party/nfx-containers
```

```cmake
# In your CMakeLists.txt
add_subdirectory(third-party/nfx-containers)
target_link_libraries(your_target PRIVATE nfx-containers::nfx-containers)
```

#### Option 3: Using find_package (After Installation)

```cmake
find_package(nfx-containers REQUIRED)
target_link_libraries(your_target PRIVATE nfx-containers::nfx-containers)
```

### Building

> ⚠️ **Important**: For maximum performance, **compile with SIMD flags** to enable hardware-accelerated hashing (provided by nfx-hashing dependency). Without flags like `-msse4.2` or `/arch:AVX`, the compiler won't emit SSE4.2 instructions, resulting in slower software fallback.

**Recommended Compiler Flags:**

- **GCC/Clang**: `-march=native` (auto-detect) or specific flags like `-msse4.2`, `-mavx`, `-mavx2`
- **MSVC**: `/arch:AVX` or `/arch:AVX2`

**CMake Example:**

```cmake
target_compile_options(your_target PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-march=native>
)
```

**Build Commands:**

```bash
# Clone the repository
git clone https://github.com/nfx-libs/nfx-containers.git
cd nfx-containers

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the library
cmake --build . --config Release --parallel

# Run tests (optional)
ctest -C Release --output-on-failure

# Run benchmarks (optional)
./bin/benchmarks/BM_FastHashMap
./bin/benchmarks/BM_PerfectHashMap
```

### Documentation

nfx-containers includes API documentation generated with Doxygen.

#### 📚 Online Documentation

The complete API documentation is available online at:
**https://nfx-libs.github.io/nfx-containers**

#### Building Documentation Locally

```bash
# Configure with documentation enabled
cmake .. -DCMAKE_BUILD_TYPE=Release -DNFX_CONTAINERS_BUILD_DOCUMENTATION=ON

# Build the documentation
cmake --build . --target nfx-containers-documentation
```

#### Requirements

- **Doxygen** - Documentation generation tool
- **Graphviz Dot** (optional) - For generating class diagrams

#### Accessing Local Documentation

After building, open `./build/doc/html/index.html` in your web browser.

## Usage Example

```cpp
#include <iostream>
#include <string>

#include <nfx/Containers.h>

using namespace nfx::containers;

int main()
{
    // FastHashMap - Robin Hood hash map with excellent performance
    FastHashMap<std::string, int> ages;
    ages.insertOrAssign("Alice", 30);
    ages.insertOrAssign("Bob", 25);
    ages.insertOrAssign("Charlie", 35);

    // Heterogeneous lookup - no temporary string allocation!
    if (int* age = ages.find("Alice")) {
        std::cout << "Alice is " << *age << " years old\n";
    }

    // Zero-copy lookup with string_view
    std::string_view key = "Bob";
    if (ages.contains(key)) {
        std::cout << "Bob exists in the map\n";
    }

    // Iteration
    for (const auto& [name, age] : ages) {
        std::cout << name << ": " << age << "\n";
    }

    // OrderedHashMap - Insertion-order preserving hash map
    OrderedHashMap<std::string, int> ordered;
    ordered.insertOrAssign("third", 3);
    ordered.insertOrAssign("first", 1);
    ordered.insertOrAssign("second", 2);

    // Guaranteed insertion-order iteration
    for (const auto& [key, value] : ordered) {
        std::cout << key << ": " << value << "\n";  // third, first, second
    }

    // Bidirectional iteration (unique to OrderedHashMap!)
    for (auto it = --ordered.end(); it != ordered.begin(); --it) {
        std::cout << it->first << " ";
    }
    std::cout << ordered.begin()->first << "\n";  // second, first, third

    // FastHashSet - Robin Hood hash set
    FastHashSet<std::string> names;
    names.insert("Alice");
    names.insert("Bob");

    // PerfectHashMap - O(1) guaranteed lookups for static data
    std::vector<std::pair<std::string, int>> data = {
        {"one", 1}, {"two", 2}, {"three", 3}
    };
    PerfectHashMap<std::string, int> perfect(std::move(data));

    if (const int* value = perfect.find("two")) {
        std::cout << "two = " << *value << "\n";  // Guaranteed O(1)!
    }

    // TransparentHashMap - Enhanced std::unordered_map wrapper
    TransparentHashMap<std::string, double> prices;
    prices["apple"] = 1.99;
    prices["banana"] = 0.99;

    // Heterogeneous lookup without temporary allocations
    if (auto it = prices.find("apple"); it != prices.end()) {
        std::cout << "Apple costs $" << it->second << "\n";
    }

    // StackVector - Stack-optimized vector for small collections
    StackVector<int, 4> numbers;  // Stores up to 4 ints on stack
    numbers.push_back(10);
    numbers.push_back(20);
    numbers.push_back(30);
    
    // No heap allocation for small sizes!
    for (int num : numbers) {
        std::cout << num << " ";
    }
    std::cout << "\n";

    return 0;
}
```

**Output:**

```
Alice is 30 years old
Bob exists in the map
Charlie: 35
Bob: 25
Alice: 30
third: 3
first: 1
second: 2
second first third
two = 2
Apple costs $1.99
10 20 30
```

## Installation & Packaging

nfx-containers provides packaging options for distribution.

### Package Generation

```bash
# Configure with packaging options
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DNFX_CONTAINERS_PACKAGE_ARCHIVE=ON \
         -DNFX_CONTAINERS_PACKAGE_DEB=ON \
         -DNFX_CONTAINERS_PACKAGE_RPM=ON

# Generate binary packages
cmake --build . --target package
# or
cd build && cpack

# Generate source packages
cd build && cpack --config CPackSourceConfig.cmake
```

### Supported Package Formats

| Format      | Platform       | Description                        | Requirements |
| ----------- | -------------- | ---------------------------------- | ------------ |
| **TGZ/ZIP** | Cross-platform | Compressed archive packages        | None         |
| **DEB**     | Debian/Ubuntu  | Native Debian packages             | `dpkg-dev`   |
| **RPM**     | RedHat/SUSE    | Native RPM packages                | `rpm-build`  |
| **WiX**     | Windows        | Professional MSI installer         | `WiX 3.11+`  |
| **Source**  | Cross-platform | Source code distribution (TGZ+ZIP) | None         |

### Installation

```bash
# Linux (DEB-based systems)
sudo dpkg -i nfx-containers_*_amd64.deb

# Linux (RPM-based systems)
sudo rpm -ivh nfx-containers-*-Linux.rpm

# Windows (MSI installer)
nfx-containers-0.1.0-MSVC.msi

# Manual installation (extract archive)
tar -xzf nfx-containers-*-Linux.tar.gz -C /usr/local/
```

## Project Structure

```
nfx-containers/
├── benchmark/                   # Performance benchmarks with Google Benchmark
├── cmake/                       # CMake modules and configuration
├── include/nfx/                 # Public headers: containers and functors
│   ├── containers/              # Container implementations
│   │   ├── FastHashMap.h        # Robin Hood hash map implementation
│   │   ├── FastHashSet.h        # Robin Hood hash set implementation
│   │   ├── OrderedHashMap.h     # Insertion-order preserving hash map
│   │   ├── PerfectHashMap.h     # Perfect hash map (CHD algorithm)
│   │   ├── StackVector.h        # Small vector optimization with stack storage
│   │   ├── TransparentHashMap.h # Enhanced unordered_map wrapper
│   │   └── TransparentHashSet.h # Enhanced unordered_set wrapper
│   └── detail/                  # Implementation details
├── samples/                     # Example usage and demonstrations
└── test/                        # Unit tests with GoogleTest
```

## Performance

nfx-containers is optimized for high performance with:

- **Robin Hood Hashing**: Backward shift deletion reduces clustering and improves lookup times
- **Perfect Hashing (CHD)**: O(1) guaranteed lookups with zero collisions for static data
- **Cache-Friendly Layout**: Contiguous memory storage improves cache locality
- **Zero-Copy Lookups**: Heterogeneous lookup with compatible types eliminates temporary allocations
- **Hardware Acceleration**: SSE4.2 CRC32 instructions for high-performance hashing when available
- **Minimal Overhead**: Header-only design with inline implementations for optimal compiler optimization

For detailed performance metrics and benchmarks, see the [benchmark documentation](benchmark/README.md).

## Roadmap

See [TODO.md](TODO.md) for upcoming features and project roadmap.

## Changelog

See the [CHANGELOG.md](CHANGELOG.md) for a detailed history of changes, new features, and bug fixes.

## License

This project is licensed under the MIT License.

## Dependencies

### Runtime Dependencies

- **[nfx-hashing](https://github.com/nfx-libs/nfx-hashing)**: High-performance hashing library with hardware acceleration (MIT License)

### Development Dependencies

- **[GoogleTest](https://github.com/google/googletest)**: Testing framework (BSD 3-Clause License) - Development only
- **[Google Benchmark](https://github.com/google/benchmark)**: Performance benchmarking framework (Apache 2.0 License) - Development only

All dependencies are automatically fetched via CMake FetchContent when building the library, tests, or benchmarks.

---

_Updated on January 24, 2026_
