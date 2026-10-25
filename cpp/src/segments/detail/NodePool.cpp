#include "symbolic/segments/detail/NodePool.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace symbolic::detail {

NodePool::NodePool(std::size_t initialCapacity) {
    reserveCapacity(initialCapacity);
    nodes_.emplace_back();
}

void NodePool::reserveCapacity(std::size_t segments) {
    // An index and the subtree segment count both fit in uint32_t.
    const auto maxSlots = std::min(static_cast<std::size_t>(std::numeric_limits<NodeId>::max()),
                                   nodes_.max_size());
    if (segments >= maxSlots) {
        throw std::length_error("Segment pool exceeds its index range");
    }
    if (segments + 1 > nodes_.capacity()) {
        nodes_.reserve(segments + 1);
    }
}

void NodePool::ensureExtra(std::size_t live, std::size_t extra) {
    const auto maxSlots = std::min(static_cast<std::size_t>(std::numeric_limits<NodeId>::max()),
                                   nodes_.max_size());
    if (extra >= maxSlots - live) {
        throw std::length_error("Segment pool exceeds its index range");
    }
    const auto required = live + extra + 1;
    if (required > nodes_.capacity()) {
        const auto doubled = nodes_.capacity() > maxSlots / 2 ? maxSlots : nodes_.capacity() * 2;
        nodes_.reserve(std::max(required, doubled));
    }
}

} // namespace symbolic::detail
