/*
 * Copyright 2025-2026 AxionOS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "effects/FirEqualizer.h"
#include <cmath>
#include <cstring>

namespace veynfx {

static constexpr float FIR_FREQS[FIR_NUM_BANDS] = {
    25, 40, 63, 100, 160, 250, 400, 630,
    1000, 1600, 2500, 4000, 6300, 10000, 16000
};

static BiquadCoeffs peakingEq(float sampleRate, float freq, float Q, float gainDb) {
    float A = std::pow(10.0f, gainDb / 40.0f);
    float w0 = 2.0f * M_PI * freq / sampleRate;
    float sinW = std::sin(w0);
    float cosW = std::cos(w0);
    float alpha = sinW / (2.0f * Q);

    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * cosW;
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * cosW;
    float a2 = 1.0f - alpha / A;

    return { b0/a0, b1/a0, b2/a0, a1/a0, a2/a0 };
}

static inline float processBiquad(BiquadState& s, const BiquadCoeffs& c, float in) {
    float out = c.b0 * in + c.b1 * s.x1 + c.b2 * s.x2 - c.a1 * s.y1 - c.a2 * s.y2;
    s.x2 = s.x1; s.x1 = in;
    s.y2 = s.y1; s.y1 = out;
    return out;
}

FirEqualizer::FirEqualizer()
    : mEnabled(false), mFilterDirty(true), mSampleRate(48000) {
    std::fill(mBandGainDb, mBandGainDb + FIR_NUM_BANDS, 0.0f);
    mCoeffs.resize(FIR_NUM_BANDS);
    mStateL.resize(FIR_NUM_BANDS);
    mStateR.resize(FIR_NUM_BANDS);
    std::memset(mStateL.data(), 0, FIR_NUM_BANDS * sizeof(BiquadState));
    std::memset(mStateR.data(), 0, FIR_NUM_BANDS * sizeof(BiquadState));
}

FirEqualizer::~FirEqualizer() {
}

void FirEqualizer::configure(int sampleRate) {
    mSampleRate = sampleRate;
    mFilterDirty = true;
    reset();
}

void FirEqualizer::setBandGain(int band, float dB) {
    if (band < 0 || band >= FIR_NUM_BANDS) return;
    mBandGainDb[band] = dB;
    mFilterDirty = true;
}

void FirEqualizer::rebuildFilter() {
    for (int i = 0; i < FIR_NUM_BANDS; ++i) {
        mCoeffs[i] = peakingEq(mSampleRate, FIR_FREQS[i], 1.4f, mBandGainDb[i]);
    }
    mFilterDirty = false;
}

void FirEqualizer::process(float* buffer, int frames) {
    if (!mEnabled || frames <= 0) return;
    if (mFilterDirty) rebuildFilter();

    for (int f = 0; f < frames; ++f) {
        float L = buffer[f * 2];
        float R = buffer[f * 2 + 1];

        for (int b = 0; b < FIR_NUM_BANDS; ++b) {
            L = processBiquad(mStateL[b], mCoeffs[b], L);
            R = processBiquad(mStateR[b], mCoeffs[b], R);
        }

        buffer[f * 2] = L;
        buffer[f * 2 + 1] = R;
    }
}

void FirEqualizer::setEnabled(bool enabled) { mEnabled = enabled; }

void FirEqualizer::reset() {
    std::memset(mStateL.data(), 0, FIR_NUM_BANDS * sizeof(BiquadState));
    std::memset(mStateR.data(), 0, FIR_NUM_BANDS * sizeof(BiquadState));
    mFilterDirty = true;
}

}  // namespace veynfx
