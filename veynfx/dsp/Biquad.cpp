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

#include "Biquad.h"

#include <cmath>

namespace veynfx {

static constexpr double PI = 3.14159265358979323846;

void Biquad::configure(BiquadType type, float sampleRate, float freq, float gainDb, float q) {
    double w0 = 2.0 * PI * freq / sampleRate;
    double cosW0 = cos(w0);
    double sinW0 = sin(w0);
    double alpha = sinW0 / (2.0 * q);
    double A = pow(10.0, gainDb / 40.0);

    double b0, b1, b2, a0, a1, a2;

    switch (type) {
        case BiquadType::LOWPASS:
            b0 = (1.0 - cosW0) / 2.0;
            b1 = 1.0 - cosW0;
            b2 = (1.0 - cosW0) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosW0;
            a2 = 1.0 - alpha;
            break;

        case BiquadType::HIGHPASS:
            b0 = (1.0 + cosW0) / 2.0;
            b1 = -(1.0 + cosW0);
            b2 = (1.0 + cosW0) / 2.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosW0;
            a2 = 1.0 - alpha;
            break;

        case BiquadType::BANDPASS:
            b0 = alpha;
            b1 = 0.0;
            b2 = -alpha;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosW0;
            a2 = 1.0 - alpha;
            break;

        case BiquadType::NOTCH:
            b0 = 1.0;
            b1 = -2.0 * cosW0;
            b2 = 1.0;
            a0 = 1.0 + alpha;
            a1 = -2.0 * cosW0;
            a2 = 1.0 - alpha;
            break;

        case BiquadType::PEAKING:
            b0 = 1.0 + alpha * A;
            b1 = -2.0 * cosW0;
            b2 = 1.0 - alpha * A;
            a0 = 1.0 + alpha / A;
            a1 = -2.0 * cosW0;
            a2 = 1.0 - alpha / A;
            break;

        case BiquadType::LOW_SHELF: {
            double sqrtA = sqrt(A);
            double twoSqrtAAlpha = 2.0 * sqrtA * alpha;
            b0 = A * ((A + 1.0) - (A - 1.0) * cosW0 + twoSqrtAAlpha);
            b1 = 2.0 * A * ((A - 1.0) - (A + 1.0) * cosW0);
            b2 = A * ((A + 1.0) - (A - 1.0) * cosW0 - twoSqrtAAlpha);
            a0 = (A + 1.0) + (A - 1.0) * cosW0 + twoSqrtAAlpha;
            a1 = -2.0 * ((A - 1.0) + (A + 1.0) * cosW0);
            a2 = (A + 1.0) + (A - 1.0) * cosW0 - twoSqrtAAlpha;
            break;
        }

        case BiquadType::HIGH_SHELF: {
            double sqrtA = sqrt(A);
            double twoSqrtAAlpha = 2.0 * sqrtA * alpha;
            b0 = A * ((A + 1.0) + (A - 1.0) * cosW0 + twoSqrtAAlpha);
            b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosW0);
            b2 = A * ((A + 1.0) + (A - 1.0) * cosW0 - twoSqrtAAlpha);
            a0 = (A + 1.0) - (A - 1.0) * cosW0 + twoSqrtAAlpha;
            a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosW0);
            a2 = (A + 1.0) - (A - 1.0) * cosW0 - twoSqrtAAlpha;
            break;
        }
    }

    double invA0 = 1.0 / a0;
    mB0 = static_cast<float>(b0 * invA0);
    mB1 = static_cast<float>(b1 * invA0);
    mB2 = static_cast<float>(b2 * invA0);
    mA1 = static_cast<float>(a1 * invA0);
    mA2 = static_cast<float>(a2 * invA0);
}

void Biquad::setPassthrough() {
    mB0 = 1.0f;
    mB1 = 0.0f;
    mB2 = 0.0f;
    mA1 = 0.0f;
    mA2 = 0.0f;
}

float Biquad::process(float input) {
    float output = mB0 * input + mB1 * mX1 + mB2 * mX2 - mA1 * mY1 - mA2 * mY2;

    mX2 = mX1;
    mX1 = input;
    mY2 = mY1;
    mY1 = output;

    return output;
}

void Biquad::reset() {
    mX1 = 0.0f;
    mX2 = 0.0f;
    mY1 = 0.0f;
    mY2 = 0.0f;
}

}  // namespace veynfx
