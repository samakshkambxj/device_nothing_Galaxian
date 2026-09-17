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

class Exciter {
public:
    void configure(float sampleRate);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }

    void setDrive(float percent);
    void setBlend(float percent);
    void setHarmonicFreq(float hz);

    void reset();

private:
    bool mEnabled = false;
    float mSampleRate = 48000.0f;

    float mDrive = 1.0f;
    float mBlend = 0.3f;
    float mHarmonicFreq = 3000.0f;

    Biquad mHighpassL;
    Biquad mHighpassR;
    Biquad mSmoothL;
    Biquad mSmoothR;
};

}  // namespace veynfx
