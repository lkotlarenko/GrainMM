// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument.hpp"
#include "grainmm/output.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
namespace {
bool guard = false;
unsigned allocations = 0;
int fail(int n) {
    std::fprintf(stderr, "Output regression line%d\n", n);
    return 1;
}
} // namespace
void *operator new(std::size_t n) {
    if (guard)
        ++allocations;
    if (auto *p = std::malloc(n ? n : 1))
        return p;
    throw std::bad_alloc();
}
void *operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
#define CHECK(x)                                                                                   \
    if (!(x))                                                                                      \
    return fail(__LINE__)
int main() {
    using namespace grainmm;
    for (int rate : {8000, 48000, 96000}) {
        Output neutral(rate);
        std::array<float, 512> original{}, one{}, split{};
        for (unsigned i = 0; i < original.size(); ++i)
            original[i] = std::sin(static_cast<float>(i) * .17F) * .8F;
        one = original;
        neutral.process(one.data(), 256, 2);
        CHECK(one == original);
        Output a(rate), b(rate);
        OutputParameters p{18, 1, -3, 100, .2F, 0};
        a.apply(prepare_output(p, rate));
        b.apply(prepare_output(p, rate));
        guard = true;
        for (unsigned block = 0; block < 100; ++block) {
            one = split = original;
            a.process(one.data(), 256, 2);
            b.process(split.data(), 71, 2);
            b.process(split.data() + 142, 185, 2);
            CHECK(one == split);
            for (auto x : one)
                CHECK(std::isfinite(x) && std::abs(x) <= .891251F);
            if (block > static_cast<unsigned>(rate) / 256)
                for (auto x : one)
                    CHECK(std::abs(x) <= .707947F);
        }
        guard = false;
        CHECK(allocations == 0 && a.reduction() > 0);
        p = {6, 0, -1, 200, 0, 0};
        a.apply(prepare_output(p, rate));
        for (unsigned block = 0; block < static_cast<unsigned>(rate) * 3 / 256; ++block) {
            one.fill(.1F);
            a.process(one.data(), 256, 2);
        }
        CHECK(std::abs(one[400] - .199526F) < .0001F);
        p.mono = 1;
        a.apply(prepare_output(p, rate));
        for (unsigned block = 0; block < static_cast<unsigned>(rate) * 3 / 256; ++block) {
            one = original;
            a.process(one.data(), 256, 2);
        }
        for (unsigned i = 0; i < one.size(); i += 2)
            CHECK(one[i] == one[i + 1]);
        a.process(nullptr, 1, 2);
        a.process(one.data(), 256, 0);
        p = {18, 1, 0, 20, -1, 0};
        a.apply(prepare_output(p, rate));
        for (unsigned block = 0; block < static_cast<unsigned>(rate) * 3 / 256; ++block) {
            one.fill(.8F);
            a.process(one.data(), 256, 2);
        }
        CHECK(one[400] > 0 && one[401] == 0 && one[400] <= 1);
        one[0] = std::numeric_limits<float>::quiet_NaN();
        a.process(one.data(), 1, 1);
        CHECK(std::isfinite(one[0]));
    }
    // Extreme finite values previously overflowed during gain/matrix processing,
    // producing NaNs even with limiting enabled. Keep this independent of PCM I/O.
    for (float mono : {0.F, 1.F}) {
        Output guarded(48000);
        guarded.apply(prepare_output({18, 1, 0, 20, 0, mono}, 48000));
        std::array<float, 512> extreme{};
        for (unsigned iteration = 0; iteration < 200; ++iteration) {
            for (unsigned i = 0; i < extreme.size(); ++i)
                extreme[i] = (i % 3 == 0 ? -1.F : 1.F) * std::numeric_limits<float>::max();
            guarded.process(extreme.data(), 256, 2);
            for (auto value : extreme)
                CHECK(std::isfinite(value) && std::abs(value) <= 1.000001F);
        }
    }
    Instrument dry(nullptr, 48000), louder(nullptr, 48000);
    InstrumentParameters p;
    p.oscillator = {.05F, 60, 0, 0, .5F, 0};
    dry.set_parameters(p);
    p.output.gain_db = 6;
    p.output.limiter = 1;
    louder.set_parameters(p);
    std::array<std::int16_t, 1024> d{}, l{};
    double dp = 0, lp = 0;
    for (unsigned block = 0; block < 150; ++block) {
        dry.render(d.data(), 512, 2, true);
        louder.render(l.data(), 512, 2, true);
        if (block > 100)
            for (unsigned i = 0; i < d.size(); ++i) {
                dp += double(d[i]) * d[i];
                lp += double(l[i]) * l[i];
            }
    }
    CHECK(std::abs(std::sqrt(lp / dp) - 1.99526) < .002 && louder.clipped_samples() == 0);
    CHECK(reset_effects(p).output.gain_db == 6 &&
          reset_effects(p, ResetScope::Sound).output.gain_db == 0);
    std::puts("Output: neutral exact, gain, limiting, stereo link, mono, chunking, bounds and rack "
              "integration passed");
}
