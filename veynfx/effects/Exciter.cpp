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

#include "Exciter.h"
#include <cmath>
#include <algorithm>

namespace veynfx {

void Exciter::configure(float sampleRate) {
    mSampleRate = sampleRate;
    mHighpassL.configure(BiquadType::HIGHPASS, sampleRate, mHarmonicFreq, 0.0f, 0.707f);
    mHighpassR.configure(BiquadType::HIGHPASS, sampleRate, mHarmonicFreq, 0.0f, 0.707f);
    mSmoothL.configure(BiquadType::LOWPASS, sampleRate, std::min(sampleRate * 0.45f, 18000.0f), 0.0f, 0.707f);
    mSmoothR.configure(BiquadType::LOWPASS, sampleRate, std::min(sampleRate * 0.45f, 18000.0f), 0.0f, 0.707f);
}

void Exciter::process(float* buffer, int frames) {
    if (!mEnabled || frames <= 0) return;

    for (int i = 0; i < frames; ++i) {
        float l = buffer[i * 2];
        float r = buffer[i * 2 + 1];

        float highL = mHighpassL.process(l);
        float highR = mHighpassR.process(r);

        float saturatedL = std::tanh(highL * mDrive);
        float saturatedR = std::tanh(highR * mDrive);

        float harmonicsL = saturatedL - highL;
        float harmonicsR = saturatedR - highR;

        harmonicsL = mSmoothL.process(harmonicsL);
        harmonicsR = mSmoothR.process(harmonicsR);

        buffer[i * 2] = l + harmonicsL * mBlend;
        buffer[i * 2 + 1] = r + harmonicsR * mBlend;
    }
}

void Exciter::setEnabled(bool enabled) { mEnabled = enabled; }

void Exciter::setDrive(float percent) {
    mDrive = std::max(0.1f, percent / 100.0f * 5.0f);
}

void Exciter::setBlend(float percent) {
    mBlend = std::clamp(percent / 100.0f, 0.0f, 1.0f);
}

void Exciter::setHarmonicFreq(float hz) {
    mHarmonicFreq = std::clamp(hz, 500.0f, 10000.0f);
    mHighpassL.configure(BiquadType::HIGHPASS, mSampleRate, hz, 0.0f, 0.707f);
    mHighpassR.configure(BiquadType::HIGHPASS, mSampleRate, hz, 0.0f, 0.707f);
}

void Exciter::reset() {
    mHighpassL.reset();
    mHighpassR.reset();
    mSmoothL.reset();
    mSmoothR.reset();
}

}  // namespace veynfx
