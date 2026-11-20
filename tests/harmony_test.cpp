// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/granular.hpp"
#include "grainmm/harmony.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <vector>
namespace {
void require(bool value) {
    if (!value)
        throw std::runtime_error("harmony regression");
}
void sequence(grainmm::HarmonyParameters p, const std::vector<int> &notes) {
    grainmm::GrainHarmony harmony;
    harmony.apply(grainmm::prepare_harmony(p));
    for (int note : notes)
        require(std::abs(harmony.next() - std::exp2(static_cast<float>(note) / 12)) < 1e-5F);
}
} // namespace
int main() {
    try {
        sequence({}, {0, 0, 0, 0});
        sequence({0, 3, 0, 2, 1, 0}, {0, 4, 7, 12, 16, 19, 0});
        sequence({0, 4, 1, 1, 1, 0}, {7, 3, 0, 7, 3, 0});
        sequence({-12, 5, 2, 1, 1, 0}, {-12, -7, -5, -7, -12, -7});
        sequence({12, 1, 0, 3, 1, 0}, {12, 24, 36, 12});
        sequence({-24, 3, 0, 3, 0, 0}, {-24, -24, -24, -24});
        grainmm::GrainHarmony a, b;
        const auto prepared = grainmm::prepare_harmony({0, 3, 3, 3, .6F, 50});
        a.apply(prepared);
        b.apply(prepared);
        for (unsigned i = 0; i < 10000; ++i) {
            const float value = a.next();
            require(value == b.next() && value >= .97F && value <= 6.2F);
        }
        a.reset();
        b.reset();
        require(a.next() == b.next());
        auto invalid = prepared;
        invalid.count = 1000;
        invalid.ratios.fill(std::numeric_limits<float>::quiet_NaN());
        a.apply(invalid);
        for (unsigned i = 0; i < 100; ++i)
            require(std::isfinite(a.next()));
        // Applying unrelated controls must not restart a phrase.
        a.apply(grainmm::prepare_harmony({0, 3, 0, 1, 1, 0}));
        require(a.next() == 1);
        a.apply(grainmm::prepare_harmony({0, 3, 0, 1, 1, 0}));
        require(std::abs(a.next() - std::exp2(4.0F / 12)) < 1e-5F);
        std::vector<float> source(4096);
        for (std::size_t i = 0; i < source.size(); ++i)
            source[i] = static_cast<float>(std::sin(static_cast<double>(i) * .1));
        grainmm::Sample sample(48000, 1, std::move(source));
        grainmm::Granular plain(sample, 48000), explicit_off(sample, 48000), whole(sample, 48000),
            split(sample, 48000);
        explicit_off.set_harmony(grainmm::prepare_harmony({}));
        whole.set_harmony(prepared);
        split.set_harmony(prepared);
        std::array<float, 4096> x{}, y{}, w{}, s{};
        plain.render_float(x.data(), 2048, 2, true);
        explicit_off.render_float(y.data(), 2048, 2, true);
        require(x == y);
        whole.render_float(w.data(), 2048, 2, true);
        for (unsigned i = 0; i < 8; ++i)
            split.render_float(s.data() + i * 512, 256, 2, true);
        require(w == s && w != x);
        require(whole.spawned() == plain.spawned() && whole.dropped() == plain.dropped());
        whole.reset();
        whole.render_float(s.data(), 2048, 2, true);
        require(s == w);
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 1;
    }
}
