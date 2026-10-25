#pragma once

#include "symbolic/MemoryStats.hpp"
#include "symbolic/segments/detail/SegmentNode.hpp"

#include <cassert>
#include <cstddef>
#include <vector>

namespace symbolic::detail {

// Contiguous storage with index-based links and an intrusive free list.
// Reserve capacity before mutations; allocate/release then cannot allocate.
class NodePool {
public:
    explicit NodePool(std::size_t initialCapacity);
    NodePool(const NodePool&) = delete;
    NodePool& operator=(const NodePool&) = delete;

    void reserveCapacity(std::size_t segments);
    void ensureExtra(std::size_t live, std::size_t extra);

    NodeId allocate(GridSegment segment) noexcept {
        NodeId id;
        if (freeHead_ != 0) {
            id = freeHead_;
            freeHead_ = nodes_[id].links[treeIndex(TreeKind::Position)].left;
            nodes_[id] = SegmentNode{};
        } else {
            assert(nodes_.size() < nodes_.capacity());
            id = static_cast<NodeId>(nodes_.size());
            nodes_.emplace_back();
        }
        nodes_[id].segment = segment;
        nodes_[id].count = 1;
        nodes_[id].totalLength = static_cast<std::uint64_t>(segment.length);
        nodes_[id].links[treeIndex(TreeKind::Position)].height = nodes_[id].links[treeIndex(TreeKind::Length)].height = 1;
        return id;
    }

    void release(NodeId id) noexcept {
        assert(id != 0);
        nodes_[id] = SegmentNode{};
        nodes_[id].links[treeIndex(TreeKind::Position)].left = freeHead_;
        freeHead_ = id;
    }

    SegmentNode& operator[](NodeId id) noexcept { return nodes_[id]; }
    const SegmentNode& operator[](NodeId id) const noexcept { return nodes_[id]; }
    const std::vector<SegmentNode>& slots() const noexcept { return nodes_; }
    NodeId freeHead() const noexcept { return freeHead_; }
    MemoryStats memoryStats(std::size_t live) const noexcept {
        return {live, nodes_.size() - 1, nodes_.capacity() - 1,
                nodes_.capacity() * sizeof(SegmentNode)};
    }

private:
    std::vector<SegmentNode> nodes_; // Slot 0 is the immutable null sentinel.
    NodeId freeHead_ = 0;
};

} // namespace symbolic::detail
