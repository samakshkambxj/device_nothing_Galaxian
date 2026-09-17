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

#include <cstdint>
#include "phonon.h"

namespace veynfx {

class SteamSpatial {
public:
    SteamSpatial();
    ~SteamSpatial();

    void configure(int sampleRate);
    void process(float* buffer, int frames);
    void setEnabled(bool enabled);
    bool isEnabled() const { return mEnabled; }
    void setWidth(int percent);
    void setBlend(int percent);
    void setDirection(float azimuth, float elevation);
    void setHrtfProfile(int profile);
    void reset();

private:
    void initSteamAudio();
    void teardown();

    bool mEnabled;
    bool mInitialized;
    bool mInitFailed = false;
    int mSampleRate;
    int mFrameSize;
    int mWidth;
    int mBlend;
    float mAzimuth;
    float mElevation;
    int mHrtfProfile;

    IPLContext mContext;
    IPLHRTF mHrtf;
    IPLBinauralEffect mBinauralL;

    IPLAudioBuffer mInBuf;
    IPLAudioBuffer mOutBufL;
    bool mInBufAllocated;
    bool mOutBufAllocated;
};

}  // namespace veynfx
