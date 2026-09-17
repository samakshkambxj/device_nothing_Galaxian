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

#include "WavLoader.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace veynfx {

struct WavHeader {
    char riffId[4];
    uint32_t fileSize;
    char waveId[4];
};

struct ChunkHeader {
    char id[4];
    uint32_t size;
};

struct FmtChunk {
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
};

static WavData parseWav(FILE* f) {
    WavData result;

    WavHeader header;
    if (fread(&header, sizeof(header), 1, f) != 1) return result;
    if (std::memcmp(header.riffId, "RIFF", 4) != 0) return result;
    if (std::memcmp(header.waveId, "WAVE", 4) != 0) return result;

    FmtChunk fmt = {};
    bool foundFmt = false;
    bool foundData = false;
    uint32_t dataSize = 0;

    while (!foundData) {
        ChunkHeader chunk;
        if (fread(&chunk, sizeof(chunk), 1, f) != 1) break;

        if (std::memcmp(chunk.id, "fmt ", 4) == 0) {
            if (chunk.size < sizeof(FmtChunk)) break;
            if (fread(&fmt, sizeof(FmtChunk), 1, f) != 1) break;
            if (chunk.size > sizeof(FmtChunk)) {
                fseek(f, chunk.size - sizeof(FmtChunk), SEEK_CUR);
            }
            foundFmt = true;
        } else if (std::memcmp(chunk.id, "data", 4) == 0) {
            dataSize = chunk.size;
            foundData = true;
        } else {
            fseek(f, chunk.size, SEEK_CUR);
        }
    }

    if (!foundFmt || !foundData) return result;
    if (fmt.audioFormat != 1 && fmt.audioFormat != 3) return result;
    if (fmt.numChannels < 1 || fmt.numChannels > 8) return result;
    if (fmt.bitsPerSample != 16 && fmt.bitsPerSample != 24 &&
        fmt.bitsPerSample != 32) return result;

    int bytesPerSample = fmt.bitsPerSample / 8;
    int totalSamples = dataSize / bytesPerSample;
    int numFrames = totalSamples / fmt.numChannels;

    static constexpr int MAX_FRAMES = 48000 * 30;
    if (numFrames > MAX_FRAMES) numFrames = MAX_FRAMES;

    result.samples.resize(numFrames);
    result.sampleRate = fmt.sampleRate;
    result.channels = fmt.numChannels;
    result.numFrames = numFrames;

    if (fmt.audioFormat == 3 && fmt.bitsPerSample == 32) {
        std::vector<float> raw(numFrames * fmt.numChannels);
        fread(raw.data(), sizeof(float), numFrames * fmt.numChannels, f);
        if (fmt.numChannels == 1) {
            std::memcpy(result.samples.data(), raw.data(), numFrames * sizeof(float));
        } else {
            for (int i = 0; i < numFrames; ++i) {
                float sum = 0.0f;
                for (int c = 0; c < fmt.numChannels; ++c) {
                    sum += raw[i * fmt.numChannels + c];
                }
                result.samples[i] = sum / fmt.numChannels;
            }
        }
    } else if (fmt.bitsPerSample == 16) {
        std::vector<int16_t> raw(numFrames * fmt.numChannels);
        fread(raw.data(), sizeof(int16_t), numFrames * fmt.numChannels, f);
        for (int i = 0; i < numFrames; ++i) {
            float sum = 0.0f;
            for (int c = 0; c < fmt.numChannels; ++c) {
                sum += static_cast<float>(raw[i * fmt.numChannels + c]) / 32768.0f;
            }
            result.samples[i] = sum / fmt.numChannels;
        }
    } else if (fmt.bitsPerSample == 24) {
        std::vector<uint8_t> raw(numFrames * fmt.numChannels * 3);
        fread(raw.data(), 3, numFrames * fmt.numChannels, f);
        for (int i = 0; i < numFrames; ++i) {
            float sum = 0.0f;
            for (int c = 0; c < fmt.numChannels; ++c) {
                int idx = (i * fmt.numChannels + c) * 3;
                int32_t val = (raw[idx] | (raw[idx + 1] << 8) | (raw[idx + 2] << 16));
                if (val & 0x800000) val |= 0xFF000000;
                sum += static_cast<float>(val) / 8388608.0f;
            }
            result.samples[i] = sum / fmt.numChannels;
        }
    } else if (fmt.bitsPerSample == 32 && fmt.audioFormat == 1) {
        std::vector<int32_t> raw(numFrames * fmt.numChannels);
        fread(raw.data(), sizeof(int32_t), numFrames * fmt.numChannels, f);
        for (int i = 0; i < numFrames; ++i) {
            float sum = 0.0f;
            for (int c = 0; c < fmt.numChannels; ++c) {
                sum += static_cast<float>(raw[i * fmt.numChannels + c]) / 2147483648.0f;
            }
            result.samples[i] = sum / fmt.numChannels;
        }
    }

    result.valid = true;
    return result;
}

WavData loadWavFile(const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return {};
    auto result = parseWav(f);
    fclose(f);
    return result;
}

