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

#include <cstdint>
#include <vector>

namespace veynfx {

static constexpr int FIR_NUM_BANDS = 15;

struct BiquadCoeffs {
    float b0, b1, b2, a1, a2;
};

struct BiquadState {
    float x1, x2, y1, y2;
};

class FirEqualizer {
public:
    FirEqualizer();
    ~FirEqualizer();

    void configure(int sampleRate);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }

    void setBandGain(int band, float dB);
    float getBandGain(int band) const { return (band >= 0 && band < FIR_NUM_BANDS) ? mBandGainDb[band] : 0.0f; }

    void reset();

private:
    void rebuildFilter();

    bool mEnabled;
    bool mFilterDirty;
    int mSampleRate;

    float mBandGainDb[FIR_NUM_BANDS];

    std::vector<BiquadCoeffs> mCoeffs;
    std::vector<BiquadState> mStateL;
    std::vector<BiquadState> mStateR;
};

}  // namespace veynfx
