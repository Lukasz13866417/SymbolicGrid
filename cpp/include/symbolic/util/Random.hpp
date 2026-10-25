#pragma once

#include <cassert>
#include <cstdint>
#include <optional>

namespace symbolic::detail {

class Random {
public:
    explicit Random(std::optional<std::uint64_t> seed);
    void setSeed(std::uint64_t seed) noexcept { state_ = seed; }

    std::uint64_t below(std::uint64_t bound) noexcept {
        assert(bound != 0);
        const std::uint64_t threshold = (std::uint64_t{0} - bound) % bound;
        for (;;) {
            std::uint64_t value = (state_ += UINT64_C(0x9e3779b97f4a7c15));
            value = (value ^ (value >> 30U)) * UINT64_C(0xbf58476d1ce4e5b9);
            value = (value ^ (value >> 27U)) * UINT64_C(0x94d049bb133111eb);
            value ^= value >> 31U;
            if (value >= threshold) {
                return value % bound;
            }
        }
    }

private:
    std::uint64_t state_;
};

} // namespace symbolic::detail
