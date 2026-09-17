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

#include <cstdint>

#include "../dsp/Biquad.h"

namespace veynfx {

enum class BassMode : int32_t {
    NATURAL = 0,
    PUNCH = 1,
    SUBWOOFER = 2,
};

class BassBoost {
public:
    static constexpr float DEFAULT_FREQUENCY = 80.0f;
    static constexpr float MIN_GAIN_DB = 0.0f;
    static constexpr float MAX_GAIN_DB = 15.0f;

    void configure(float sampleRate);
    void setGain(float dB);
    void setFrequency(float hz);
    void setMode(BassMode mode);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }
    void reset();

private:
    void updateFilters();

    bool mEnabled = false;
    float mSampleRate = 48000.0f;
    float mGainDb = 0.0f;
    float mFrequency = DEFAULT_FREQUENCY;
    BassMode mMode = BassMode::NATURAL;

    Biquad mShelfL;
    Biquad mShelfR;
    Biquad mSubLpL;
    Biquad mSubLpR;
};

}  // namespace veynfx
