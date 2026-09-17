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

#include <cstddef>
#include <cstring>
#include <vector>

namespace veynfx {

class AllpassFilter {
public:
    void setSize(size_t size) {
        mBuffer.resize(size, 0.0f);
        mPos = 0;
    }

    float process(float input, float feedback) {
        float delayed = mBuffer[mPos];
        float output = -input + delayed;
        mBuffer[mPos] = input + delayed * feedback;
        mPos = (mPos + 1) % mBuffer.size();
        return output;
    }

    void reset() {
        std::memset(mBuffer.data(), 0, mBuffer.size() * sizeof(float));
        mPos = 0;
    }

private:
    std::vector<float> mBuffer;
    size_t mPos = 0;
};

class CombFilter {
public:
    void setSize(size_t size) {
        mBuffer.resize(size, 0.0f);
        mPos = 0;
    }

    float process(float input, float feedback, float damp) {
        float output = mBuffer[mPos];
        mFilterStore = output * (1.0f - damp) + mFilterStore * damp;
        mBuffer[mPos] = input + mFilterStore * feedback;
        mPos = (mPos + 1) % mBuffer.size();
        return output;
    }

    void reset() {
        std::memset(mBuffer.data(), 0, mBuffer.size() * sizeof(float));
        mFilterStore = 0.0f;
        mPos = 0;
    }

private:
    std::vector<float> mBuffer;
    size_t mPos = 0;
    float mFilterStore = 0.0f;
};

}  // namespace veynfx
