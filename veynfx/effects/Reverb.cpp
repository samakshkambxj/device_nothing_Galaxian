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

#include "Reverb.h"

#include <algorithm>
#include <cmath>

namespace veynfx {

static constexpr int kCombTuningsL[8] = {1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617};
static constexpr int kCombTuningsR[8] = {1139, 1211, 1300, 1379, 1445, 1514, 1580, 1640};
static constexpr int kAllpassTuningsL[4] = {556, 441, 341, 225};
static constexpr int kAllpassTuningsR[4] = {579, 464, 364, 248};

static constexpr float SCALE_ROOM = 0.28f;
static constexpr float OFFSET_ROOM = 0.7f;
static constexpr float SCALE_DAMP = 0.4f;
static constexpr float SCALE_WET = 3.0f;
static constexpr float FIXED_GAIN = 0.015f;
static constexpr float ALLPASS_FEEDBACK = 0.5f;

static int scaleTuning(int tuning, float sampleRate) {
    return static_cast<int>(tuning * sampleRate / 44100.0f);
}

void Reverb::configure(float sampleRate) {
    mSampleRate = sampleRate;

    for (int i = 0; i < NUM_COMBS; i++) {
        mCombL[i].setSize(scaleTuning(kCombTuningsL[i], sampleRate));
        mCombR[i].setSize(scaleTuning(kCombTuningsR[i], sampleRate));
    }

    for (int i = 0; i < NUM_ALLPASSES; i++) {
        mAllpassL[i].setSize(scaleTuning(kAllpassTuningsL[i], sampleRate));
        mAllpassR[i].setSize(scaleTuning(kAllpassTuningsR[i], sampleRate));
    }

    updateParams();
}

void Reverb::setRoomSize(float size) {
    mRoomSize = std::clamp(size, 0.0f, 1.0f);
    updateParams();
}

void Reverb::setDamping(float damp) {
    mDamping = std::clamp(damp, 0.0f, 1.0f);
    updateParams();
}

void Reverb::setWetLevel(float wet) {
    mWetLevel = std::clamp(wet, 0.0f, 1.0f);
    updateParams();
}

void Reverb::setDryLevel(float dry) {
    mDryLevel = std::clamp(dry, 0.0f, 1.0f);
    updateParams();
}

void Reverb::setWidth(float width) {
    mWidth = std::clamp(width, 0.0f, 1.0f);
    updateParams();
}

void Reverb::updateParams() {
    mFeedback = mRoomSize * SCALE_ROOM + OFFSET_ROOM;
    mDamp1 = mDamping * SCALE_DAMP;

    mWet1 = mWetLevel * SCALE_WET * (mWidth / 2.0f + 0.5f);
    mWet2 = mWetLevel * SCALE_WET * ((1.0f - mWidth) / 2.0f);
    mDry = mDryLevel;
}

void Reverb::process(float* buffer, int frames) {
    if (!mEnabled) return;

    for (int f = 0; f < frames; f++) {
        float inL = buffer[f * 2];
        float inR = buffer[f * 2 + 1];

        float input = (inL + inR) * FIXED_GAIN;

        float outL = 0.0f;
        float outR = 0.0f;

        for (int i = 0; i < NUM_COMBS; i++) {
            outL += mCombL[i].process(input, mFeedback, mDamp1);
            outR += mCombR[i].process(input, mFeedback, mDamp1);
        }

        for (int i = 0; i < NUM_ALLPASSES; i++) {
            outL = mAllpassL[i].process(outL, ALLPASS_FEEDBACK);
            outR = mAllpassR[i].process(outR, ALLPASS_FEEDBACK);
        }

        buffer[f * 2] = outL * mWet1 + outR * mWet2 + inL * mDry;
        buffer[f * 2 + 1] = outR * mWet1 + outL * mWet2 + inR * mDry;
    }
}

void Reverb::setEnabled(bool enabled) {
    mEnabled = enabled;
}

void Reverb::reset() {
    for (int i = 0; i < NUM_COMBS; i++) {
        mCombL[i].reset();
        mCombR[i].reset();
    }
    for (int i = 0; i < NUM_ALLPASSES; i++) {
        mAllpassL[i].reset();
        mAllpassR[i].reset();
    }
}

}  // namespace veynfx
