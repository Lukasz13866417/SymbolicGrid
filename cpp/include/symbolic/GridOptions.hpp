#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>

namespace symbolic {

struct GridOptions {
    // nullopt = take seed from std::random_device.
    std::optional<std::uint64_t> seed;
    // How much memory to allocate for initially existing free segments
    std::size_t initialSegmentCapacity = 0;
};

} // namespace symbolic
