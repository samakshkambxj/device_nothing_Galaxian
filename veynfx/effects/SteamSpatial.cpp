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

#define LOG_TAG "VeynFxSpatial"
#include <log/log.h>
#include "effects/SteamSpatial.h"
#include <cmath>
#include <cstring>
#include <algorithm>
#include <unistd.h>

namespace veynfx {

SteamSpatial::SteamSpatial()
    : mEnabled(false), mInitialized(false), mSampleRate(48000), mFrameSize(480),
      mWidth(100), mBlend(70), mAzimuth(0), mElevation(0), mHrtfProfile(0),
      mContext(nullptr), mHrtf(nullptr),
      mBinauralL(nullptr),
      mInBufAllocated(false), mOutBufAllocated(false) {
    std::memset(&mInBuf, 0, sizeof(mInBuf));
    std::memset(&mOutBufL, 0, sizeof(mOutBufL));
}

SteamSpatial::~SteamSpatial() {
    teardown();
}

void SteamSpatial::configure(int sampleRate) {
    teardown();
    mSampleRate = sampleRate;
    mFrameSize = sampleRate / 100;
    if (mFrameSize > 4096) mFrameSize = 4096;
}

void SteamSpatial::initSteamAudio() {
    if (mInitialized) return;

    IPLContextSettings contextSettings{};
    contextSettings.version = STEAMAUDIO_VERSION;
    IPLerror err = iplContextCreate(&contextSettings, &mContext);
    if (err != IPL_STATUS_SUCCESS) {
        ALOGE("iplContextCreate failed: %d", err);
        return;
    }

    IPLAudioSettings audioSettings{};
    audioSettings.samplingRate = mSampleRate;
    audioSettings.frameSize = mFrameSize;

    static const char* kSofaPath = "/vendor/etc/veynfx/hrtf/sadie_h12.sofa";

    IPLHRTFSettings hrtfSettings{};
    hrtfSettings.volume = 1.0f;

    if (mHrtfProfile == 1 && access(kSofaPath, R_OK) == 0) {
        hrtfSettings.type = IPL_HRTFTYPE_SOFA;
        hrtfSettings.sofaFileName = kSofaPath;
        ALOGI("Loading HRTF profile SADIE H12 from %s", kSofaPath);
    } else {
        hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
        ALOGI("Using default embedded HRTF (profile %d)", mHrtfProfile);
    }

    err = iplHRTFCreate(mContext, &audioSettings, &hrtfSettings, &mHrtf);
    if (err != IPL_STATUS_SUCCESS && hrtfSettings.type == IPL_HRTFTYPE_SOFA) {
        ALOGW("SOFA HRTF failed (%d), falling back to default", err);
        hrtfSettings.type = IPL_HRTFTYPE_DEFAULT;
        hrtfSettings.sofaFileName = nullptr;
        err = iplHRTFCreate(mContext, &audioSettings, &hrtfSettings, &mHrtf);
    }
    if (err != IPL_STATUS_SUCCESS) {
        ALOGE("iplHRTFCreate failed: %d", err);
        iplContextRelease(&mContext);
        mContext = nullptr;
        return;
    }

    err = iplAudioBufferAllocate(mContext, 2, mFrameSize, &mInBuf);
    if (err != IPL_STATUS_SUCCESS) {
        ALOGE("iplAudioBufferAllocate inBuf failed: %d", err);
        goto fail_hrtf;
    }
    mInBufAllocated = true;

    err = iplAudioBufferAllocate(mContext, 2, mFrameSize, &mOutBufL);
    if (err != IPL_STATUS_SUCCESS) {
        ALOGE("iplAudioBufferAllocate outBufL failed: %d", err);
        goto fail_inbuf;
    }
    mOutBufAllocated = true;

    {
        IPLBinauralEffectSettings binauralSettings{};
        binauralSettings.hrtf = mHrtf;

        err = iplBinauralEffectCreate(mContext, &audioSettings, &binauralSettings, &mBinauralL);
        if (err != IPL_STATUS_SUCCESS) {
            ALOGE("iplBinauralEffectCreate failed: %d", err);
            goto fail_outL;
        }
    }

    mInitialized = true;
    ALOGI("Steam Audio HRTF initialized: sr=%d frameSize=%d", mSampleRate, mFrameSize);
    return;

fail_outL:
    iplAudioBufferFree(mContext, &mOutBufL);
    mOutBufAllocated = false;
fail_inbuf:
    iplAudioBufferFree(mContext, &mInBuf);
    mInBufAllocated = false;
fail_hrtf:
    iplHRTFRelease(&mHrtf);
    iplContextRelease(&mContext);
    mHrtf = nullptr;
    mContext = nullptr;
}

void SteamSpatial::teardown() {
    if (mBinauralL) { iplBinauralEffectRelease(&mBinauralL); mBinauralL = nullptr; }
    if (mOutBufAllocated && mContext) {
        iplAudioBufferFree(mContext, &mOutBufL);
        mOutBufAllocated = false;
    }
    if (mInBufAllocated && mContext) {
        iplAudioBufferFree(mContext, &mInBuf);
        mInBufAllocated = false;
    }
    if (mHrtf) { iplHRTFRelease(&mHrtf); mHrtf = nullptr; }
    if (mContext) { iplContextRelease(&mContext); mContext = nullptr; }
    mInitialized = false;
}

void SteamSpatial::process(float* buffer, int frames) {
    if (!mEnabled || !mInitialized || frames <= 0) return;

    float azRad = mAzimuth * (M_PI / 180.0f);
    float elRad = mElevation * (M_PI / 180.0f);
    float blend = static_cast<float>(mBlend) / 100.0f;

    IPLVector3 direction{
        std::cos(elRad) * std::sin(azRad),
        std::sin(elRad),
        -std::cos(elRad) * std::cos(azRad)
    };

    int remaining = frames;
    int offset = 0;

    while (remaining > 0) {
        int block = std::min(remaining, mFrameSize);

        for (int i = 0; i < mFrameSize; ++i) {
            mInBuf.data[0][i] = (i < block) ? buffer[(offset + i) * 2] : 0.0f;
            mInBuf.data[1][i] = (i < block) ? buffer[(offset + i) * 2 + 1] : 0.0f;
        }

        IPLBinauralEffectParams params{};
        params.direction = direction;
        params.interpolation = IPL_HRTFINTERPOLATION_NEAREST;
        params.spatialBlend = blend;
        params.hrtf = mHrtf;
        params.peakDelays = nullptr;
        iplBinauralEffectApply(mBinauralL, &params, &mInBuf, &mOutBufL);

        for (int i = 0; i < block; ++i) {
            buffer[(offset + i) * 2] = mOutBufL.data[0][i];
            buffer[(offset + i) * 2 + 1] = mOutBufL.data[1][i];
        }

        offset += block;
        remaining -= block;
    }
}

void SteamSpatial::setEnabled(bool enabled) {
    if (enabled && !mInitialized && !mInitFailed) {
        initSteamAudio();
        if (!mInitialized) { mInitFailed = true; }
    }
    mEnabled = enabled;
    if (!enabled) mInitFailed = false;
}

void SteamSpatial::setWidth(int percent) {
    mWidth = std::clamp(percent, 0, 200);
}

void SteamSpatial::setBlend(int percent) {
    mBlend = std::clamp(percent, 0, 100);
}

void SteamSpatial::setDirection(float azimuth, float elevation) {
    mAzimuth = azimuth;
    mElevation = elevation;
}

void SteamSpatial::setHrtfProfile(int profile) {
    if (profile == mHrtfProfile) return;
    mHrtfProfile = profile;
    if (mInitialized) {
        teardown();
        mInitFailed = false;
        if (mEnabled) {
            initSteamAudio();
            if (!mInitialized) mInitFailed = true;
        }
    }
}

void SteamSpatial::reset() {
    mWidth = 0;
}

}  // namespace veynfx
