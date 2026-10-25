#include "symbolic/segments/detail/SegmentIndex.hpp"

#include <algorithm>
#include <ostream>
#include <stdexcept>

namespace symbolic::detail {

void SegmentIndex::renderLine(Id root, int line, std::string& output) const {
    if (root == 0) {
        return;
    }
    const auto& node = pool_[root];
    const int nodeLine = vertical_ ? node.segment.col : node.segment.row;
    if (line <= nodeLine) {
        renderLine(node.links[treeIndex(TreeKind::Position)].left, line, output);
    }
    if (line == nodeLine) {
        const int start = vertical_ ? node.segment.row : node.segment.col;
        std::fill_n(output.begin() + (start - 1), node.segment.length, '.');
    }
    if (line >= nodeLine) {
        renderLine(node.links[treeIndex(TreeKind::Position)].right, line, output);
    }
}

std::string SegmentIndex::toString() const {
    std::string output;
    const auto stride = static_cast<std::uint64_t>(cols_) + 1;
    const auto total = stride * static_cast<std::uint64_t>(rows_);
    if (total > output.max_size()) {
        throw std::length_error("Grid too large to print");
    }
    output.assign(static_cast<std::size_t>(total), '#');
    const auto rowStride = static_cast<std::size_t>(stride);
    for (int row = 0; row < rows_; ++row)
        output[static_cast<std::size_t>(row) * rowStride + static_cast<std::size_t>(cols_)] = '\n';
    // Iterating slots is cache-friendly and skips recycled nodes (count == 0).
    for (const auto& node : pool_.slots()) {
        if (node.count == 0) {
            continue;
        }
        const auto segment = node.segment;
        auto offset = static_cast<std::size_t>(segment.row - 1) * rowStride + static_cast<std::size_t>(segment.col - 1);
        const std::size_t step = vertical_ ? rowStride : 1;
        for (int i = 0; i < segment.length; ++i) {
            output[offset] = '.';
            if (i != segment.length - 1) {
                offset += step;
            }
        }
    }
    return output;
}

void SegmentIndex::printGrid(std::ostream& out) const {
    if (vertical_) {
        out << toString();
        return;
    }
    std::string row(static_cast<std::size_t>(cols_), '#');
    for (int i = 0; i < rows_; ++i) {
        std::fill(row.begin(), row.end(), '#');
        renderLine(roots_[treeIndex(TreeKind::Position)], i + 1, row);
        out << row << '\n';
    }
}

} // namespace symbolic::detail
