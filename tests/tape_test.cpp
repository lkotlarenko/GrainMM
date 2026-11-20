// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument.hpp"
#include "grainmm/tape.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
namespace {
void check(bool ok, const char *message) {
    if (!ok)
        throw std::runtime_error(message);
}
std::vector<float> signal(int rate, float hz, float amplitude) {
    std::vector<float> x(static_cast<std::size_t>(rate) * 2);
    for (std::size_t i = 0; i < x.size() / 2; ++i)
        x[2 * i] = x[2 * i + 1] = amplitude * std::sin(6.28318530718F * hz * static_cast<float>(i) /
                                                       static_cast<float>(rate));
    return x;
}
float energy(const std::vector<float> &x) {
    double sum = 0;
    for (std::size_t i = x.size() / 2; i < x.size(); ++i)
        sum += static_cast<double>(x[i]) * x[i];
    return static_cast<float>(sum / static_cast<double>(x.size() / 2));
}
std::vector<float> render(grainmm::TapeParameters p, int rate, float hz, float amplitude) {
    auto x = signal(rate, hz, amplitude);
    grainmm::Tape tape(rate);
    tape.apply(grainmm::prepare_tape(p, rate));
    tape.process(x.data(), x.size() / 2, 2);
    return x;
}
} // namespace
int main() {
    try {
        for (const int rate : {8000, 44100, 48000, 96000}) {
            auto x = signal(rate, 220, 0.1F), a = x, b = x;
            grainmm::Tape tape(rate), split(rate);
            tape.process(a.data(), a.size() / 2, 2);
            check(a == x, "bypass must be exact dry");
            const grainmm::TapeParameters p{18, 3500, 2, 0.25F, 1, -6};
            tape.apply(grainmm::prepare_tape(p, rate));
            split.apply(grainmm::prepare_tape(p, rate));
            tape.process(a.data(), a.size() / 2, 2);
            for (std::size_t at = 0; at < b.size() / 2;) {
                const auto count = std::min<std::size_t>(137, b.size() / 2 - at);
                split.process(b.data() + 2 * at, count, 2);
                at += count;
            }
            check(a == b && a != x, "effect must be block invariant and audible");
            for (std::size_t i = 0; i < a.size(); i += 2)
                check(std::isfinite(a[i]) && std::abs(a[i]) < 1 && a[i] == a[i + 1],
                      "bounded linked stereo");
            tape.reset();
            std::fill(a.begin(), a.end(), 0);
            tape.process(a.data(), a.size() / 2, 2);
            check(std::all_of(a.begin(), a.end(), [](float v) { return v == 0; }),
                  "reset must erase old sound");
            tape.apply(grainmm::prepare_tape({}, rate));
            tape.process(a.data(), a.size() / 2, 2); // Allow bypass fade to settle.
            a = x;
            tape.process(a.data(), a.size() / 2, 2);
            check(a == x, "return to exact bypass");
            auto mono = signal(rate, 310, 0.2F);
            tape.apply(grainmm::prepare_tape(p, rate));
            tape.process(mono.data(), mono.size(), 1);
            check(std::all_of(mono.begin(), mono.end(), [](float v) { return std::isfinite(v); }),
                  "mono extremes finite");
            // Invalid data cannot poison the wet path or later silence.
            mono.assign(4096, std::numeric_limits<float>::quiet_NaN());
            tape.process(mono.data(), mono.size(), 1);
            check(std::all_of(mono.begin(), mono.end(), [](float v) { return std::isfinite(v); }),
                  "nonfinite input containment");
            tape.process(nullptr, 50, 2);
            tape.process(mono.data(), 50, 3);
        }
        grainmm::TapeParameters p;
        p.mix = 1;
        const auto plain = render(p, 48000, 220, 0.1F);
        p.drive_db = 18;
        const float driven_energy = energy(render(p, 48000, 220, 0.1F));
        check(driven_energy > energy(plain) * 5, "drive changes sound");
        p.output_db = -12;
        check(std::abs(energy(render(p, 48000, 220, 0.1F)) / driven_energy - 0.063095734F) < 1e-4F,
              "minus12dB output follows reference power ratio");
        p = {};
        p.mix = 1;
        p.tone_hz = 16000;
        const auto bright = render(p, 48000, 6000, 0.01F);
        p.tone_hz = 500;
        check(energy(render(p, 48000, 6000, 0.01F)) < energy(bright) * 0.03F,
              "tone attenuates high frequencies");
        p = {};
        p.mix = 1;
        p.wow_ms = 1;
        check(render(p, 48000, 220, 0.1F) != plain, "wow changes signal");
        p.wow_ms = 0;
        p.flutter_ms = 0.1F;
        check(render(p, 48000, 220, 0.1F) != plain, "flutter changes signal");
        // Reference DC gain: the odd saturator maps +0.25 to +0.2 at unity drive.
        grainmm::Tape dc(48000);
        p = {};
        p.mix = 1;
        dc.apply(grainmm::prepare_tape(p, 48000));
        std::vector<float> constant(96000, 0.25F);
        for (std::size_t i = 1; i < constant.size(); i += 2)
            constant[i] = 0;
        dc.process(constant.data(), constant.size() / 2, 2);
        check(std::abs(constant[constant.size() - 2] - 0.2F) < 1e-5F && constant.back() == 0,
              "DC transfer reference and no channel leakage");
        grainmm::PreparedTape hostile;
        hostile.values.mix = 1;
        hostile.drive = hostile.tone = hostile.output = std::numeric_limits<float>::quiet_NaN();
        dc.apply(hostile);
        dc.process(constant.data(), constant.size() / 2, 2);
        check(
            std::all_of(constant.begin(), constant.end(), [](float v) { return std::isfinite(v); }),
            "invalid prepared coefficients contained");
        grainmm::TapeParameters bad;
        bad.mix = std::numeric_limits<float>::infinity();
        bad.drive_db = 1000;
        bad.wow_ms = -1;
        const auto safe = grainmm::bounded(bad);
        check(safe.mix == 0 && safe.drive_db == 24 && safe.wow_ms == 0, "parameter bounds");
        bool rejected = false;
        try {
            grainmm::Tape invalid(0);
        } catch (const std::invalid_argument &) {
            rejected = true;
        }
        check(rejected, "invalid rate");
        // Graph stops fully silent and never replays a tape buffer on resume.
        grainmm::Instrument graph(nullptr, 48000);
        grainmm::InstrumentParameters params;
        params.oscillator.level = 0.05F;
        params.tape = {12, 5000, 1, 0.1F, 1, -6};
        graph.set_parameters(params);
        std::vector<std::int16_t> pcm(96000);
        graph.render(pcm.data(), 48000, 2, true);
        check(std::any_of(pcm.begin(), pcm.end(), [](auto v) { return v != 0; }), "live graph");
        graph.render(pcm.data(), 48000, 2, false);
        check(std::all_of(pcm.end() - 48000, pcm.end(), [](auto v) { return v == 0; }), "mute");
        graph.render(pcm.data(), 48000, 2, true);
        check(graph.clipped_samples() == 0, "gentle graph never clips");
        std::cout << "tape numerical and graph regressions passed\n";
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
