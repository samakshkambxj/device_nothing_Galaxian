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

#include "StereoWidener.h"

#include <algorithm>

namespace veynfx {

void StereoWidener::configure(float /* sampleRate */) {
}

void StereoWidener::setWidth(float width) {
    mWidth = std::clamp(width, MIN_WIDTH, MAX_WIDTH);
}

void StereoWidener::process(float* buffer, int frames) {
    if (!mEnabled) return;

    for (int f = 0; f < frames; f++) {
        float left = buffer[f * 2];
        float right = buffer[f * 2 + 1];

        float mid = (left + right) * 0.5f;
        float side = (left - right) * 0.5f;

        buffer[f * 2] = mid + side * mWidth;
        buffer[f * 2 + 1] = mid - side * mWidth;
    }
}

void StereoWidener::setEnabled(bool enabled) {
    mEnabled = enabled;
}

void StereoWidener::reset() {
}

}  // namespace veynfx
