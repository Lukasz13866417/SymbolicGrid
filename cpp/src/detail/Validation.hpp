#pragma once

#include "symbolic/GridSegment.hpp"

#include <optional>
#include <stdexcept>

namespace symbolic::detail {

inline int positiveDimension(int value) {
    if (value <= 0) {
        throw std::invalid_argument("Grid dimensions must be positive");
    }
    return value;
}

inline void positiveLength(int length) {
    if (length <= 0) {
        throw std::invalid_argument("Segment length must be positive");
    }
}

inline GridSegment requireSpace(std::optional<GridSegment> segment) {
    if (!segment) {
        throw std::out_of_range("No fitting free segment");
    }
    return *segment;
}

}
