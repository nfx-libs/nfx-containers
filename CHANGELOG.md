# Changelog

## [Unreleased]

### Added

- NIL
  
### Changed

- Renamed **SmallVector** to **StackVector**
  
### Deprecated

- NIL

### Removed

- NIL

### Fixed

- NIL

### Security

- NIL

## [0.3.2] - 2026-02-03

### Changed

- Removed commented-out code from FastHashMap implementation
  
### Fixed

- Fixed PerfectHashMap documentation for `size()` and `count()` methods

## [0.3.1] - 2026-01-31

### Changed

- Bump nfx-hashing from 0.1.1 to 0.1.2
- Bump Google Benchmark from 1.9.4 to 1.9.5

## [0.3.0] - 2026-01-27

### Added

- **OrderedHashMap**: Insertion-order preserving hash map with Robin Hood hashing and doubly-linked list
  - Hybrid hash table + intrusive doubly-linked list structure
  - Guaranteed insertion-order iteration
  - Bidirectional iterator support with operator++ and operator--
  - O(1) average-case lookups, insertions, and deletions
  - Heterogeneous lookup with zero-copy string operations
  - Order-independent equality comparison

- **OrderedHashSet**: Insertion-order preserving hash set with Robin Hood hashing and doubly-linked list
  - Hybrid hash table + intrusive doubly-linked list structure
  - Guaranteed insertion-order iteration
  - Bidirectional iterator support with operator++ and operator--
  - O(1) average-case lookups, insertions, and deletions
  - Heterogeneous lookup with zero-copy string operations
  - Order-independent equality comparison

## [0.2.0] - 2026-01-12

### Added

- **SmallVector**: Small vector optimization with stack storage and heap fallback

### Changed

- Relaxed type constraints across all containers - no longer require default-constructible types

### Fixed

- Implemented proper move semantics for FastHashMap and FastHashSet

## [0.1.1] - 2025-11-27

### Changed

- Consolidated packaging tool detection in CMake configuration

### Fixed

- Removed incorrect runtime dependencies from DEB/RPM packages (header-only library)

## [0.1.0] - 2025-11-15 - Initial Release

- **FastHashMap**: Mutable hash map with Robin Hood hashing algorithm

  - Bounded probe distances for predictable performance
  - Heterogeneous lookup with zero-copy string operations
  - Configurable 32-bit or 64-bit hash types
  - Perfect forwarding for efficient insertions

- **FastHashSet**: Hash set with Robin Hood hashing

  - Consistent API with FastHashMap
  - Transparent heterogeneous comparison support
  - Optimal cache-friendly memory layout

- **PerfectHashMap**: Perfect hash map using CHD (Compress, Hash, Displace) algorithm for immutable datasets

  - O(1) guaranteed lookups with minimal memory overhead
  - Heterogeneous lookup support
  - Zero-collision perfect hashing for static data

- **TransparentHashMap**: Thin wrapper around `std::unordered_map` with enhanced hashing

  - High-performance CRC32-C string hashing
  - Heterogeneous lookup support via `is_transparent`
  - Full STL compatibility

- **TransparentHashSet**: Thin wrapper around `std::unordered_set` with enhanced hashing

  - Consistent API with TransparentHashMap
  - Hardware-accelerated hashing with SSE4.2 support
  - Automatic fallback for custom types

- **Core Features**:

  - Header-only library for easy integration
  - C++20 standard compliance
  - CMake build system with FetchContent support
  - Cross-platform support
  - Compiler support: MSVC 2022, GCC 14+, Clang 18+

- **Documentation**

  - README with feature overview and usage examples
  - Sample applications
  - Build and installation instructions

- **Testing & Benchmarking**

  - Unit test suite
  - Performance benchmarks for all operations
  - Cross-compiler performance validation
