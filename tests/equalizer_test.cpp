// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/equalizer.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>

namespace {
unsigned allocations = 0;
constexpr double pi = 3.14159265358979323846;
void require(bool ok, const char *message) {
    if (!ok)
        throw std::runtime_error(message);
}
double response(const grainmm::BiquadCoefficients &c, double frequency, int rate) {
    const auto z = std::polar(1.0, -2 * pi * frequency / rate);
    return std::abs((double(c.b0) + double(c.b1) * z + double(c.b2) * z * z) /
                    (1.0 + double(c.a1) * z + double(c.a2) * z * z));
}
void settle(grainmm::Equalizer &eq, int rate) {
    float zero = 0;
    for (int i = 0; i < rate / 50; ++i)
        eq.process(&zero, 1, 1);
    eq.reset();
}
} // namespace
void *operator new(std::size_t size) {
    ++allocations;
    if (void *p = std::malloc(size ? size : 1))
        return p;
    throw std::bad_alloc();
}
void *operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
int main() {
    try {
        using namespace grainmm;
        for (const int rate : {8000, 44100, 48000, 96000}) {
            auto bands = default_eq_bands();
            const auto wire = design_equalizer(bands, rate);
            Equalizer eq(rate);
            require(eq.set_coefficients(wire), "identity coefficients refused");
            std::array<float, 64> exact{};
            for (std::size_t i = 0; i < exact.size(); ++i)
                exact[i] = static_cast<float>(i) * 0.001F - 0.03F;
            const auto original = exact;
            eq.process(exact.data(), 32, 2);
            require(exact == original, "flat EQ must be an exact wire");
            bands[0] = {1000, 0.70710678F, 12, EqKind::Bell};
            auto coefficients = design_equalizer(bands, rate);
            require(std::abs(20 * std::log10(response(coefficients[0], 1000, rate)) - 12) < 0.01,
                    "bell center gain does not match dB setting");
            bands[0].kind = EqKind::LowPass;
            coefficients = design_equalizer(bands, rate);
            require(std::abs(response(coefficients[0], 1000, rate) - std::sqrt(0.5)) < 1e-5,
                    "lowpass corner response");
            require(response(coefficients[0], 0, rate) > 0.999 &&
                        response(coefficients[0], rate / 2., rate) < 1e-6,
                    "lowpass endpoints");
            bands[0].kind = EqKind::HighPass;
            coefficients = design_equalizer(bands, rate);
            require(response(coefficients[0], 0, rate) < 1e-6 &&
                        response(coefficients[0], rate / 2., rate) > 0.999,
                    "highpass endpoints");
            for (const auto kind : {EqKind::LowShelf, EqKind::HighShelf}) {
                bands[0].kind = kind;
                coefficients = design_equalizer(bands, rate);
                const auto boost_at = kind == EqKind::LowShelf ? 0. : rate / 2.;
                const auto flat_at = kind == EqKind::LowShelf ? rate / 2. : 0.;
                require(std::abs(20 * std::log10(response(coefficients[0], boost_at, rate)) - 12) <
                                0.01 &&
                            std::abs(20 * std::log10(response(coefficients[0], flat_at, rate))) <
                                0.001,
                        "shelf endpoint gains");
            }
            // Real PCM response, channel isolation and arbitrary block equivalence.
            bands[0] = {1000, 0.9F, 6, EqKind::Bell};
            coefficients = design_equalizer(bands, rate);
            Equalizer whole(rate), split(rate);
            require(whole.set_coefficients(coefficients) && split.set_coefficients(coefficients),
                    "valid EQ update rejected");
            std::array<float, 4096> a{}, b{};
            a[0] = b[0] = 0.5F;
            const auto before = allocations;
            whole.process(a.data(), 2048, 2);
            for (std::size_t at = 0; at < 2048;) {
                const auto count = std::min<std::size_t>(37, 2048 - at);
                split.process(b.data() + at * 2, count, 2);
                at += count;
            }
            require(allocations == before, "EQ allocated during coefficient update/render");
            require(a == b, "EQ/transition changed at block boundaries");
            for (std::size_t i = 1; i < a.size(); i += 2)
                require(a[i] == 0, "left impulse leaked into right channel");
            require(whole.set_coefficients(coefficients), "reset coefficient setup");
            settle(whole, rate);
            double in_energy = 0, out_energy = 0;
            for (int frame = 0; frame < rate; ++frame) {
                float sample = static_cast<float>(0.1 * std::sin(2 * pi * 1000 * frame / rate));
                const auto input = sample;
                whole.process(&sample, 1, 1);
                if (frame > rate / 2) {
                    in_energy += double(input) * input;
                    out_energy += double(sample) * sample;
                }
            }
            require(std::abs(10 * std::log10(out_energy / in_energy) - 6) < 0.03,
                    "processed sine gain");
            whole.reset();
            a.fill(0);
            whole.process(a.data(), 2048, 2);
            require(std::all_of(a.begin(), a.end(), [](float v) { return v == 0; }),
                    "reset leaked filter history");
            // Boundaries must remain finite at each supported output rate.
            for (const auto kind : {EqKind::Bell, EqKind::LowShelf, EqKind::HighShelf,
                                    EqKind::LowPass, EqKind::HighPass})
                for (const float frequency : {25.F, 18000.F})
                    for (const float q : {0.35F, 5.F})
                        for (const float gain : {-20.F, 20.F}) {
                            bands = default_eq_bands();
                            bands[0] = {frequency, q, gain, kind};
                            const auto edge = design_equalizer(bands, rate);
                            if (kind == EqKind::Bell) {
                                const auto actual_frequency =
                                    std::min<double>(frequency, rate * 0.45);
                                require(std::abs(20 * std::log10(response(edge[0], actual_frequency,
                                                                          rate)) -
                                                 gain) < 0.001,
                                        "low-frequency/high-Q coefficient precision");
                            }
                            require(whole.set_coefficients(edge), "edge coefficients rejected");
                            a.fill(0);
                            a[0] = 0.1F;
                            whole.process(a.data(), 2048, 2);
                            require(std::all_of(a.begin(), a.end(),
                                                [](float v) {
                                                    return std::isfinite(v) && std::abs(v) < 1000;
                                                }),
                                    "unstable boundary/ramp");
                        }
        }
        // A newly arrived target must supersede an older pending target even
        // if it arrives exactly when the first bank crossfade finishes.
        auto bands_latest = default_eq_bands();
        Equalizer latest(48000), expected(48000);
        std::array<float, 1920> silence{};
        bands_latest[0].gain_db = 6;
        require(latest.set_coefficients(design_equalizer(bands_latest, 48000)), "initial target");
        bands_latest[0].gain_db = -6;
        require(latest.set_coefficients(design_equalizer(bands_latest, 48000)), "pending target");
        latest.process(silence.data(), 960, 1);
        bands_latest[0].gain_db = 12;
        const auto final_target = design_equalizer(bands_latest, 48000);
        const auto before = allocations;
        require(latest.set_coefficients(final_target) && expected.set_coefficients(final_target),
                "latest target");
        latest.process(silence.data(), 1920, 1);
        expected.process(silence.data(), 1920, 1);
        latest.reset();
        expected.reset();
        std::array<float, 256> latest_pcm{0.1F}, expected_pcm = latest_pcm;
        latest.process(latest_pcm.data(), 256, 1);
        expected.process(expected_pcm.data(), 256, 1);
        require(latest_pcm == expected_pcm, "stale pending target replaced a newer update");
        require(allocations == before, "EQ allocated in realtime methods");
        // Repeating an identical complete snapshot must preserve filter history.
        require(latest.set_coefficients(final_target), "repeat target");
        latest_pcm.fill(0);
        expected_pcm.fill(0);
        latest.process(latest_pcm.data(), 256, 1);
        expected.process(expected_pcm.data(), 256, 1);
        require(latest_pcm == expected_pcm, "identical update restarted the crossfade");
        Equalizer eq(48000), control(48000);
        auto invalid = EqCoefficients{};
        invalid[5].a1 = 2;
        require(!eq.set_coefficients(invalid), "unstable poles accepted");
        invalid = {};
        invalid[5].b0 = std::numeric_limits<float>::quiet_NaN();
        require(!eq.set_coefficients(invalid), "NaN coefficients accepted");
        std::array<float, 8> a{0.5F}, b = a;
        eq.process(a.data(), 8, 1);
        control.process(b.data(), 8, 1);
        require(a == b, "rejected update changed existing target");
        auto bands = default_eq_bands();
        bands[0].frequency = std::numeric_limits<float>::infinity();
        bands[0].gain_db = std::numeric_limits<float>::quiet_NaN();
        require(eq.set_coefficients(design_equalizer(bands, 48000)),
                "nonfinite design input not sanitized");
        a[0] = std::numeric_limits<float>::infinity();
        eq.process(a.data(), 8, 1);
        require(std::all_of(a.begin(), a.end(), [](float v) { return std::isfinite(v); }),
                "nonfinite PCM escaped");
        eq.process(nullptr, 10, 2);
        eq.process(a.data(), 0, 1);
        eq.process(a.data(), 1, 3);
        bool rejected = false;
        try {
            Equalizer bad(7999);
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        require(rejected, "invalid sample rate accepted");
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "EQ: %s\n", error.what());
        return 1;
    }
}
