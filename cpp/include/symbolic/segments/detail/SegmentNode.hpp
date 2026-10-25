#pragma once

#include "symbolic/GridSegment.hpp"
#include "symbolic/segments/detail/TreeKind.hpp"

#include <cstdint>

namespace symbolic::detail {

using NodeId = std::uint32_t;

struct AvlLinks {
    NodeId left = 0;
    NodeId right = 0;
    std::uint8_t height = 0;
};

// One free run participates in both indexes without duplicating its payload.
struct SegmentNode {
    std::uint64_t totalLength = 0;
    GridSegment segment{0, 0, 0};
    NodeId count = 0;
    AvlLinks links[2]; // Indexed by treeIndex(TreeKind).
};

static_assert(sizeof(SegmentNode) <= 48, "Keep the pooled node compact");

} // namespace symbolic::detail
