#pragma once

#include "symbolic/GridSegment.hpp"
#include "symbolic/MemoryStats.hpp"
#include "symbolic/segments/detail/NodePool.hpp"
#include "symbolic/segments/detail/TreeKind.hpp"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>

namespace symbolic::detail {

// Two AVL indexes over one pool: position lookup and weighted length selection.
// This class maintains free runs; the handler supplies reservation policy/RNG.
class SegmentIndex {
public:
    using Id = NodeId;

    SegmentIndex(int nRows, int nCols, bool vertical, std::size_t initialCapacity);
    void validateBounds(int row, int col, int length) const;
    Id findContaining(int row, int col, int length) const noexcept;
    // Caller must first validate that the segment is free and ensure capacity.
    void reserveUnchecked(int row, int col, int length) noexcept;
    std::uint64_t countFittingSpaces(int length) const;
    GridSegment getKthFittingSpace(int length, std::uint64_t k) const;
    GridSegment select(int length, std::uint64_t k, std::uint64_t total) const noexcept;

    void reserveCapacity(std::size_t segments) { pool_.reserveCapacity(segments); }
    void ensureExtra(std::size_t extra) { pool_.ensureExtra(pool_[roots_[treeIndex(TreeKind::Length)]].count, extra); }
    MemoryStats memoryStats() const noexcept { return pool_.memoryStats(pool_[roots_[treeIndex(TreeKind::Length)]].count); }

    std::string toString() const;
    void printGrid(std::ostream& out) const;
    bool checkInvariants() const;

private:
    int rows_;
    int cols_;
    bool vertical_;
    NodePool pool_;
    Id roots_[2]{0, 0};

    template<TreeKind Kind> bool less(Id a, Id b) const noexcept;
    template<TreeKind Kind> void update(Id id) noexcept;
    template<TreeKind Kind> Id rotateLeft(Id id) noexcept;
    template<TreeKind Kind> Id rotateRight(Id id) noexcept;
    template<TreeKind Kind> Id balance(Id id) noexcept;
    template<TreeKind Kind> Id insert(Id root, Id id) noexcept;
    template<TreeKind Kind> Id detachMin(Id root, Id& minimum) noexcept;
    template<TreeKind Kind> Id erase(Id root, Id id) noexcept;
    void add(GridSegment segment) noexcept;
    void remove(Id id) noexcept;
    std::uint64_t subtreeSpaces(Id root, int length) const noexcept;
    void renderLine(Id root, int line, std::string& output) const;
};

} // namespace symbolic::detail
