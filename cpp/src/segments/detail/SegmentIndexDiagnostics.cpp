#include "symbolic/segments/detail/SegmentIndex.hpp"
#include "AvlOperations.inl"

#include <algorithm>
#include <exception>
#include <functional>
#include <initializer_list>
#include <vector>

namespace symbolic::detail {

bool SegmentIndex::checkInvariants() const {
    std::vector<unsigned char> seen(pool_.slots().size(), 0);
    bool valid = true;
    for (const TreeKind kind : {TreeKind::Position, TreeKind::Length}) {
        Id previous = 0;
        const auto bit = static_cast<unsigned char>(1U << treeIndex(kind));
        std::function<void(Id)> visit = [&](Id id) {
            if (id == 0 || !valid) {
                return;
            }
            if (id >= pool_.slots().size() || (seen[id] & bit) != 0) {
                valid = false;
                return;
            }
            seen[id] |= bit;
            const auto& node = pool_[id];
            const auto links = node.links[treeIndex(kind)];
            visit(links.left);
            if (!valid) {
                return;
            }
            if (previous != 0 && !(kind == TreeKind::Position
                    ? less<TreeKind::Position>(previous, id) : less<TreeKind::Length>(previous, id))) {
                valid = false;
                return;
            }
            if (kind == TreeKind::Position) {
                const auto segment = node.segment;
                try { validateBounds(segment.row, segment.col, segment.length); }
                catch (const std::exception&) { valid = false; return; }
                if (previous != 0) {
                    const auto before = pool_[previous].segment;
                    const int line = vertical_ ? segment.col : segment.row;
                    const int prevLine = vertical_ ? before.col : before.row;
                    const int start = vertical_ ? segment.row : segment.col;
                    const int prevStart = vertical_ ? before.row : before.col;
                    // Maximal free segments cannot overlap or be adjacent.
                    if (line == prevLine && static_cast<std::int64_t>(prevStart) + before.length >= start) {
                        valid = false;
                    }
                }
            }
            previous = id;
            visit(links.right);
            if (!valid) {
                return;
            }
            const int leftHeight = pool_[links.left].links[treeIndex(kind)].height;
            const int rightHeight = pool_[links.right].links[treeIndex(kind)].height;
            if (leftHeight - rightHeight > 1 || rightHeight - leftHeight > 1 ||
                links.height != 1 + std::max(leftHeight, rightHeight)) {
                valid = false;
            }
            if (kind == TreeKind::Length &&
                (node.count != 1 + pool_[links.left].count + pool_[links.right].count ||
                 node.totalLength != static_cast<std::uint64_t>(node.segment.length)
                     + pool_[links.left].totalLength + pool_[links.right].totalLength)) {
                valid = false;
            }
        };
        visit(roots_[treeIndex(kind)]);
    }
    Id free = pool_.freeHead();
    while (valid && free != 0) {
        if (free >= pool_.slots().size() || seen[free] != 0 || pool_[free].count != 0) {
            return false;
        }
        seen[free] = 4;
        free = pool_[free].links[treeIndex(TreeKind::Position)].left;
    }
    for (std::size_t i = 1; i < seen.size(); ++i) {
        if (seen[i] != 3 && seen[i] != 4) {
            return false;
        }
    }
    return valid;
}

} // namespace symbolic::detail
