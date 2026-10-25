#include "symbolic/GridCreator.hpp"
#include "detail/Validation.hpp"

namespace symbolic {

GridCreator::GridCreator(int nRows, int nCols, GridOptions options)
    : rows_(detail::positiveDimension(nRows)), cols_(detail::positiveDimension(nCols)),
      vertical_(rows_, cols_, true, options), horizontal_(rows_, cols_, false, options) {
    if (options.seed) {
        setSeed(*options.seed);
    }
}

GridCreator::GridCreator(int nRows, int nCols, GridCreator& parent, int parentRowOffset, GridOptions options)
    : GridCreator(nRows, nCols, options) {
    if (cols_ != parent.cols_ || parentRowOffset < 0 || parentRowOffset > parent.rows_ ||
        rows_ > parent.rows_ - parentRowOffset) {
        throw std::invalid_argument("Child grid must fit in parent and have the same columns");
    }
    // Check ancestors as well: child indexes intentionally propagate upwards only.
    const GridCreator* ancestor = &parent;
    int firstRow = parentRowOffset + 1;
    while (ancestor != nullptr) {
        for (int c = 0; c < cols_; ++c) {
            if (!ancestor->vertical_.isFree(firstRow, c + 1, rows_)) {
                throw std::invalid_argument("Child area must initially be free in every ancestor");
            }
        }
        firstRow += ancestor->parentRowOffset_;
        ancestor = ancestor->parent_;
    }
    parent_ = &parent;
    parentRowOffset_ = parentRowOffset;
}

void GridCreator::prepare(int row, int col, int length, bool vertical) {
    auto& primary = vertical ? vertical_ : horizontal_;
    auto& perpendicular = vertical ? horizontal_ : vertical_;
    primary.validateBounds(row, col, length);
    if (primary.findContaining(row, col, length) == 0) {
        throw std::invalid_argument("Segment overlaps reserved positions");
    }
    // Verify all allocations and parents before running them.
    primary.ensureExtra(1);
    perpendicular.ensureExtra(static_cast<std::size_t>(length));
    if (parent_) {
        parent_->prepare(row + parentRowOffset_, col, length, vertical);
    }
}

void GridCreator::apply(int row, int col, int length, bool vertical) noexcept {
    if (parent_) {
        parent_->apply(row + parentRowOffset_, col, length, vertical);
    }
    if (vertical) {
        vertical_.reserveUnchecked(row, col, length);
        for (int i = 0; i < length; ++i) horizontal_.reserveUnchecked(row + i, col, 1);
    } else {
        horizontal_.reserveUnchecked(row, col, length);
        for (int i = 0; i < length; ++i) vertical_.reserveUnchecked(row, col + i, 1);
    }
}

void GridCreator::reserveVertical(int row, int col, int length) {
    prepare(row, col, length, true);
    apply(row, col, length, true);
}

void GridCreator::reserveHorizontal(int row, int col, int length) {
    prepare(row, col, length, false);
    apply(row, col, length, false);
}

std::optional<GridSegment> GridCreator::reserveRandom(int length, bool vertical) {
    auto& primary = vertical ? vertical_ : horizontal_;
    const auto total = primary.countFittingSpaces(length);
    if (total == 0) {
        return std::nullopt;
    }
    const auto segment = primary.select(length, primary.randomBelow(total) + 1, total);
    prepare(segment.row, segment.col, length, vertical);
    apply(segment.row, segment.col, length, vertical);
    return segment;
}

std::optional<GridSegment> GridCreator::tryReserveRandomFittingVertical(int length) { return reserveRandom(length, true); }
std::optional<GridSegment> GridCreator::tryReserveRandomFittingHorizontal(int length) { return reserveRandom(length, false); }
GridSegment GridCreator::reserveRandomFittingVertical(int length) { return detail::requireSpace(reserveRandom(length, true)); }
GridSegment GridCreator::reserveRandomFittingHorizontal(int length) { return detail::requireSpace(reserveRandom(length, false)); }
std::uint64_t GridCreator::countFittingVertical(int length) const { return vertical_.countFittingSpaces(length); }
std::uint64_t GridCreator::countFittingHorizontal(int length) const { return horizontal_.countFittingSpaces(length); }
GridSegment GridCreator::getKthFittingVertical(int length, std::uint64_t k) const { return vertical_.getKthFittingSpace(length, k); }
GridSegment GridCreator::getKthFittingHorizontal(int length, std::uint64_t k) const { return horizontal_.getKthFittingSpace(length, k); }
bool GridCreator::isFree(int row, int col) const { return horizontal_.isFree(row, col); }
void GridCreator::setSeed(std::uint64_t seed) noexcept {
    vertical_.setSeed(seed);
    horizontal_.setSeed(seed ^ UINT64_C(0xd1b54a32d192ed03));
}
void GridCreator::reserveCapacity(std::size_t segmentsPerOrientation) {
    vertical_.reserveCapacity(segmentsPerOrientation);
    horizontal_.reserveCapacity(segmentsPerOrientation);
}
MemoryStats GridCreator::memoryStats() const noexcept {
    const auto v = vertical_.memoryStats();
    const auto h = horizontal_.memoryStats();
    return {v.liveSegments + h.liveSegments, v.allocatedSlots + h.allocatedSlots,
            v.capacity + h.capacity, v.bytesAllocated + h.bytesAllocated};
}
std::string GridCreator::toString() const { return horizontal_.toString(); }
void GridCreator::printGrid(std::ostream& out) const { horizontal_.printGrid(out); }
void GridCreator::printGrid() const { horizontal_.printGrid(); }
bool GridCreator::checkInvariants() const { return horizontal_.checkInvariants() && vertical_.checkInvariants(); }

} // namespace symbolic
