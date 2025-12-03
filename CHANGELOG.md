# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- NIL

### Changed

- Relaxed type constraints across all containers - no longer require default-constructible types

### Deprecated

- NIL

### Removed

- NIL

### Fixed

- NIL

### Security

- NIL

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
