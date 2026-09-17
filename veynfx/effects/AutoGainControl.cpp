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

#include "AutoGainControl.h"

#include <algorithm>
#include <cmath>

namespace veynfx {

void AutoGainControl::configure(float sampleRate) {
    mSampleRate = sampleRate;
    setSpeed(500.0f);
}

void AutoGainControl::setTargetLevel(float dB) {
    dB = std::clamp(dB, -30.0f, 0.0f);
    mTargetLevel = std::pow(10.0f, dB / 20.0f);
}

void AutoGainControl::setMaxGain(float dB) {
    dB = std::clamp(dB, 0.0f, 30.0f);
    mMaxGain = std::pow(10.0f, dB / 20.0f);
}

void AutoGainControl::setSpeed(float ms) {
    ms = std::clamp(ms, 10.0f, 5000.0f);
    mAttackCoeff = std::exp(-1.0f / (mSampleRate * ms * 0.25f / 1000.0f));
    mReleaseCoeff = std::exp(-1.0f / (mSampleRate * ms / 1000.0f));
}

void AutoGainControl::process(float* buffer, int frames) {
    if (!mEnabled) return;

    for (int f = 0; f < frames; f++) {
        float left = buffer[f * 2];
        float right = buffer[f * 2 + 1];

        float peak = std::max(std::fabs(left), std::fabs(right));

        float coeff = (peak > mEnvelope) ? mAttackCoeff : mReleaseCoeff;
        mEnvelope = coeff * mEnvelope + (1.0f - coeff) * peak;

        float targetGain = 1.0f;
        if (mEnvelope > 1e-6f) {
            targetGain = mTargetLevel / mEnvelope;
            targetGain = std::clamp(targetGain, 0.1f, mMaxGain);
        }

        float smoothCoeff = mReleaseCoeff;
        mGain = smoothCoeff * mGain + (1.0f - smoothCoeff) * targetGain;

        buffer[f * 2] = left * mGain;
        buffer[f * 2 + 1] = right * mGain;
    }
}

void AutoGainControl::setEnabled(bool enabled) {
    if (enabled && !mEnabled) {
        mEnvelope = mTargetLevel;
        mGain = 1.0f;
    }
    mEnabled = enabled;
}

void AutoGainControl::reset() {
    mEnvelope = mTargetLevel;
    mGain = 1.0f;
}

}  // namespace veynfx
