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

class StereoWidener {
public:
    static constexpr float MIN_WIDTH = 0.0f;
    static constexpr float MAX_WIDTH = 3.0f;
    static constexpr float DEFAULT_WIDTH = 1.0f;

    void configure(float sampleRate);
    void setWidth(float width);
    float getWidth() const { return mWidth; }
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }
    void reset();

private:
    bool mEnabled = false;
    float mWidth = DEFAULT_WIDTH;
};

}  // namespace veynfx
