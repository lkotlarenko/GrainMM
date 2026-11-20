// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/chorus.hpp"
#include "grainmm/crusher.hpp"
#include "grainmm/filter.hpp"
#include "grainmm/instrument_parameters.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <vector>
namespace {
std::size_t allocations = 0;
void require_at(bool ok, int line) {
    if (!ok)
        throw std::runtime_error("texture/gesture regression line" + std::to_string(line));
}
} // namespace
#define check(x) require_at((x), __LINE__)
void *operator new(std::size_t n) {
    ++allocations;
    if (auto *p = std::malloc(n ? n : 1))
        return p;
    throw std::bad_alloc();
}
void operator delete(void *p) noexcept { std::free(p); }
void operator delete(void *p, std::size_t) noexcept { std::free(p); }
namespace {
template <class Effect, class Prepared> void blocks(int rate, Prepared p) {
    Effect whole(rate), split(rate);
    whole.apply(p);
    split.apply(p);
    std::vector<float> a(24000), b;
    for (std::size_t i = 0; i < a.size(); ++i)
        a[i] = static_cast<float>(std::sin(static_cast<double>(i) * .137)) * .3F;
    b = a;
    const auto before = allocations;
    whole.process(a.data(), 12000, 2);
    for (unsigned i = 0; i < 12000; i += 100)
        split.process(b.data() + i * 2, 100, 2);
    check(before == allocations && a == b);
    check(std::all_of(a.begin(), a.end(),
                      [](float x) { return std::isfinite(x) && std::abs(x) < 20; }));
    auto off = p;
    off.values.mix = 0;
    whole.apply(off);
    split.apply(off);
    std::fill(a.begin(), a.end(), .125F);
    b = a;
    whole.process(a.data(), 12000, 2);
    split.process(b.data(), 12000, 2);
    check(a == b);
    for (unsigned repeat = 0; repeat < 4; ++repeat) {
        std::fill(a.begin(), a.end(), .125F);
        whole.process(a.data(), 12000, 2);
    }
    a.assign(a.size(), .125F);
    const auto bytes = a;
    whole.process(a.data(), 12000, 2);
    check(a == bytes); // settled dry bypass
    whole.reset();
    split.reset();
}
} // namespace
int main() {
    try {
        for (int rate : {8000, 48000, 96000}) {
            blocks<grainmm::Crusher>(rate, grainmm::prepare_crusher({4, 16, 6, 3000, 1, -6}, rate));
            blocks<grainmm::Filter>(rate, grainmm::prepare_filter({800, .707F, 0, 0, 1, 0}, rate));
            blocks<grainmm::Chorus>(rate,
                                    grainmm::prepare_chorus({1.2F, 5, 12, .5F, .6F, 1}, rate));
        }
        grainmm::Crusher crusher(48000);
        std::array<float, 96000> constant;
        constant.fill(.3F);
        crusher.apply(grainmm::prepare_crusher({4, 1, 0, 16000, 1, 0}, 48000));
        crusher.process(constant.data(), constant.size(), 1);
        check(std::abs(constant.back() - .25F) < 1e-5F); // nearest signed 4-bit step (1/8)
        grainmm::Filter filter(48000);
        filter.apply(grainmm::prepare_filter({500, .707F, 0, 0, 1, 0}, 48000));
        for (unsigned i = 0; i < constant.size(); ++i)
            constant[i] = static_cast<float>(std::sin(i * 6.283185307 * 8000 / 48000));
        filter.process(constant.data(), constant.size(), 1);
        double energy = 0;
        for (unsigned i = 48000; i < constant.size(); ++i)
            energy += constant[i] * constant[i];
        check(std::sqrt(energy / 48000) < .01); // lowpass rejection independently measured
        grainmm::Chorus echo(48000);
        echo.apply(grainmm::prepare_chorus({1, 0, 2, .5F, 1, 0}, 48000));
        constant.fill(0);
        echo.process(constant.data(), constant.size(), 1);
        echo.reset();
        std::array<float, 512> impulse{};
        impulse[0] = 1;
        echo.process(impulse.data(), impulse.size(), 1);
        check(std::abs(impulse[96] - 1) < 1e-3F && std::abs(impulse[192] - .5F) < 1e-3F &&
              std::abs(impulse[288] - .25F) < 1e-3F);
        grainmm::InstrumentParameters p;
        p.grain.gain = .1F;
        p.harmony.chord = 3;
        p.oscillator.note = 64;
        p.delay.mix = .8F;
        p.crusher.mix = 1;
        p.filter.mix = 1;
        p.chorus.mix = 1;
        p.eq_db[0] = 8;
        const auto dry = grainmm::reset_effects(p);
        check(dry.grain.gain == p.grain.gain && dry.harmony.chord == 3 &&
              dry.oscillator.note == 64 && dry.delay.mix == 0 && dry.crusher.mix == 0 &&
              dry.filter.mix == 0 && dry.chorus.mix == 0 && dry.eq_db[0] == 0);
        p.grain.reverse = .8F;
        p.grain.start = .25F;
        p.grain.end = .75F;
        p.grain.curve = 1;
        p.oscillator.level = .07F;
        p.oscillator.wave = 2;
        p.oscillator.detune_cents = 12;
        p.oscillator.vibrato_cents = 20;
        const auto mod = grainmm::reset_effects(p, grainmm::ResetScope::Modulation);
        check(mod.grain.reverse == 0 && mod.harmony.chord == 0 &&
              mod.oscillator.detune_cents == 0 && mod.oscillator.vibrato_cents == 0);
        check(mod.grain.start == .25F && mod.grain.end == .75F && mod.oscillator.note == 64 &&
              mod.oscillator.wave == 2 && mod.oscillator.level == .07F);
        check(mod.grain.gain == .1F && mod.grain.curve == 0 && mod.grain.width == 1);
        const auto all = grainmm::reset_effects(p, grainmm::ResetScope::Sound);
        check(all.grain.gain == .2F && all.grain.start == 0 && all.grain.end == 1 &&
              all.oscillator.level == 0 && all.oscillator.note == 57);
        return 0;
    } catch (const std::exception &e) {
        std::fprintf(stderr, "%s\n", e.what());
        return 1;
    }
}
