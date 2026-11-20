// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument.hpp"
#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
namespace {
constexpr double tau = 6.2831853071795864769;
void check(bool ok, const char *why) {
    if (!ok)
        throw std::runtime_error(why);
}
void finite(const std::vector<float> &v) {
    check(std::all_of(v.begin(), v.end(),
                      [](float x) { return std::isfinite(x) && std::abs(x) < 17; }),
          "finite/bounded output");
}
void resonator_test() {
    for (int rate : {8000, 48000, 96000}) {
        grainmm::Resonator a(rate), b(rate);
        grainmm::ResonatorParameters p{57, 0.1F, 0.5F, 0.7F, 1, 1};
        const auto prepared = grainmm::prepare_resonator(p, rate);
        std::vector<float> x(static_cast<std::size_t>(rate) * 2, 0), y;
        x[0] = 0.5F;
        y = x;
        a.process(x.data(), x.size() / 2, 2);
        check(x == y, "resonator exact bypass");
        a.apply(prepared);
        b.apply(prepared);
        a.process(x.data(), x.size() / 2, 2);
        for (std::size_t at = 0; at < y.size() / 2;) {
            auto n = std::min<std::size_t>(113, y.size() / 2 - at);
            b.apply(prepared); // Repeated snapshots must not restart the retuning fade.
            b.process(y.data() + at * 2, n, 2);
            at += n;
        }
        check(x == y, "resonator block invariance");
        finite(x);
        check(std::any_of(x.begin() + 2, x.end(), [](float v) { return v != 0; }),
              "resonator tail");
        for (std::size_t i = 1; i < x.size(); i += 2)
            check(x[i] == 0, "resonator channel isolation");
        a.reset();
        x.assign(x.size(), 0);
        a.process(x.data(), x.size() / 2, 2);
        check(std::all_of(x.begin(), x.end(), [](float v) { return v == 0; }),
              "resonator reset erases sound");
        for (float note : {36.0F, 84.0F})
            for (float decay : {.05F, 3.0F}) {
                p.note = note;
                p.decay_s = decay;
                p.width = 0;
                a.apply(grainmm::prepare_resonator(p, rate));
                for (int i = 0; i < 6; ++i) {
                    x.assign(x.size(), 0);
                    x[0] = 1;
                    a.process(x.data(), x.size() / 2, 2);
                    finite(x);
                }
            }
        a.apply(grainmm::prepare_resonator({}, rate));
        x.assign(x.size(), 0);
        a.process(x.data(), x.size() / 2, 2);
        x.assign(x.size(), .125F);
        y = x;
        a.process(x.data(), x.size() / 2, 2);
        check(x == y, "resonator returns dry");
        auto bad = prepared;
        bad.modes[0].a = std::numeric_limits<double>::infinity();
        a.apply(bad);
        a.process(x.data(), x.size() / 2, 2);
        finite(x);
    }
    // Compare steady-state gain with the analytical transfer function, not another renderer.
    constexpr int rate = 48000;
    const double omega = tau * 220 / rate;
    grainmm::Resonator a(rate);
    grainmm::ResonatorParameters p;
    p.decay_s = .05F;
    p.mix = 1;
    const auto coefficients = grainmm::prepare_resonator(p, rate);
    a.apply(coefficients);
    const auto z = std::polar(1.0, -omega);
    std::complex<double> response{};
    for (const auto &m : coefficients.modes)
        response += m.gain * (1.0 - z * z) / (1.0 - m.a * z + m.radius2 * z * z);
    std::vector<float> x(rate * 2);
    for (std::size_t i = 0; i < x.size(); ++i)
        x[i] = .1F * static_cast<float>(std::sin(omega * static_cast<double>(i)));
    a.process(x.data(), x.size(), 1);
    double squares = 0;
    for (std::size_t i = rate; i < x.size(); ++i)
        squares += static_cast<double>(x[i]) * x[i];
    const double measured = std::sqrt(squares / rate) * std::sqrt(2.0) / .1;
    check(std::abs(measured - std::abs(response)) < 2e-4, "modal analytic transfer");
}
void rhythm_test() {
    for (int rate : {8000, 48000, 96000}) {
        grainmm::Rhythm a(rate), b(rate);
        std::vector<float> x(static_cast<std::size_t>(rate) * 2, 1), y = x;
        a.process(x.data(), x.size() / 2, 2);
        check(x == y, "rhythm exact bypass");
        grainmm::RhythmParameters p;
        p.tremolo = 1;
        auto prepared = grainmm::prepare_rhythm(p, rate);
        a.apply(prepared);
        b.apply(prepared);
        a.process(x.data(), x.size() / 2, 2);
        for (std::size_t at = 0; at < y.size() / 2;) {
            auto n = std::min<std::size_t>(79, y.size() / 2 - at);
            b.process(y.data() + at * 2, n, 2);
            at += n;
        }
        check(x == y, "rhythm block invariance");
        for (std::size_t i = static_cast<std::size_t>(rate) / 2; i < x.size() / 2; ++i) {
            const double expected = .5 - .5 * std::sin(tau * 2 * static_cast<double>(i) / rate);
            check(std::abs(x[i * 2] - expected) < .003, "tremolo reference sine");
            check(x[i * 2] == x[i * 2 + 1] && x[i * 2] >= 0 && x[i * 2] <= 1,
                  "tremolo linked/no boost");
        }
        p.stereo_degrees = 180;
        a.apply(grainmm::prepare_rhythm(p, rate));
        x.assign(x.size(), 1);
        a.process(x.data(), x.size() / 2, 2);
        for (std::size_t i = x.size() / 2; i < x.size(); i += 2)
            check(std::abs(x[i] + x[i + 1] - 1) < .001, "opposite stereo tremolo");
        p.pan = 1;
        p.division = 3;
        p.bpm = 240;
        p.shape = 1;
        a.apply(grainmm::prepare_rhythm(p, rate));
        x.assign(x.size(), 1);
        a.process(x.data(), x.size() / 2, 2);
        finite(x);
        check(std::any_of(x.begin(), x.end(), [](float v) { return v < .1F; }),
              "rhythm attenuates");
        check(std::any_of(x.begin(), x.end(), [](float v) { return v > .1F; }), "rhythm opens");
        a.apply(grainmm::prepare_rhythm({}, rate));
        x.assign(x.size(), 1);
        a.process(x.data(), x.size() / 2, 2);
        x.assign(x.size(), .25F);
        y = x;
        a.process(x.data(), x.size() / 2, 2);
        check(x == y, "rhythm returns exact dry");
    }
}
void graph_test() {
    grainmm::Instrument graph(nullptr, 48000);
    grainmm::InstrumentParameters p;
    p.oscillator = {.08F, 57, 2, 0, .5F, 0};
    p.resonator = {57, .3F, .5F, .2F, 1, .5F};
    p.rhythm = {120, 1, .7F, .5F, 1, 90};
    graph.set_parameters(p);
    std::vector<std::int16_t> x(96000);
    graph.render(x.data(), 48000, 2, true);
    check(std::any_of(x.begin(), x.end(), [](auto v) { return v != 0; }), "rack graph sounds");
    graph.render(x.data(), 48000, 2, false);
    check(std::all_of(x.end() - 48000, x.end(), [](auto v) { return v == 0; }), "rack graph mutes");
    check(graph.clipped_samples() == 0, "gentle rack no clipping");
}
} // namespace
int main() {
    try {
        resonator_test();
        rhythm_test();
        graph_test();
        std::cout << "modal/rhythm/reference/graph tests passed\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
