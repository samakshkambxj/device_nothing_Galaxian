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
#include <cstdint>
#include <memory>
#include <vector>

struct PFFFT_Setup;

namespace veynfx {

class Convolver {
public:
    static constexpr int BLOCK_SIZE = 512;
    static constexpr int FFT_SIZE = BLOCK_SIZE * 2;
    static constexpr int MAX_IR_SAMPLES = 48000 * 10;

    Convolver();
    ~Convolver();

    void configure(float sampleRate);
    void loadImpulseResponse(const float* ir, int irLength);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    void setMix(float mix);
    bool isEnabled() const { return mEnabled; }
    bool isLoaded() const { return mNumPartitions > 0; }
    void reset();

private:
    bool mEnabled = false;
    float mSampleRate = 48000.0f;
    float mMix = 1.0f;

    PFFFT_Setup* mFftSetup = nullptr;

    int mNumPartitions = 0;
    std::vector<float> mIrSegmentsFreq;

    std::vector<float> mInputBufL;
    std::vector<float> mInputBufR;
    int mInputPos = 0;

    std::vector<float> mOutputBufL;
    std::vector<float> mOutputBufR;
    int mOutputPos = 0;

    std::vector<float> mOverlapL;
    std::vector<float> mOverlapR;

    std::vector<float> mFdlL;
    std::vector<float> mFdlR;
    int mFdlPos = 0;

    std::vector<float> mFftWork;
    std::vector<float> mFftBuf;
    std::vector<float> mFftBuf2;
    std::vector<float> mAccumFreq;
};

}  // namespace veynfx
