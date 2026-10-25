// ---------------------------- AI ----------------------------------
// ---------------------------- AI ----------------------------------
// ---------------------------- AI ----------------------------------
// ---------------------------- AI ----------------------------------
#include "symbolic/SymbolicGrid.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <random>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

// Allocation instrumentation covers the library's ordinary vector allocations.
// Tests are single threaded; the production library has no global pool/state.
namespace allocations {
std::size_t calls = 0;
long failAfter = -1;
}
void* operator new(std::size_t size) {
    if (allocations::failAfter == 0) {
        throw std::bad_alloc();
    }
    if (allocations::failAfter > 0) {
        --allocations::failAfter;
    }
    ++allocations::calls;
    if (void* p = std::malloc(size == 0 ? 1 : size)) {
        return p;
    }
    throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

namespace {
using symbolic::GridCreator;
using symbolic::GridSegment;
using symbolic::PartialSegmentHandler;

#define CHECK(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error(std::string("Check failed at line ") \
                + std::to_string(__LINE__) + ": " #condition); \
        } \
    } while (false)

template<class Exception, class Function>
void expectThrow(Function&& function) {
    bool thrown = false;
    try { function(); } catch (const Exception&) { thrown = true; }
    CHECK(thrown);
}

struct Model {
    int rows;
    int cols;
    std::vector<unsigned char> taken;
    Model(int r, int c) : rows(r), cols(c), taken(static_cast<std::size_t>(r * c), 0) {}
    bool free(int r, int c) const { return taken[static_cast<std::size_t>((r - 1) * cols + c - 1)] == 0; }
    void reserve(GridSegment s, bool vertical) {
        for (int i = 0; i < s.length; ++i) {
            const int row = s.row + (vertical ? i : 0);
            const int col = s.col + (vertical ? 0 : i);
            CHECK(row >= 1 && row <= rows && col >= 1 && col <= cols && free(row, col));
            taken[static_cast<std::size_t>((row - 1) * cols + col - 1)] = 1;
        }
    }
    std::vector<GridSegment> spaces(int length, bool vertical) const {
        std::vector<GridSegment> runs;
        const int lines = vertical ? cols : rows;
        const int extent = vertical ? rows : cols;
        for (int line = 1; line <= lines; ++line) {
            int start = 1;
            while (start <= extent) {
                if (!free(vertical ? start : line, vertical ? line : start)) {
                    ++start;
                    continue;
                }
                int end = start;
                while (end < extent && free(vertical ? end + 1 : line, vertical ? line : end + 1)) ++end;
                runs.push_back(vertical ? GridSegment{start, line, end - start + 1}
                                        : GridSegment{line, start, end - start + 1});
                start = end + 1;
            }
        }
        std::sort(runs.begin(), runs.end(), [](GridSegment a, GridSegment b) {
            return std::tie(a.length, a.row, a.col) < std::tie(b.length, b.row, b.col);
        });
        std::vector<GridSegment> result;
        for (const auto run : runs) {
            for (int i = 0; i <= run.length - length; ++i)
                result.push_back({run.row + (vertical ? i : 0), run.col + (vertical ? 0 : i), length});
        }
        return result;
    }
    std::string str() const {
        std::string result;
        for (int r = 1; r <= rows; ++r) {
            for (int c = 1; c <= cols; ++c) result += free(r, c) ? '.' : '#';
            result += '\n';
        }
        return result;
    }
};

void checkGrid(const GridCreator& grid, const Model& model, bool allRanks) {
    CHECK(grid.checkInvariants());
    CHECK(grid.toString() == model.str());
    for (int r = 1; r <= model.rows; ++r)
        for (int c = 1; c <= model.cols; ++c) CHECK(grid.isFree(r, c) == model.free(r, c));
    for (const bool vertical : {false, true}) {
        for (int length = 1; length <= (vertical ? model.rows : model.cols) + 1; ++length) {
            const auto expected = model.spaces(length, vertical);
            const auto count = vertical ? grid.countFittingVertical(length) : grid.countFittingHorizontal(length);
            CHECK(count == expected.size());
            for (std::size_t k = 0; k < expected.size(); ++k) {
                if (!allRanks && k != 0 && k != expected.size() / 2 && k != expected.size() - 1) {
                    continue;
                }
                const auto actual = vertical ? grid.getKthFittingVertical(length, k + 1)
                                             : grid.getKthFittingHorizontal(length, k + 1);
                CHECK(actual == expected[k]);
            }
        }
    }
}

