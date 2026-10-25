#include "symbolic/GridSegment.hpp"

#include <ostream>

namespace symbolic {

std::ostream& operator<<(std::ostream& out, GridSegment segment) {
    return out << "(r" << segment.row << ", c" << segment.col << ", l" << segment.length << ')';
}

} // namespace symbolic
