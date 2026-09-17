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

#include "../dsp/DelayLine.h"

namespace veynfx {

class DiffSurround {
public:
    static constexpr float MAX_DELAY_MS = 50.0f;

    void configure(float sampleRate);
    void setDelay(float ms);
    void setWidth(float width);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }
    void reset();

private:
    bool mEnabled = false;
    float mSampleRate = 48000.0f;
    float mDelayMs = 20.0f;
    float mWidth = 0.6f;

    int mDelayMidL = 0;
    int mDelayMidR = 0;
    int mDelaySide1 = 0;

    DelayLine mMidLine;
    DelayLine mSideLine;
};

}  // namespace veynfx
