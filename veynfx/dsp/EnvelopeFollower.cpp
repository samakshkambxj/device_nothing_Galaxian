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

#include "EnvelopeFollower.h"

#include <cmath>

namespace veynfx {

void EnvelopeFollower::configure(float sampleRate, float attackMs, float releaseMs) {
    if (attackMs > 0.0f) {
        mAttackCoeff = std::exp(-1.0f / (sampleRate * attackMs / 1000.0f));
    } else {
        mAttackCoeff = 0.0f;
    }

    if (releaseMs > 0.0f) {
        mReleaseCoeff = std::exp(-1.0f / (sampleRate * releaseMs / 1000.0f));
    } else {
        mReleaseCoeff = 0.0f;
    }
}

float EnvelopeFollower::process(float input) {
    float level = std::fabs(input);

    if (level > mEnvelope) {
        mEnvelope = mAttackCoeff * mEnvelope + (1.0f - mAttackCoeff) * level;
    } else {
        mEnvelope = mReleaseCoeff * mEnvelope + (1.0f - mReleaseCoeff) * level;
    }

    return mEnvelope;
}

void EnvelopeFollower::reset() {
    mEnvelope = 0.0f;
}

}  // namespace veynfx
