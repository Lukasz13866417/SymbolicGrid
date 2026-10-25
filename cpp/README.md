# SymbolicGrid — C++17 port

A standalone native implementation of the Java SymbolicGrid in the parent
directory. It keeps the same 1-based coordinates, explicit and uniformly random
horizontal/vertical reservations, and upward propagation from child grids.
It also exposes counts and ranked selection for inspection and testing.

Build directories matching `cpp/build*/` are ignored by `cpp/.gitignore`.
The C++ source, tests, examples, and documentation can be committed normally.

## Build and run

From the project root:

```sh
cmake -S cpp -B cpp/build -DCMAKE_BUILD_TYPE=Release \
  -DSYMBOLIC_GRID_BUILD_TESTS=ON \
  -DSYMBOLIC_GRID_BUILD_EXAMPLES=ON \
  -DSYMBOLIC_GRID_BUILD_BENCHMARKS=ON
cmake --build cpp/build --parallel
ctest --test-dir cpp/build --output-on-failure
./cpp/build/symbolic_grid_example
./cpp/build/symbolic_grid_benchmark 256 7
```

There are no third-party dependencies. Tests, examples, and benchmarks are
disabled by default when adding the project as a dependency.

## Source layout

```text
cpp/
├── include/symbolic/
│   ├── GridCreator.hpp
│   ├── GridSegment.hpp
│   ├── GridOptions.hpp
│   ├── MemoryStats.hpp
│   ├── SymbolicGrid.hpp                 # optional umbrella include
│   ├── segments/
│   │   ├── PartialSegmentHandler.hpp
│   │   └── detail/
│   │       ├── SegmentIndex.hpp
│   │       ├── SegmentNode.hpp
│   │       ├── TreeKind.hpp
│   │       └── NodePool.hpp
│   └── util/Random.hpp
├── src/
│   ├── GridCreator.cpp
│   ├── GridSegment.cpp
│   ├── detail/Validation.hpp
│   ├── segments/
│   │   ├── PartialSegmentHandler.cpp
│   │   └── detail/
│   │       ├── SegmentIndex.cpp
│   │       ├── SegmentIndexDiagnostics.cpp
│   │       ├── SegmentIndexRendering.cpp
│   │       ├── AvlOperations.inl
│   │       └── NodePool.cpp
│   └── util/Random.cpp
├── tests/grid_tests.cpp
├── examples/basic.cpp
├── benchmarks/grid_benchmark.cpp
└── CMakeLists.txt
```

`GridCreator` coordinates both orientations and parent/child propagation.
`PartialSegmentHandler` implements reservations and random selection for one
orientation. Its `SegmentIndex` maintains the two AVL indexes over a shared
`NodePool`; `SegmentNode` defines the compact storage layout. Tree balancing
selects `TreeKind::Position` or `TreeKind::Length` at compile time. The
templates live in the private `AvlOperations.inl`, with rendering and invariant
checks in separate translation units. `Random` owns the per-instance generator.

The public API stays in namespace `symbolic`. Types under `segments/detail/`
and the random utility are implementation details in `symbolic::detail`.
Small pool and random operations remain inline; these module boundaries add
no virtual dispatch or extra allocations. Both trees still share each node.

Include `symbolic/GridCreator.hpp` or
`symbolic/segments/PartialSegmentHandler.hpp` directly. The small
`symbolic/SymbolicGrid.hpp` umbrella remains available for existing callers.

## Use

```cpp
#include "symbolic/GridCreator.hpp"

// Optional fixed seed and initial capacity per orientation.
symbolic::GridOptions options;
options.seed = 42;
options.initialSegmentCapacity = 4096;

symbolic::GridCreator parent(100, 80, options);
parent.reserveVertical(2, 2, 2);
parent.reserveHorizontal(1, 1, 4);

// A length-L free run contributes L - length + 1 possible placements.
// Every valid placement has the same probability, including overlapping ones.
if (auto segment = parent.tryReserveRandomFittingVertical(5)) {
    // segment->row, segment->col, segment->length
}

// Child rows 1..20 map to parent rows 61..80; the area must be free.
symbolic::GridCreator child(20, 80, parent, 60, options);
child.reserveHorizontal(1, 1, 5); // also reserves parent row 61

const std::uint64_t available = parent.countFittingHorizontal(3);
if (available != 0) {
    const auto first = parent.getKthFittingHorizontal(3, 1); // does not reserve
    (void)first;
}
parent.printGrid();
// Storage is released by destructors. No destroy() call is needed.
```

