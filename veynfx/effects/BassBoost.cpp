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

#include "BassBoost.h"

#include <algorithm>
#include <cmath>

namespace veynfx {

static constexpr float TWO_OVER_PI = 0.6366197723675814f;

void BassBoost::configure(float sampleRate) {
    mSampleRate = sampleRate;
    updateFilters();
}

void BassBoost::setGain(float dB) {
    mGainDb = std::clamp(dB, MIN_GAIN_DB, MAX_GAIN_DB);
    updateFilters();
}

void BassBoost::setFrequency(float hz) {
    mFrequency = std::clamp(hz, 30.0f, 300.0f);
    updateFilters();
}

void BassBoost::setMode(BassMode mode) {
    mMode = mode;
    updateFilters();
}

void BassBoost::updateFilters() {
    mShelfL.configure(BiquadType::LOW_SHELF, mSampleRate, mFrequency, mGainDb, 0.707f);
    mShelfR.configure(BiquadType::LOW_SHELF, mSampleRate, mFrequency, mGainDb, 0.707f);

    if (mMode == BassMode::SUBWOOFER) {
        float subFreq = mFrequency * 0.5f;
        mSubLpL.configure(BiquadType::LOWPASS, mSampleRate, subFreq, 0.0f, 0.707f);
        mSubLpR.configure(BiquadType::LOWPASS, mSampleRate, subFreq, 0.0f, 0.707f);
    }
}

void BassBoost::process(float* buffer, int frames) {
    if (!mEnabled || mGainDb < 0.05f) return;

    float harmonicDrive = 1.0f + mGainDb * 0.2f;

    for (int f = 0; f < frames; f++) {
        float left = buffer[f * 2];
        float right = buffer[f * 2 + 1];

        left = mShelfL.process(left);
        right = mShelfR.process(right);

        switch (mMode) {
            case BassMode::NATURAL:
                break;

            case BassMode::PUNCH: {
                float bassL = left - buffer[f * 2];
                float bassR = right - buffer[f * 2 + 1];
                left += TWO_OVER_PI * std::atan(harmonicDrive * bassL) - bassL;
                right += TWO_OVER_PI * std::atan(harmonicDrive * bassR) - bassR;
                break;
            }

            case BassMode::SUBWOOFER: {
                float subL = mSubLpL.process(left) * 0.5f;
                float subR = mSubLpR.process(right) * 0.5f;
                left += subL;
                right += subR;
                break;
            }
        }

        buffer[f * 2] = left;
        buffer[f * 2 + 1] = right;
    }
}

void BassBoost::setEnabled(bool enabled) {
    mEnabled = enabled;
}

void BassBoost::reset() {
    mShelfL.reset();
    mShelfR.reset();
    mSubLpL.reset();
    mSubLpR.reset();
}

}  // namespace veynfx
