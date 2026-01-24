# nfx-containers TODO

Project roadmap and task tracking for the nfx-containers library.

### Todo

- [ ] Add `FlatHashMap` and `FlatHashSet` containers (optimized for low load factors and extreme performance)
  - [ ] Tombstone handling for deletions
- [ ] Add advanced STL methods
  - [ ] `extract()` method for node-handle-like semantics
  - [ ] `merge()` method for bulk transfer between containers
  - [ ] `rehash(n)` and `reserve(n)` for explicit capacity control
  - [ ] `contains()` method
- [ ] Add `FastHashMultiMap` and `FastHashMultiSet` containers (indices, graph adjacency lists, grouped data)
  - [ ] Support duplicate keys (many-to-many relationships)
  - [ ] `equalRange()` method for retrieving all values for a key
  - [ ] Efficient storage with value chaining or bucketing
- [ ] Add `SmallHashMap<N>` and `SmallHashSet<N>` with SBO (inline storage for ≤N elements)
  - [ ] Template parameter for capacity threshold
  - [ ] Automatic spillover to heap when N exceeded
- [ ] Add custom allocator support
  - [ ] `Allocator` template parameter for all containers
  - [ ] Arena/pool allocator compatibility
- [ ] Add `ConcurrentHashMap` and `ConcurrentHashSet` containers (read-optimized for high concurrency scenarios)
  - [ ] Thread-safe lock-free or fine-grained locking
- [ ] Add `BidirectionalHashMap` container (bijective mappings, reverse lookups)
  - [ ] Bidirectional key-value mapping (lookup by key OR value)
  - [ ] Two underlying hash tables with synchronized operations
- [ ] Add `DynamicPerfectHashMap` container (mutable perfect hash map with overflow)
  - [ ] Hybrid approach: perfect hash table + overflow buffer for new insertions
  - [ ] Automatic rebuild when overflow threshold exceeded
  - [ ] O(1) lookups in primary table, fallback to overflow map

### In Progress

- [ ] Add `OrderedHashMap` and `OrderedHashSet` containers (config files, JSON parsing, stable iteration, LRU caches)
  - [x] `OrderedHashMap` - Complete (header, implementation, samples, tests - 42/42 passing)
  - [x] Preserve insertion order during iteration
  - [x] Hybrid hash table + doubly-linked list structure
  - [x] Bidirectional iterator support
  - [ ] `OrderedHashSet` - Not started

### Done ✓

- [x] `OrderedHashMap` container fully implemented and tested
