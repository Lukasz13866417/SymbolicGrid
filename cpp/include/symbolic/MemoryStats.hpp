#pragma once

#include <cstddef>

namespace symbolic {

struct MemoryStats {
    std::size_t liveSegments;
    std::size_t allocatedSlots;
    std::size_t capacity;
    std::size_t bytesAllocated;
};

}
