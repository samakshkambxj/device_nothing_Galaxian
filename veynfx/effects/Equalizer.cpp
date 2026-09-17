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

#include "Equalizer.h"

#include <algorithm>
#include <cmath>

namespace veynfx {

constexpr float Equalizer::CENTER_FREQS[];
constexpr int Equalizer::LEGACY_BAND_TO_RESPONSE[];

void Equalizer::configure(float sampleRate) {
    mSampleRate = sampleRate;
    for (int i = 0; i < NUM_BANDS; i++) {
        updateBand(i);
    }
}

void Equalizer::setBandLevel(int band, float dB) {
    if (band < 0 || band >= NUM_BANDS) return;
    dB = std::clamp(dB, MIN_GAIN_DB, MAX_GAIN_DB);
    mBandLevels[band] = dB;
    updateBand(band);
}

void Equalizer::setLegacyBandLevel(int band, float dB) {
    if (band < 0 || band >= LEGACY_NUM_BANDS) return;
    setBandLevel(LEGACY_BAND_TO_RESPONSE[band], dB);
}

float Equalizer::getBandLevel(int band) const {
    if (band < 0 || band >= NUM_BANDS) return 0.0f;
    return mBandLevels[band];
}

void Equalizer::updateBand(int band) {
    if (std::fabs(mBandLevels[band]) < 0.05f) {
        mFiltersL[band].setPassthrough();
        mFiltersR[band].setPassthrough();
    } else {
        mFiltersL[band].configure(BiquadType::PEAKING, mSampleRate,
                                  CENTER_FREQS[band], mBandLevels[band], DEFAULT_Q);
        mFiltersR[band].configure(BiquadType::PEAKING, mSampleRate,
                                  CENTER_FREQS[band], mBandLevels[band], DEFAULT_Q);
    }
}

void Equalizer::process(float* buffer, int frames) {
    if (!mEnabled) return;

    for (int f = 0; f < frames; f++) {
        float left = buffer[f * 2];
        float right = buffer[f * 2 + 1];

        for (int b = 0; b < NUM_BANDS; b++) {
            left = mFiltersL[b].process(left);
            right = mFiltersR[b].process(right);
        }

        buffer[f * 2] = left;
        buffer[f * 2 + 1] = right;
    }
}

void Equalizer::setEnabled(bool enabled) {
    mEnabled = enabled;
}

void Equalizer::reset() {
    for (int i = 0; i < NUM_BANDS; i++) {
        mFiltersL[i].reset();
        mFiltersR[i].reset();
        mBandLevels[i] = 0.0f;
    }
}

}  // namespace veynfx
