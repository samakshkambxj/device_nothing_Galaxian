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

#pragma once

#include "../dsp/Biquad.h"
#include <cmath>

namespace veynfx {

static constexpr int MCOMP_BANDS = 4;

struct MCompBand {
    float thresholdDb = -20.0f;
    float ratio = 4.0f;
    float attackMs = 5.0f;
    float releaseMs = 50.0f;
    float makeupDb = 0.0f;

    float envelopeDb = -96.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;
    float makeupLinear = 1.0f;
};

class MultibandCompressor {
public:
    void configure(float sampleRate);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }

    void setBandThreshold(int band, float dB);
    void setBandRatio(int band, float ratio);
    void setBandAttack(int band, float ms);
    void setBandRelease(int band, float ms);
    void setBandMakeup(int band, float dB);
    void setCrossoverFreq(int index, float hz);

    void reset();

private:
    static constexpr int MAX_BLOCK_FRAMES = 4096;

    void updateCoeffs(MCompBand& band);
    float compressGain(const MCompBand& band, float inputDb) const;
    void processBlock(float* buffer, int frames);

    bool mEnabled = false;
    float mSampleRate = 48000.0f;

    float mCrossoverFreqs[MCOMP_BANDS - 1] = {200.0f, 1000.0f, 5000.0f};

    Biquad mLowpassL[MCOMP_BANDS - 1];
    Biquad mLowpassR[MCOMP_BANDS - 1];
    Biquad mHighpassL[MCOMP_BANDS - 1];
    Biquad mHighpassR[MCOMP_BANDS - 1];

    MCompBand mBands[MCOMP_BANDS];

    float mBandBufL[MCOMP_BANDS][MAX_BLOCK_FRAMES];
    float mBandBufR[MCOMP_BANDS][MAX_BLOCK_FRAMES];
};

}  // namespace veynfx
