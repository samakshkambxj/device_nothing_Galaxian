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

#define LOG_TAG "VeynFxSurround"
#include <log/log.h>

#include "DiffSurround.h"

#include <algorithm>
#include <cmath>

namespace veynfx {

void DiffSurround::configure(float sampleRate) {
    mSampleRate = sampleRate;
    int maxSamples = static_cast<int>(sampleRate * MAX_DELAY_MS / 1000.0f) + 1;
    mMidLine.setSize(maxSamples);
    mSideLine.setSize(maxSamples);
    setDelay(mDelayMs);
}

void DiffSurround::setDelay(float ms) {
    mDelayMs = std::clamp(ms, 0.1f, MAX_DELAY_MS);
    float base = mSampleRate * mDelayMs / 1000.0f;
    mDelayMidL = std::max(1, static_cast<int>(base * 0.73f));
    mDelayMidR = std::max(1, static_cast<int>(base * 1.29f));
    mDelaySide1 = std::max(1, static_cast<int>(base));
    ALOGI("setDelay ms=%.1f base=%.0f midL=%d midR=%d side=%d",
          ms, base, mDelayMidL, mDelayMidR, mDelaySide1);
}

void DiffSurround::setWidth(float width) {
    mWidth = std::clamp(width, 0.0f, 1.0f);
    ALOGI("setWidth width=%.2f", mWidth);
}

static int sLogCounter = 0;

void DiffSurround::process(float* buffer, int frames) {
    if (!mEnabled) return;

    const float w         = mWidth;
    const float sideGain  = 1.0f + w * 1.5f;
    const float ambGain   = w * 2.0f;
    const float norm      = 1.0f / (1.0f + w * 2.5f);

    if (++sLogCounter >= 500) {
        float peakIn = 0.0f;
        for (int i = 0; i < std::min(frames * 2, 64); ++i) {
            float a = std::fabs(buffer[i]);
            if (a > peakIn) peakIn = a;
        }
        ALOGI("process: frames=%d w=%.2f sideGain=%.2f ambGain=%.2f norm=%.3f peakIn=%.4f",
              frames, w, sideGain, ambGain, norm, peakIn);
        sLogCounter = 0;
    }

    for (int f = 0; f < frames; f++) {
        float L = buffer[f * 2];
        float R = buffer[f * 2 + 1];

        float mid  = (L + R) * 0.5f;
        float side = (L - R) * 0.5f;

        mMidLine.write(mid);
        mSideLine.write(side);

        float dMidL = mMidLine.read(mDelayMidL);
        float dMidR = mMidLine.read(mDelayMidR);
        float dSide = mSideLine.read(mDelaySide1);

        float wideSide = side * sideGain + dSide * w;
        float amb      = (dMidL - dMidR) * ambGain;

        buffer[f * 2]     = (mid + wideSide + amb) * norm;
        buffer[f * 2 + 1] = (mid - wideSide - amb) * norm;
    }

    if (sLogCounter == 0 && frames > 0) {
        float diffLR = std::fabs(buffer[0] - buffer[1]);
        ALOGI("  output: L=%.4f R=%.4f diff=%.6f", buffer[0], buffer[1], diffLR);
    }
}

void DiffSurround::setEnabled(bool enabled) {
    mEnabled = enabled;
    ALOGI("setEnabled=%d width=%.2f delayMs=%.1f sr=%.0f",
          enabled, mWidth, mDelayMs, mSampleRate);
}

void DiffSurround::reset() {
    mMidLine.reset();
    mSideLine.reset();
}

}  // namespace veynfx
