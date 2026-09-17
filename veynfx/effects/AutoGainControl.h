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

namespace veynfx {

class AutoGainControl {
public:
    void configure(float sampleRate);
    void setTargetLevel(float dB);
    void setMaxGain(float dB);
    void setSpeed(float ms);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }
    void reset();

private:
    bool mEnabled = false;
    float mSampleRate = 48000.0f;

    float mTargetLevel = 0.5f;
    float mMaxGain = 10.0f;
    float mAttackCoeff = 0.0f;
    float mReleaseCoeff = 0.0f;
    float mEnvelope = 0.5f;
    float mGain = 1.0f;
};

}  // namespace veynfx
