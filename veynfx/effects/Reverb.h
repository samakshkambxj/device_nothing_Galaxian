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

#include "../dsp/AllpassFilter.h"

namespace veynfx {

class Reverb {
public:
    static constexpr int NUM_COMBS = 8;
    static constexpr int NUM_ALLPASSES = 4;

    void configure(float sampleRate);
    void setRoomSize(float size);
    void setDamping(float damp);
    void setWetLevel(float wet);
    void setDryLevel(float dry);
    void setWidth(float width);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }
    void reset();

private:
    void updateParams();

    bool mEnabled = false;
    float mSampleRate = 48000.0f;

    float mRoomSize = 0.5f;
    float mDamping = 0.5f;
    float mWetLevel = 0.3f;
    float mDryLevel = 1.0f;
    float mWidth = 1.0f;

    float mFeedback = 0.0f;
    float mDamp1 = 0.0f;
    float mWet1 = 0.0f;
    float mWet2 = 0.0f;
    float mDry = 0.0f;

    CombFilter mCombL[NUM_COMBS];
    CombFilter mCombR[NUM_COMBS];
    AllpassFilter mAllpassL[NUM_ALLPASSES];
    AllpassFilter mAllpassR[NUM_ALLPASSES];
};

}  // namespace veynfx
