#pragma once

#include "symbolic/GridOptions.hpp"
#include "symbolic/GridSegment.hpp"
#include "symbolic/MemoryStats.hpp"
#include "symbolic/segments/detail/SegmentIndex.hpp"
#include "symbolic/util/Random.hpp"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>

namespace symbolic {

// An independent index for one orientation. Useful when the caller does not
// need to keep horizontal and vertical reservations synchronized.
class PartialSegmentHandler {
public:
    PartialSegmentHandler(int nRows, int nCols, bool vertical, GridOptions options = {});
    PartialSegmentHandler(const PartialSegmentHandler&) = delete;
    PartialSegmentHandler& operator=(const PartialSegmentHandler&) = delete;
    PartialSegmentHandler(PartialSegmentHandler&&) = delete;
    PartialSegmentHandler& operator=(PartialSegmentHandler&&) = delete;

    void reserve(int row, int col, int length);
    GridSegment reserveRandomFitting(int length);
    std::optional<GridSegment> tryReserveRandomFitting(int length);
    bool isFree(int row, int col, int length = 1) const;

    std::uint64_t countFittingSpaces(int length) const { return index_.countFittingSpaces(length); }
    GridSegment getKthFittingSpace(int length, std::uint64_t k) const {
        return index_.getKthFittingSpace(length, k);
    }
    void setSeed(std::uint64_t seed) noexcept { random_.setSeed(seed); }
    void reserveCapacity(std::size_t segments) { index_.reserveCapacity(segments); }
    MemoryStats memoryStats() const noexcept { return index_.memoryStats(); }
    std::string toString() const { return index_.toString(); }
    void printGrid(std::ostream& out) const { index_.printGrid(out); }
    void printGrid() const;
    // Expensive diagnostic for tests/debugging; never called on the hot path.
    bool checkInvariants() const { return index_.checkInvariants(); }

private:
    friend class GridCreator;
    detail::SegmentIndex index_;
    detail::Random random_;

    // Inline hooks let GridCreator preflight both orientations and all parents
    // before applying a reservation, without introducing allocation/indirection.
    void validateBounds(int row, int col, int length) const { index_.validateBounds(row, col, length); }
    detail::NodeId findContaining(int row, int col, int length) const noexcept {
        return index_.findContaining(row, col, length);
    }
    void ensureExtra(std::size_t extra) { index_.ensureExtra(extra); }
    void reserveUnchecked(int row, int col, int length) noexcept { index_.reserveUnchecked(row, col, length); }
    GridSegment select(int length, std::uint64_t k, std::uint64_t total) const noexcept {
        return index_.select(length, k, total);
    }
    std::uint64_t randomBelow(std::uint64_t bound) noexcept { return random_.below(bound); }
};

} // namespace symbolic
