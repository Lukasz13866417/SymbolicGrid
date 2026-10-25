#pragma once

#include "symbolic/segments/detail/SegmentIndex.hpp"

#include <algorithm>
#include <cassert>
#include <tuple>

namespace symbolic::detail {

template<TreeKind Kind>
bool SegmentIndex::less(Id a, Id b) const noexcept {
    const auto& x = pool_[a].segment;
    const auto& y = pool_[b].segment;
    if constexpr (Kind == TreeKind::Length) {
        return std::tie(x.length, x.row, x.col) < std::tie(y.length, y.row, y.col);
    } else {
        const int xLine = vertical_ ? x.col : x.row;
        const int yLine = vertical_ ? y.col : y.row;
        const int xEnd = (vertical_ ? x.row : x.col) + (x.length - 1);
        const int yEnd = (vertical_ ? y.row : y.col) + (y.length - 1);
        return xLine < yLine || (xLine == yLine && xEnd < yEnd);
    }
}

template<TreeKind Kind>
void SegmentIndex::update(Id id) noexcept {
    auto& node = pool_[id];
    auto& links = node.links[treeIndex(Kind)];
    links.height = static_cast<std::uint8_t>(1 + std::max(pool_[links.left].links[treeIndex(Kind)].height,
                                                       pool_[links.right].links[treeIndex(Kind)].height));
    if constexpr (Kind == TreeKind::Length) {
        node.count = 1 + pool_[links.left].count + pool_[links.right].count;
        node.totalLength = static_cast<std::uint64_t>(node.segment.length)
                         + pool_[links.left].totalLength + pool_[links.right].totalLength;
    }
}

template<TreeKind Kind>
SegmentIndex::Id SegmentIndex::rotateLeft(Id id) noexcept {
    const Id next = pool_[id].links[treeIndex(Kind)].right;
    pool_[id].links[treeIndex(Kind)].right = pool_[next].links[treeIndex(Kind)].left;
    pool_[next].links[treeIndex(Kind)].left = id;
    update<Kind>(id);
    update<Kind>(next);
    return next;
}

template<TreeKind Kind>
SegmentIndex::Id SegmentIndex::rotateRight(Id id) noexcept {
    const Id next = pool_[id].links[treeIndex(Kind)].left;
    pool_[id].links[treeIndex(Kind)].left = pool_[next].links[treeIndex(Kind)].right;
    pool_[next].links[treeIndex(Kind)].right = id;
    update<Kind>(id);
    update<Kind>(next);
    return next;
}

template<TreeKind Kind>
SegmentIndex::Id SegmentIndex::balance(Id id) noexcept {
    update<Kind>(id);
    auto& links = pool_[id].links[treeIndex(Kind)];
    const int difference = pool_[links.left].links[treeIndex(Kind)].height - pool_[links.right].links[treeIndex(Kind)].height;
    if (difference > 1) {
        const auto left = pool_[links.left].links[treeIndex(Kind)];
        if (pool_[left.left].links[treeIndex(Kind)].height < pool_[left.right].links[treeIndex(Kind)].height) {
            links.left = rotateLeft<Kind>(links.left);
        }
        return rotateRight<Kind>(id);
    }
    if (difference < -1) {
        const auto right = pool_[links.right].links[treeIndex(Kind)];
        if (pool_[right.right].links[treeIndex(Kind)].height < pool_[right.left].links[treeIndex(Kind)].height) {
            links.right = rotateRight<Kind>(links.right);
        }
        return rotateLeft<Kind>(id);
    }
    return id;
}

template<TreeKind Kind>
SegmentIndex::Id SegmentIndex::insert(Id root, Id id) noexcept {
    if (root == 0) {
        return id;
    }
    if (less<Kind>(id, root)) {
        pool_[root].links[treeIndex(Kind)].left = insert<Kind>(pool_[root].links[treeIndex(Kind)].left, id);
    } else {
        pool_[root].links[treeIndex(Kind)].right = insert<Kind>(pool_[root].links[treeIndex(Kind)].right, id);
    }
    return balance<Kind>(root);
}

template<TreeKind Kind>
SegmentIndex::Id SegmentIndex::detachMin(Id root, Id& minimum) noexcept {
    auto& links = pool_[root].links[treeIndex(Kind)];
    if (links.left == 0) {
        minimum = root;
        return links.right;
    }
    links.left = detachMin<Kind>(links.left, minimum);
    return balance<Kind>(root);
}

template<TreeKind Kind>
SegmentIndex::Id SegmentIndex::erase(Id root, Id id) noexcept {
    assert(root != 0);
    auto& links = pool_[root].links[treeIndex(Kind)];
    if (root == id) {
        if (links.left == 0) {
            return links.right;
        }
        if (links.right == 0) {
            return links.left;
        }
        Id replacement = 0;
        const Id right = detachMin<Kind>(links.right, replacement);
        pool_[replacement].links[treeIndex(Kind)].left = links.left;
        pool_[replacement].links[treeIndex(Kind)].right = right;
        return balance<Kind>(replacement);
    }
    if (less<Kind>(id, root)) {
        links.left = erase<Kind>(links.left, id);
    } else {
        links.right = erase<Kind>(links.right, id);
    }
    return balance<Kind>(root);
}

} // namespace symbolic::detail
