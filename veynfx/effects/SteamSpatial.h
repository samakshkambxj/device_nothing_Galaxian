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

// No-op stand-in for the Steam Audio-backed spatial stage.
//
// Valve's Steam Audio SDK (phonon.h + libsteamaudio) is not vendored in
// this tree, so the real implementation (effects/SteamSpatial.cpp, kept
// on disk but excluded from the build) cannot compile. This stub keeps
// the class API and the PARAM_SPATIAL_* protocol intact: the app keeps
// addressing the same params and the engine links unchanged — the
// spatial block simply passes audio through.
//
// To re-enable real HRTF spatialization: restore the phonon-based
// header and effects/SteamSpatial.cpp from git history, re-add the
// .cpp to veynfx_engine_srcs and "libsteamaudio" to static_libs in
// Android.bp, and vendor the SDK headers + prebuilt.

namespace veynfx {

class SteamSpatial {
public:
    SteamSpatial() = default;
    ~SteamSpatial() = default;

    void configure(int /*sampleRate*/) {}
    void process(float* /*buffer*/, int /*frames*/) {}
    void setEnabled(bool enabled) { mEnabled = enabled; }
    bool isEnabled() const { return mEnabled; }
    void setWidth(int /*percent*/) {}
    void setBlend(int /*percent*/) {}
    void setDirection(float /*azimuth*/, float /*elevation*/) {}
    void setHrtfProfile(int /*profile*/) {}
    void reset() {}

private:
    bool mEnabled = false;
};

}  // namespace veynfx
