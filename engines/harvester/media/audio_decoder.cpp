/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "harvester/media/audio_decoder.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "common/endian.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/util.h"
#include "harvester/resources.h"

namespace Harvester {

namespace {

static const int8 kHarvesterImaIndexAdjustTable[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

static const uint16 kHarvesterImaStepTable[89] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static uint32 decodeHarvesterFcmp(byte *dest, const byte *src, uint32 srcSize, uint16 bitsPerSample) {
	const uint32 decodedBytesPerInputByte = bitsPerSample >> 2;
	const uint32 decodedSize = srcSize * decodedBytesPerInputByte;
	const uint32 sampleCount = (bitsPerSample == 16) ? (decodedSize >> 1) : decodedSize;
	int predictor = 0;
	int stepIndex = 0;
	int stepSize = 7;
	byte cachedByte = 0;
	uint32 srcPos = 0;

	for (uint32 sample = 0; sample < sampleCount; ++sample) {
		const byte nibble = ((sample & 1) == 0)
			? ((cachedByte = src[srcPos++]) & 0x0f)
			: ((cachedByte >> 4) & 0x0f);
		int delta = stepSize >> 3;
		if (nibble & 4)
			delta += stepSize;
		if (nibble & 2)
			delta += stepSize >> 1;
		if (nibble & 1)
			delta += stepSize >> 2;
		if (nibble & 8)
			delta = -delta;

		predictor += delta;
		if (predictor > 0x7fff)
			predictor = 0x7fff;
		else if (predictor < -0x8000)
			predictor = -0x8000;

		if (bitsPerSample == 16) {
			WRITE_LE_INT16(dest, predictor);
			dest += 2;
		} else {
			*dest++ = (byte)(((predictor >> 8) & 0xff) ^ 0x80);
		}

		stepIndex += kHarvesterImaIndexAdjustTable[nibble];
		if (stepIndex < 0)
			stepIndex = 0;
		else if (stepIndex > 88)
			stepIndex = 88;
		stepSize = kHarvesterImaStepTable[stepIndex];
	}

	return decodedSize;
}

static void applyStartupFcmpWarmup(byte *decodedPcm, uint32 &decodedSize, FcmpWarmupMode warmupMode) {
	if (!decodedPcm || decodedSize == 0)
		return;

	if (warmupMode == FcmpWarmupMode::kMusic) {
		// Native start_music_stream zeros the first 0x64 decoded bytes of the first FCMP chunk.
		memset(decodedPcm, 0, MIN<uint32>(decodedSize, 100));
		return;
	}

	// Native load_sound_sample/load_dialogue_voice_sample queue from decoded_pcm + 0x1f4.
	const uint32 trimBytes = MIN<uint32>(decodedSize, 500);
	if (trimBytes == decodedSize) {
		memset(decodedPcm, 0, decodedSize);
		return;
	}

	memmove(decodedPcm, decodedPcm + trimBytes, decodedSize - trimBytes);
	decodedSize -= trimBytes;
}

} // End of anonymous namespace

Audio::SeekableAudioStream *decodeStartupAudioStream(Common::SeekableReadStream *stream,
		FcmpWarmupMode warmupMode) {
	if (!stream)
		return nullptr;

	Common::ScopedPtr<Common::SeekableReadStream> input(stream);

	if (input->size() >= 14) {
		char magic[4];
		input->read(magic, sizeof(magic));
		if (!memcmp(magic, "FCMP", sizeof(magic))) {
			const uint32 payloadSizeFromHeader = input->readUint32LE();
			const uint32 sampleRate = input->readUint32LE();
			const uint16 bitsPerSample = input->readUint16LE();
			const uint32 availablePayloadSize = input->size() - 14;
			const uint32 payloadSize = payloadSizeFromHeader != 0
				? MIN<uint32>(payloadSizeFromHeader, availablePayloadSize)
				: availablePayloadSize;

			if (sampleRate == 0 || (bitsPerSample != 8 && bitsPerSample != 16) || payloadSize == 0)
				return nullptr;

			if (!input->seek(14))
				return nullptr;

			byte *compressedPayload = (byte *)malloc(payloadSize);
			if (!compressedPayload)
				return nullptr;
			if (input->read(compressedPayload, payloadSize) != payloadSize) {
				free(compressedPayload);
				return nullptr;
			}

			uint32 decodedSize = payloadSize * (bitsPerSample >> 2);
			byte *decodedPcm = (byte *)malloc(decodedSize);
			if (!decodedPcm) {
				free(compressedPayload);
				return nullptr;
			}

			decodeHarvesterFcmp(decodedPcm, compressedPayload, payloadSize, bitsPerSample);
			applyStartupFcmpWarmup(decodedPcm, decodedSize, warmupMode);
			free(compressedPayload);
			input.reset();
			return Audio::makeRawStream(decodedPcm, decodedSize, sampleRate,
				Audio::FLAG_LITTLE_ENDIAN | ((bitsPerSample == 16) ? Audio::FLAG_16BITS : 0),
				DisposeAfterUse::YES);
		}
	}

	input->seek(0);
	return Audio::makeWAVStream(input.release(), DisposeAfterUse::YES);
}

Audio::SeekableAudioStream *openStartupAudioStream(ResourceManager &resources,
		const Common::String &path, FcmpWarmupMode warmupMode) {
	return decodeStartupAudioStream(resources.openFile(path), warmupMode);
}

} // End of namespace Harvester