`reserveRandomFittingVertical/Horizontal` return a `GridSegment` and throw
`std::out_of_range` if none fits. The corresponding `tryReserve...` methods
return `std::nullopt` for this normal failure case, avoiding exception overhead.
Lengths greater than the grid extent have zero fitting placements. Zero or
negative lengths/dimensions throw `std::invalid_argument`; invalid coordinates
or ranks throw `std::out_of_range`; overlapping reservations throw
`std::invalid_argument`.

`getKthFitting...` ranks placements by the containing maximal free run's
`(length, row, col)`, followed by the offset within that run, matching the Java
length index. Ranks are 1-based and 64-bit. Seeds are per instance; sequences are
repeatable within this port, but do not match `java.util.Random`.

For an independent single-orientation index, use
`PartialSegmentHandler(rows, cols, vertical, options)`. Its methods include
`reserve`, `tryReserveRandomFitting`, `countFittingSpaces`,
`getKthFittingSpace`, and `isFree`. This avoids updating perpendicular runs if
your algorithm only needs one orientation.

## Performance design

- Each orientation stores its maximal free runs in a contiguous vector of
  **48-byte nodes**. Two intrusive AVL indexes share each node: one ordered by
  line/end position, the other by length/row/column. Links are 32-bit indices,
  so moving the backing vector does not invalidate them.
- The length index stores subtree run counts and 64-bit total lengths.
  For runs of length at least `d`, the number of placements is
  `totalLength - (d - 1) * count`. Suffix counting and ranked selection each
  follow logarithmic tree paths. The Java sparse tree's hashed coordinate
  universe and its extra path nodes are unnecessary.
- Removed nodes return immediately to an intrusive free list. Storage follows
  the peak number of live runs, rather than the history of reservations.
  Separate grids have separate pools and random state.
- Successful reservations perform no heap allocations when capacity suffices.
  Growth is geometric. Set `initialSegmentCapacity` or call
  `reserveCapacity(segmentsPerOrientation)` before a latency-sensitive phase.
  Allow room for peak live runs **plus the longest reservation**: preflight
  conservatively budgets up to one new perpendicular run per reserved cell.
- Random selection uses an 8-byte SplitMix64 state and rejection sampling to
  avoid modulo bias. It selects placements by their actual multiplicities,
  rather than selecting free runs with equal probability.
- Reservations validate and acquire capacity throughout the ancestor chain
  before modifying any cells. Invalid input, overlap, and allocation failure
  leave all grid contents unchanged. Capacity and random state may advance
  after a failed attempt.

For `S` live free runs per orientation and a reservation of length `d`:

| Operation | Time |
| --- | --- |
| Construct an `n × m` grid | `O(n log n + m log m)` |
| Count placements / select a ranked placement | `O(log(S + 1))` |
| Explicit or random reservation, full grid | `O((d + 1) log(S + 1))` |
| Explicit or random reservation, partial handler | `O(log(S + 1))` |
| Test whether a cell is free | `O(log(S + 1))` |

These reservation costs exclude occasional vector growth, which copies the
allocated slots in that orientation. Preallocation removes that source of
latency. Ancestor propagation repeats the reservation work at each level.
Initialization stores `n + m` runs, not `n * m` cells. Node storage is
`48 * (verticalCapacity + horizontalCapacity + 2)` bytes, excluding the small
grid objects and allocator bookkeeping. `memoryStats()` reports actual pool
capacity, allocated slots, live runs, and bytes. Capacity remains reserved until
destruction; emptied slots are reused during the grid's lifetime.

## Android / CMake integration

Add the directory to your native game's existing CMake project:

```cmake
add_subdirectory(path/to/SymbolicGrid/cpp symbolic-grid)
target_link_libraries(your_game_target PRIVATE symbolic::grid)
```

