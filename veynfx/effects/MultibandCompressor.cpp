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

#include "MultibandCompressor.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace veynfx {

static constexpr float DB_FLOOR = -96.0f;

static inline float fastLog2(float x) {
    union { float f; uint32_t i; } u = {x};
    float log2 = static_cast<float>((int)(u.i >> 23) - 127);
    u.i = (u.i & 0x007FFFFFu) | 0x3F800000u;
    log2 += u.f * (u.f * -0.3446825f + 1.3446825f) - 1.0f;
    return log2;
}

static inline float fastExp2(float x) {
    float xi = std::floor(x);
    float xf = x - xi;
    union { uint32_t i; float f; } u;
    u.i = static_cast<uint32_t>((static_cast<int>(xi) + 127) << 23);
    float poly = xf * (xf * 0.3446825f + 0.6553175f) + 1.0f;
    return u.f * poly;
}

static inline float toDb(float linear) {
    return (linear > 1e-10f) ? 6.0205999f * fastLog2(linear) : DB_FLOOR;
}

static inline float fromDb(float dB) {
    return fastExp2(dB * 0.16609640f);
}

void MultibandCompressor::configure(float sampleRate) {
    mSampleRate = sampleRate;
    for (int i = 0; i < MCOMP_BANDS - 1; ++i) {
        mLowpassL[i].configure(BiquadType::LOWPASS, sampleRate, mCrossoverFreqs[i], 0.0f, 0.707f);
        mLowpassR[i].configure(BiquadType::LOWPASS, sampleRate, mCrossoverFreqs[i], 0.0f, 0.707f);
        mHighpassL[i].configure(BiquadType::HIGHPASS, sampleRate, mCrossoverFreqs[i], 0.0f, 0.707f);
        mHighpassR[i].configure(BiquadType::HIGHPASS, sampleRate, mCrossoverFreqs[i], 0.0f, 0.707f);
    }
    for (auto& band : mBands) {
        updateCoeffs(band);
    }
}

void MultibandCompressor::updateCoeffs(MCompBand& band) {
    if (mSampleRate <= 0.0f) return;
    float attackSamples = (band.attackMs / 1000.0f) * mSampleRate;
    float releaseSamples = (band.releaseMs / 1000.0f) * mSampleRate;
    band.attackCoeff = (attackSamples > 0.0f) ? std::exp(-1.0f / attackSamples) : 0.0f;
    band.releaseCoeff = (releaseSamples > 0.0f) ? std::exp(-1.0f / releaseSamples) : 0.0f;
    band.makeupLinear = fromDb(band.makeupDb);
}

float MultibandCompressor::compressGain(const MCompBand& band, float inputDb) const {
    float overshoot = inputDb - band.thresholdDb;
    if (overshoot <= 0.0f) return 0.0f;
    float gainReduction = overshoot * (1.0f - 1.0f / band.ratio);
    return -gainReduction;
}

void MultibandCompressor::process(float* buffer, int frames) {
    if (!mEnabled || frames <= 0) return;

    int offset = 0;
    while (offset < frames) {
        int block = std::min(frames - offset, MAX_BLOCK_FRAMES);
        processBlock(buffer + offset * 2, block);
        offset += block;
    }
}

void MultibandCompressor::processBlock(float* buffer, int frames) {
    for (int f = 0; f < frames; ++f) {
        float l = buffer[f * 2];
        float r = buffer[f * 2 + 1];

        float remainL = l;
        float remainR = r;

        for (int b = 0; b < MCOMP_BANDS - 1; ++b) {
            mBandBufL[b][f] = mLowpassL[b].process(remainL);
            mBandBufR[b][f] = mLowpassR[b].process(remainR);
            remainL = mHighpassL[b].process(remainL);
            remainR = mHighpassR[b].process(remainR);
        }
        mBandBufL[MCOMP_BANDS - 1][f] = remainL;
        mBandBufR[MCOMP_BANDS - 1][f] = remainR;
    }

    for (int b = 0; b < MCOMP_BANDS; ++b) {
        auto& band = mBands[b];
        for (int f = 0; f < frames; ++f) {
            float peakL = std::fabs(mBandBufL[b][f]);
            float peakR = std::fabs(mBandBufR[b][f]);
            float peak = std::max(peakL, peakR);
            float peakDb = toDb(peak);

            float coeff = (peakDb > band.envelopeDb) ? band.attackCoeff : band.releaseCoeff;
            band.envelopeDb = coeff * band.envelopeDb + (1.0f - coeff) * peakDb;

            float gainDb = compressGain(band, band.envelopeDb);
            float gainLinear = fromDb(gainDb) * band.makeupLinear;

            mBandBufL[b][f] *= gainLinear;
            mBandBufR[b][f] *= gainLinear;
        }
    }

    for (int f = 0; f < frames; ++f) {
        float l = 0.0f, r = 0.0f;
        for (int b = 0; b < MCOMP_BANDS; ++b) {
            l += mBandBufL[b][f];
            r += mBandBufR[b][f];
        }
        buffer[f * 2] = l;
        buffer[f * 2 + 1] = r;
    }
}

void MultibandCompressor::setEnabled(bool enabled) { mEnabled = enabled; }

void MultibandCompressor::setBandThreshold(int band, float dB) {
    if (band >= 0 && band < MCOMP_BANDS) {
        mBands[band].thresholdDb = dB;
    }
}

void MultibandCompressor::setBandRatio(int band, float ratio) {
    if (band >= 0 && band < MCOMP_BANDS) {
        mBands[band].ratio = std::max(1.0f, ratio);
    }
}

void MultibandCompressor::setBandAttack(int band, float ms) {
    if (band >= 0 && band < MCOMP_BANDS) {
        mBands[band].attackMs = ms;
        updateCoeffs(mBands[band]);
    }
}

void MultibandCompressor::setBandRelease(int band, float ms) {
    if (band >= 0 && band < MCOMP_BANDS) {
        mBands[band].releaseMs = ms;
        updateCoeffs(mBands[band]);
    }
}

void MultibandCompressor::setBandMakeup(int band, float dB) {
    if (band >= 0 && band < MCOMP_BANDS) {
        mBands[band].makeupDb = dB;
        mBands[band].makeupLinear = fromDb(dB);
    }
}

void MultibandCompressor::setCrossoverFreq(int index, float hz) {
    if (index >= 0 && index < MCOMP_BANDS - 1) {
        mCrossoverFreqs[index] = hz;
        mLowpassL[index].configure(BiquadType::LOWPASS, mSampleRate, hz, 0.0f, 0.707f);
        mLowpassR[index].configure(BiquadType::LOWPASS, mSampleRate, hz, 0.0f, 0.707f);
        mHighpassL[index].configure(BiquadType::HIGHPASS, mSampleRate, hz, 0.0f, 0.707f);
        mHighpassR[index].configure(BiquadType::HIGHPASS, mSampleRate, hz, 0.0f, 0.707f);
    }
}

void MultibandCompressor::reset() {
    for (int i = 0; i < MCOMP_BANDS - 1; ++i) {
        mLowpassL[i].reset();
        mLowpassR[i].reset();
        mHighpassL[i].reset();
        mHighpassR[i].reset();
    }
    for (auto& band : mBands) {
        band.envelopeDb = DB_FLOOR;
    }
}

}  // namespace veynfx