static WavDataMulti parseWavMulti(FILE* f, int maxChannels) {
    WavDataMulti result;

    WavHeader header;
    if (fread(&header, sizeof(header), 1, f) != 1) return result;
    if (std::memcmp(header.riffId, "RIFF", 4) != 0) return result;
    if (std::memcmp(header.waveId, "WAVE", 4) != 0) return result;

    FmtChunk fmt = {};
    bool foundFmt = false;
    bool foundData = false;
    uint32_t dataSize = 0;

    while (!foundData) {
        ChunkHeader chunk;
        if (fread(&chunk, sizeof(chunk), 1, f) != 1) break;

        if (std::memcmp(chunk.id, "fmt ", 4) == 0) {
            if (chunk.size < sizeof(FmtChunk)) break;
            if (fread(&fmt, sizeof(FmtChunk), 1, f) != 1) break;
            if (chunk.size > sizeof(FmtChunk)) {
                fseek(f, chunk.size - sizeof(FmtChunk), SEEK_CUR);
            }
            foundFmt = true;
        } else if (std::memcmp(chunk.id, "data", 4) == 0) {
            dataSize = chunk.size;
            foundData = true;
        } else {
            fseek(f, chunk.size, SEEK_CUR);
        }
    }

    if (!foundFmt || !foundData) return result;
    if (fmt.audioFormat != 1 && fmt.audioFormat != 3) return result;
    if (fmt.numChannels < 1 || fmt.numChannels > maxChannels) return result;
    if (fmt.bitsPerSample != 16 && fmt.bitsPerSample != 24 &&
        fmt.bitsPerSample != 32) return result;

    int bytesPerSample = fmt.bitsPerSample / 8;
    int totalSamples = dataSize / bytesPerSample;
    int numFrames = totalSamples / fmt.numChannels;

    static constexpr int MAX_FRAMES = 48000 * 10;
    if (numFrames > MAX_FRAMES) numFrames = MAX_FRAMES;

    int totalOut = numFrames * fmt.numChannels;
    result.samples.resize(totalOut);
    result.sampleRate = fmt.sampleRate;
    result.channels = fmt.numChannels;
    result.numFrames = numFrames;

    if (fmt.audioFormat == 3 && fmt.bitsPerSample == 32) {
        fread(result.samples.data(), sizeof(float), totalOut, f);
    } else if (fmt.bitsPerSample == 16) {
        std::vector<int16_t> raw(totalOut);
        fread(raw.data(), sizeof(int16_t), totalOut, f);
        for (int i = 0; i < totalOut; ++i) {
            result.samples[i] = static_cast<float>(raw[i]) / 32768.0f;
        }
    } else if (fmt.bitsPerSample == 24) {
        std::vector<uint8_t> raw(totalOut * 3);
        fread(raw.data(), 3, totalOut, f);
        for (int i = 0; i < totalOut; ++i) {
            int idx = i * 3;
            int32_t val = (raw[idx] | (raw[idx + 1] << 8) | (raw[idx + 2] << 16));
            if (val & 0x800000) val |= 0xFF000000;
            result.samples[i] = static_cast<float>(val) / 8388608.0f;
        }
    } else if (fmt.bitsPerSample == 32 && fmt.audioFormat == 1) {
        std::vector<int32_t> raw(totalOut);
        fread(raw.data(), sizeof(int32_t), totalOut, f);
        for (int i = 0; i < totalOut; ++i) {
            result.samples[i] = static_cast<float>(raw[i]) / 2147483648.0f;
        }
    }

    result.valid = true;
    return result;
}

WavDataMulti loadWavFileMulti(const std::string& path, int maxChannels) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) return {};
    auto result = parseWavMulti(f, maxChannels);
    fclose(f);
    return result;
}

WavDataMulti loadWavFromFdMulti(int fd, int64_t offset, int64_t length, int maxChannels) {
    (void)length;
    int dupFd = dup(fd);
    if (dupFd < 0) return {};
    FILE* f = fdopen(dupFd, "rb");
    if (!f) {
        close(dupFd);
        return {};
    }
    if (offset > 0) fseek(f, offset, SEEK_SET);
    auto result = parseWavMulti(f, maxChannels);
    fclose(f);
    return result;
}

WavData loadWavFromMemory(const uint8_t* data, size_t size) {
    if (!data || size == 0) return {};
    FILE* f = fmemopen(const_cast<uint8_t*>(data), size, "rb");
    if (!f) return {};
    auto result = parseWav(f);
    fclose(f);
    return result;
}

WavData loadWavFromFd(int fd, int64_t offset, int64_t length) {
    (void)length;
    int dupFd = dup(fd);
    if (dupFd < 0) return {};
    FILE* f = fdopen(dupFd, "rb");
    if (!f) {
        close(dupFd);
        return {};
    }
    if (offset > 0) fseek(f, offset, SEEK_SET);
    auto result = parseWav(f);
    fclose(f);
    return result;
}

}  // namespace veynfx
