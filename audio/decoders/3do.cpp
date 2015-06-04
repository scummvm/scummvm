/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/textconsole.h"
#include "common/stream.h"
#include "common/util.h"

#include "audio/decoders/3do.h"
#include "audio/decoders/raw.h"

namespace Audio {

#define AUDIO_3DO_ADP4_STEPSIZETABLE_MAX 88

static int16 audio_3DO_ADP4_stepSizeTable[AUDIO_3DO_ADP4_STEPSIZETABLE_MAX + 1] = {
        7,     8,     9,    10,    11,    12,    13,    14,    16,    17,
	   19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
	   50,    55,    60,    66,    73,    80,    88,    97,   107,   118,
	  130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
	  337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
	  876,   963,  1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
	 2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
	 5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static int16 audio_3DO_ADP4_stepSizeIndex[] = {
	-1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8 
};

int16 audio_3DO_ADP4_DecodeSample(uint8 dataNibble, int16 &decoderLastSample, int16 &decoderStepIndex) {
	int16 currentStep = audio_3DO_ADP4_stepSizeTable[decoderStepIndex];
	int32 decodedSample = decoderLastSample;
	int16 delta = currentStep >> 3;

	if (dataNibble & 1)
		delta += currentStep >> 2;

	if (dataNibble & 2)
		delta += currentStep >> 1;

	if (dataNibble & 4)
		delta += currentStep;

	if (dataNibble & 8) {
		decodedSample -= delta;
	} else {
		decodedSample += delta;
	}

	decoderLastSample = CLIP<int32>(decodedSample, -32768, 32767);

	decoderStepIndex += audio_3DO_ADP4_stepSizeIndex[dataNibble & 0x07];
	decoderStepIndex = CLIP<int16>(decoderStepIndex, 0, AUDIO_3DO_ADP4_STEPSIZETABLE_MAX);

   return decoderLastSample;
}

SeekableAudioStream *make3DO_ADP4Stream(Common::SeekableReadStream *stream,	uint32 size, uint16 sampleRate, byte audioFlags, DisposeAfterUse::Flag disposeAfterUse, audio_3DO_ADP4_PersistentSpace *persistentSpace) {
	int32 streamPos = 0;
	int32 compressedSize = size;
	int32 decompressedSize = 0;
	int32 decompressedPos = 0;
	byte  compressedByte = 0;

	audio_3DO_ADP4_PersistentSpace decoderData;

	assert(compressedSize <= stream->size());

	if (audioFlags & Audio::FLAG_UNSIGNED) {
		// Unsigned data is not allowed
		warning("make3DO_ADP4Stream(): sample data result is expected to be signed");
		return 0;
	}
	if (!(audioFlags & Audio::FLAG_16BITS)) {
		// 8-bit sample data is not allowed
		warning("make3DO_ADP4Stream(): sample data result is expected to be 16-bit");
		return 0;
	}
	if (audioFlags & Audio::FLAG_LITTLE_ENDIAN) {
		// LE sample data is not allowed
		warning("make3DO_ADP4Stream(): sample data result is expected to be Big Endian");
		return 0;
	}
	if (audioFlags & Audio::FLAG_STEREO) {
		warning("make3DO_ADP4Stream(): stereo currently not supported");
		return 0;
	}

	if (persistentSpace) {
		memcpy(&decoderData, persistentSpace, sizeof(decoderData));
	} else {
		memset(&decoderData, 0, sizeof(decoderData));
	}

	assert(compressedSize < 0x40000000); // safety check

	decompressedSize = compressedSize * 4; // 4 bits == 1 16-bit sample
	byte *decompressedData = (byte *)malloc(decompressedSize);
	assert(decompressedData);

	if (!(audioFlags & Audio::FLAG_STEREO)) {
		// Mono
		for (streamPos = 0; streamPos < compressedSize; streamPos++) {
			compressedByte = stream->readByte();

			WRITE_BE_UINT16(decompressedData + decompressedPos, audio_3DO_ADP4_DecodeSample(compressedByte >> 4, decoderData.lastSample, decoderData.stepIndex));
			decompressedPos += 2;
			WRITE_BE_UINT16(decompressedData + decompressedPos, audio_3DO_ADP4_DecodeSample(compressedByte & 0x0F, decoderData.lastSample, decoderData.stepIndex));
			decompressedPos += 2;
		}
	}

	if (disposeAfterUse == DisposeAfterUse::YES)
		delete stream;

	if (persistentSpace) {
		memcpy(persistentSpace, &decoderData, sizeof(decoderData));
	}

	// Since we allocated our own buffer for the data, we must specify DisposeAfterUse::YES.
	return makeRawStream(decompressedData, decompressedSize, sampleRate, audioFlags);
}

static int16 audio_3DO_SDX2_SquareTable[256] = {
-32768,-32258,-31752,-31250,-30752,-30258,-29768,-29282,-28800,-28322,
-27848,-27378,-26912,-26450,-25992,-25538,-25088,-24642,-24200,-23762,
-23328,-22898,-22472,-22050,-21632,-21218,-20808,-20402,-20000,-19602,
-19208,-18818,-18432,-18050,-17672,-17298,-16928,-16562,-16200,-15842,
-15488,-15138,-14792,-14450,-14112,-13778,-13448,-13122,-12800,-12482,
-12168,-11858,-11552,-11250,-10952,-10658,-10368,-10082, -9800, -9522,
 -9248, -8978, -8712, -8450, -8192, -7938, -7688, -7442, -7200, -6962,
 -6728, -6498, -6272, -6050, -5832, -5618, -5408, -5202, -5000, -4802,
 -4608, -4418, -4232, -4050, -3872, -3698, -3528, -3362, -3200, -3042,
 -2888, -2738, -2592, -2450, -2312, -2178, -2048, -1922, -1800, -1682,
 -1568, -1458, -1352, -1250, -1152, -1058,  -968,  -882,  -800,  -722,
  -648,  -578,  -512,  -450,  -392,  -338,  -288,  -242,  -200,  -162,
  -128,   -98,   -72,   -50,   -32,   -18,    -8,    -2,     0,     2,
     8,    18,    32,    50,    72,    98,   128,   162,   200,   242,
   288,   338,   392,   450,   512,   578,   648,   722,   800,   882,
   968,  1058,  1152,  1250,  1352,  1458,  1568,  1682,  1800,  1922,
  2048,  2178,  2312,  2450,  2592,  2738,  2888,  3042,  3200,  3362,
  3528,  3698,  3872,  4050,  4232,  4418,  4608,  4802,  5000,  5202,
  5408,  5618,  5832,  6050,  6272,  6498,  6728,  6962,  7200,  7442,
  7688,  7938,  8192,  8450,  8712,  8978,  9248,  9522,  9800, 10082,
 10368, 10658, 10952, 11250, 11552, 11858, 12168, 12482, 12800, 13122,
 13448, 13778, 14112, 14450, 14792, 15138, 15488, 15842, 16200, 16562,
 16928, 17298, 17672, 18050, 18432, 18818, 19208, 19602, 20000, 20402,
 20808, 21218, 21632, 22050, 22472, 22898, 23328, 23762, 24200, 24642,
 25088, 25538, 25992, 26450, 26912, 27378, 27848, 28322, 28800, 29282,
 29768, 30258, 30752, 31250, 31752, 32258
};

SeekableAudioStream *make3DO_SDX2Stream(Common::SeekableReadStream *stream,	uint32 size, uint16 sampleRate, byte audioFlags, DisposeAfterUse::Flag disposeAfterUse, audio_3DO_SDX2_PersistentSpace *persistentSpace) {
	int32 streamPos = 0;
	int32 compressedSize = size;
	int32 decompressedSize = 0;
	int32 decompressedPos = 0;

	int8  compressedByte = 0;
	uint8 squareTableOffset = 0;
	int16 decodedSample = 0;

	audio_3DO_SDX2_PersistentSpace decoderData;

	assert(compressedSize <= stream->size());

	if (audioFlags & Audio::FLAG_UNSIGNED) {
		// Unsigned data is not allowed
		warning("make3DO_SDX2Stream(): sample data result is expected to be signed");
		return 0;
	}
	if (!(audioFlags & Audio::FLAG_16BITS)) {
		// 8-bit sample data is not allowed
		warning("make3DO_SDX2Stream(): sample data result is expected to be 16-bit");
		return 0;
	}
	if (audioFlags & Audio::FLAG_LITTLE_ENDIAN) {
		// LE sample data is not allowed
		warning("make3DO_SDX2Stream(): sample data result is expected to be Big Endian");
		return 0;
	}
	if (audioFlags & Audio::FLAG_STEREO) {
		if (compressedSize & 1) {
			warning("make3DO_SDX2Stream(): stereo data is uneven size");
			return 0;
		}
	}

	if (persistentSpace) {
		memcpy(&decoderData, persistentSpace, sizeof(decoderData));
	} else {
		memset(&decoderData, 0, sizeof(decoderData));
	}

	assert(compressedSize < 0x40000000); // safety check

	decompressedSize = compressedSize * 2; // 1 byte == 1 16-bit sample
	byte *decompressedData = (byte *)malloc(decompressedSize);
	assert(decompressedData);

	if (!(audioFlags & Audio::FLAG_STEREO)) {
		// Mono
		for (streamPos = 0; streamPos < compressedSize; streamPos++) {
			compressedByte = stream->readSByte();
			squareTableOffset = compressedByte + 128;

			if (!(compressedByte & 1))
				decoderData.lastSample1 = 0;

			decodedSample = decoderData.lastSample1 + audio_3DO_SDX2_SquareTable[squareTableOffset];
			decoderData.lastSample1 = decodedSample;

			WRITE_BE_UINT16(decompressedData + decompressedPos, decodedSample);
			decompressedPos += 2;
		}

	} else {
		// Stereo
		for (streamPos = 0; streamPos < compressedSize; streamPos++) {
			compressedByte = stream->readSByte();
			squareTableOffset = compressedByte + 128;

			if (!(streamPos & 1)) {
				// First channel
				if (!(compressedByte & 1))
					decoderData.lastSample1 = 0;

				decodedSample = decoderData.lastSample1 + audio_3DO_SDX2_SquareTable[squareTableOffset];
				decoderData.lastSample1 = decodedSample;
			} else {
				// Second channel
				if (!(compressedByte & 1))
					decoderData.lastSample2 = 0;

				decodedSample = decoderData.lastSample2 + audio_3DO_SDX2_SquareTable[squareTableOffset];
				decoderData.lastSample2 = decodedSample;
			}

			WRITE_BE_UINT16(decompressedData + decompressedPos, decodedSample);
			decompressedPos += 2;
		}
	}

	if (disposeAfterUse == DisposeAfterUse::YES)
		delete stream;

	if (persistentSpace) {
		memcpy(persistentSpace, &decoderData, sizeof(decoderData));
	}

	// Since we allocated our own buffer for the data, we must specify DisposeAfterUse::YES.
	return makeRawStream(decompressedData, decompressedSize, sampleRate, audioFlags);
}

} // End of namespace Audio
