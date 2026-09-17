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

enum class BiquadType {
    LOWPASS,
    HIGHPASS,
    BANDPASS,
    NOTCH,
    PEAKING,
    LOW_SHELF,
    HIGH_SHELF
};

class Biquad {
public:
    void configure(BiquadType type, float sampleRate, float freq, float gainDb, float q);
    void setPassthrough();
    float process(float input);
    void reset();

private:
    float mB0 = 1.0f;
    float mB1 = 0.0f;
    float mB2 = 0.0f;
    float mA1 = 0.0f;
    float mA2 = 0.0f;

    float mX1 = 0.0f;
    float mX2 = 0.0f;
    float mY1 = 0.0f;
    float mY2 = 0.0f;
};

}  // namespace veynfx
