// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/delay.hpp"
#include "grainmm/granular.hpp"
#include "grainmm/instrument.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <stdexcept>
#include <vector>
namespace {
std::size_t allocations = 0;
void require(bool ok, const char *message) {
    if (!ok)
        throw std::runtime_error(message);
}
bool near(float a, float b) { return std::abs(a - b) < 1e-5F; }
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
        grainmm::DelayParameters p;
        p.time_ms = 10;
        p.mix = 1;
        p.feedback = 0.5F;
        p.ping_pong = 1;
        grainmm::Delay delay(8000, p);
        std::vector<float> impulse(642, 0);
        impulse[0] = 1;
        auto before = allocations;
        delay.process(impulse.data(), 321, 2);
        require(allocations == before, "delay allocated during render");
        require(impulse[0] == 0 && impulse[158] == 0, "noncausal delay");
        require(near(impulse[160], 1) && impulse[161] == 0, "first impulse timing");
        require(near(impulse[321], 0.5F) && impulse[320] == 0, "ping-pong feedback");
        require(near(impulse[480], 0.25F), "feedback decay");
        std::fill(impulse.begin(), impulse.end(), 0);
        delay.reset();
        delay.process(impulse.data(), 321, 2);
        require(std::all_of(impulse.begin(), impulse.end(), [](float v) { return v == 0; }),
                "reset leaked old delay history");
        p.time_ms = 10.0625F; // 80.5 frames at8kHz.
        p.feedback = 0;
        grainmm::Delay fractional(8000, p);
        impulse[0] = 1;
        fractional.process(impulse.data(), 321, 2);
        require(near(impulse[160], 0.5F) && near(impulse[162], 0.5F),
                "fractional delay interpolation");
        p.time_ms = 10;
        p.spread = 0;
        grainmm::Delay mono_wet(8000, p);
        std::fill(impulse.begin(), impulse.end(), 0);
        impulse[0] = 1;
        mono_wet.process(impulse.data(), 321, 2);
        require(near(impulse[160], 0.5F) && near(impulse[161], 0.5F), "stereo spread");
        p.spread = 1;
        p.ping_pong = 0;
        p.feedback = 0.5F;
        p.lowpass_hz = 500;
        grainmm::Delay damped(8000, p);
        std::fill(impulse.begin(), impulse.end(), 0);
        impulse[0] = 1;
        damped.process(impulse.data(), 321, 2);
        require(impulse[320] > 0 && impulse[320] < 0.5F, "feedback filter inactive");
        const float nan = std::numeric_limits<float>::quiet_NaN();
        const auto safe = grainmm::bounded(grainmm::DelayParameters{nan, nan, nan, nan, nan, nan});
        require(safe.mix == 0 && safe.feedback == 0 && safe.time_ms == 450,
                "nonfinite delay controls");
        for (const int rate : {8000, 44100, 48000, 96000}) {
            grainmm::Delay whole(rate), chunked(rate);
            p = {19.7F, 0.95F, 0.7F, 1, 0.6F, 900};
            whole.set_parameters(p);
            chunked.set_parameters(p);
            std::vector<float> a(static_cast<std::size_t>(rate) * 6);
            for (std::size_t i = 0; i < a.size(); ++i)
                a[i] = static_cast<float>(std::sin(static_cast<double>(i) * 0.1));
            auto b = a;
            before = allocations;
            whole.process(a.data(), a.size() / 2, 2);
            for (std::size_t at = 0; at < b.size() / 2;) {
                const auto count = std::min<std::size_t>(137, b.size() / 2 - at);
                chunked.process(b.data() + at * 2, count, 2);
                at += count;
            }
            require(allocations == before && a == b, "block invariance/allocation regression");
            require(std::all_of(a.begin(), a.end(),
                                [](float v) { return std::isfinite(v) && std::abs(v) <= 4.0001F; }),
                    "unbounded feedback output");
            require(whole.storage_bytes() <= 1536016, "delay memory budget");
        }
        grainmm::Delay dry(48000);
        float invalid[] = {nan, std::numeric_limits<float>::infinity()};
        dry.process(invalid, 1, 2);
        require(invalid[0] == 0 && invalid[1] == 0, "nonfinite input propagated");
        bool failed = false;
        try {
            grainmm::Delay unsupported(0);
        } catch (const std::invalid_argument &) {
            failed = true;
        }
        require(failed, "unsupported rate accepted");
        const grainmm::Sample sample(48000, 1, std::vector<float>(1200, 0.75F));
        grainmm::Granular pcm(sample, 48000), floating(sample, 48000);
        std::vector<std::int16_t> reference(4800);
        std::vector<float> output(4800);
        before = allocations;
        pcm.render(reference.data(), 2400, 2, true);
        floating.render_float(output.data(), 2400, 2, true);
        require(allocations == before, "float granular path allocated");
        grainmm::Instrument graph(sample, 48000);
        grainmm::InstrumentParameters controls;
        controls.delay.mix = 0.8F;
        controls.delay.feedback = 0.95F;
        controls.delay.time_ms = 10;
        graph.set_parameters(controls);
        before = allocations;
        graph.render(reference.data(), 2400, 2, true);
        graph.render(reference.data(), 2400, 2, false);
        require(allocations == before, "instrument allocated in real-time path");
        require(
            std::all_of(reference.begin() + 1922, reference.end(), [](auto v) { return v == 0; }),
            "mute failed to suppress effect tails");
        graph.render(reference.data(), 2400, 2, false);
        require(std::all_of(reference.begin(), reference.end(), [](auto v) { return v == 0; }),
                "paused instrument emitted delayed audio");
        // Compare fresh engines, not the deliberately muted graph.
        grainmm::Granular check_pcm(sample, 48000);
        check_pcm.render(reference.data(), 2400, 2, true);
        for (std::size_t i = 0; i < output.size(); ++i)
            require(std::abs(static_cast<float>(reference[i]) - output[i] * 32767) <= 1.01F,
                    "float render changed grain signal");
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
