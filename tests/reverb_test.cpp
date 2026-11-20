// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/instrument.hpp"
#include "grainmm/reverb.hpp"
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
void require(bool ok, const char *why) {
    if (!ok)
        throw std::runtime_error(why);
}
void warm(grainmm::Reverb &reverb, int rate) {
    std::array<float, 256> zero{};
    for (int i = 0; i < rate / 2; i += 128) {
        zero.fill(0);
        reverb.process(zero.data(), 128, 2);
    }
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
            grainmm::Reverb a(rate), b(rate);
            require(a.storage_bytes() < 600000, "reverb storage exceeded budget");
            std::array<float, 64> dry{};
            dry[0] = 0.4F;
            const auto saved = dry;
            a.process(dry.data(), 32, 2);
            require(dry == saved, "default reverb is not dry");
            grainmm::ReverbParameters p;
            p.mix = 1;
            p.decay_s = 0.6F;
            p.width = 0;
            const auto prepared = grainmm::prepare_reverb(p, rate);
            a.apply(prepared);
            b.apply(prepared);
            warm(a, rate);
            warm(b, rate);
            std::vector<float> impulse(static_cast<std::size_t>(rate) * 4, 0);
            impulse[0] = 0.5F;
            auto split = impulse;
            const auto before = allocations;
            a.process(impulse.data(), static_cast<std::size_t>(rate) * 2, 2);
            for (std::size_t at = 0; at < static_cast<std::size_t>(rate) * 2;) {
                const auto n = std::min<std::size_t>(37, static_cast<std::size_t>(rate) * 2 - at);
                b.process(split.data() + at * 2, n, 2);
                at += n;
            }
            require(before == allocations, "reverb allocated in process");
            require(impulse == split, "reverb depends on block boundaries");
            double early = 0, late = 0;
            bool heard = false;
            for (std::size_t i = 0; i < impulse.size(); i += 2) {
                require(std::isfinite(impulse[i]) && std::abs(impulse[i]) < 1, "unstable impulse");
                require(impulse[i] == impulse[i + 1], "width zero is not mono");
                if (i / 2 < static_cast<unsigned>(rate) / 25)
                    require(impulse[i] == 0, "reverb arrived before predelay/comb");
                heard = heard || impulse[i] != 0;
                if (i / 2 < static_cast<unsigned>(rate))
                    early += double(impulse[i]) * impulse[i];
                else
                    late += double(impulse[i]) * impulse[i];
            }
            require(heard && early > 0 && late < early * 0.01,
                    "reverb tail missing or failed to decay");
            a.reset();
            std::fill(impulse.begin(), impulse.end(), 0);
            a.process(impulse.data(), static_cast<unsigned>(rate) * 2, 2);
            require(std::all_of(impulse.begin(), impulse.end(), [](float v) { return v == 0; }),
                    "reset leaked tail");
            p.mix = 1;
            p.size = 1;
            p.decay_s = 20;
            p.damp_hz = 20000;
            p.predelay_ms = 250;
            p.width = 1;
            a.apply(grainmm::prepare_reverb(p, rate));
            impulse[0] = std::numeric_limits<float>::infinity();
            const auto changes = allocations;
            for (unsigned round = 0; round < 4; ++round) {
                a.process(impulse.data(), static_cast<unsigned>(rate) * 2, 2);
                require(std::all_of(impulse.begin(), impulse.end(),
                                    [](float v) { return std::isfinite(v); }),
                        "nonfinite output");
            }
            a.reset();
            a.apply(prepared);
            require(changes == allocations, "reverb update/reset allocated");
        }
        // Prepared combined graph produces a genuine changed sound and mutes tails.
        grainmm::Sample sample(48000, 1, std::vector<float>(48000, 0.4F));
        grainmm::Instrument engine(sample, 48000);
        grainmm::InstrumentParameters p;
        p.eq_db[0] = -12;
        p.reverb.mix = 0.4F;
        p.delay.mix = 0.2F;
        const auto prepared = grainmm::prepare_parameters(p, 48000);
        std::array<std::int16_t, 2048> pcm{};
        const auto before = allocations;
        engine.apply(prepared);
        engine.render(pcm.data(), 1024, 2, true);
        require(std::any_of(pcm.begin(), pcm.end(), [](auto v) { return v != 0; }),
                "combined graph silent");
        engine.render(pcm.data(), 1024, 2, false);
        engine.render(pcm.data(), 1024, 2, false);
        require(std::all_of(pcm.begin(), pcm.end(), [](auto v) { return v == 0; }),
                "effects mute leaked tail");
        require(before == allocations, "prepared graph allocated during apply/render");
        return 0;
    } catch (const std::exception &error) {
        std::fprintf(stderr, "Reverb: %s\n", error.what());
        return 1;
    }
}