void examplesAndValidation() {
    GridCreator parent(7, 5, {1, 0});
    parent.reserveVertical(2, 2, 2);
    parent.reserveHorizontal(1, 1, 4);
    CHECK(parent.toString() == "####.\n.#...\n.#...\n.....\n.....\n.....\n.....\n");
    GridCreator child(4, 5, parent, 3, {2, 0});
    child.reserveVertical(4, 5, 1);
    CHECK(!parent.isFree(7, 5));
    std::ostringstream out;
    parent.printGrid(out);
    CHECK(out.str() == parent.toString());
    out.str("");
    out << GridSegment::GS(1, 2, 3);
    CHECK(out.str() == "(r1, c2, l3)");

    const auto before = parent.toString();
    expectThrow<std::invalid_argument>([&] { parent.reserveHorizontal(1, 1, 1); });
    expectThrow<std::invalid_argument>([&] { parent.reserveVertical(4, 1, 0); });
    expectThrow<std::invalid_argument>([&] { parent.reserveVertical(4, 1, -1); });
    expectThrow<std::out_of_range>([&] { parent.reserveVertical(0, 1, 1); });
    expectThrow<std::out_of_range>([&] { parent.reserveHorizontal(1, 6, 1); });
    expectThrow<std::out_of_range>([&] { parent.reserveVertical(7, 1, 2); });
    expectThrow<std::out_of_range>([&] { parent.reserveHorizontal(4, 1, std::numeric_limits<int>::max()); });
    expectThrow<std::invalid_argument>([&] { parent.countFittingHorizontal(0); });
    expectThrow<std::invalid_argument>([&] { parent.tryReserveRandomFittingVertical(-2); });
    expectThrow<std::out_of_range>([&] { parent.getKthFittingVertical(1, 0); });
    expectThrow<std::out_of_range>([&] { parent.getKthFittingHorizontal(5, UINT64_MAX); });
    expectThrow<std::invalid_argument>([] { GridCreator invalid(0, 1); });
    expectThrow<std::invalid_argument>([&] { GridCreator invalid(1, 4, parent, 3); });
    expectThrow<std::invalid_argument>([&] { GridCreator invalid(4, 5, parent, 5); });
    expectThrow<std::invalid_argument>([&] { GridCreator invalid(1, 5, parent, -1); });
    expectThrow<std::invalid_argument>([&] { GridCreator invalid(1, 5, parent, 0); });
    CHECK(before == parent.toString());
    CHECK(!parent.tryReserveRandomFittingHorizontal(6));
    expectThrow<std::out_of_range>([&] { parent.reserveRandomFittingVertical(8); });
    CHECK(parent.checkInvariants());
}

void exhaustiveSmallGrids() {
    // All 512 possible occupancies, every valid rank in both orientations.
    for (int mask = 0; mask < 512; ++mask) {
        GridCreator grid(3, 3, {0, 0});
        Model model(3, 3);
        for (int i = 0; i < 9; ++i) {
            if ((mask & (1 << i)) != 0) {
                const GridSegment segment{i / 3 + 1, i % 3 + 1, 1};
                if ((i & 1) != 0) {
                    grid.reserveVertical(segment.row, segment.col, 1);
                } else {
                    grid.reserveHorizontal(segment.row, segment.col, 1);
                }
                model.reserve(segment, true);
            }
        }
        checkGrid(grid, model, true);
    }
}

void randomizedMixedReservations() {
    std::mt19937 random(1729);
    for (std::uint64_t trial = 0; trial < 100; ++trial) {
        const int rows = 1 + static_cast<int>(random() % 18);
        const int cols = 1 + static_cast<int>(random() % 18);
        GridCreator grid(rows, cols, {trial, 0});
        Model model(rows, cols);
        for (int step = 0; step < rows * cols; ++step) {
            const bool vertical = (random() & 1U) != 0;
            const int length = 1 + static_cast<int>(random() % static_cast<unsigned>(vertical ? rows : cols));
            const auto expected = model.spaces(length, vertical);
            const auto snapshot = grid.toString();
            if (expected.empty()) {
                CHECK(!(vertical ? grid.tryReserveRandomFittingVertical(length)
                                 : grid.tryReserveRandomFittingHorizontal(length)));
                CHECK(snapshot == grid.toString());
            } else {
                GridSegment segment{};
                if ((random() & 1U) != 0) {
                    segment = vertical ? grid.reserveRandomFittingVertical(length)
                                       : grid.reserveRandomFittingHorizontal(length);
                    CHECK(std::find(expected.begin(), expected.end(), segment) != expected.end());
                } else {
                    segment = expected[random() % expected.size()];
                    if (vertical) {
                        grid.reserveVertical(segment.row, segment.col, length);
                    } else {
                        grid.reserveHorizontal(segment.row, segment.col, length);
                    }
                }
                model.reserve(segment, vertical);
                const auto after = grid.toString();
                expectThrow<std::invalid_argument>([&] {
                    if (vertical) {
                        grid.reserveVertical(segment.row, segment.col, length);
                    } else {
                        grid.reserveHorizontal(segment.row, segment.col, length);
                    }
                });
                CHECK(after == grid.toString());
            }
            checkGrid(grid, model, rows * cols <= 40);
        }
        while (grid.countFittingHorizontal(1) != 0) {
            const auto segment = grid.reserveRandomFittingHorizontal(1);
            model.reserve(segment, false);
        }
        CHECK(!grid.tryReserveRandomFittingVertical(1));
        CHECK(!grid.tryReserveRandomFittingHorizontal(1));
        CHECK(grid.memoryStats().liveSegments == 0);
        checkGrid(grid, model, true);
    }
}

