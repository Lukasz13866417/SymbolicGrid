#include "symbolic/segments/PartialSegmentHandler.hpp"
#include "detail/Validation.hpp"

#include <iostream>
#include <stdexcept>

namespace symbolic {

PartialSegmentHandler::PartialSegmentHandler(int nRows, int nCols, bool vertical, GridOptions options)
    : index_(nRows, nCols, vertical, options.initialSegmentCapacity), random_(options.seed) {}

bool PartialSegmentHandler::isFree(int row, int col, int length) const {
    validateBounds(row, col, length);
    return findContaining(row, col, length) != 0;
}

void PartialSegmentHandler::reserve(int row, int col, int length) {
    validateBounds(row, col, length);
    if (findContaining(row, col, length) == 0) {
        throw std::invalid_argument("Segment would contain reserved positions");
    }
    ensureExtra(1);
    reserveUnchecked(row, col, length);
}

std::optional<GridSegment> PartialSegmentHandler::tryReserveRandomFitting(int length) {
    const auto total = countFittingSpaces(length);
    if (total == 0) {
        return std::nullopt;
    }
    const auto segment = select(length, randomBelow(total) + 1, total);
    ensureExtra(1);
    reserveUnchecked(segment.row, segment.col, length);
    return segment;
}

GridSegment PartialSegmentHandler::reserveRandomFitting(int length) {
    return detail::requireSpace(tryReserveRandomFitting(length));
}

void PartialSegmentHandler::printGrid() const { printGrid(std::cout); }

} // namespace symbolic
