#include "symbolic/util/Random.hpp"

#include <random>

namespace symbolic::detail {

Random::Random(std::optional<std::uint64_t> seed) {
    if (seed) {
        state_ = *seed;
    } else {
        std::random_device entropy;
        state_ = (static_cast<std::uint64_t>(entropy()) << 32U) ^ entropy();
    }
}

} // namespace symbolic::detail