void partialHandlersAndLargeCounts() {
    for (const bool vertical : {false, true}) {
        PartialSegmentHandler partial(11, 11, vertical, {19, 0});
        Model model(11, 11);
        for (int i = 0; i < 40; ++i) {
            const auto selected = partial.tryReserveRandomFitting(1 + i % 4);
            if (selected) {
                model.reserve(*selected, vertical);
            }
            CHECK(partial.checkInvariants());
            CHECK(partial.toString() == model.str());
            for (int length = 1; length <= 12; ++length) {
                const auto expected = model.spaces(length, vertical);
                CHECK(partial.countFittingSpaces(length) == expected.size());
                for (std::size_t k = 0; k < expected.size(); ++k)
                    CHECK(partial.getKthFittingSpace(length, k + 1) == expected[k]);
            }
        }
        std::ostringstream out;
        partial.printGrid(out);
        CHECK(out.str() == model.str());
    }
    constexpr int max = std::numeric_limits<int>::max();
    PartialSegmentHandler tall(max, 2, true, {0, 0});
    CHECK(tall.countFittingSpaces(1) == UINT64_C(4294967294));
    CHECK(tall.getKthFittingSpace(1, UINT64_C(4294967294)) == (GridSegment{max, 2, 1}));
    tall.reserve(max, 1, 1);
    tall.reserve(1, 2, max);
    tall.reserve(1, 1, max - 1);
    CHECK(tall.countFittingSpaces(1) == 0);
    CHECK(tall.checkInvariants());
    PartialSegmentHandler wide(2, max, false, {0, 0});
    wide.reserve(1, max, 1);
    CHECK(wide.isFree(1, 1, max - 1));
    CHECK(wide.checkInvariants());

    GridCreator large(70000, 70000, {17, 0});
    CHECK(large.countFittingVertical(1) == UINT64_C(4900000000));
    CHECK(large.countFittingHorizontal(2) == UINT64_C(4899930000));
    CHECK(large.getKthFittingVertical(1, UINT64_C(4900000000)) == (GridSegment{70000, 70000, 1}));
    CHECK(large.getKthFittingHorizontal(1, UINT64_C(4900000000)) == (GridSegment{70000, 70000, 1}));
    large.reserveHorizontal(35000, 35000, 3);
    CHECK(large.countFittingVertical(1) == UINT64_C(4899999997));
    CHECK(large.countFittingHorizontal(1) == UINT64_C(4899999997));
    CHECK(large.checkInvariants());
}

