#pragma once

#include <cstddef>
#include <cstdint>

namespace symbolic::detail {

enum class TreeKind : std::uint8_t {
    Position = 0, // Ordered by (line, end position).
    Length = 1,   // Ordered by (length, row, column).
};

// Convert only at the boundary to the compact arrays of roots and node links.
constexpr std::size_t treeIndex(TreeKind kind) noexcept {
    return static_cast<std::size_t>(kind);
}

} // namespace symbolic::detail
