#include "symbolic/GridCreator.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using Clock = std::chrono::steady_clock;
double millis(Clock::time_point start, Clock::time_point end) {
    return std::chrono::duration<double, std::milli>(end - start).count();
}
struct Sample {
    double constructionMs;
    double reservationMs;
    double queryMs;
    std::uint64_t operations;
    symbolic::MemoryStats memory;
};
volatile std::uint64_t sink = 0;

Sample run(int side, std::uint64_t seed, bool preallocate) {
    const std::size_t cells = static_cast<std::size_t>(side) * static_cast<std::size_t>(side);
    const auto start = Clock::now();
    symbolic::GridCreator grid(side, side, {seed, preallocate ? cells / 3 + static_cast<std::size_t>(side) : 0});
    const auto constructed = Clock::now();
    std::uint64_t operations = 0;
    std::uint64_t checksum = 0;
    // Alternating orientations and lengths, falling back to single cells when
    // fragmented. Fill 75% so queries also exercise a nonempty fragmented grid.
    const std::uint64_t target = cells / 4;
    std::uint64_t remaining = cells;
    while (remaining > target) {
        int length = 1 + static_cast<int>(operations % 8);
        const bool vertical = (operations & 1U) != 0;
        auto segment = vertical ? grid.tryReserveRandomFittingVertical(length)
                                : grid.tryReserveRandomFittingHorizontal(length);
        if (!segment) {
            length = 1;
            segment = vertical ? grid.tryReserveRandomFittingVertical(1)
                               : grid.tryReserveRandomFittingHorizontal(1);
        }
        if (!segment) {
            throw std::runtime_error("Unexpected empty grid");
        }
        remaining -= static_cast<std::uint64_t>(length);
        checksum += static_cast<std::uint64_t>(segment->row) * 1000003U + segment->col;
        ++operations;
    }
    const auto reserved = Clock::now();
    constexpr int queries = 200000;
    for (int i = 0; i < queries; ++i) {
        const int length = 1 + i % 8;
        checksum += grid.countFittingHorizontal(length);
        checksum += grid.countFittingVertical(length);
    }
    const auto queried = Clock::now();
    sink = checksum;
    return {millis(start, constructed), millis(constructed, reserved), millis(reserved, queried),
            operations, grid.memoryStats()};
}
} // namespace

int main(int argc, char** argv) {
    try {
        const int side = argc > 1 ? std::stoi(argv[1]) : 256;
        const int repetitions = argc > 2 ? std::stoi(argv[2]) : 7;
        const bool preallocate = argc > 3 && std::string(argv[3]) == "preallocate";
        if (side < 8 || side > 8192 || repetitions < 1 || repetitions > 1000) {
            throw std::invalid_argument("Usage: symbolic_grid_benchmark [side 8..8192] [repetitions 1..1000] [preallocate]");
        }
        run(side, 1, preallocate); // warmup outside measured samples
        std::vector<Sample> samples;
        for (int i = 0; i < repetitions; ++i) samples.push_back(run(side, 42, preallocate));
        std::sort(samples.begin(), samples.end(), [](const Sample& a, const Sample& b) {
            return a.reservationMs < b.reservationMs;
        });
        const auto median = samples[samples.size() / 2];
        std::cout << std::fixed << std::setprecision(3)
                  << "Grid: " << side << 'x' << side << ", samples: " << repetitions
                  << ", capacity: " << (preallocate ? "preallocated" : "growing") << '\n'
                  << "Median reservation sample; construction: " << median.constructionMs << " ms\n"
                  << median.operations << " mixed reservations: " << median.reservationMs << " ms ("
                  << median.reservationMs * 1000000.0 / static_cast<double>(median.operations) << " ns/reservation)\n"
                  << "400000 count queries: " << median.queryMs << " ms (" << median.queryMs * 2.5 << " ns/query)\n"
                  << "Live segments: " << median.memory.liveSegments << ", pool slots used: " << median.memory.allocatedSlots
                  << ", capacity: " << median.memory.capacity << '\n'
                  << "Node storage: " << median.memory.bytesAllocated << " bytes; checksum: " << sink << '\n';
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
