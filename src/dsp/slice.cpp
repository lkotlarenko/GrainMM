// SPDX-License-Identifier: GPL-3.0-only
#include "grainmm/slice.hpp"
#include "grainmm/dsp_utils.hpp"
namespace grainmm {
SliceParameters bounded(SliceParameters p) noexcept {
    p.mode = std::round(dsp::limit(p.mode, 0, 4));
    p.regions = std::round(dsp::limit(p.regions, 2, 16, 8));
    p.division = std::round(dsp::limit(p.division, 0, 3, 2));
    p.gate = dsp::limit(p.gate, .1F, 1, .8F);
    p.variation = std::round(dsp::limit(p.variation, 0, 3));
    p.reverse = dsp::limit(p.reverse, 0, 1);
    return p;
}
GrooveParameters bounded(GrooveParameters p) noexcept {
    p.swing = dsp::limit(p.swing, 0, .65F);
    p.length = std::round(dsp::limit(p.length, 1, 16, 16));
    p.rotate = std::round(dsp::limit(p.rotate, 0, 15));
    p.order = std::round(dsp::limit(p.order, 0, 2));
    p.accent = dsp::limit(p.accent, 0, 1);
    p.accent_every = std::round(dsp::limit(p.accent_every, 1, 16, 4));
    return p;
}
PreparedSlice prepare_slice(SliceParameters p, float bpm, int rate, const SlicePattern &pattern,
                            const GrooveParameters &groove) {
    dsp::checked_rate(rate);
    p = bounded(p);
    bpm = dsp::limit(bpm, 20, 300, 120);
    PreparedSlice result;
    result.values = p;
    result.groove = bounded(groove);
    const auto count = static_cast<unsigned>(p.regions);
    const auto division = 1U << static_cast<unsigned>(p.division);
    result.frames_per_step = static_cast<float>(rate) * 60 / (bpm * static_cast<float>(division));
    if (pattern.manual) {
        result.pattern = bounded(pattern);
        result.cuts = slice_cuts(result.pattern, count);
        return result;
    }
    std::uint32_t seed = 0x53a97U;
    for (unsigned i = 0; i < result.pattern.steps.size(); ++i) {
        auto &s = result.pattern.steps[i];
        s.region = i % count;
        if (p.mode == 2)
            s.region = count - 1 - s.region;
        if (p.mode == 3) {
            const auto at = i % (2 * count - 2);
            s.region = at < count ? at : 2 * count - 2 - at;
        }
        if (p.mode == 4) {
            seed ^= seed << 13;
            seed ^= seed >> 17;
            seed ^= seed << 5;
            s.region = seed % count;
        }
        if (p.variation == 1)
            s.enabled = i % 2 == 0;
        if (p.variation == 2)
            s.reverse = i % 2 == 1;
        if (p.variation == 3)
            s.probability = .75F;
    }
    return result;
}
SlicePlayer::SlicePlayer(const Sample &sample, int rate) : sample_(sample), rate_(rate) {
    dsp::checked_rate(rate);
    target_ = prepare_slice({}, 120, rate);
}
void SlicePlayer::apply(const PreparedSlice &p, float pitch, float gain, float start,
                        float end) noexcept {
    target_ = p;
    target_.values = bounded(p.values);
    target_.groove = bounded(p.groove);
    target_.frames_per_step = dsp::limit(p.frames_per_step, 1, 288000, 6000);
    for (auto &s : target_.pattern.steps) {
        s.region = std::min(s.region, static_cast<unsigned>(target_.values.regions) - 1);
        s.probability = dsp::limit(s.probability, 0, 1);
    }
    target_.cuts[0] = 0;
    for (unsigned i = 1; i < target_.cuts.size(); ++i)
        target_.cuts[i] = dsp::limit(target_.cuts[i], target_.cuts[i - 1], 1, 1);
    pitch_ = dsp::limit(pitch, .25F, 2, 1);
    target_gain_ = dsp::limit(gain, 0, .5F);
    start_ = dsp::limit(start, 0, .99F);
    end_ = dsp::limit(end, start_ + .01F, 1, 1);
}
void SlicePlayer::reset() noexcept {
    voices_ = {};
    step_ = next_voice_ = triggered_ = 0;
    random_ = 1;
    view_ = 0;
    gain_ = 0;
    starting_ = true;
}
float SlicePlayer::random() noexcept {
    random_ ^= random_ << 13;
    random_ ^= random_ >> 17;
    random_ ^= random_ << 5;
    return static_cast<float>(random_ >> 8) / 16777216.0F;
}
void SlicePlayer::trigger(float duration_scale) noexcept {
    const auto length_steps = static_cast<unsigned>(target_.groove.length);
    unsigned at = step_ % length_steps;
    if (target_.groove.order == 1)
        at = length_steps - 1 - at;
    else if (target_.groove.order == 2 && length_steps > 1) {
        const auto bounce = step_ % (2 * length_steps - 2);
        at = bounce < length_steps ? bounce : 2 * length_steps - 2 - bounce;
    }
    at = (at + static_cast<unsigned>(target_.groove.rotate)) % length_steps;
    const bool accent = step_ % static_cast<unsigned>(target_.groove.accent_every) == 0;
    ++step_;
    const auto &s = target_.pattern.steps[at];
    view_ = at | (s.region << 4);
    for (auto &v : voices_)
        if (v.age < v.duration)
            v.fade = static_cast<unsigned>(rate_ / 250);
    if (!s.enabled || (s.probability < 1 && random() >= s.probability))
        return;
    auto &v = voices_[next_voice_++ % 2];
    const auto first =
        std::min(sample_.frames() - 1,
                 static_cast<std::size_t>(start_ * static_cast<double>(sample_.frames())));
    const auto end =
        std::clamp(static_cast<std::size_t>(end_ * static_cast<double>(sample_.frames())),
                   first + 1, sample_.frames());
    const auto length = end - first;
    const auto regions = static_cast<unsigned>(target_.values.regions);
    v.first = std::min(end - 1, first + length * s.region / regions);
    v.end = std::clamp(first + length * (s.region + 1) / regions, v.first + 1, end);
    if (target_.pattern.manual) {
        v.first = std::min(end - 1, first + static_cast<std::size_t>(static_cast<double>(length) *
                                                                     target_.cuts[s.region]));
        v.end = std::clamp(first + static_cast<std::size_t>(static_cast<double>(length) *
                                                            target_.cuts[s.region + 1]),
                           v.first + 1, end);
    }
    v.increment = static_cast<double>(sample_.rate()) / rate_ * pitch_;
    const bool reverse =
        s.reverse != (target_.values.reverse >= 1 ||
                      (target_.values.reverse > 0 && random() < target_.values.reverse));
    v.position = static_cast<double>(reverse ? v.end - 1 : v.first);
    if (reverse)
        v.increment = -v.increment;
    v.duration = static_cast<unsigned>(std::max(
        1.0, std::min(static_cast<double>(target_.frames_per_step * duration_scale *
                                          target_.values.gate),
                      static_cast<double>(v.end - v.first - 1) / std::abs(v.increment) + 1)));
    v.age = v.fade = 0;
    v.velocity = accent ? 1 : 1 - target_.groove.accent;
    view_ |= 1U << 16;
    ++triggered_;
}
void SlicePlayer::render(float *out, std::size_t frames, int channels, bool playing,
                         FrameClock clock) noexcept {
    if (!out || (channels != 1 && channels != 2))
        return;
    if (playing && clock.first()) {
        step_ = 0;
        random_ = 1;
        starting_ = true;
    }
    const float ramp = .5F / (.02F * static_cast<float>(rate_));
    for (std::size_t i = 0; i < frames; ++i) {
        gain_ += std::clamp((playing ? target_gain_ : 0) - gain_, -ramp, ramp);
        const auto division = static_cast<unsigned>(target_.values.division);
        const auto phase = clock.phase() << division;
        std::uint64_t tick =
            (static_cast<std::uint64_t>(clock.beats()) << division) + (phase >> 48);
        if ((tick & 1U) && (phase & (FrameClock::unit - 1)) <
                               static_cast<std::uint64_t>(target_.groove.swing *
                                                          static_cast<double>(FrameClock::unit)))
            --tick;
        if (playing && (starting_ || tick != last_tick_)) {
            starting_ = false;
            last_tick_ = tick;
            trigger(1 + ((tick & 1U) ? -target_.groove.swing : target_.groove.swing));
        }
        float left = 0, right = 0;
        for (auto &v : voices_) {
            if (v.age >= v.duration)
                continue;
            const auto at =
                std::clamp(static_cast<std::size_t>(std::max(0.0, v.position)), v.first, v.end - 1);
            const auto next = std::min(at + 1, v.end - 1);
            const float fraction = static_cast<float>(v.position - static_cast<double>(at));
            const auto edge =
                std::max(1U, std::min(static_cast<unsigned>(rate_ / 250), v.duration / 2));
            float envelope =
                std::min(1.0F, static_cast<float>(std::min(v.age, v.duration - 1 - v.age)) /
                                   static_cast<float>(edge));
            if (v.fade) {
                envelope *= static_cast<float>(v.fade) / static_cast<float>(rate_ / 250);
                if (--v.fade == 0)
                    v.duration = v.age + 1;
            }
            const float a = sample_.value(at, 0), b = sample_.value(at, 1);
            left += (a + (sample_.value(next, 0) - a) * fraction) * envelope * v.velocity;
            right += (b + (sample_.value(next, 1) - b) * fraction) * envelope * v.velocity;
            v.position += v.increment;
            ++v.age;
        }
        const float scale = gain_ * .25F;
        if (channels == 1)
            out[i] = (left + right) * .5F * scale;
        else {
            out[i * 2] = left * scale;
            out[i * 2 + 1] = right * scale;
        }
        if (playing)
            clock.advance();
        else if (gain_ == 0) {
            voices_ = {};
            starting_ = true;
            view_ &= ~(1U << 16);
        }
    }
}
} // namespace grainmm
