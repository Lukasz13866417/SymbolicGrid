#pragma once

#include "symbolic/GridOptions.hpp"
#include "symbolic/GridSegment.hpp"
#include "symbolic/MemoryStats.hpp"
#include "symbolic/segments/PartialSegmentHandler.hpp"

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>

namespace symbolic {

class GridCreator {
public:
    GridCreator(int nRows, int nCols, GridOptions options = {});
    // The parent must outlive the child. Offset counts rows BEFORE the child.
    GridCreator(int nRows, int nCols, GridCreator& parent, int parentRowOffset,
                GridOptions options = {});
    GridCreator(const GridCreator&) = delete;
    GridCreator& operator=(const GridCreator&) = delete;
    GridCreator(GridCreator&&) = delete;
    GridCreator& operator=(GridCreator&&) = delete;

    void reserveVertical(int row, int col, int length);
    void reserveHorizontal(int row, int col, int length);
    GridSegment reserveRandomFittingVertical(int length);
    GridSegment reserveRandomFittingHorizontal(int length);
    std::optional<GridSegment> tryReserveRandomFittingVertical(int length);
    std::optional<GridSegment> tryReserveRandomFittingHorizontal(int length);
    std::uint64_t countFittingVertical(int length) const;
    std::uint64_t countFittingHorizontal(int length) const;
    GridSegment getKthFittingVertical(int length, std::uint64_t k) const;
    GridSegment getKthFittingHorizontal(int length, std::uint64_t k) const;
    bool isFree(int row, int col) const;
    int rows() const noexcept { return rows_; }
    int cols() const noexcept { return cols_; }
    void setSeed(std::uint64_t seed) noexcept;
    void reserveCapacity(std::size_t segmentsPerOrientation);
    MemoryStats memoryStats() const noexcept;
    std::string toString() const;
    void printGrid(std::ostream& out) const;
    void printGrid() const;
    bool checkInvariants() const;

private:
    int rows_;
    int cols_;
    PartialSegmentHandler vertical_;
    PartialSegmentHandler horizontal_;
    GridCreator* parent_ = nullptr;
    int parentRowOffset_ = 0;

    void prepare(int row, int col, int length, bool vertical);
    void apply(int row, int col, int length, bool vertical) noexcept;
    std::optional<GridSegment> reserveRandom(int length, bool vertical);
};

} // namespace symbolic
