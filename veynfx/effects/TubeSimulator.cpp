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

#include "TubeSimulator.h"

#include <algorithm>
#include <cmath>

namespace veynfx {

constexpr float TubeSimulator::CROSSOVER_FREQS[];

static float softClip(float x, float drive) {
    x *= drive;
    return (2.0f / 3.14159265f) * std::atan(x);
}

static float evenHarmonic(float x) {
    return x * std::fabs(x);
}

void TubeSimulator::configure(float sampleRate) {
    mSampleRate = sampleRate;
    for (int i = 0; i < NUM_BANDS - 1; i++) {
        mLpL[i].configure(BiquadType::LOWPASS, sampleRate, CROSSOVER_FREQS[i], 0.0f, 0.707f);
        mLpR[i].configure(BiquadType::LOWPASS, sampleRate, CROSSOVER_FREQS[i], 0.0f, 0.707f);
        mHpL[i].configure(BiquadType::HIGHPASS, sampleRate, CROSSOVER_FREQS[i], 0.0f, 0.707f);
        mHpR[i].configure(BiquadType::HIGHPASS, sampleRate, CROSSOVER_FREQS[i], 0.0f, 0.707f);
    }
}

void TubeSimulator::setDrive(float drive) {
    mDrive = std::clamp(drive, 0.1f, 5.0f);
}

void TubeSimulator::setMix(float mix) {
    mMix = std::clamp(mix, 0.0f, 1.0f);
}

void TubeSimulator::process(float* buffer, int frames) {
    if (!mEnabled) return;

    for (int f = 0; f < frames; f++) {
        float dryL = buffer[f * 2];
        float dryR = buffer[f * 2 + 1];

        float lowL = mLpL[0].process(dryL);
        float lowR = mLpR[0].process(dryR);
        float restL = mHpL[0].process(dryL);
        float restR = mHpR[0].process(dryR);

        float midL = mLpL[1].process(restL);
        float midR = mLpR[1].process(restR);
        float restL2 = mHpL[1].process(restL);
        float restR2 = mHpR[1].process(restR);

        float hiMidL = mLpL[2].process(restL2);
        float hiMidR = mLpR[2].process(restR2);
        float hiL = mHpL[2].process(restL2);
        float hiR = mHpR[2].process(restR2);

        float harmonicL = evenHarmonic(lowL) - evenHarmonic(midL)
                        + evenHarmonic(hiMidL) - evenHarmonic(hiL);
        float harmonicR = evenHarmonic(lowR) - evenHarmonic(midR)
                        + evenHarmonic(hiMidR) - evenHarmonic(hiR);

        float wetL = softClip(dryL + harmonicL * 0.25f * mDrive, mDrive);
        float wetR = softClip(dryR + harmonicR * 0.25f * mDrive, mDrive);

        buffer[f * 2] = dryL * (1.0f - mMix) + wetL * mMix;
        buffer[f * 2 + 1] = dryR * (1.0f - mMix) + wetR * mMix;
    }
}

void TubeSimulator::setEnabled(bool enabled) {
    mEnabled = enabled;
}

void TubeSimulator::reset() {
    for (int i = 0; i < NUM_BANDS - 1; i++) {
        mLpL[i].reset();
        mLpR[i].reset();
        mHpL[i].reset();
        mHpR[i].reset();
    }
}

}  // namespace veynfx
