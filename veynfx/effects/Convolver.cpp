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

#include "Convolver.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include <pffft.h>

namespace veynfx {

Convolver::Convolver() {
    mFftSetup = pffft_new_setup(FFT_SIZE, PFFFT_REAL);
    mFftWork.resize(FFT_SIZE, 0.0f);
    mFftBuf.resize(FFT_SIZE, 0.0f);
    mFftBuf2.resize(FFT_SIZE, 0.0f);
    mAccumFreq.resize(FFT_SIZE, 0.0f);
    mInputBufL.resize(BLOCK_SIZE, 0.0f);
    mInputBufR.resize(BLOCK_SIZE, 0.0f);
    mOutputBufL.resize(BLOCK_SIZE, 0.0f);
    mOutputBufR.resize(BLOCK_SIZE, 0.0f);
    mOutputPos = BLOCK_SIZE;
    mOverlapL.resize(BLOCK_SIZE, 0.0f);
    mOverlapR.resize(BLOCK_SIZE, 0.0f);
}

Convolver::~Convolver() {
    if (mFftSetup) {
        pffft_destroy_setup(mFftSetup);
    }
}

void Convolver::configure(float sampleRate) {
    mSampleRate = sampleRate;
    reset();
}

void Convolver::loadImpulseResponse(const float* ir, int irLength) {
    if (!ir || irLength <= 0 || !mFftSetup) {
        mNumPartitions = 0;
        return;
    }

    irLength = std::min(irLength, MAX_IR_SAMPLES);
    mNumPartitions = (irLength + BLOCK_SIZE - 1) / BLOCK_SIZE;

    mIrSegmentsFreq.resize(mNumPartitions * FFT_SIZE, 0.0f);

    for (int p = 0; p < mNumPartitions; p++) {
        std::memset(mFftBuf.data(), 0, FFT_SIZE * sizeof(float));

        int offset = p * BLOCK_SIZE;
        int copyLen = std::min(BLOCK_SIZE, irLength - offset);
        std::memcpy(mFftBuf.data(), ir + offset, copyLen * sizeof(float));

        pffft_transform(mFftSetup, mFftBuf.data(),
                        mIrSegmentsFreq.data() + p * FFT_SIZE,
                        mFftWork.data(), PFFFT_FORWARD);
    }

    mFdlL.resize(mNumPartitions * FFT_SIZE, 0.0f);
    mFdlR.resize(mNumPartitions * FFT_SIZE, 0.0f);
    mFdlPos = 0;
    mInputPos = 0;

    std::memset(mOverlapL.data(), 0, BLOCK_SIZE * sizeof(float));
    std::memset(mOverlapR.data(), 0, BLOCK_SIZE * sizeof(float));
}

void Convolver::setMix(float mix) {
    mMix = std::clamp(mix, 0.0f, 1.0f);
}

void Convolver::process(float* buffer, int frames) {
    if (!mEnabled || mNumPartitions == 0 || !mFftSetup) return;

    for (int f = 0; f < frames; f++) {
        float dryL = buffer[f * 2];
        float dryR = buffer[f * 2 + 1];

        mInputBufL[mInputPos] = dryL;
        mInputBufR[mInputPos] = dryR;

        if (mOutputPos < BLOCK_SIZE) {
            buffer[f * 2] = dryL * (1.0f - mMix) + mOutputBufL[mOutputPos] * mMix;
            buffer[f * 2 + 1] = dryR * (1.0f - mMix) + mOutputBufR[mOutputPos] * mMix;
            mOutputPos++;
        }

        mInputPos++;

        if (mInputPos >= BLOCK_SIZE) {
            auto processChannel = [&](std::vector<float>& inputBuf,
                                       std::vector<float>& fdl,
                                       std::vector<float>& overlap,
                                       std::vector<float>& outputBuf) {
                std::memset(mFftBuf.data(), 0, FFT_SIZE * sizeof(float));
                std::memcpy(mFftBuf.data(), inputBuf.data(), BLOCK_SIZE * sizeof(float));

                pffft_transform(mFftSetup, mFftBuf.data(),
                                fdl.data() + mFdlPos * FFT_SIZE,
                                mFftWork.data(), PFFFT_FORWARD);

                std::memset(mAccumFreq.data(), 0, FFT_SIZE * sizeof(float));

                for (int p = 0; p < mNumPartitions; p++) {
                    int fdlIdx = (mFdlPos - p + mNumPartitions) % mNumPartitions;
                    pffft_zconvolve_accumulate(mFftSetup,
                                               fdl.data() + fdlIdx * FFT_SIZE,
                                               mIrSegmentsFreq.data() + p * FFT_SIZE,
                                               mAccumFreq.data(), 1.0f);
                }

                pffft_transform(mFftSetup, mAccumFreq.data(), mFftBuf.data(),
                                mFftWork.data(), PFFFT_BACKWARD);

                float scale = 1.0f / FFT_SIZE;
                for (int i = 0; i < BLOCK_SIZE; i++) {
                    outputBuf[i] = mFftBuf[i] * scale + overlap[i];
                }

                std::memcpy(overlap.data(), mFftBuf.data() + BLOCK_SIZE,
                             BLOCK_SIZE * sizeof(float));
                for (int i = 0; i < BLOCK_SIZE; i++) {
                    overlap[i] *= scale;
                }
            };

            processChannel(mInputBufL, mFdlL, mOverlapL, mOutputBufL);
            processChannel(mInputBufR, mFdlR, mOverlapR, mOutputBufR);

            mFdlPos = (mFdlPos + 1) % mNumPartitions;
            mInputPos = 0;
            mOutputPos = 0;
        }
    }
}

void Convolver::setEnabled(bool enabled) {
    mEnabled = enabled;
}

void Convolver::reset() {
    mInputPos = 0;
    mOutputPos = BLOCK_SIZE;
    mFdlPos = 0;
    std::fill(mInputBufL.begin(), mInputBufL.end(), 0.0f);
    std::fill(mInputBufR.begin(), mInputBufR.end(), 0.0f);
    std::fill(mOutputBufL.begin(), mOutputBufL.end(), 0.0f);
    std::fill(mOutputBufR.begin(), mOutputBufR.end(), 0.0f);
    std::fill(mOverlapL.begin(), mOverlapL.end(), 0.0f);
    std::fill(mOverlapR.begin(), mOverlapR.end(), 0.0f);
    std::fill(mFdlL.begin(), mFdlL.end(), 0.0f);
    std::fill(mFdlR.begin(), mFdlR.end(), 0.0f);
}

}  // namespace veynfx