void childrenAndFailureAtomicity() {
    GridCreator parent(10, 6, {1, 0});
    GridCreator child(6, 6, parent, 2, {2, 0});
    GridCreator grandchild(3, 6, child, 1, {3, 0});
    const auto segment = grandchild.reserveRandomFittingVertical(3);
    for (int i = 0; i < 3; ++i) {
        CHECK(!child.isFree(segment.row + 1 + i, segment.col));
        CHECK(!parent.isFree(segment.row + 3 + i, segment.col));
    }
    CHECK(parent.checkInvariants() && child.checkInvariants() && grandchild.checkInvariants());

    GridCreator root(5, 3, {0, 0});
    GridCreator local(3, 3, root, 1, {0, 0});
    root.reserveHorizontal(2, 1, 3);
    const auto localBefore = local.toString();
    const auto rootBefore = root.toString();
    expectThrow<std::invalid_argument>([&] { local.reserveVertical(1, 1, 3); });
    CHECK(local.toString() == localBefore && root.toString() == rootBefore);
    // Every locally fitting vertical segment of length 3 now conflicts above.
    expectThrow<std::invalid_argument>([&] { local.reserveRandomFittingVertical(3); });
    CHECK(local.toString() == localBefore && root.toString() == rootBefore);
    expectThrow<std::invalid_argument>([&] { GridCreator invalid(1, 3, local, 0); });
    CHECK(root.checkInvariants() && local.checkInvariants());

    // Force allocation failure at each preflight step across parent + child.
    int failures = 0;
    for (long fail = 0; fail < 6; ++fail) {
        GridCreator top(12, 12, {0, 0});
        GridCreator bottom(10, 12, top, 1, {0, 0});
        const auto topBefore = top.toString();
        const auto bottomBefore = bottom.toString();
        bool failed = false;
        allocations::failAfter = fail;
        try { bottom.reserveVertical(2, 6, 8); }
        catch (const std::bad_alloc&) { failed = true; }
        allocations::failAfter = -1;
        if (failed) {
            ++failures;
            CHECK(topBefore == top.toString() && bottomBefore == bottom.toString());
        } else {
            CHECK(!top.isFree(3, 6) && !bottom.isFree(2, 6));
        }
        CHECK(top.checkInvariants() && bottom.checkInvariants());
    }
    CHECK(failures >= 4);
}

void capacityAndRandomness() {
    GridCreator grid(64, 64, {0, 4096});
    const auto before = allocations::calls;
    for (int i = 0; i < 4096; ++i) {
        if ((i & 1) != 0) {
            grid.reserveRandomFittingVertical(1);
        } else {
            grid.reserveRandomFittingHorizontal(1);
        }
    }
    CHECK(allocations::calls == before);
    CHECK(grid.memoryStats().liveSegments == 0);
    CHECK(grid.memoryStats().allocatedSlots < 4096);
    CHECK(grid.checkInvariants());

    // Splits then full deletions must reuse slots, including AVL nodes with two children.
    PartialSegmentHandler recycled(100, 100, false, {0, 0});
    for (int row = 1; row <= 100; ++row) {
        recycled.reserve(row, 50, 1);
        recycled.reserve(row, 1, 49);
        recycled.reserve(row, 51, 50);
        CHECK(recycled.checkInvariants());
    }
    CHECK(recycled.memoryStats().allocatedSlots <= 101);
    CHECK(recycled.memoryStats().liveSegments == 0);

    GridCreator a(20, 20, {1234, 0});
    GridCreator b(20, 20, {1234, 0});
    GridCreator unrelated(3, 3, {999, 0});
    for (int i = 0; i < 100; ++i) {
        const int length = 1 + i % 5;
        if ((i & 1) != 0) {
            CHECK(a.tryReserveRandomFittingVertical(length) == b.tryReserveRandomFittingVertical(length));
        } else {
            CHECK(a.tryReserveRandomFittingHorizontal(length) == b.tryReserveRandomFittingHorizontal(length));
        }
        unrelated.tryReserveRandomFittingVertical(1);
    }
    CHECK(a.toString() == b.toString());
    std::array<int, 10> frequencies{};
    for (std::uint64_t seed = 0; seed < 12000; ++seed) {
        PartialSegmentHandler sample(1, 10, false, {seed, 0});
        sample.reserve(1, 4, 2); // lengths 3 and 5: 2 and 4 placements of size 2
        ++frequencies[static_cast<std::size_t>(sample.reserveRandomFitting(2).col - 1)];
    }
    for (int col = 1; col <= 10; ++col) {
        const bool valid = col == 1 || col == 2 || (col >= 6 && col <= 9);
        const int frequency = frequencies[static_cast<std::size_t>(col - 1)];
        if (valid) {
            CHECK(frequency > 1700 && frequency < 2300);
        } else {
            CHECK(frequency == 0);
        }
    }
}
} // namespace

int main() {
    try {
        examplesAndValidation();
        exhaustiveSmallGrids();
        randomizedMixedReservations();
        partialHandlersAndLargeCounts();
        childrenAndFailureAtomicity();
        capacityAndRandomness();
        std::cout << "All tests passed: exhaustive ranks, randomized grids, 64-bit counts,\n"
                     "children, failure atomicity, pool reuse, and allocation-free reservations.\n";
    } catch (const std::exception& error) {
        allocations::failAfter = -1;
        std::cerr << error.what() << '\n';
        return 1;
    }
}
