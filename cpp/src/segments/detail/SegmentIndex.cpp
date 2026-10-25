#include "symbolic/segments/detail/SegmentIndex.hpp"
#include "detail/Validation.hpp"
#include "AvlOperations.inl"

#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace symbolic::detail {

SegmentIndex::SegmentIndex(int nRows, int nCols, bool vertical, std::size_t initialCapacity)
    : rows_(positiveDimension(nRows)), cols_(positiveDimension(nCols)), vertical_(vertical),
      pool_(std::max(initialCapacity, static_cast<std::size_t>(vertical_ ? cols_ : rows_))) {
    const int lines = vertical_ ? cols_ : rows_;
    for (int i = 0; i < lines; ++i) {
        add(vertical_ ? GridSegment{1, i + 1, rows_} : GridSegment{i + 1, 1, cols_});
    }
}

void SegmentIndex::add(GridSegment segment) noexcept {
    const Id id = pool_.allocate(segment);
    roots_[treeIndex(TreeKind::Position)] = insert<TreeKind::Position>(roots_[treeIndex(TreeKind::Position)], id);
    roots_[treeIndex(TreeKind::Length)] = insert<TreeKind::Length>(roots_[treeIndex(TreeKind::Length)], id);
}

void SegmentIndex::remove(Id id) noexcept {
    roots_[treeIndex(TreeKind::Position)] = erase<TreeKind::Position>(roots_[treeIndex(TreeKind::Position)], id);
    roots_[treeIndex(TreeKind::Length)] = erase<TreeKind::Length>(roots_[treeIndex(TreeKind::Length)], id);
    pool_.release(id);
}

void SegmentIndex::validateBounds(int row, int col, int length) const {
    positiveLength(length);
    if (row < 1 || row > rows_ || col < 1 || col > cols_) {
        throw std::out_of_range("Grid coordinates are out of bounds");
    }
    const int available = vertical_ ? rows_ - row + 1 : cols_ - col + 1;
    if (length > available) {
        throw std::out_of_range("Segment extends beyond the grid");
    }
}

SegmentIndex::Id SegmentIndex::findContaining(int row, int col, int length) const noexcept {
    const int line = vertical_ ? col : row;
    const int start = vertical_ ? row : col;
    Id current = roots_[treeIndex(TreeKind::Position)];
    Id candidate = 0;
    while (current != 0) {
        const auto& node = pool_[current];
        const int nodeLine = vertical_ ? node.segment.col : node.segment.row;
        const int end = (vertical_ ? node.segment.row : node.segment.col) + (node.segment.length - 1);
        if (nodeLine > line || (nodeLine == line && end >= start)) {
            candidate = current;
            current = node.links[treeIndex(TreeKind::Position)].left;
        } else {
            current = node.links[treeIndex(TreeKind::Position)].right;
        }
    }
    if (candidate == 0) {
        return 0;
    }
    const auto segment = pool_[candidate].segment;
    const int candidateLine = vertical_ ? segment.col : segment.row;
    const int candidateStart = vertical_ ? segment.row : segment.col;
    if (candidateLine != line || candidateStart > start) {
        return 0;
    }
    return length <= segment.length - (start - candidateStart) ? candidate : 0;
}

void SegmentIndex::reserveUnchecked(int row, int col, int length) noexcept {
    const Id id = findContaining(row, col, length);
    assert(id != 0);
    const auto old = pool_[id].segment;
    const int before = vertical_ ? row - old.row : col - old.col;
    const int after = old.length - before - length;
    remove(id);
    if (before != 0) {
        add({old.row, old.col, before});
    }
    if (after != 0) {
        add(vertical_ ? GridSegment{row + length, col, after} : GridSegment{row, col + length, after});
    }
}

std::uint64_t SegmentIndex::subtreeSpaces(Id root, int length) const noexcept {
    // Only called for subtrees whose lengths are all >= length.
    return pool_[root].totalLength - static_cast<std::uint64_t>(length - 1) * pool_[root].count;
}

std::uint64_t SegmentIndex::countFittingSpaces(int length) const {
    positiveLength(length);
    std::uint64_t count = 0;
    Id current = roots_[treeIndex(TreeKind::Length)];
    while (current != 0) {
        const auto& node = pool_[current];
        if (node.segment.length >= length) {
            count += static_cast<std::uint64_t>(node.segment.length - length + 1)
                   + subtreeSpaces(node.links[treeIndex(TreeKind::Length)].right, length);
            current = node.links[treeIndex(TreeKind::Length)].left;
        } else {
            current = node.links[treeIndex(TreeKind::Length)].right;
        }
    }
    return count;
}

GridSegment SegmentIndex::select(int length, std::uint64_t k, std::uint64_t total) const noexcept {
    Id current = roots_[treeIndex(TreeKind::Length)];
    while (current != 0) {
        const auto& node = pool_[current];
        if (node.segment.length < length) {
            current = node.links[treeIndex(TreeKind::Length)].right;
            continue;
        }
        const auto here = static_cast<std::uint64_t>(node.segment.length - length + 1);
        const auto left = total - here - subtreeSpaces(node.links[treeIndex(TreeKind::Length)].right, length);
        if (k <= left) {
            total = left;
            current = node.links[treeIndex(TreeKind::Length)].left;
        } else if (k <= left + here) {
            auto result = node.segment;
            const int offset = static_cast<int>(k - left - 1);
            if (vertical_) {
                result.row += offset;
            } else {
                result.col += offset;
            }
            result.length = length;
            return result;
        } else {
            k -= left + here;
            total -= left + here;
            current = node.links[treeIndex(TreeKind::Length)].right;
        }
    }
    assert(false && "Invalid rank (k)");
    return {0, 0, 0};
}

GridSegment SegmentIndex::getKthFittingSpace(int length, std::uint64_t k) const {
    const auto total = countFittingSpaces(length);
    if (k == 0 || k > total) {
        throw std::out_of_range("Rank (k) is out of bounds");
    }
    return select(length, k, total);
}

} // namespace symbolic::detail
