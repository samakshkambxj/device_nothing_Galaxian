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

class Compressor {
public:
    void configure(float sampleRate);
    void setThreshold(float dB);
    void setRatio(float ratio);
    void setAttack(float ms);
    void setRelease(float ms);
    void setKnee(float dB);
    void setMakeupGain(float dB);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }
    void reset();

private:
    float computeGain(float inputDb) const;

    bool mEnabled = false;
    float mSampleRate = 48000.0f;

    float mThresholdDb = -20.0f;
    float mRatio = 4.0f;
    float mKneeDb = 6.0f;
    float mMakeupGainLinear = 1.0f;

    float mAttackCoeff = 0.0f;
    float mReleaseCoeff = 0.0f;
    float mEnvelopeDb = -96.0f;
};

}  // namespace veynfx