The target supplies its include directory and C++17 requirement, builds as a
position-independent static library, and can be linked into an Android shared
library. Build the game's native target in Release mode. This API uses C++
exceptions for invalid input and allocation failures; keep exceptions enabled.
If you expose it through JNI, translate exceptions at that boundary.

To build the static library directly with an installed NDK:

```sh
cmake -S cpp -B cpp/build-android \
  -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake" \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-24 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build cpp/build-android --parallel
```

Choose the ABI and minimum API level for your game. No Android NDK was available
in the porting environment, so Android compilation and device timings remain
to be verified. The implementation uses portable standard C++17 and no host
CPU-specific compiler flags.

## Child-grid contract and lifetime

As in Java, propagation is **child to parent only**. Children have local indexes;
writing to the parent or to an overlapping sibling does not update those
indexes. After creating a child, perform reservations in its region through
that child and use disjoint sibling regions. Nested children are supported.
If a child's locally selected segment conflicts with an ancestor, the operation
throws before modifying any grid. Counts and selection in the child describe
its local index; they do not filter later ancestor-only reservations.

The parent must outlive its children. Grids cannot be copied or moved, which
keeps parent addresses stable. Each independent grid can be used on its own
thread; accessing the same grid or parent/child chain concurrently requires
external synchronization. As in the original API, reservations cannot be freed
and dimensions cannot be changed.

## Validation and measurements

The test executable checks all 512 occupancies of a 3×3 grid and every valid
placement rank, 100 randomized mixed-orientation workloads against a cell
model, AVL ordering/balance/aggregates and free-list integrity, counts above
`UINT32_MAX`, coordinates near `INT_MAX`, nested children, invalid reservations,
injected allocation failures, deterministic seeds, node reuse, and zero heap
allocations during preallocated reservations. Test checks remain enabled in
Release builds.

For address and undefined-behavior checks:

```sh
cmake -S cpp -B cpp/build-sanitize -DCMAKE_BUILD_TYPE=Debug \
  -DSYMBOLIC_GRID_BUILD_TESTS=ON -DSYMBOLIC_GRID_SANITIZERS=ON
cmake --build cpp/build-sanitize --parallel
ctest --test-dir cpp/build-sanitize --output-on-failure
```

Verified with GCC 14.2 Release and Clang 18.1 Debug with ASan/UBSan. The local
traced execution environment cannot run LeakSanitizer; sanitizer tests passed
with `ASAN_OPTIONS=detect_leaks=0 UBSAN_OPTIONS=halt_on_error=1`. Leave leak
detection enabled when running outside that environment.

Example local results on an Intel Core i7-14700KF, GCC 14.2 Release, dynamically
growing pools (2026-10-25):

| Grid | Mixed reservations to ~75% occupied | Time | Time / reservation | Pool capacity bytes |
| --- | ---: | ---: | ---: | ---: |
| 256×256 | 11,478 | 34.2 ms | 2.98 µs | 1,579,008 |
| 1024×1024 | 183,918 | 1,013 ms | 5.51 µs | 25,190,400 |

These are the samples with median reservation time over 7 and 5 repetitions,
respectively, after one warmup. Construction, reservations, and queries are
timed separately; destruction is excluded. Reservations alternate orientations
and lengths 1–8, falling back to length 1 when fragmented. On those fragmented
grids, 400,000 fitting-count queries averaged 13.3 ns and 23.6 ns, respectively.
Run `symbolic_grid_benchmark 256 7 preallocate` to compare reserved capacity.
These are host measurements of this implementation, not Android measurements
or a measured speedup over Java.

## Attribution

Adapted from [Lukasz13866417/SymbolicGrid](https://github.com/Lukasz13866417/SymbolicGrid).
The original project declares the
[Creative Commons Attribution 4.0 International license](https://creativecommons.org/licenses/by/4.0/)
in its README. This port changes the implementation to C++17, combines the two
indexes in a shared node pool, replaces the sparse length tree with an augmented
AVL tree, and adds validation, tests, and benchmarks. Preserve this attribution
when distributing the port.
