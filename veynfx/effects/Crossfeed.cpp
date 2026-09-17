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

#include "Crossfeed.h"

#include <algorithm>

namespace veynfx {

void Crossfeed::configure(float sampleRate) {
    mSampleRate = sampleRate;
    updateFilters();
}

void Crossfeed::setLevel(float level) {
    mLevel = std::clamp(level, 0.0f, 1.0f);
}

void Crossfeed::setCutoff(float hz) {
    mCutoff = std::clamp(hz, 200.0f, 2000.0f);
    updateFilters();
}

void Crossfeed::updateFilters() {
    mLpL.configure(BiquadType::LOWPASS, mSampleRate, mCutoff, 0.0f, 0.707f);
    mLpR.configure(BiquadType::LOWPASS, mSampleRate, mCutoff, 0.0f, 0.707f);
}

void Crossfeed::process(float* buffer, int frames) {
    if (!mEnabled) return;

    for (int f = 0; f < frames; f++) {
        float left = buffer[f * 2];
        float right = buffer[f * 2 + 1];

        float crossL = mLpL.process(right) * mLevel;
        float crossR = mLpR.process(left) * mLevel;

        buffer[f * 2] = left * (1.0f - mLevel * 0.5f) + crossL;
        buffer[f * 2 + 1] = right * (1.0f - mLevel * 0.5f) + crossR;
    }
}

void Crossfeed::setEnabled(bool enabled) {
    mEnabled = enabled;
}

void Crossfeed::reset() {
    mLpL.reset();
    mLpR.reset();
}

}  // namespace veynfx
