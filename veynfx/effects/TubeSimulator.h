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

#include "../dsp/Biquad.h"

namespace veynfx {

class TubeSimulator {
public:
    static constexpr int NUM_BANDS = 4;
    static constexpr float CROSSOVER_FREQS[NUM_BANDS - 1] = {300.0f, 2200.0f, 6000.0f};

    void configure(float sampleRate);
    void setDrive(float drive);
    void setMix(float mix);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }
    void reset();

private:
    bool mEnabled = false;
    float mSampleRate = 48000.0f;
    float mDrive = 1.0f;
    float mMix = 0.5f;

    Biquad mLpL[NUM_BANDS - 1];
    Biquad mLpR[NUM_BANDS - 1];
    Biquad mHpL[NUM_BANDS - 1];
    Biquad mHpR[NUM_BANDS - 1];
};

}  // namespace veynfx
