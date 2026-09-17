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
#include <string>
#include <vector>

namespace veynfx {

struct WavData {
    std::vector<float> samples;
    int sampleRate = 0;
    int channels = 0;
    int numFrames = 0;
    bool valid = false;
};

struct WavDataMulti {
    std::vector<float> samples;
    int sampleRate = 0;
    int channels = 0;
    int numFrames = 0;
    bool valid = false;
};

WavData loadWavFile(const std::string& path);
WavData loadWavFromFd(int fd, int64_t offset, int64_t length);
WavData loadWavFromMemory(const uint8_t* data, size_t size);

WavDataMulti loadWavFileMulti(const std::string& path, int maxChannels = 16);
WavDataMulti loadWavFromFdMulti(int fd, int64_t offset, int64_t length, int maxChannels = 16);

}  // namespace veynfx
