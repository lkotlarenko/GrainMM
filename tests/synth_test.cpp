// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/compressor.hpp"
#include "grainmm/instrument.hpp"
#include "grainmm/oscillator.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>
#include <vector>
namespace {
unsigned allocations = 0;
constexpr double tau = 6.28318530717958647692;
void require(bool ok, const char *why) {
    if (!ok)
        throw std::runtime_error(why);
}
} // namespace
void *operator new(std::size_t n) {
    ++allocations;
    if (void *p = std::malloc(n ? n : 1))
        return p;
    throw std::bad_alloc();
}
void *operator new[](std::size_t n) { return ::operator new(n); }
void operator delete(void *p) noexcept { std::free(p); }
void operator delete[](void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
void operator delete[](void *p, std::size_t) noexcept { std::free(p); }
int main() {
    try {
        for (const int rate : {8000, 44100, 48000, 96000}) {
            grainmm::Oscillator osc(rate), split(rate);
            grainmm::OscillatorParameters p;
            p.level = 0.125F;
            const auto control = grainmm::prepare_oscillator(p, rate);
            osc.apply(control);
            split.apply(control);
            std::vector<float> a(static_cast<std::size_t>(rate) * 2, 0), b(a.size(), 0);
            const auto before = allocations;
            osc.add(a.data(), static_cast<unsigned>(rate), 2, true);
            for (std::size_t at = 0; at < static_cast<unsigned>(rate);) {
                const auto n = std::min<std::size_t>(71, static_cast<unsigned>(rate) - at);
                split.add(b.data() + at * 2, n, 2, true);
                at += n;
            }
            require(allocations == before && a == b, "oscillator allocation/block dependence");
            unsigned crossings = 0;
            double dc = 0, square = 0;
            for (std::size_t i = static_cast<unsigned>(rate) / 5; i < static_cast<unsigned>(rate);
                 ++i) {
                require(a[i * 2] == a[i * 2 + 1] && std::abs(a[i * 2]) <= 0.125F,
                        "oscillator stereo/level bound");
                if (a[i * 2] > 0 && a[(i - 1) * 2] <= 0)
                    ++crossings;
                dc += a[i * 2];
                square += double(a[i * 2]) * a[i * 2];
            }
            require(crossings >= 175 && crossings <= 177, "MIDI57 is not220Hz");
            require(std::abs(dc / rate) < 0.001 && square / rate > 0.005, "sine level/DC");
            for (const float wave : {0.F, 1.F, 2.F, 3.F}) {
                p.wave = wave;
                p.note = 96;
                p.detune_cents = 30;
                p.vibrato_cents = 100;
                p.pulse_width = wave == 3 ? 0.1F : 0.9F;
                osc.apply(grainmm::prepare_oscillator(p, rate));
                a.assign(a.size(), 0);
                osc.add(a.data(), static_cast<unsigned>(rate), 2, true);
                require(
                    std::all_of(a.begin(), a.end(),
                                [](float v) { return std::isfinite(v) && std::abs(v) <= 0.25F; }),
                    "waveform extreme failure");
            }
            a.assign(a.size(), 0);
            osc.add(a.data(), static_cast<unsigned>(rate), 2, false);
            require(a.back() == 0, "oscillator mute did not settle");
        }
        // Compare an exact-period rising saw against a finite, non-aliased Fourier
        // reference. The edge correction should reduce error versus naive sampling.
        grainmm::Oscillator saw(48000);
        auto prepared = grainmm::prepare_oscillator({0.25F, 57, 2, 0, 0.5F, 0}, 48000);
        prepared.left_step = prepared.right_step = 1.0 / 24;
        saw.apply(prepared);
        std::vector<float> warm(24000, 0);
        saw.add(warm.data(), warm.size(), 1, true);
        std::array<float, 24> period{};
        saw.add(period.data(), period.size(), 1, true);
        double corrected_error = 0, naive_error = 0, mean = 0;
        const double alpha = 1 - std::exp(-1.0F / (0.02F * 48000));
        const double initial_step = 220.0 / 48000, target_step = 1.0 / 24;
        const double phase_sum = 24000 * target_step + (initial_step - target_step) * (1 - alpha) *
                                                           (1 - std::pow(1 - alpha, 24000)) / alpha;
        const double offset = phase_sum - std::floor(phase_sum);
        for (unsigned i = 0; i < 24; ++i) {
            const double raw_phase = offset + double(i) / 24;
            const double phase = raw_phase - std::floor(raw_phase);
            double ideal = 0;
            for (unsigned h = 1; h < 12; ++h)
                ideal -= 2 * std::sin(tau * h * phase) / (3.14159265358979323846 * h);
            const double corrected = period[i] / 0.25;
            corrected_error += (corrected - ideal) * (corrected - ideal);
            const double naive = 2 * phase - 1;
            naive_error += (naive - ideal) * (naive - ideal);
            mean += corrected;
        }
        require(corrected_error < naive_error * 0.5 && std::abs(mean) < 0.01,
                "saw edge correction ineffective");
        // Narrow pulse must not inject its duty-cycle DC bias.
        prepared.values.wave = 3;
        prepared.values.pulse_width = 0.25F;
        saw.apply(prepared);
        std::fill(warm.begin(), warm.end(), 0);
        saw.add(warm.data(), warm.size(), 1, true);
        period.fill(0);
        saw.add(period.data(), period.size(), 1, true);
        mean = 0;
        for (float v : period)
            mean += v;
        require(std::abs(mean / 24) < 0.001, "pulse has excessive DC");
        grainmm::Compressor compressor(48000), chunked(48000);
        std::vector<float> pcm(96000, 0.5F);
        const auto dry = pcm;
        compressor.process(pcm.data(), 48000, 2);
        require(pcm == dry, "default compression is not dry");
        grainmm::CompressorParameters cp{-12, 4, 1, 100, 0, 1};
        const auto cc = grainmm::prepare_compressor(cp, 48000);
        compressor.apply(cc);
        chunked.apply(cc);
        auto split_pcm = pcm;
        const auto before = allocations;
        compressor.process(pcm.data(), 48000, 2);
        for (unsigned at = 0; at < 48000;) {
            const auto n = std::min(53U, 48000 - at);
            chunked.process(split_pcm.data() + at * 2, n, 2);
            at += n;
        }
        require(allocations == before && pcm == split_pcm,
                "compressor allocation/block dependence");
        const double input_db = 20 * std::log10(0.5),
                     expected = std::pow(10.0, (-12 + (input_db + 12) / 4) / 20);
        require(std::abs(pcm.back() - expected) < 0.001, "compressor steady gain curve");
        for (std::size_t i = 0; i < pcm.size(); i += 2) {
            pcm[i] = 0.5F;
            pcm[i + 1] = 0.25F;
        }
        compressor.process(pcm.data(), 48000, 2);
        require(std::abs(pcm[pcm.size() - 1] * 2 - pcm[pcm.size() - 2]) < 1e-6F,
                "compressor stereo link");
        cp.ratio = 1;
        compressor.apply(grainmm::prepare_compressor(cp, 48000));
        std::fill(pcm.begin(), pcm.end(), 0.5F);
        compressor.process(pcm.data(), 48000, 2);
        require(std::abs(pcm.back() - 0.5F) < 0.001, "unity ratio changes steady gain");
        // One graph supports no-WAV synthesis, the safe fallback and normal mute.
        grainmm::Instrument engine(static_cast<const grainmm::Sample *>(nullptr), 48000);
        grainmm::InstrumentParameters p;
        p.oscillator.level = 0.1F;
        p.oscillator.note = 69;
        p.delay.mix = 0.2F;
        p.compressor.mix = 0.5F;
        p.resonator.mix = 0.5F;
        p.rhythm.tremolo = 0.5F;
        p.rhythm.pan = 0.5F;
        auto controls = grainmm::prepare_parameters(p, 48000);
        std::array<std::int16_t, 2048> out{};
        auto count = allocations;
        engine.apply(controls);
        engine.render(out.data(), 1024, 2, true);
        require(std::any_of(out.begin(), out.end(), [](auto v) { return v != 0; }),
                "no-WAV synth silent");
        engine.render(out.data(), 1024, 2, false);
        engine.render(out.data(), 1024, 2, false);
        require(std::all_of(out.begin(), out.end(), [](auto v) { return v == 0; }),
                "synth tail/mute leak");
        require(count == allocations, "graph allocated during apply/render");
        p.oscillator.level = 0;
        engine.apply(grainmm::prepare_parameters(p, 48000));
        engine.render(out.data(), 1024, 2, true);
        require(std::any_of(out.begin(), out.end(), [](auto v) { return v != 0; }) &&
                    std::all_of(out.begin(), out.end(),
                                [](auto v) { return std::abs(int(v)) <= 1311; }),
                "safe fallback missing/loud");
        return 0;
    } catch (const std::exception &e) {
        std::fprintf(stderr, "Synth: %s\n", e.what());
        return 1;
    }
}
