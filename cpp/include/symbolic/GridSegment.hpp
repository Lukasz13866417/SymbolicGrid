#pragma once

#include <iosfwd>

namespace symbolic {

struct GridSegment {
    int row;
    int col;
    int length;

    static constexpr GridSegment GS(int row, int col, int length) noexcept {
        return {row, col, length};
    }
    friend constexpr bool operator==(GridSegment a, GridSegment b) noexcept {
        return a.row == b.row && a.col == b.col && a.length == b.length;
    }
    friend constexpr bool operator!=(GridSegment a, GridSegment b) noexcept { return !(a == b); }
};

std::ostream& operator<<(std::ostream& out, GridSegment segment);

} // namespace symbolic
