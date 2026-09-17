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

#include "../dsp/EnvelopeFollower.h"

namespace veynfx {

class Limiter {
public:
    static constexpr int MAX_LOOKAHEAD_SAMPLES = 960;
    static constexpr float DEFAULT_THRESHOLD_DB = -0.1f;
    static constexpr float DEFAULT_RELEASE_MS = 50.0f;
    static constexpr float LOOKAHEAD_MS = 5.0f;

    void configure(float sampleRate);
    void setThreshold(float dB);
    void setRelease(float ms);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }
    void reset();

private:
    bool mEnabled = false;
    float mSampleRate = 48000.0f;
    float mThreshold = 0.989f;
    float mReleaseMs = DEFAULT_RELEASE_MS;
    float mReleaseCoeff = 0.0f;

    int mLookaheadSamples = 240;
    float mDelayL[MAX_LOOKAHEAD_SAMPLES] = {};
    float mDelayR[MAX_LOOKAHEAD_SAMPLES] = {};
    int mDelayPos = 0;

    float mGain = 1.0f;
};

}  // namespace veynfx
