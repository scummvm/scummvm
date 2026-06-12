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

#include "phoenixvr/phoenixvr.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/file.h"
#include "common/language.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/str-enc.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/fonts/ttf.h"
#include "graphics/framelimiter.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "image/gif.h"
#include "image/pcx.h"
#include "phoenixvr/arn.h"
#include "phoenixvr/console.h"
#include "phoenixvr/game_state.h"
#include "phoenixvr/math.h"
#include "phoenixvr/pakf.h"
#include "phoenixvr/region_set.h"
#include "phoenixvr/script.h"
#include "phoenixvr/vr.h"
#include "video/4xm_decoder.h"
#include "video/smk_decoder.h"
#include "video/subtitles.h"
#include <math.h>

namespace PhoenixVR {

PhoenixVREngine *g_engine;

static Common::CodePage getTextCodePage(Common::Language language) {
	switch (language) {
	case Common::RU_RUS:
		return Common::kWindows1251;
	default:
		return Common::kWindows1252;
	}
}

static bool isAmerzoneGame(const ADGameDescription *gameDesc) {
	return !strcmp(gameDesc->gameId, "amerzone");
}

static Common::String getAmerzoneLevelLabel(const Common::String &script) {
	static const struct {
		const char *prefix;
		const char *label;
	} levels[] = {
		{"01VR_PHARE", "Le Phare"},
		{"02VR_ILE", "L'Ile"},
		{"03VR_PUEBLO", "Le Pueblo"},
		{"04VR_FLEUVE", "Le Fleuve"},
		{"05VR_VILLAGEMARAIS", "Le Village"},
		{"07VRTEMPLE_VOLCAN", "Le Temple"}
	};

	for (const auto &level : levels) {
		if (script.hasPrefixIgnoreCase(level.prefix))
			return level.label;
	}

	return "Amerzone";
}

enum Raw4XMChunkId {
	kRaw4XMFile = 0xfb814000,
	kRaw4XMFrameContainer = 0xfb814100,
	kRaw4XMFullFrame = 0xfb814210,
	kRaw4XMDeltaFrame = 0xfb814220,
	kRaw4XMCompressedAudio = 0xfb814230,
	kRaw4XMCachedFrame = 0xfb814240,
	kRaw4XMRawAudio = 0xfb814250
};

struct Raw4XMAudioState {
	int channels = 0;
	int bits = 0;
	int sampleRate = 0;
	int16 predictor[2] = { 0, 0 };
	int stepIndex[2] = { 0, 0 };
};

struct Raw4XMTransform {
	bool swapAxes;
	bool flipX;
	bool flipY;
};

struct Raw4XMCoefficientToken {
	byte zeroes;
	int8 values[2];
	byte valueCount;
};

struct Raw4XMCacheEntry {
	int32 frame = -1;
	uint32 declaredSize = 0;
	Common::Array<byte> data;
};

struct Raw4XMDeltaReader {
	const byte *data = nullptr;
	uint32 size = 0;
	uint32 mode = 0;
	uint32 wordBitsLeft = 16;
	uint32 wordIndex = 1;
	uint32 controlWord = 0;
	uint32 pairIndex = 0;
	uint32 byteIndex = 0;
	uint32 nibbleBitsLeft = 8;
	uint32 nibbleWordIndex = 1;
	uint32 nibbleWord = 0;
	uint32 pairOffset = 0;
	uint32 byteOffset = 0;
	uint32 nibbleOffset = 0;

	Raw4XMDeltaReader(const byte *ptr, uint32 len) : data(ptr), size(len) {
		mode = READ_LE_UINT32(data);
		pairOffset = READ_LE_UINT32(data + 8) + 0x18;
		byteOffset = READ_LE_UINT32(data + 12) + 0x18;
		nibbleOffset = READ_LE_UINT32(data + 16) + 0x18;
		controlWord = READ_LE_UINT32(data + 0x18);
		if (nibbleOffset + 4 <= size)
			nibbleWord = READ_LE_UINT32(data + nibbleOffset);
	}

	bool valid() const {
		return size >= 0x18;
	}

	uint32 readControl2() {
		uint32 result = controlWord & 3;
		if (--wordBitsLeft == 0) {
			uint32 off = 0x18 + wordIndex * 4;
			controlWord = off + 4 <= size ? READ_LE_UINT32(data + off) : 0;
			++wordIndex;
			wordBitsLeft = 16;
		} else {
			controlWord >>= 2;
		}
		return result;
	}

	uint16 readPair() {
		uint32 off = pairOffset + pairIndex * 2;
		++pairIndex;
		return off + 2 <= size ? READ_LE_UINT16(data + off) : 0;
	}

	byte readByteIndex() {
		uint32 off = byteOffset + byteIndex;
		++byteIndex;
		return off < size ? data[off] : 0;
	}

	int8 readSignedByte() {
		return (int8)readByteIndex();
	}

	uint32 readNibble() {
		uint32 result = nibbleWord & 0xf;
		if (--nibbleBitsLeft == 0) {
			uint32 off = nibbleOffset + nibbleWordIndex * 4;
			nibbleWord = off + 4 <= size ? READ_LE_UINT32(data + off) : 0;
			++nibbleWordIndex;
			nibbleBitsLeft = 8;
		} else {
			nibbleWord >>= 4;
		}
		return result;
	}
};

static const int kRaw4XMAudioIndexDelta[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };

static const int kRaw4XMAudioStepTable[89] = {
	    7,     8,     9,    10,    11,    12,    13,    14,
	   16,    17,    19,    21,    23,    25,    28,    31,
	   34,    37,    41,    45,    50,    55,    60,    66,
	   73,    80,    88,    97,   107,   118,   130,   143,
	  157,   173,   190,   209,   230,   253,   279,   307,
	  337,   371,   408,   449,   494,   544,   598,   658,
	  724,   796,   876,   963,  1060,  1166,  1282,  1411,
	 1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
	 3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
	 7132,  7845,  8630,  9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
	32767
};

static const Raw4XMTransform kRaw4XMTransforms[8] = {
	{ false, false, false },
	{ false, true,  false },
	{ false, false, true  },
	{ false, true,  true  },
	{ true,  true,  true  },
	{ true,  false, true  },
	{ true,  true,  false },
	{ true,  false, false }
};

static const int kRaw4XMCoefficientOrder[64] = {
	 0,  1,  8,  9,  2,  3, 10, 11,
	16, 17, 24, 25, 18, 19, 26, 27,
	 4,  5, 12, 20, 13,  6,  7, 14,
	21, 28, 29, 22, 15, 23, 30, 31,
	32, 33, 40, 48, 41, 34, 35, 42,
	49, 56, 57, 50, 43, 51, 58, 59,
	36, 37, 44, 52, 45, 38, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

static const Raw4XMCoefficientToken kRaw4XMCoefficientTokens[8] = {
	{ 64, {  0,  0 }, 0 },
	{  5, {  0,  0 }, 0 },
	{  1, {  1,  0 }, 1 },
	{  1, { -1,  0 }, 1 },
	{  2, {  1,  0 }, 1 },
	{  1, {  0, -1 }, 2 },
	{  2, {  0,  1 }, 2 },
	{  3, { -1,  0 }, 1 }
};

static int raw4XMSign2(byte value) {
	return (value & 2) ? (value | ~3) : (value + 1);
}

static int raw4XMSign4(byte value) {
	return (value & 8) ? (value | ~15) : (value + 1);
}

static byte raw4XMAudioFlags(const Raw4XMAudioState &state) {
	byte flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
	if (state.channels == 2)
		flags |= Audio::FLAG_STEREO;
	return flags;
}

static void raw4XMQueueOwnedAudio(Audio::QueuingAudioStream *audioStream, byte *data, uint32 size, const Raw4XMAudioState &state) {
	if (audioStream && data)
		audioStream->queueBuffer(data, size, DisposeAfterUse::YES, raw4XMAudioFlags(state));
	else
		free(data);
}

static int16 raw4XMDecodeNibble(byte nibble, int16 &predictor, int &stepIndex) {
	const int step = kRaw4XMAudioStepTable[stepIndex];
	int diff = ((step / 2) + (nibble & 7) * step) >> 3;
	if (nibble & 8) {
		diff = -diff;
	}

	predictor = (int16)(predictor + diff);
	stepIndex = CLIP(stepIndex + kRaw4XMAudioIndexDelta[nibble & 7], 0, 88);
	return predictor;
}

static void raw4XMDecodeADPCMChannel(const byte *src, uint32 srcSize, Raw4XMAudioState &state,
		int channel, int16 *dst, uint32 samples) {
	for (uint32 i = 0; i < samples; ++i) {
		uint32 byteOffset = (i >> 3) * 4 + ((i & 7) >> 1);
		if (byteOffset >= srcSize)
			break;

		byte packed = src[byteOffset];
		byte nibble = (i & 1) ? (packed >> 4) : (packed & 0xf);
		dst[i] = raw4XMDecodeNibble(nibble, state.predictor[channel], state.stepIndex[channel]);
	}
}

static void raw4XMQueueAudio(Audio::QueuingAudioStream *audioStream, Raw4XMAudioState &state,
		const byte *src, uint32 srcSize, uint32 decodedBytes) {
	if (!audioStream || state.channels <= 0 || state.bits != 16 || decodedBytes == 0)
		return;

	const uint32 samplesPerChannel = decodedBytes / (state.channels * 2);
	if (samplesPerChannel == 0)
		return;

	byte *out = (byte *)malloc(decodedBytes);
	if (!out)
		return;
	Common::fill(out, out + decodedBytes, 0);
	int16 *pcm = (int16 *)out;

	if (state.channels == 1) {
		raw4XMDecodeADPCMChannel(src, srcSize, state, 0, pcm, samplesPerChannel);
	} else if (state.channels == 2) {
		Common::Array<int16> left;
		Common::Array<int16> right;
		left.resize(samplesPerChannel);
		right.resize(samplesPerChannel);

		const uint32 compressedChannelSize = ((samplesPerChannel + 7) / 8) * 4;
		raw4XMDecodeADPCMChannel(src, MIN<uint32>(srcSize, compressedChannelSize), state, 0, left.data(), samplesPerChannel);
		if (srcSize > compressedChannelSize)
			raw4XMDecodeADPCMChannel(src + compressedChannelSize, srcSize - compressedChannelSize, state, 1, right.data(), samplesPerChannel);

		for (uint32 i = 0; i < samplesPerChannel; ++i) {
			pcm[i * 2] = left[i];
			pcm[i * 2 + 1] = right[i];
		}
	}

	raw4XMQueueOwnedAudio(audioStream, out, decodedBytes, state);
}

static void buildRaw4XMMotionTables(int width, Common::Array<int> &fullOffsets, Common::Array<int> &expOffsets) {
	Common::Array<int> sorted;
	sorted.reserve(0x1000);
	for (int radius = 0; radius < 0x801; ++radius) {
		for (int y = -0x20; y < 0x20; ++y) {
			for (int x = -0x20; x < 0x20; ++x) {
				if (x * x + y * y == radius)
					sorted.push_back(y * width + x);
			}
		}
	}

	fullOffsets.resize(sorted.size());
	for (uint i = 0; i < sorted.size(); ++i)
		fullOffsets[i] = sorted[i];

	static const float kExpMotionScale = 0.021327873691916466f;
	expOffsets.resize(0x100);
	int lastIndex = -1;
	uint out = 0;
	for (int i = 0; out < expOffsets.size(); ++i) {
		int index = static_cast<int>(floorf(expf(i * kExpMotionScale))) - 1;
		if (index >= (int)fullOffsets.size())
			index = fullOffsets.size() - 1;
		if (lastIndex < index) {
			expOffsets[out++] = fullOffsets[index];
			lastIndex = index;
		}
	}
}

static void raw4XMTransformPixel(int sx, int sy, int width, int height, uint32 mode, int &dx, int &dy) {
	const Raw4XMTransform &transform = kRaw4XMTransforms[mode & 7];
	dx = transform.swapAxes ? sy : sx;
	dy = transform.swapAxes ? sx : sy;
	if (transform.flipX)
		dx = width - 1 - dx;
	if (transform.flipY)
		dy = height - 1 - dy;
}

static void copyRaw4XMBlock(uint16 *dst, const uint16 *src, int stride, int width, int height, uint32 mode, int add = 0) {
	const int sourceWidth = (mode >= 4) ? height : width;
	const int sourceHeight = (mode >= 4) ? width : height;

	for (int sy = 0; sy < sourceHeight; ++sy) {
		for (int sx = 0; sx < sourceWidth; ++sx) {
			int dx, dy;
			raw4XMTransformPixel(sx, sy, width, height, mode, dx, dy);
			if (dx >= 0 && dx < width && dy >= 0 && dy < height)
				dst[dy * stride + dx] = src[sy * stride + sx] + add;
		}
	}
}

static void raw4XMReadCoefficientToken(Raw4XMDeltaReader &reader, int coeff[64], int &index) {
	auto zero = [&](int count) {
		for (int i = 0; i < count && index < 64; ++i, ++index)
			coeff[kRaw4XMCoefficientOrder[index]] = 0;
	};
	auto write = [&](int value) {
		if (index < 64)
			coeff[kRaw4XMCoefficientOrder[index++]] = value;
	};
	auto writeSigned2 = [&](byte value) {
		write(raw4XMSign2(value));
	};
	auto writeSigned4 = [&]() {
		write(raw4XMSign4(reader.readNibble()));
	};

	byte code = reader.readNibble();
	if (code <= 7) {
		const Raw4XMCoefficientToken &token = kRaw4XMCoefficientTokens[code];

		zero(code == 0 ? 64 - index : token.zeroes);
		for (byte i = 0; i < token.valueCount; ++i)
			write(token.values[i]);
		return;
	}

	if (code == 8) {
		byte header = reader.readNibble();
		byte pair = reader.readNibble();
		zero((header >> 2) + 1);
		writeSigned2(pair >> 2);
		writeSigned2(pair & 3);

		int valueCount = header & 3;
		if (valueCount > 0) {
			byte extra = reader.readNibble();
			writeSigned2(extra >> 2);
			if (valueCount > 1)
				writeSigned2(extra & 3);
			if (valueCount > 2)
				writeSigned2(reader.readNibble() >> 2);
		}
	} else if (code == 9) {
		byte header = reader.readNibble();
		zero((header >> 2) + 1);
		for (int i = 0; i <= (header & 3); ++i)
			writeSigned4();
	} else if (code == 10) {
		byte value = reader.readNibble();
		writeSigned2(value >> 2);
		writeSigned2(value & 3);
	} else if (code >= 11 && code <= 13) {
		for (byte i = 0; i < code - 10; ++i)
			writeSigned4();
	} else if (code == 14) {
		write(0);
	} else {
		write(reader.readSignedByte());
	}
}

static void raw4XMReadCoefficients(Raw4XMDeltaReader &reader, int coeffs[3][64]) {
	for (int channel = 0; channel < 3; ++channel) {
		int *coeff = coeffs[channel];
		int index = 1;
		coeff[0] = reader.readSignedByte();

		while (index <= 63)
			raw4XMReadCoefficientToken(reader, coeff, index);
	}
}

static void raw4XMTransformCoefficients(int coeff[64], int dst[64], int scaleCode, bool chroma) {
	if (!chroma)
		coeff[0] <<= 1;

	int temp[64] = {};
	const int dcScale = 1 << scaleCode;
	const int stage1Scale = 1 << (scaleCode + 2);
	const int stage2Scale = 1 << (scaleCode + 4);

	int first[10] = {};
	int base0 = (coeff[8] + coeff[0]) * dcScale;
	int base1 = (coeff[0] - coeff[8]) * dcScale;
	int base2 = (coeff[1] + coeff[9]) * dcScale;
	int base3 = (coeff[1] - coeff[9]) * dcScale;
	first[0] = base2 + base0;
	first[1] = base0 - base2;
	first[8] = base3 + base1;
	first[9] = base1 - base3;

	for (int y = 0; y < 4; y += 2) {
		for (int x = 0; x < 4; x += 2) {
			int index = (x >> 1) + (y >> 1) * 8;
			int v0 = first[index] + coeff[index + 0x10] * stage1Scale;
			int v1 = first[index] - coeff[index + 0x10] * stage1Scale;
			int v2 = coeff[index + 2] + coeff[index + 0x12];
			int v3 = coeff[index + 2] * stage1Scale - coeff[index + 0x12] * stage1Scale;
			temp[y * 8 + x] = v2 * stage1Scale + v0;
			temp[y * 8 + x + 1] = v0 - v2 * stage1Scale;
			temp[(y + 1) * 8 + x] = v3 + v1;
			temp[(y + 1) * 8 + x + 1] = v1 - v3;
		}
	}

	for (int y = 0; y < 8; y += 2) {
		for (int x = 0; x < 8; x += 2) {
			int index = (x >> 1) + (y >> 1) * 8;
			int v0 = coeff[index + 0x20] * stage2Scale + temp[index];
			int v1 = temp[index] - coeff[index + 0x20] * stage2Scale;
			int v2 = coeff[index + 4] + coeff[index + 0x24];
			int v3 = coeff[index + 4] * stage2Scale - coeff[index + 0x24] * stage2Scale;
			dst[y * 8 + x] = (v0 + v2 * stage2Scale) >> 6;
			dst[y * 8 + x + 1] = (v0 - v2 * stage2Scale) >> 6;
			dst[(y + 1) * 8 + x] = (v3 + v1) >> 6;
			dst[(y + 1) * 8 + x + 1] = (v1 - v3) >> 6;
		}
	}
}

static void raw4XMWriteDctBlock(const int yBlock[64], const int cbBlock[64], const int crBlock[64],
		uint16 *dst, int stride) {
	for (int y = 0; y < 8; ++y) {
		uint16 *dstPixel = dst + y * stride;
		for (int x = 0; x < 8; ++x) {
			int index = y * 8 + x;
			int luma = yBlock[index] + 0x80;
			int cr = crBlock[index];
			int cb = cbBlock[index];
			int red = (cr + luma) >> 3;
			int green = (luma - ((cr + cb) >> 1)) >> 3;
			int blue = (luma + cb * 2) >> 3;
			red = CLIP(red, 0, 0x1f);
			green = CLIP(green, 0, 0x1f);
			blue = CLIP(blue, 0, 0x1f);
			dstPixel[x] = (red << 10) | (green << 5) | blue;
		}
	}
}

static void raw4XMCopyFrameToScreen(Graphics::ManagedSurface &screenFrame, const Common::Array<uint16> &rawFrame,
		const Graphics::PixelFormat &format, int width, int height) {
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint16 pixel = rawFrame[y * width + x];
			byte r = ((pixel >> 10) & 0x1f) << 3;
			byte g = ((pixel >> 5) & 0x1f) << 3;
			byte b = (pixel & 0x1f) << 3;
			screenFrame.setPixel(x, y, format.RGBToColor(r | (r >> 5), g | (g >> 5), b | (b >> 5)));
		}
	}
}

struct Raw4XMDeltaDecoder {
	Raw4XMDeltaReader reader;
	uint16 *dst = nullptr;
	const uint16 *src = nullptr;
	int frameWidth = 0;
	int frameHeight = 0;
	const Common::Array<int> &fullOffsets;
	const Common::Array<int> &expOffsets;

	Raw4XMDeltaDecoder(const byte *data, uint32 size, uint16 *dstPixels, const uint16 *srcPixels,
			int width, int height, const Common::Array<int> &fullMotionOffsets, const Common::Array<int> &expMotionOffsets) :
			reader(data, size), dst(dstPixels), src(srcPixels), frameWidth(width), frameHeight(height),
			fullOffsets(fullMotionOffsets), expOffsets(expMotionOffsets) {}

	bool copyLeaf(int dstOffset, int blockWidth, int blockHeight, uint32 op) {
		uint32 motionIndex = 0;
		uint32 transform = 0;
		uint32 addFlag = 0;
		if (reader.mode == 0) {
			uint16 packed = reader.readPair();
			motionIndex = packed & 0xfff;
			transform = (packed >> 12) & 7;
			addFlag = packed >> 15;
		} else {
			motionIndex = reader.readByteIndex();
			uint32 packed = reader.readNibble();
			transform = packed & 7;
			addFlag = (packed & 0xf) >> 3;
		}

		int add = 0;
		if (op == 3 && !addFlag)
			return true;

		if (addFlag) {
			uint16 packed = reader.readPair();
			if (op == 3)
				add = (((int8)(((packed >> 5) & 0x1f) - 0x10) & 0xf) * 0x40 +
						((int8)(((packed >> 10) & 0x1f) - 0x10) & 0xf) * 0x800 +
						((byte)((((byte)packed & 0x1f) - 0x10) * 2) & 0x1f));
			else
				add = ((((int8)(((packed >> 10) & 0x1f) - 0x10)) & 0x1f) * 0x20 +
						(((int8)(((packed >> 5) & 0x1f) - 0x10)) & 0x1f)) * 0x20 +
						(((int8)((packed & 0x1f) - 0x10)) & 0x1f);
		}

		const Common::Array<int> &motionTable = reader.mode == 0 ? fullOffsets : expOffsets;
		int srcOffset = dstOffset;
		if (motionIndex < motionTable.size())
			srcOffset += motionTable[motionIndex];
		const int sourceWidth = transform < 4 ? blockWidth : blockHeight;
		const int sourceHeight = transform < 4 ? blockHeight : blockWidth;
		if (srcOffset < 0 || srcOffset + frameWidth * (sourceHeight - 1) + sourceWidth - 1 >= frameWidth * frameHeight)
			return false;

		copyRaw4XMBlock(dst + dstOffset, src + srcOffset, frameWidth, blockWidth, blockHeight, transform, add);
		return true;
	}

	bool decodeDctBlock(int dstOffset) {
		byte scaleY = reader.readNibble();
		byte scaleCb = reader.readNibble();
		byte scaleCr = reader.readNibble();
		int coeffs[3][64] = {};
		int blocks[3][64] = {};
		raw4XMReadCoefficients(reader, coeffs);
		raw4XMTransformCoefficients(coeffs[0], blocks[0], scaleY, false);
		raw4XMTransformCoefficients(coeffs[1], blocks[1], scaleCb, true);
		raw4XMTransformCoefficients(coeffs[2], blocks[2], scaleCr, true);
		raw4XMWriteDctBlock(blocks[0], blocks[1], blocks[2], dst + dstOffset, frameWidth);
		return true;
	}

	bool decodeCopyBlock(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		return copyLeaf(dstOffset, blockWidth, blockHeight, 0);
	}

	bool decodeVerticalSplit(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		if (blockHeight < 2)
			return false;
		decodeBlock(dstOffset, blockWidth, blockHeight / 2, false);
		decodeBlock(dstOffset + frameWidth * (blockHeight / 2), blockWidth, blockHeight / 2, false);
		return true;
	}

	bool decodeHorizontalSplit(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		if (blockWidth < 2)
			return false;
		decodeBlock(dstOffset, blockWidth / 2, blockHeight, false);
		decodeBlock(dstOffset + blockWidth / 2, blockWidth / 2, blockHeight, false);
		return true;
	}

	bool decodeDctOrCopyBlock(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		uint32 subOp = reader.readControl2();
		if (subOp == 0 && allowDct)
			return decodeDctBlock(dstOffset);
		if (subOp == 2)
			return copyLeaf(dstOffset, blockWidth, blockHeight, 3);
		return false;
	}

	bool decodeBlock(int dstOffset, int blockWidth, int blockHeight, bool allowDct) {
		typedef bool (Raw4XMDeltaDecoder::*BlockOp)(int, int, int, bool);
		static const BlockOp kBlockOps[4] = {
			&Raw4XMDeltaDecoder::decodeCopyBlock,
			&Raw4XMDeltaDecoder::decodeVerticalSplit,
			&Raw4XMDeltaDecoder::decodeHorizontalSplit,
			&Raw4XMDeltaDecoder::decodeDctOrCopyBlock
		};

		return (this->*kBlockOps[reader.readControl2() & 3])(dstOffset, blockWidth, blockHeight, allowDct);
	}
};

static bool applyRaw4XMDelta(const byte *data, uint32 size, uint16 *dst, const uint16 *src,
		int width, int height, const Common::Array<int> &fullOffsets, const Common::Array<int> &expOffsets) {
	Raw4XMDeltaDecoder decoder(data, size, dst, src, width, height, fullOffsets, expOffsets);
	if (!decoder.reader.valid())
		return false;

	const int blocksX = (width + 7) / 8;
	const int blocksY = (height + 7) / 8;
	int blockOffset = 0;

	for (int y = 0; y < blocksY; ++y) {
		for (int x = 0; x < blocksX; ++x) {
			decoder.decodeBlock(blockOffset, 8, 8, true);
			blockOffset += 8;
		}
		blockOffset += width * 7;
	}

	return true;
}

static Raw4XMCacheEntry &raw4XMFindCacheEntry(Common::Array<Raw4XMCacheEntry> &cache, int32 frame) {
	for (uint i = 0; i < cache.size(); ++i) {
		if (cache[i].frame == frame)
			return cache[i];
	}

	for (uint i = 0; i < cache.size(); ++i) {
		if (cache[i].frame == -1) {
			cache[i].frame = frame;
			cache[i].declaredSize = 0;
			cache[i].data.clear();
			return cache[i];
		}
	}

	cache[0].frame = frame;
	cache[0].declaredSize = 0;
	cache[0].data.clear();
	return cache[0];
}

static bool decodeRaw4XMContainerPayload(const byte *payload, uint32 payloadSize, uint32 currentFrame,
		Common::Array<uint16> &frame, const Common::Array<uint16> &previousFrame,
		int width, int height, const Common::Array<int> &fullMotionOffsets,
		const Common::Array<int> &expMotionOffsets, Common::Array<Raw4XMCacheEntry> &cache,
		Audio::QueuingAudioStream *audioStream, Raw4XMAudioState &audioState);

static bool raw4XMDecodeFullFrame(const byte *payload, uint32 payloadSize, Common::Array<uint16> &frame,
		int width, int height) {
	if (payloadSize < (uint32)(width * height * 2))
		return false;

	Common::MemoryReadStream fullFrameStream(payload, payloadSize);
	for (int i = 0; i < width * height; ++i)
		frame[i] = fullFrameStream.readUint16LE();
	return true;
}

static void raw4XMDecodeCompressedAudio(const byte *payload, uint32 payloadSize, Audio::QueuingAudioStream *audioStream,
		Raw4XMAudioState &audioState) {
	if (payloadSize < 4)
		return;

	const uint32 decodedBytes = READ_LE_UINT32(payload);
	raw4XMQueueAudio(audioStream, audioState, payload + 4, payloadSize - 4, decodedBytes);
}

static bool raw4XMDecodeCachedFrame(const byte *payload, uint32 payloadSize, uint32 currentFrame,
		Common::Array<uint16> &frame, const Common::Array<uint16> &previousFrame,
		int width, int height, const Common::Array<int> &fullMotionOffsets,
		const Common::Array<int> &expMotionOffsets, Common::Array<Raw4XMCacheEntry> &cache,
		Audio::QueuingAudioStream *audioStream, Raw4XMAudioState &audioState) {
	if (payloadSize < 8)
		return false;

	const int32 cacheFrame = (int32)READ_LE_UINT32(payload);
	const uint32 declaredSize = READ_LE_UINT32(payload + 4);
	Raw4XMCacheEntry &entry = raw4XMFindCacheEntry(cache, cacheFrame);
	entry.declaredSize = declaredSize;
	const uint oldSize = entry.data.size();
	entry.data.resize(oldSize + payloadSize - 8);
	Common::copy(payload + 8, payload + payloadSize, entry.data.begin() + oldSize);

	if (cacheFrame != (int32)currentFrame || entry.data.size() < 8 ||
			READ_LE_UINT32(entry.data.data()) != kRaw4XMFrameContainer)
		return false;

	const uint32 nestedDeclaredSize = entry.declaredSize >= 8 ? entry.declaredSize : READ_LE_UINT32(entry.data.data() + 4);
	const uint32 nestedPayloadSize = MIN<uint32>(entry.data.size() - 8, nestedDeclaredSize - 8);
	const bool changed = decodeRaw4XMContainerPayload(entry.data.data() + 8, nestedPayloadSize, currentFrame,
		frame, previousFrame, width, height, fullMotionOffsets, expMotionOffsets, cache, audioStream, audioState);

	entry.frame = -1;
	entry.declaredSize = 0;
	entry.data.clear();
	return changed;
}

static void raw4XMDecodeRawAudio(const byte *payload, uint32 payloadSize, Audio::QueuingAudioStream *audioStream,
		Raw4XMAudioState &audioState) {
	if (payloadSize < 4 || !audioStream)
		return;

	byte *rawAudio = (byte *)malloc(payloadSize - 4);
	if (!rawAudio)
		return;

	Common::copy(payload + 4, payload + payloadSize, rawAudio);
	raw4XMQueueOwnedAudio(audioStream, rawAudio, payloadSize - 4, audioState);
}

static bool decodeRaw4XMContainerPayload(const byte *payload, uint32 payloadSize, uint32 currentFrame,
		Common::Array<uint16> &frame, const Common::Array<uint16> &previousFrame,
		int width, int height, const Common::Array<int> &fullMotionOffsets,
		const Common::Array<int> &expMotionOffsets, Common::Array<Raw4XMCacheEntry> &cache,
		Audio::QueuingAudioStream *audioStream, Raw4XMAudioState &audioState) {
	bool changed = false;
	uint32 pos = 0;
	while (pos + 8 <= payloadSize) {
		uint32 subChunkId = READ_LE_UINT32(payload + pos);
		uint32 subChunkSize = READ_LE_UINT32(payload + pos + 4);
		if (subChunkSize < 8 || pos + subChunkSize > payloadSize)
			break;

		const byte *subPayload = payload + pos + 8;
		const uint32 subPayloadSize = subChunkSize - 8;
		if (subChunkId == kRaw4XMFullFrame) {
			changed = raw4XMDecodeFullFrame(subPayload, subPayloadSize, frame, width, height) || changed;
		} else if (subChunkId == kRaw4XMDeltaFrame) {
			changed = applyRaw4XMDelta(subPayload, subPayloadSize, frame.data(), previousFrame.data(), width, height,
				fullMotionOffsets, expMotionOffsets) || changed;
		} else if (subChunkId == kRaw4XMCachedFrame) {
			changed = raw4XMDecodeCachedFrame(subPayload, subPayloadSize, currentFrame, frame, previousFrame,
				width, height, fullMotionOffsets, expMotionOffsets, cache, audioStream, audioState) || changed;
		} else if (subChunkId == kRaw4XMCompressedAudio) {
			raw4XMDecodeCompressedAudio(subPayload, subPayloadSize, audioStream, audioState);
		} else if (subChunkId == kRaw4XMRawAudio) {
			raw4XMDecodeRawAudio(subPayload, subPayloadSize, audioStream, audioState);
		}

		pos += subChunkSize;
	}

	return changed;
}

static const char *mfull[] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};

static const char *wday[] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

static Common::String makeSaveText(const Common::String &firstLine, const Common::String &secondLine) {
	Common::String result = firstLine;
	result += '\0';
	result += secondLine;
	return result;
}

static Common::String makeSaveText(const Common::String &firstLine, const Common::String &secondLine, const Common::String &thirdLine) {
	Common::String result = makeSaveText(firstLine, secondLine);
	result += '\0';
	result += thirdLine;
	return result;
}

static Common::String formatSaveInfo(const TimeDate &td, bool longDate, const Common::String &place = Common::String()) {
	if (longDate) {
		return makeSaveText(
			Common::String::format("%s, %s %d, %04d", wday[td.tm_wday], mfull[td.tm_mon], td.tm_mday, td.tm_year + 1900),
			Common::String::format("%02d:%02d:%02d %s", td.tm_hour, td.tm_min, td.tm_sec, td.tm_hour < 12 ? "AM" : "PM"),
			place);
	}

	return makeSaveText(
		Common::String::format("%s %02d %02d %04d", wday[td.tm_wday], td.tm_mday, td.tm_mon + 1, td.tm_year + 1900),
		Common::String::format("%02d h %02d", td.tm_hour, td.tm_min));
}

static int mapSaveSlotY(int y, bool splitV, int tileY) {
	int splitLine = (tileY + 1) * 256;
	if (splitV && y >= splitLine)
		return (tileY + 3) * 256 + y - splitLine;
	return y;
}

static void fillSaveSlotRect(Graphics::Surface &dst, const Common::Rect &rect, uint32 color, bool splitV, int tileY) {
	if (splitV) {
		int splitLine = (tileY + 1) * 256;
		int topH = CLIP<int>(splitLine - rect.top, 0, rect.height());
		if (topH > 0) {
			Common::Rect top = rect;
			top.bottom = rect.top + topH;
			dst.fillRect(top, color);
		}
		if (topH < rect.height()) {
			int bottomY = (tileY + 3) * 256 + MAX(rect.top - splitLine, 0);
			Common::Rect bottom(rect.left, bottomY, rect.right, bottomY + rect.height() - topH);
			dst.fillRect(bottom, color);
		}
	} else {
		dst.fillRect(rect, color);
	}
}

static int drawSaveTextBlock(Graphics::Surface &dst, const Graphics::Font *font, const Common::String &text,
		int x, int y, int width, uint32 color, Graphics::TextAlign align, int lineHeight, bool splitV, int tileY,
		bool reserveEmptyFinalLine = false) {
	bool hasText = false;
	for (uint i = 0; i < text.size(); ++i) {
		if (text[i] != '\n' && text[i] != '\0') {
			hasText = true;
			break;
		}
	}
	if (!hasText)
		return y;

	uint start = 0;
	for (uint i = 0; i < text.size(); ++i) {
		if (text[i] == '\n' || text[i] == '\0') {
			if (i > start) {
				Common::String line;
				for (uint j = start; j < i; ++j)
					line += text[j];
				font->drawString(&dst, line, x, mapSaveSlotY(y, splitV, tileY), width, color, align);
				y += lineHeight;
			} else if (reserveEmptyFinalLine && i == text.size() - 1) {
				y += lineHeight;
			}
			start = i + 1;
		}
	}
	if (start < text.size()) {
		Common::String line;
		for (uint j = start; j < text.size(); ++j)
			line += text[j];
		font->drawString(&dst, line, x, mapSaveSlotY(y, splitV, tileY), width, color, align);
		y += lineHeight;
	}

	return y;
}

static int saveCardTileId(int face, int x, int y) {
	return (face << 2) + ((y < 256) ? (x < 256 ? 0 : 1) : (x < 256 ? 3 : 2));
}

static void copyCubeFaceToSurface(Graphics::ManagedSurface &faceSurface, const Graphics::Surface &vrSurface, int face) {
	for (int y = 0; y < 512; ++y) {
		for (int x = 0; x < 512; ++x) {
			const int tileId = saveCardTileId(face, x, y);
			faceSurface.setPixel(x, y, vrSurface.getPixel(x & 0xff, (tileId << 8) + (y & 0xff)));
		}
	}
}

static void copySurfaceToCubeFace(Graphics::Surface &vrSurface, const Graphics::ManagedSurface &faceSurface, int face) {
	for (int y = 0; y < 512; ++y) {
		for (int x = 0; x < 512; ++x) {
			const int tileId = saveCardTileId(face, x, y);
			vrSurface.setPixel(x & 0xff, (tileId << 8) + (y & 0xff), faceSurface.getPixel(x, y));
		}
	}
}

static void projectSaveCard(Graphics::ManagedSurface &faceSurface, const Graphics::ManagedSurface &card, float angle) {
	struct Vertex {
		float x;
		float y;
		float invW;
		float uOverW;
		float vOverW;
	};

	const float srcW = static_cast<float>(card.w);
	const float srcH = static_cast<float>(card.h);
	const float distance = srcW / 8.0f + srcW * 8.0f / 6.283100128173828f;
	const float cosA = cosf(angle);
	const float sinA = sinf(angle);

	auto makeVertex = [&](float modelU, float modelV, float textureU, float textureV) {
		const float modelX = modelU - srcW / 2.0f;
		const float modelY = distance;
		const float modelZ = srcH / 2.0f - modelV + 32.0f;
		const float projectedW = (modelX * sinA - modelY * cosA) / 256.0f;
		const float invW = 1.0f / projectedW;

		Vertex vertex;
		vertex.x = (modelX * (cosA + sinA) + modelY * (sinA - cosA)) * invW;
		vertex.y = (modelX * sinA - modelY * cosA - modelZ) * invW;
		vertex.invW = invW;
		vertex.uOverW = textureU * invW;
		vertex.vOverW = textureV * invW;
		return vertex;
	};

	Vertex vertices[4] = {
		makeVertex(0.0f, 0.0f, srcW, srcH),
		makeVertex(static_cast<float>(card.w), 0.0f, 0.0f, srcH),
		makeVertex(static_cast<float>(card.w), static_cast<float>(card.h), 0.0f, 0.0f),
		makeVertex(0.0f, static_cast<float>(card.h), srcW, 0.0f)
	};

	auto rasterizeTriangle = [&](const Vertex &a, const Vertex &b, const Vertex &c) {
		const float area = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
		if (ABS(area) < 0.0001f)
			return;

		int minX = CLIP<int>(static_cast<int>(floorf(MIN(a.x, MIN(b.x, c.x)))), 0, faceSurface.w - 1);
		int maxX = CLIP<int>(static_cast<int>(ceilf(MAX(a.x, MAX(b.x, c.x)))), 0, faceSurface.w - 1);
		int minY = CLIP<int>(static_cast<int>(floorf(MIN(a.y, MIN(b.y, c.y)))), 0, faceSurface.h - 1);
		int maxY = CLIP<int>(static_cast<int>(ceilf(MAX(a.y, MAX(b.y, c.y)))), 0, faceSurface.h - 1);

		for (int y = minY; y <= maxY; ++y) {
			for (int x = minX; x <= maxX; ++x) {
				const float px = static_cast<float>(x) + 0.5f;
				const float py = static_cast<float>(y) + 0.5f;
				const float w0 = ((b.x - px) * (c.y - py) - (b.y - py) * (c.x - px)) / area;
				const float w1 = ((c.x - px) * (a.y - py) - (c.y - py) * (a.x - px)) / area;
				const float w2 = 1.0f - w0 - w1;
				if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f)
					continue;

				const float invW = w0 * a.invW + w1 * b.invW + w2 * c.invW;
				if (ABS(invW) < 0.0001f)
					continue;
				const float u = (w0 * a.uOverW + w1 * b.uOverW + w2 * c.uOverW) / invW;
				const float v = (w0 * a.vOverW + w1 * b.vOverW + w2 * c.vOverW) / invW;
				if (u < 0.0f || u > srcW || v < 0.0f || v > srcH)
					continue;

				const int srcX = CLIP<int>(static_cast<int>(floorf(u)), 0, card.w - 1);
				const int srcY = CLIP<int>(static_cast<int>(floorf(v)), 0, card.h - 1);
				faceSurface.setPixel(x, y, card.getPixel(srcX, srcY));
			}
		}
	};

	rasterizeTriangle(vertices[0], vertices[1], vertices[2]);
	rasterizeTriangle(vertices[0], vertices[2], vertices[3]);
}

PhoenixVREngine::PhoenixVREngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																					 _frameLimiter(g_system, kFPSLimit),
																					 _gameDescription(gameDesc),
																					 _randomSource("PhoenixVR"),
																					 _rgb565(2, 5, 6, 5, 0, 11, 5, 0, 0),
																					 _thumbnail(isAmerzoneGame(gameDesc) ? 232 : 139, isAmerzoneGame(gameDesc) ? 174 : 103, _rgb565),
																					 _lockKey(13),
																					 _fov(kPi2),
																					 _angleX(0),
																					 _angleY(-kPi2),
																					 _mixer(syst->getMixer()) {
	g_engine = this;

	if (gameIdMatches("amerzone")) {
		_levels.push_back("01VR_PHARE");
		_levels.push_back("02VR_ILE");
		_levels.push_back("03VR_PUEBLO");
		_levels.push_back("04VR_FLEUVE");
		_levels.push_back("05VR_VILLAGEMARAIS");
		_levels.push_back("07VRTEMPLE_VOLCAN");
	}
}

void PhoenixVREngine::resetState() {
	_angleX.resetRange();
	_angleX.set(0);
	_angleY.resetRange();
	_angleY.set(-kPi2);
	_imageOverlay.reset();
	_cibleActive = false;
	_cibleBounds.clear();
}

PhoenixVREngine::~PhoenixVREngine() {
	_system->lockMouse(false);
	for (auto it = _cursorCache.begin(); it != _cursorCache.end(); ++it) {
		auto *s = it->_value;
		s->free();
		delete s;
	}
	delete _screen;
}

void PhoenixVREngine::showWaves() {
	_vr.showWaves();
}

uint32 PhoenixVREngine::getFeatures() const {
	return _gameDescription->flags;
}

bool PhoenixVREngine::gameIdMatches(const char *gameId) const {
	return strcmp(_gameDescription->gameId, gameId) == 0;
}

uint PhoenixVREngine::currentAmerzoneLevel() const {
	if (!gameIdMatches("amerzone"))
		return 0;

	uint index = 0;
	for (const Common::String &level : _levels) {
		++index;
		if (_contextScript.hasPrefixIgnoreCase(level))
			return index;
	}

	return _currentLevel;
}

Common::String PhoenixVREngine::removeDrive(const Common::String &path) {
	if (path.size() < 2 || path[1] != ':')
		return path;
	else
		return path.substr(2);
}

Common::SeekableReadStream *PhoenixVREngine::tryOpen(const Common::Path &name, Common::String *origName) {
	Common::ScopedPtr<Common::File> s(new Common::File());
	if (s->open(name)) {
		auto nameStr = name.toString();
		debug("opened %s", nameStr.c_str());
		if (nameStr.hasSuffixIgnoreCase(".pak"))
			return unpack(*s, origName);
		return s.release();
	}
	auto pakName = name.toString();
	auto dotPos = pakName.rfind('.');
	if (dotPos == pakName.npos)
		return nullptr;
	pakName = pakName.substr(0, dotPos) + ".pak";
	if (s->open(Common::Path{pakName})) {
		debug("opened %s", pakName.c_str());
		return unpack(*s, origName);
	}

	return nullptr;
}

Common::SeekableReadStream *PhoenixVREngine::open(const Common::String &filename, Common::String *origName) {
	debug("open %s", filename.c_str());
	auto *stream = tryOpen(_currentScriptPath.append(filename, '\\').normalize(), origName);
	if (stream)
		return stream;

	stream = tryOpen(Common::Path{filename}, origName);
	if (stream)
		return stream;

	return nullptr;
}

bool PhoenixVREngine::setNextLevel() {
	if (_currentLevel < _levels.size()) {
		auto &level = _levels[_currentLevel++];
		debug("next level is %s", level.c_str());
		setNextScript(Common::String::format("%s\\%s.lst", level.c_str(), _gameDescription->gameId));
		_loaded = true;

		// reset flag or interface.vr will skip menu
		if (_currentLevel == 1)
			_loaded = false;
		return true;
	} else
		return false;
}

void PhoenixVREngine::setNextScript(const Common::String &nextScript) {
	debug("setNextScript %s", nextScript.c_str());
	_contextScript = nextScript;
	if (nextScript.find('\\') == nextScript.npos) {
		// simple filename, e.g. "script.lst"
		_nextScript = nextScript;
		return;
	}

	auto nextPath = Common::Path(removeDrive(nextScript), '\\');
	_currentScriptPath = nextPath.getParent();
	debug("changed script directory to %s", _currentScriptPath.toString().c_str());
	_nextScript = nextPath.getLastComponent().toString();
}

void PhoenixVREngine::loadNextScript() {
	debug("loading script from %s", _nextScript.c_str());
	auto nextScript = Common::move(_nextScript);
	_nextScript.clear();

	Common::ScopedPtr<Common::SeekableReadStream> s(open(nextScript));
	if (!s)
		error("can't open script file %s", nextScript.c_str());

	_script.reset(new Script(*s));
	for (auto &var : _script->getVarNames())
		declareVariable(var);
	if (gameIdMatches("amerzone"))
		declareVariable("oeuf_pose"); // crash in chapter 7
	if (gameIdMatches("dracula1")) {
		declareVariable("P_Alliance"); // Referenced by 0M1Script.lst, declared by 0M2Script.lst
		declareVariable("reloaddone"); // Referenced by InsertCD.lst, declared by chapter scripts
	}
	if (gameIdMatches("dracula2")) {
		for (const char *var : {
			"ComeFromCombinaison",
			"Diag_Seward_Intro",
			"Etat_CarnetDracCode",
			"Etat_Inventaire_ValeurRoue1",
			"Etat_Inventaire_ValeurRoue2",
			"Etat_Inventaire_ValeurRoue3",
			"Etat_Inventaire_ValeurRoue4",
			"Etat_Passerelle",
			"Etat_TelChateau",
			"InventairePos50",
			"ResObj",
			"ResObj_inverse",
			"Resultat_inventaire"
		})
			declareVariable(var);
	}

	int numWarps = _script->numWarps();
	_cursors.clear();
	_cursors.resize(numWarps);
	for (int i = 0; i != numWarps; ++i) {
		auto warp = _script->getWarp(i);
		_cursors[i].resize(warp->tests.size());
	}
	_warpIdx = 0;
	resetState();
}

void PhoenixVREngine::end() {
	debug("end");
	stopAllSounds();
	if (_nextScript.empty() && _nextWarp < 0) {
		if (!setNextLevel()) {
			debug("quit game");
			quitGame();
		}
	}
}

void PhoenixVREngine::interpolateAngle(float x, float y, float speed, float zoom) {
	debug("interpolateAngle %g,%g, speed: %g, zoom: %g", x, y, speed, zoom);
	unsigned frameDuration = 0;
	static constexpr float kDuration = 4096 * 16 / 1000.0f;
	auto x0 = _angleY.angle() + kPi2, y0 = _angleX.angle(), z0 = _fov;
	auto dx = x - x0, dy = y - y0, dz = zoom - z0;
	if (dy < -kPi)
		dy += kTau;
	if (dy > kPi)
		dy -= kTau;
	if (dx < -kPi)
		dx += kTau;
	if (dx > kPi)
		dx -= kTau;
	debug("dx: %g, dy: %g, dz: %g", dx, dy, dz);
	float t = 0;
	bool waiting = true;
	while (!shouldQuit() && waiting && t < kDuration) {
		auto t1 = t / kDuration; // normalise to 0..1 range
		// angles are animated using square function, zoom is linear
		auto t2 = t1 * t1;

		setAngle(x0 + t2 * dx, y0 + t2 * dy);
		if (zoom > 0) {
			setZoom(z0 + t1 * dz);
		}

		renderVR(frameDuration / 1000.0f);

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				if (event.kbd.ascii == ' ') {
					waiting = false;
				}
				break;
			}
			default:
				break;
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_frameLimiter.delayBeforeSwap();
		_screen->update();
		frameDuration = _frameLimiter.startFrame();
		t += frameDuration / 1000.0f * speed;
	}
	setAngle(x, y);
	if (zoom > 0)
		setZoom(zoom);
}

void PhoenixVREngine::renderFade(int color) {
	auto &format = _screen->format;
	for (int y = 0; y != _screen->h; ++y) {
		for (int x = 0; x != _screen->w; ++x) {
			uint8 r, g, b;
			format.colorToRGB(_screen->getPixel(x, y), r, g, b);
			int ri = CLIP(static_cast<int>(r) + color, 0, 255);
			int gi = CLIP(static_cast<int>(g) + color, 0, 255);
			int bi = CLIP(static_cast<int>(b) + color, 0, 255);
			_screen->setPixel(x, y, format.RGBToColor(ri, gi, bi));
		}
	}
}

void PhoenixVREngine::fade(int start, int stop, int speed) {
	debug("fade %d %d speed: %d", start, stop, speed);

	if (start == stop)
		return;

	bool waiting = true;
	float pos = start, dt = 0;
	bool increment = start < stop;
	if (!increment)
		speed = -speed;

	float speedMs = speed * 1000.0f / 16;

	while (!shouldQuit() && waiting && (increment ? pos < stop : pos > stop)) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				if (event.kbd.ascii == ' ') {
					waiting = false;
				}
				break;
			}

			default:
				break;
			}
		}
		renderVR(dt);
		renderFade(pos);

		pos += dt * speedMs + ((speed >= 0) ? 1 : -1);

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_frameLimiter.delayBeforeSwap();
		_screen->update();
		dt = _frameLimiter.startFrame() / 1000.0f;
	}
}

static uint32 transFadePixel(const Graphics::PixelFormat &format, uint32 left, int leftAmount, uint32 right, int rightAmount) {
	uint8 lr, lg, lb, rr, rg, rb;
	format.colorToRGB(left, lr, lg, lb);
	format.colorToRGB(right, rr, rg, rb);
	return format.RGBToColor(
		CLIP(CLIP(static_cast<int>(lr) + leftAmount, 0, 255) + CLIP(static_cast<int>(rr) + rightAmount, 0, 255), 0, 255),
		CLIP(CLIP(static_cast<int>(lg) + leftAmount, 0, 255) + CLIP(static_cast<int>(rg) + rightAmount, 0, 255), 0, 255),
		CLIP(CLIP(static_cast<int>(lb) + leftAmount, 0, 255) + CLIP(static_cast<int>(rb) + rightAmount, 0, 255), 0, 255));
}

void PhoenixVREngine::transFade(int speed) {
	debug("transfade speed: %d", speed);

	Graphics::ManagedSurface oldFrame(_screen->w, _screen->h, _screen->format);
	Graphics::ManagedSurface newFrame(_screen->w, _screen->h, _screen->format);
	Graphics::ManagedSurface workFrame(_screen->w, _screen->h, _screen->format);

	oldFrame.simpleBlitFrom(*_screen);
	renderVR(0);
	newFrame.simpleBlitFrom(*_screen);

	bool waiting = true;
	float dt = 0;

	auto renderTransition = [&](int oldAmount, int newAmount) {
		for (int y = 0; y < _screen->h; ++y) {
			for (int x = 0; x < _screen->w; ++x) {
				workFrame.setPixel(x, y, transFadePixel(_screen->format, oldFrame.getPixel(x, y), oldAmount, newFrame.getPixel(x, y), newAmount));
			}
		}
		_screen->simpleBlitFrom(workFrame);
	};

	auto runTransition = [&](int pos, int direction) {
		while (!shouldQuit() && waiting && (direction > 0 ? pos < 0 : pos > -256)) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_KEYDOWN && event.kbd.ascii == ' ')
					waiting = false;
			}

			renderTransition(direction > 0 ? 0 : pos, direction > 0 ? pos : 0);
			_frameLimiter.delayBeforeSwap();
			_screen->update();
			dt = _frameLimiter.startFrame() / 1000.0f;

			pos += direction * static_cast<int>(dt * speed * 1000.0f / 16);
			if (direction > 0 ? pos < 0 : pos > -256)
				pos += direction;
		}
	};

	runTransition(-255, 1);
	runTransition(0, -1);
}

void PhoenixVREngine::until(const Common::String &var, int value) {
	debug("until %s %d", var.c_str(), value);
	unsigned frameDuration = 0;
	while (!shouldQuit() && getVariable(var) != value) {
		Common::Event event;
		renderVR(frameDuration / 1000.0f);
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			default:
				break;
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_frameLimiter.delayBeforeSwap();
		drawAudioSubtitles();
		_screen->update();
		frameDuration = _frameLimiter.startFrame();
	}
}

void PhoenixVREngine::wait(float seconds) {
	debug("wait %gs", seconds);
	auto begin = g_system->getMillis();
	unsigned millis = seconds * 1000;
	bool waiting = true;
	unsigned frameDuration = 0;
	while (!shouldQuit() && waiting && g_system->getMillis() - begin < millis) {
		Common::Event event;
		renderVR(frameDuration / 1000.0f);
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				if (event.kbd.ascii == ' ') {
					waiting = false;
				}
				break;
			}

			default:
				break;
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_frameLimiter.delayBeforeSwap();
		drawAudioSubtitles();
		_screen->update();
		frameDuration = _frameLimiter.startFrame();
	}
}

void PhoenixVREngine::restart() {
	debug("restart");
	resetState();
	_restarted = true;
	_currentLevel = 0;
	setNextLevel();
	_prevWarp = -1;
	_loaded = false;
}

bool PhoenixVREngine::goToWarp(const Common::String &warp, bool savePrev) {
	debug("gotowarp %s, save prev: %d", warp.c_str(), savePrev);
	if (_warp && _warp->vrFile == warp) {
		debug("already at this location, skipping");
		return false;
	}

	// Typo in Necronomicon's Script4.lst
	if (gameIdMatches("necrono") && warp == "N3M09L03W515E1.vr")
		_nextWarp = _script->getWarp("N3M09L03W51E1.vr");
	else
		_nextWarp = _script->getWarp(warp);

	_hoverIndex = -1;
	if (savePrev) {
		assert(_warpIdx >= 0);
		_prevWarp = _warpIdx;
		// saving thumbnail
		Common::ScopedPtr<Graphics::ManagedSurface> screenshot(_screen->scale(_thumbnail.w, _thumbnail.h, true));
		screenshot->convertToInPlace(_rgb565);
		_thumbnail.simpleBlitFrom(*screenshot, Graphics::FLIP_V);
	}
	return true;
}

void PhoenixVREngine::returnToWarp() {
	if (_prevWarp < 0) {
		warning("return: no previous warp");
	}
	debug("returning to previous warp: %d", _prevWarp);
	_nextWarp = _prevWarp;
	_prevWarp = -1;
}

const Region *PhoenixVREngine::getRegion(int idx) const {
	if (!_regSet)
		return nullptr;
	return (idx < static_cast<int>(_regSet->size())) ? &_regSet->getRegion(idx) : nullptr;
}

void PhoenixVREngine::setCursorDefault(int idx, const Common::String &path) {
	debug("setCursorDefault %d: %s", idx, path.c_str());
	if (idx == 0 || idx == 1) {
		_defaultCursor[idx] = path;
	} else
		warning("only 2 default cursors supported, got %d", idx);
}

void PhoenixVREngine::setCursor(const Common::String &path, const Common::String &wname, int idx) {
	debug("setCursor %s %s:%d", path.c_str(), wname.c_str(), idx);
	auto warp = _script->getWarp(wname);
	if (warp < 0) {
		debug("no warp %s", wname.c_str());
		return;
	}
	auto &cursors = _cursors[warp];
	if (idx >= 0 && idx < static_cast<int>(cursors.size()))
		cursors[idx] = path;
	else
		debug("index %d is out of range", idx);
}

void PhoenixVREngine::hideCursor(const Common::String &wname, int idx) {
	debug("hide cursor %s:%d", wname.c_str(), idx);
	auto warp = _script->getWarp(wname);
	if (warp < 0) {
		debug("no warp %s", wname.c_str());
		return;
	}
	auto &cursors = _cursors[warp];
	if (idx >= 0 && idx < static_cast<int>(cursors.size()))
		cursors[idx].clear();
	else
		debug("index %d is out of range", idx);
}

void PhoenixVREngine::declareVariable(const Common::String &name) {
	if (!_variables.contains(name))
		_variables.setVal(name, 0);
}

void PhoenixVREngine::setVariable(const Common::String &name, int value) {
	debug("set %s %d", name.c_str(), value);
	_variables.setVal(name, value);
}

int PhoenixVREngine::getVariable(const Common::String &name) const {
	if (gameIdMatches("lochness") && name == "tumuAccpet")
		return _variables.getVal("tumuAccept");
	if (gameIdMatches("dracula1") && name == "Resultay_inverse")
		return _variables.getVal("resultat_inverse");
	return _variables.getVal(name);
}

void PhoenixVREngine::playSound(const Common::String &sound, Audio::Mixer::SoundType type, uint8 volume, int loops, bool spatial, float angle) {
	const bool music = type == Audio::Mixer::kMusicSoundType;
	debug("play sound %s %d %d, music: %d, 3d: %d, angle: %g", sound.c_str(), volume, loops, music, spatial, angle);
	if (_sounds.contains(sound)) {
		debug("already playing, skipping...");
		return;
	}
	Audio::SoundHandle h;
	Common::ScopedPtr<Common::SeekableReadStream> stream(open(sound));
	if (!stream) {
		warning("can't load sound %s", sound.c_str());
		return;
	}

	if (music) {
		if (!_currentMusic.empty())
			stopSound(_currentMusic);
		_currentMusic = sound;
		_currentMusicVolume = volume;
	}

	_mixer->playStream(type, &h, Audio::makeWAVStream(stream.release(), DisposeAfterUse::YES), -1, volume);
	if (loops < 0 || music)
		_mixer->loopChannel(h);
	Common::SharedPtr<Video::Subtitles> subtitles;
	if (!music)
		subtitles = loadSubtitles(sound);

	_sounds[sound] = Sound{h, spatial, angle, volume, loops, subtitles};
}

void PhoenixVREngine::stopSound(const Common::String &sound) {
	debug("stop sound %s", sound.c_str());
	if (sound == _currentMusic)
		_currentMusic.clear();
	auto it = _sounds.find(sound);
	if (it != _sounds.end()) {
		_mixer->stopHandle(it->_value.handle);
		if (it->_value.subtitles)
			it->_value.subtitles->clearSubtitle();
		_sounds.erase(it);
	}
}

void PhoenixVREngine::stopAllSounds() {
	_mixer->stopAll();
	_currentMusic.clear();
	for (auto &kv : _sounds) {
		if (kv._value.subtitles)
			kv._value.subtitles->clearSubtitle();
	}
	_sounds.clear();
}

Common::Path PhoenixVREngine::getSubtitlePath(const Common::String &path) const {
	Common::Path assetPath(removeDrive(path), '\\');
	Common::String filename = assetPath.toString('/') + ".srt";
	filename.replace('/', '_');
	filename.replace('\\', '_');

	Common::String language = Common::getLanguageCode(_gameDescription->language);
	if (language == "us")
		language = "en";

	return Common::Path("subtitle").appendComponent(language).appendComponent(filename);
}

Common::SharedPtr<Video::Subtitles> PhoenixVREngine::loadSubtitles(const Common::String &path) const {
	Common::SharedPtr<Video::Subtitles> subtitles;
	if (!ConfMan.getBool("subtitles"))
		return subtitles;

	subtitles = Common::SharedPtr<Video::Subtitles>(new Video::Subtitles());
	subtitles->loadSRTFile(getSubtitlePath(path));
	if (!subtitles->isLoaded())
		return Common::SharedPtr<Video::Subtitles>();

	setupSubtitles(*subtitles);
	return subtitles;
}

void PhoenixVREngine::setupSubtitles(Video::Subtitles &subtitles) const {
	// Subtitle positioning constants (as percentages of screen height)
	const int HORIZONTAL_MARGIN = 20;
	const int MIN_BOTTOM_MARGIN = 4;
	const int MIN_SUBTITLE_HEIGHT = 90;
	const float BOTTOM_MARGIN_PERCENT = 0.01f;
	const float SUBTITLE_HEIGHT_PERCENT = 0.2f;

	// Font sizing constants (as percentage of screen height)
	const int MIN_FONT_SIZE = 18;
	const float BASE_FONT_SIZE_PERCENT = 1.0f / 36.0f;

	int16 h = g_system->getOverlayHeight();
	int16 w = g_system->getOverlayWidth();
	int bottomMargin = MAX<int>(MIN_BOTTOM_MARGIN, int(h * BOTTOM_MARGIN_PERCENT));
	int topOffset = MAX<int>(MIN_SUBTITLE_HEIGHT, int(h * SUBTITLE_HEIGHT_PERCENT));
	int fontSize = MAX<int>(MIN_FONT_SIZE, int(h * BASE_FONT_SIZE_PERCENT));

	subtitles.setBBox(Common::Rect(HORIZONTAL_MARGIN, h - topOffset, w - HORIZONTAL_MARGIN, h - bottomMargin));
	subtitles.setColor(0xff, 0xff, 0x80);
	subtitles.setFont("LiberationSans-Regular.ttf", fontSize, Video::Subtitles::kFontStyleRegular);
	subtitles.setFont("LiberationSans-Italic.ttf", fontSize, Video::Subtitles::kFontStyleItalic);
}

void PhoenixVREngine::playMovie(const Common::String &movie) {
	debug("playMovie %s", movie.c_str());
	Common::ScopedPtr<Common::SeekableReadStream> stream(open(movie));
	if (!stream) {
		warning("can't open movie %s", movie.c_str());
		return;
	}

	Common::ScopedPtr<Video::VideoDecoder> dec;
	if (movie.hasSuffixIgnoreCase(".4xm")) {
		uint32 magic = stream->readUint32BE();
		stream->seek(0);
		if (magic == MKTAG('R', 'I', 'F', 'F')) {
			dec.reset(new Video::FourXMDecoder);
		} else if (stream->readUint32LE() == kRaw4XMFile) {
			stream->seek(0);
			if (!playRaw4XMMovie(*stream, movie))
				warning("loading movie stream %s failed", movie.c_str());
			return;
		} else {
			warning("unknown 4xm movie stream %s", movie.c_str());
			return;
		}
	} else if (movie.hasSuffixIgnoreCase(".smk")) {
		dec.reset(new Video::SmackerDecoder);
	} else {
		warning("can't play %s", movie.c_str());
		return;
	}

	if (!dec->loadStream(stream.release())) {
		warning("loading movie stream %s failed", movie.c_str());
		return;
	}

	_mixer->pauseAll(true);
	_system->lockMouse(false);
	dec->start();

	Common::SharedPtr<Video::Subtitles> subtitles = loadSubtitles(movie);
	if (subtitles) {
		g_system->showOverlay(false);
		g_system->clearOverlay();
	}

	bool playing = true;
	Common::ScopedPtr<Graphics::Palette> palette;
	while (!shouldQuit() && playing && !dec->endOfVideo()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				if (event.kbd.ascii == ' ') {
					playing = false;
				}
				break;
			}

			default:
				break;
			}
		}
		if (dec->needsUpdate()) {
			auto *s = dec->decodeNextFrame();
			if (dec->hasDirtyPalette()) {
				palette.reset(new Graphics::Palette(dec->getPalette(), 256));
			}
			if (s) {
				if (!s->format.isCLUT8() || palette)
					_screen->simpleBlitFrom(*s, Graphics::FLIP_NONE, false, 0xff, palette.get());
			}
		}

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_frameLimiter.delayBeforeSwap();
		if (subtitles && !dec->isPaused())
			subtitles->drawSubtitle(dec->getTime(), false);
		_screen->update();
		_frameLimiter.startFrame();
	}
	if (subtitles)
		g_system->hideOverlay();
	_system->lockMouse(_vr.isVR());
	_mixer->pauseAll(false);
}

bool PhoenixVREngine::playRaw4XMMovie(Common::SeekableReadStream &stream, const Common::String &movie) {
	stream.seek(0);
	if (stream.readUint32LE() != kRaw4XMFile || stream.readUint32LE() != 0x01000000)
		return false;

	const int width = stream.readUint32LE();
	const int height = stream.readUint32LE();
	stream.skip(8);
	stream.skip(4);
	const uint32 frameRateBits = stream.readUint32LE();
	const float frameRate = frameRateBits == 0x41700000 ? 15.0f : 15.0f;
	Raw4XMAudioState audioState;
	const uint16 audioCodec = stream.readUint16LE();
	audioState.channels = stream.readUint16LE();
	audioState.sampleRate = stream.readUint32LE();
	stream.skip(6);
	audioState.bits = stream.readUint16LE();
	stream.seek(0x88);

	if (width <= 0 || height <= 0)
		return false;

	debug("play raw 4xm %s, %dx%d %.2f fps", movie.c_str(), width, height, frameRate);
	_mixer->pauseAll(true);
	_system->lockMouse(false);
	Audio::SoundHandle audioHandle;
	Common::ScopedPtr<Audio::QueuingAudioStream> audioStream;
	Audio::QueuingAudioStream *queuedAudioStream = nullptr;
	Audio::QueuingAudioStream *playingAudioStream = nullptr;
	if ((audioCodec == 0 || audioCodec == 1) && audioState.sampleRate > 0 &&
			(audioState.channels == 1 || audioState.channels == 2) && audioState.bits == 16) {
		audioStream.reset(Audio::makeQueuingAudioStream(audioState.sampleRate, audioState.channels == 2));
		queuedAudioStream = audioStream.get();
	}

	Common::Array<uint16> frameBuffer1;
	Common::Array<uint16> frameBuffer2;
	frameBuffer1.resize(width * height);
	frameBuffer2.resize(width * height);
	Common::Array<uint16> *frame = &frameBuffer1;
	Common::Array<uint16> *previousFrame = &frameBuffer2;
	Graphics::ManagedSurface screenFrame(width, height, _pixelFormat);
	Common::Array<int> fullMotionOffsets;
	Common::Array<int> expMotionOffsets;
	Common::Array<Raw4XMCacheEntry> cache;
	cache.resize(256);
	buildRaw4XMMotionTables(width, fullMotionOffsets, expMotionOffsets);
	uint32 frameDelay = 1000 / MAX<int>(1, static_cast<int>(frameRate));
	uint32 nextFrameTime = g_system->getMillis();
	uint32 currentFrame = 0;
	bool playing = true;
	bool endOfMovie = false;

	struct Raw4XMDecodedFrame {
		Common::Array<byte> pixels;
		bool changed = false;
	};
	Common::Array<Raw4XMDecodedFrame> frameQueue;
	const uint prebufferFrames = audioStream ? 6 : 1;
	const uint frameBytes = width * height * _pixelFormat.bytesPerPixel;
	const uint framePitch = width * _pixelFormat.bytesPerPixel;

	auto pollMovieEvents = [&]() {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN && event.kbd.ascii == ' ')
				playing = false;
		}
	};

	auto decodeNextFrame = [&]() -> bool {
		uint32 chunkStart = stream.pos();
		uint32 chunkId = stream.readUint32LE();
		uint32 chunkSize = stream.readUint32LE();
		if (chunkId != kRaw4XMFrameContainer || chunkSize < 8 || chunkStart + chunkSize > (uint32)stream.size())
			return false;

		uint32 chunkEnd = chunkStart + chunkSize;
		Common::Array<byte> payload;
		payload.resize(chunkSize - 8);
		stream.read(payload.data(), payload.size());

		bool changed = decodeRaw4XMContainerPayload(payload.data(), payload.size(), currentFrame,
			*frame, *previousFrame, width, height, fullMotionOffsets, expMotionOffsets, cache,
			queuedAudioStream, audioState);

		Raw4XMDecodedFrame queuedFrame;
		queuedFrame.changed = changed;
		if (changed) {
			raw4XMCopyFrameToScreen(screenFrame, *frame, _pixelFormat, width, height);
			queuedFrame.pixels.resize(frameBytes);
			const byte *src = (const byte *)screenFrame.getPixels();
			byte *dst = queuedFrame.pixels.data();
			for (int y = 0; y < height; ++y)
				Common::copy(src + y * screenFrame.pitch, src + y * screenFrame.pitch + framePitch, dst + y * framePitch);
			SWAP(frame, previousFrame);
		}
		frameQueue.push_back(queuedFrame);

		stream.seek(chunkEnd);
		++currentFrame;
		return true;
	};

	while (!shouldQuit() && playing && (!endOfMovie || !frameQueue.empty())) {
		pollMovieEvents();
		while (playing && !endOfMovie && frameQueue.size() < prebufferFrames && stream.pos() + 8 <= stream.size()) {
			if (!decodeNextFrame()) {
				endOfMovie = true;
				break;
			}
			pollMovieEvents();
		}
		if (!playing || frameQueue.empty())
			break;

		if (audioStream) {
			playingAudioStream = audioStream.get();
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &audioHandle, audioStream.release(), -1,
				Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::YES);
			nextFrameTime = g_system->getMillis();
		}

		Raw4XMDecodedFrame queuedFrame = frameQueue.remove_at(0);
		if (queuedFrame.changed) {
			Graphics::Surface frameSurface;
			frameSurface.init(width, height, framePitch, queuedFrame.pixels.data(), _pixelFormat);
			_screen->simpleBlitFrom(frameSurface);
			_screen->update();
		}

		nextFrameTime += frameDelay;
		const uint32 now = g_system->getMillis();
		if (nextFrameTime > now)
			g_system->delayMillis(nextFrameTime - now);
		_frameLimiter.startFrame();
	}

	if (playingAudioStream)
		playingAudioStream->finish();
	_system->lockMouse(_vr.isVR());
	_mixer->pauseAll(false);
	return true;
}

void PhoenixVREngine::playAnimation(const Common::String &name, const Common::String &var, int varValue, float speed) {
	_vr.playAnimation(name, var, varValue, speed);
}

void PhoenixVREngine::stopAnimation(const Common::String &name) {
	_vr.stopAnimation(name);
}

void PhoenixVREngine::resetLockKey() {
	debug("resetlockkey");
	_prevWarp = -1; // original game does only this o_O
}

void PhoenixVREngine::showImageOverlay(const Common::String &image, int x, int y) {
	debug("AfficheImage %s %d %d", image.c_str(), x, y);
	_imageOverlay.reset(loadSurface(image));
	_imageOverlayPos = Common::Point(x, y);
}

void PhoenixVREngine::stopImageOverlay() {
	debug("StopAffiche");
	_imageOverlay.reset();
	updateStage();
}

void PhoenixVREngine::updateStage() {
	renderVR(0);
	_screen->update();
}

void PhoenixVREngine::startCible(const Common::String &name, int periodSeconds, const Common::Array<int> &bounds) {
	debug("StartCible %s %d", name.c_str(), periodSeconds);
	_cibleActive = true;
	_cibleStartMillis = g_system->getMillis();
	_ciblePeriodSeconds = periodSeconds;
	_cibleBounds = bounds;
}

void PhoenixVREngine::stopCible() {
	debug("StopCible");
	_cibleActive = false;
}

void PhoenixVREngine::testCible(const Common::String &insideVar, const Common::String &outsideVar) {
	debug("TestCible %s %s", insideVar.c_str(), outsideVar.c_str());
	if (!_cibleActive)
		return;

	bool inside = false;
	int periodMillis = _ciblePeriodSeconds * 1000;
	if (periodMillis > 0) {
		int elapsed = (g_system->getMillis() - _cibleStartMillis) % periodMillis;
		for (uint i = 0; i + 1 < _cibleBounds.size() && _cibleBounds[i] != 0; i += 2) {
			if (_cibleBounds[i] * 1000 < elapsed && elapsed < _cibleBounds[i + 1] * 1000) {
				inside = true;
				break;
			}
		}
	}

	setVariable(insideVar, inside ? 1 : 0);
	setVariable(outsideVar, inside ? 0 : 1);
}

void PhoenixVREngine::lockKey(int idx, const Common::String &warp) {
	_lockKey[idx] = warp;
}

Graphics::Surface *PhoenixVREngine::loadSurface(const Common::String &path) {
	Common::String filename = path;
	Common::ScopedPtr<Common::SeekableReadStream> stream(open(path, &filename));
	if (!stream) {
		warning("can't find image %s", path.c_str());
		return nullptr;
	}
	Common::ScopedPtr<Image::ImageDecoder> dec;
	if (filename.hasSuffixIgnoreCase(".pcx")) {
		dec.reset(new Image::PCXDecoder);
	} else if (filename.hasSuffixIgnoreCase(".gif")) {
		dec.reset(new Image::GIFDecoder);
	} else {
		warning("can't find decoder for %s", filename.c_str());
		return nullptr;
	}
	if (!dec->loadStream(*stream)) {
		warning("decoding %s failed", filename.c_str());
		return nullptr;
	}
	auto *palette = dec->hasPalette() ? dec->getPalette().data() : nullptr;
	auto *s = dec->getSurface()->convertTo(Graphics::BlendBlit::getSupportedPixelFormat(), palette);
	if (s) {
		byte r = 0, g = 0, b = 0;
		s->applyColorKey(r, g, b);
	}
	return s;
}

Graphics::Surface *PhoenixVREngine::loadCursor(const Common::String &path) {
	if (path.empty())
		return nullptr;
	auto it = _cursorCache.find(path);
	if (it != _cursorCache.end())
		return it->_value;
	auto s = loadSurface(path);
	if (!s)
		error("can't load cursor from %s", path.c_str());
	_cursorCache[path] = s;
	return s;
}

void PhoenixVREngine::scheduleTest(int idx) {
	debug("schedule test %d for execution", idx);
	_nextTest = idx;
}

void PhoenixVREngine::executeTest(int idx) {
	debug("execute test %d", idx);
	auto test = _warp->getTest(idx);
	if (test) {
		Script::ExecutionContext ctx;
		test->scope.exec(ctx);
	} else
		warning("invalid test id %d", idx);
}

void PhoenixVREngine::startTimer(float seconds) {
	_timer = seconds;
	_initialTimer = seconds;
	_timerFlags = 5;
}

void PhoenixVREngine::pauseTimer(bool pause, bool deactivate) {
	if (_timerFlags) {
		if (pause)
			_timerFlags |= 2;
		else
			_timerFlags &= ~2;
		if (deactivate)
			_timerFlags &= ~4;
		else
			_timerFlags |= 4;
	}
}

void PhoenixVREngine::killTimer() {
	_timerFlags = 0;
}

void PhoenixVREngine::tickTimer(float dt) {
	if (_timerFlags) {
		if ((_timerFlags & 2) == 0) {
			if (_timer > dt) {
				_timer -= dt;
			} else {
				_timer = 0;
			}
			debug("timer tick %g", _timer);
		}
		if (_timerFlags & 4) {
			if (_timer <= 0) {
				debug("timer trigger");
				killTimer();
				scheduleTest(99);
			}
		}
	}
}

void PhoenixVREngine::renderTimer() {
	if (_timerFlags == 0 || !_arn)
		return;
	auto timerBg = _arn->get("cadre.bmp");
	auto timerFg = _arn->get("cadreB.bmp");
	if (!timerBg || !timerFg)
		return;

	// Loch-Ness rectangle for now.
	// Necronomicon has timer in scripts, but does not contain bitmaps for timers.
	Common::Rect bgRect{320, 16, 632, 44};
	Common::Rect fgRect{333, 23, 619, 38};
	assert(_initialTimer > 0);
	auto timeLeft = _timer / _initialTimer;
	fgRect.right = fgRect.left + fgRect.width() * timeLeft;
	Common::Rect fgSrcRect{static_cast<short>(timerFg->w * timeLeft), timerFg->h};
	if (!fgRect.isValidRect() || !fgSrcRect.isValidRect())
		return;
	_screen->blitFrom(*timerBg, bgRect.origin());
	_screen->blitFrom(*timerFg, fgSrcRect, fgRect.origin());
}

void PhoenixVREngine::renderVR(float dt) {
	_vr.render(_screen, _angleX.angle(), _angleY.angle(), _fov, dt, _showRegions ? _regSet.get() : nullptr);
	if (_text) {
		int16 x = _textRect.left + (_textRect.width() - _text->w) / 2;
		int16 y = _textRect.top + (_textRect.height() - _text->h) / 2;
		_screen->blitFrom(*_text, {x, y});
	}
	renderImageOverlay();
	renderTimer();
}

void PhoenixVREngine::renderImageOverlay() {
	if (_imageOverlay)
		paint(*_imageOverlay, _imageOverlayPos);
}

void PhoenixVREngine::saveVariables() {
	debug("SaveVariable() - saving variable state");
	_variableSnapshot.resize(_variableOrder.size());
	for (uint i = 0, n = _variableOrder.size(); i != n; ++i) {
		_variableSnapshot[i] = _variables.getVal(_variableOrder[i]);
	}
}

void PhoenixVREngine::loadVariables() {
	debug("LoadVariable() - loading variable state");
	if (_variableSnapshot.empty()) {
		debug("skipping, no snapshot");
		return;
	}
	assert(_variableSnapshot.size() == _variableOrder.size());
	for (uint i = 0, n = _variableOrder.size(); i != n; ++i) {
		_variables.setVal(_variableOrder[i], _variableSnapshot[i]);
	}
	_variableSnapshot.clear();
}

const Graphics::Font *PhoenixVREngine::getFont(int size, bool bold) const {
#ifdef USE_FREETYPE2
	const int fontMaxSizes[] = {10, 12, 14, 16, 18, INT_MAX};

	for (uint i = 0; i < ARRAYSIZE(fontMaxSizes); ++i) {
		if (size < fontMaxSizes[i]) {
			const Graphics::Font *font = bold ? _boldFonts[i].get() : nullptr;
			return font ? font : _regularFonts[i].get();
		}
	}

	return _regularFonts[ARRAYSIZE(fontMaxSizes) - 1].get();
#else
	return FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
#endif
}

void PhoenixVREngine::rollover(int textId, RolloverType type) {
	Common::Rect dstRect;
	int size = 12;
	bool bold = false;
	uint16 color = 0xFFFF;

	if (gameIdMatches("lochness")) {
		size = 12;
		bold = false;
		switch (type) {
		case RolloverType::Default: // no default in loch ness
		case RolloverType::Malette:
			dstRect = Common::Rect{20, 178, 230, 198};
			color = 0xD698;
			break;
		case RolloverType::Secretaire:
			dstRect = Common::Rect{60, 448, 270, 468};
			color = 0xFFFF;
			break;
		}
	} else {
		// using necrono
		bold = true;
		switch (type) {
		case RolloverType::Default:
			dstRect = Common::Rect{57, 427, 409, 480};
			size = 14;
			color = 0;
			break;
		case RolloverType::Malette:
			dstRect = Common::Rect{251, 346, 522, 394};
			size = 18;
			color = 0xD698;
			break;
		case RolloverType::Secretaire:
			dstRect = Common::Rect{216, 367, 536, 430};
			size = 12;
			color = 0xFFFF;
			break;
		}
	}

	auto *font = getFont(size, bold);

	if (!font)
		return;

	if (!_textes.contains(textId)) {
		debug("rollover reset");
		_text.reset();
		return;
	}
	auto &text = _textes.getVal(textId);
	debug("rollover %s, %s font size: %d, bold: %d, color: %02x", dstRect.toString().c_str(), text.encode(Common::kUtf8).c_str(), size, bold, color);

	Common::Array<Common::U32String> lines;
	font->wordWrapText(text, dstRect.width(), lines, Graphics::kWordWrapDefault);

	auto fontH = font->getFontHeight();
	int textW = 0;
	Common::Array<int> widths(lines.size());
	for (uint i = 0, n = lines.size(); i != n; ++i) {
		auto w = font->getStringWidth(lines[i]);
		widths[i] = w;
		textW = MAX(textW, w);
	}

	auto numLines = static_cast<int>(lines.size());
	auto textH = fontH * numLines;
	debug("text %dx%d", textW, textH);
	_text.reset(new Graphics::ManagedSurface(textW, textH, Graphics::BlendBlit::getSupportedPixelFormat()));
	_text->clear();
	byte r, g, b;
	_rgb565.colorToRGB(color, r, g, b);
	auto textColor = _text->format.RGBToColor(r, g, b);
	for (int i = 0; i != numLines; ++i) {
		int dw = (textW - widths[i]) / 2;
		font->drawAlphaString(_text.get(), lines[i], dw, i * fontH, textW, textColor, Graphics::kTextAlignLeft);
	}
	_textRect = dstRect;
}

void PhoenixVREngine::tick(float dt) {
	tickTimer(dt);

	if (_vr.isVR() && (_mouseRel.x || _mouseRel.y)) {
		auto da = _mouseRel;
		_mouseRel = {};
		_mousePos = _screenCenter;
		static const float kSpeedX = 0.2f;
		static const float kSpeedY = 0.2f;
		_angleX.add(float(da.x) * kSpeedX * dt);
		_angleY.add(float(da.y) * kSpeedY * dt);
	} else
		_mouseRel = {};

	Common::Array<Common::String> finishedSounds;
	for (auto &kv : _sounds) {
		auto &sound = kv._value;
		if (!_mixer->isSoundHandleActive(sound.handle)) {
			_mixer->stopHandle(sound.handle);
			finishedSounds.push_back(kv._key);
			continue;
		}
		if (!sound.spatial)
			continue;

		int8 balance = 127 * sinf(sound.angle - _angleX.angle());
		_mixer->setChannelBalance(sound.handle, balance);
	}
	for (auto &sound : finishedSounds) {
		debug("sound %s stopped", sound.c_str());
		auto it = _sounds.find(sound);
		if (it != _sounds.end()) {
			if (it->_value.subtitles)
				it->_value.subtitles->clearSubtitle();
			_sounds.erase(it);
		}
	}

	if (!_nextScript.empty()) {
		loadNextScript();
		goToWarp(_script->getInitScript()->vrFile);
	}
	if (_nextWarp >= 0) {
		_text.reset();
		_warpIdx = _nextWarp;
		_warp = _script->getWarp(_nextWarp);
		debug("warp %d -> %s %s", _nextWarp, _warp->vrFile.c_str(), _warp->testFile.c_str());
		_nextWarp = -1;

		{
			Common::String origName;
			Common::ScopedPtr<Common::SeekableReadStream> stream(open(_warp->vrFile, &origName));
			bool isVr = origName.empty() || origName.hasSuffixIgnoreCase(".vr");
			if (stream && isVr) {
				_vr = VR::loadStatic(_pixelFormat, *stream);
				if (_vr.isVR()) {
					_mousePos = _screenCenter;
					_mouseRel = {};
				}
				_system->lockMouse(_vr.isVR());
			} else
				debug("can't find vr file %s", _warp->vrFile.c_str());
		}

		{
			Common::ScopedPtr<Common::SeekableReadStream> stream(!_warp->testFile.empty() ? open(_warp->testFile) : nullptr);
			if (stream)
				_regSet.reset(new RegionSet(*stream));
			else
				debug("no region %s", _warp->testFile.c_str());
		}

		Script::ExecutionContext ctx;
		debug("execute warp script %s", _warp->vrFile.c_str());
		auto test = _warp->getDefaultTest();
		if (test)
			test->scope.exec(ctx);
		else
			warning("no default script!");
		_restarted = false;
		return;
	}

	if (_nextTest >= 0) {
		auto nextTest = _nextTest;
		_nextTest = -1;
		executeTest(nextTest);
	}

	renderVR(dt);

	Graphics::Surface *cursor = nullptr;
	auto &cursors = _cursors[_warpIdx];
	bool anyMatched = false;
	for (int i = 0, n = cursors.size(); i != n; ++i) {
		auto *region = getRegion(i);
		if (!region)
			continue;

		if (_vr.isVR() ? region->contains3D(currentVRPos()) : region->contains2D(_mousePos.x, _mousePos.y)) {
			anyMatched = true;
			auto test = _warp->getTest(i);
			if (test && test->hover == 1 && _hoverIndex < 0) {
				debug("executing hover test %d", i);
				_hoverIndex = i;
				executeTest(i);
			}

			auto &name = cursors[i];
			if (!cursor) {
				cursor = loadCursor(name);
			}
		} else if (i == _hoverIndex) {
			debug("leaving hover region");
			auto leave = _warp->getTest(i + 1);
			if (leave && leave->hover == 2) {
				executeTest(i + 1);
			}
			_hoverIndex = -1;
		}
	}
	if (!cursor)
		cursor = loadCursor(anyMatched ? _defaultCursor[1] : _defaultCursor[0]);
	if (cursor) {
		paint(*cursor, _mousePos - Common::Point(cursor->w / 2, cursor->h / 2));
	}
}

void PhoenixVREngine::drawAudioSubtitles() {
	if (!ConfMan.getBool("subtitles"))
		return;

	for (auto &kv : _sounds) {
		auto &sound = kv._value;
		if (sound.subtitles && _mixer->isSoundHandleActive(sound.handle))
			sound.subtitles->drawSubtitle(_mixer->getElapsedTime(sound.handle).msecs(), false);
	}
}

Common::Error PhoenixVREngine::run() {
	initGraphics(640, 480, nullptr);

	_pixelFormat = g_system->getScreenFormat();
	if (_pixelFormat.isCLUT8())
		return Common::kUnsupportedColorMode;

	_arn.reset(ARN::create());
#ifdef USE_FREETYPE2
	static const Common::String regular("NotoSans-Regular.ttf");
	static const Common::String bold("NotoSans-Bold.ttf");
	const int fontSizes[] = {8, 10, 12, 14, 16, 18};
	for (uint i = 0; i < ARRAYSIZE(fontSizes); ++i) {
		_regularFonts[i].reset(Graphics::loadTTFFontFromArchive(regular, fontSizes[i]));
		_boldFonts[i].reset(Graphics::loadTTFFontFromArchive(bold, fontSizes[i]));
	}
#endif

	setCursorDefault(0, "Cursor1.pcx");
	setCursorDefault(1, "Cursor2.pcx");

	_screen = new Graphics::Screen();
	_screenCenter = _screen->getBounds().center();
	{
		Common::File vars;
		if (vars.open(Common::Path("variable.txt"))) {
			while (!vars.eos()) {
				auto var = vars.readLine();
				if (var == "*")
					break;
				declareVariable(var);
				_variableOrder.push_back(Common::move(var));
			}
		} else
			debug("no variables.txt");
	}
	{
		Common::File textes;
		if (textes.open(Common::Path("textes.txt"))) {
			Common::CodePage textCodePage = getTextCodePage(_gameDescription->language);
			while (!textes.eos()) {
				auto text = textes.readLine();
				if (text.empty() || text[0] != '*')
					continue;
				uint pos = 1;
				while (pos < text.size() && Common::isSpace(text[pos]))
					++pos;
				int textId = atoi(text.c_str() + pos);
				while (pos < text.size() && Common::isDigit(text[pos]))
					++pos;
				while (pos < text.size() && Common::isSpace(text[pos]))
					++pos;
				_textes.setVal(textId, Common::convertToU32String(text.c_str() + pos, textCodePage));
			}
			debug("loaded %u textes", _textes.size());
		}
	}

	// try load level-specific script first (amerzone)
	if (gameIdMatches("amerzone")) {
		setNextScript("intro.lst");
	} else if (gameIdMatches("lochness"))
		setNextScript("first.lst");
	else
		setNextScript("script.lst");

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1) {
		auto r = loadGameState(saveSlot);
		if (r.getCode() != Common::ErrorCode::kNoError)
			return r;
	}

	Common::Event event;

	uint frameDuration = 0;
	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN: {
				if (event.kbd.keycode == Common::KeyCode::KEYCODE_h)
					_showRegions = !_showRegions;
				if (_prevWarp != -1)
					break;
				int code = -1;
				switch (event.kbd.keycode) {
				case Common::KeyCode::KEYCODE_ESCAPE:
					code = 0;
					break;
				case Common::KeyCode::KEYCODE_F1:
					code = 1;
					break;
				case Common::KeyCode::KEYCODE_F2:
					code = 2;
					break;
				case Common::KeyCode::KEYCODE_F3:
					code = 3;
					break;
				case Common::KeyCode::KEYCODE_F4:
					code = 4;
					break;
				case Common::KeyCode::KEYCODE_F5:
					code = 5;
					break;
				case Common::KeyCode::KEYCODE_F6:
					code = 6;
					break;
				case Common::KeyCode::KEYCODE_F7:
					code = 7;
					break;
				case Common::KeyCode::KEYCODE_F8:
					code = 8;
					break;
				case Common::KeyCode::KEYCODE_F9:
					code = 9;
					break;
				case Common::KeyCode::KEYCODE_F10:
					code = 10;
					break;
				case Common::KeyCode::KEYCODE_RETURN:
					code = 11;
					break;
				case Common::KeyCode::KEYCODE_TAB:
					code = 12;
					break;
				default:
					break;
				}

				if (code >= 0) {
					debug("matched code %d", static_cast<int>(event.kbd.keycode));
					if (!_lockKey[code].empty())
						goToWarp(_lockKey[code], true);
				}
			} break;
			case Common::EVENT_RBUTTONUP: {
				if (_prevWarp != -1)
					break;
				debug("right click");
				auto &rclick = _lockKey[12];
				if (!rclick.empty())
					goToWarp(rclick, true);
			} break;
			case Common::EVENT_MOUSEMOVE:
				if (!_hasFocus)
					break;
				_mousePos = event.mouse;
				_mouseRel += event.relMouse;
				break;
			case Common::EVENT_LBUTTONUP: {
				if (!_hasFocus)
					break;
				auto vrPos = currentVRPos();
				if (_vr.isVR()) {
					debug("click ax: %g, ay: %g", vrPos.x, vrPos.y);
				} else
					debug("click %s", _mousePos.toString().c_str());

				if (_warpIdx < 0)
					break;
				auto &cursors = _cursors[_warpIdx];
				for (uint i = 0, n = cursors.size(); i != n; ++i) {
					auto *region = getRegion(i);
					if (!region)
						continue;

					auto test = _warp->getTest(i);
					if (test && test->hover != 0)
						continue;

					if (_vr.isVR() ? region->contains3D(vrPos) : region->contains2D(event.mouse.x, event.mouse.y)) {
						debug("click region %u", i);
						executeTest(i);
						break;
					}
				}
			} break;
			case Common::EVENT_FOCUS_GAINED:
				_hasFocus = true;
				break;
			case Common::EVENT_FOCUS_LOST:
				_hasFocus = false;
				break;
			default:
				break;
			}
		}
		float dt = float(frameDuration) / 1000.0f;
		if (dt > kMaxTick)
			dt = kMaxTick;
		tick(dt);

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		_frameLimiter.delayBeforeSwap();
		drawAudioSubtitles();
		_screen->update();
		frameDuration = _frameLimiter.startFrame();
	}

	return Common::kNoError;
}

void PhoenixVREngine::paint(Graphics::Surface &src, Common::Point dst) {
	Common::Rect srcRect = src.getRect();
	Common::Rect clip = _screen->getBounds();
	if (Common::Rect::getBlitRect(dst, srcRect, clip)) {
		Common::Rect dstRect(dst.x, dst.y, dst.x + srcRect.width(), dst.y + srcRect.height());
		_screen->blitFrom(src, srcRect, dstRect);
	}
}

bool PhoenixVREngine::testSaveSlot(int idx) const {
	return _saveFileMan->exists(getSaveStateName(idx));
}

void PhoenixVREngine::captureContext() {
	Common::MemoryWriteStreamDynamic ms(DisposeAfterUse::YES);

	auto writeString = [&ms](const Common::String &str) {
		assert(str.size() <= 256);
		ms.writeString(str);
		uint tail = 257 - str.size();
		while (tail--)
			ms.writeByte(0);
	};

	ms.writeSint32LE(fromAngle(_angleY.angle() + kPi2));
	ms.writeSint32LE(fromAngle(_angleX.angle()));
	ms.writeSint32LE(0);
	ms.writeSint32LE(0);
	ms.writeSint32LE(fromAngle(_angleY.rangeMax() + kPi2));
	ms.writeSint32LE(fromAngle(_angleX.rangeMin()));
	ms.writeSint32LE(fromAngle(_angleX.rangeMax()));
	ms.writeSint32LE(_warpIdx);
	ms.writeUint32LE(_warp->tests.size());
	writeString({});
	writeString({});
	for (auto &warpCursors : _cursors)
		for (auto &cursor : warpCursors)
			writeString(cursor);

	for (auto &name : _script->getVarNames()) {
		auto value = g_engine->getVariable(name);
		ms.writeUint32LE(value);
	}

	ms.writeUint32LE(0); // current subroutine
	ms.writeSint32LE(_prevWarp);

	for (uint i = 0; i != 12; ++i) {
		writeString(_lockKey[i]);
	}
	writeString(_currentMusic);
	ms.writeUint32LE(_currentMusicVolume);

	struct SoundState {
		Common::String name;
		uint8 volume;
		int angle;
	};
	Common::Array<SoundState> sounds, sounds3d;
	for (auto &kv : _sounds) {
		auto &name = kv._key;
		auto &sound = kv._value;
		if (sound.loops >= 0 || name == _currentMusic)
			continue;
		if (sound.spatial)
			sounds3d.push_back({name, sound.volume, fromAngle(sound.angle)});
		else
			sounds.push_back({name, sound.volume, 0});
	}

	// sound samples
	SoundState def{{}, 255, 0};
	for (uint i = 0; i != 8; ++i) {
		auto *soundState = i < sounds.size() ? &sounds[i] : &def;
		writeString(soundState->name);
		ms.writeUint32LE(soundState->volume);
		ms.writeUint32LE(0); // flags?
	}

	// sound samples 3D
	for (uint i = 0; i != 8; ++i) {
		auto *soundState = i < sounds3d.size() ? &sounds3d[i] : &def;
		writeString(soundState->name);
		ms.writeUint32LE(soundState->angle);
		ms.writeUint32LE(soundState->volume);
		ms.writeUint32LE(0); // flags?
	}

	auto *state = ms.getData();
	_capturedState.assign(state, state + ms.size());
	debug("captured %u bytes of state", _capturedState.size());
}

bool PhoenixVREngine::enterScript() {
	if (_loadedState.empty())
		return false;

	Common::MemoryReadStream ms(_loadedState.data(), _loadedState.size());

	auto angleX = ms.readSint32LE();
	auto angleY = ms.readSint32LE();
	auto soundVolumnPanY = ms.readSint32LE();
	auto soundVolumePanX = ms.readSint32LE();
	auto angleXMax = ms.readSint32LE();
	auto angleYMin = ms.readSint32LE();
	auto angleYMax = ms.readSint32LE();
	auto currentWarpIdx = ms.readSint32LE();
	auto currentWarpTests = ms.readUint32LE();
	auto printText = ms.readString(0, 257);
	auto text = ms.readString(0, 257);
	debug("angle: %d %d, sound pan: %d %d, angle X max %d, angle Y range %d %d, warp: %u, tests: %u",
		  angleX, angleY, soundVolumePanX, soundVolumnPanY,
		  angleXMax, angleYMin, angleYMax,
		  currentWarpIdx, currentWarpTests);

	setAngle(toAngle(angleX), toAngle(angleY));
	if (angleXMax != -1)
		setXMax(toAngle(angleXMax));
	if (angleYMin != -1 && angleYMax != -1)
		setYMax(toAngle(angleYMin), toAngle(angleYMax));

	_nextWarp = currentWarpIdx;

	for (auto &warpCursors : _cursors) {
		for (auto &warpCursor : warpCursors) {
			auto cursor = ms.readString(0, 257);
			debug("cursor %s", cursor.c_str());
			if (cursor.hasSuffix(".VR") || cursor.hasSuffix(".vr")) {
				debug("ignoring VR cursor, original engine saves `LOAD.VR` as a cursor name at loading screen");
				cursor.clear();
			}
			warpCursor = cursor;
		}
	}
	debug("vars at %08x", (uint32)ms.pos());
	for (auto &name : _script->getVarNames()) {
		auto value = ms.readSint32LE();
		debug("var %s: %d", name.c_str(), value);
		g_engine->setVariable(name, value);
	}
	debug("vars end at %08x", (uint32)ms.pos());
	auto currentSubroutine = ms.readSint32LE();
	_prevWarp = ms.readSint32LE();
	debug("currentSubroutine %d, prev warp %d", currentSubroutine, _prevWarp);
	for (uint i = 0; i != 12; ++i) {
		auto lockKey = ms.readString(0, 257);
		debug("lockKey %d %s", i, lockKey.c_str());
		_lockKey[i] = lockKey;
	}

	stopAllSounds();

	_currentMusic = ms.readString(0, 257);
	_currentMusicVolume = ms.readUint32LE();
	debug("current music %s, volume: %u", _currentMusic.c_str(), _currentMusicVolume);
	if (!_currentMusic.empty() && _currentMusicVolume > 0)
		playSound(_currentMusic, Audio::Mixer::kMusicSoundType, _currentMusicVolume, -1);

	// sound samples
	for (uint i = 0; i != 8; ++i) {
		auto name = ms.readString(0, 257);
		auto vol = ms.readUint32LE();
		auto flags = ms.readUint32LE();
		debug("sound: %s vol: %u flags: %u", name.c_str(), vol, flags);
		if (!name.empty() && name != _currentMusic)
			playSound(name, Audio::Mixer::kSFXSoundType, vol, -1);
	}

	// sound samples 3D
	for (uint i = 0; i != 8; ++i) {
		auto name = ms.readString(0, 257);
		auto angle = ms.readUint32LE();
		auto vol = ms.readUint32LE();
		auto flags = ms.readUint32LE();
		debug("3d sound: %s vol: %u flags: %u angle: %u", name.c_str(), vol, flags, angle);
		if (!name.empty())
			playSound(name, Audio::Mixer::kSFXSoundType, vol, -1, true, static_cast<float>(angle) * kPi);
	}
	_loadedState.clear();
	return true;
}

Common::Error PhoenixVREngine::loadGameStream(Common::SeekableReadStream *slot) {
	auto state = GameState::load(*slot);

	_loaded = true;
	killTimer();
	setNextScript(state.script);
	if (!_levels.empty()) {
		uint i = 0, n = _levels.size();
		for (; i != n; ++i) {
			auto &level = _levels[i];
			if (state.script.hasPrefixIgnoreCase(level)) {
				debug("current level is %u", i);
				_currentLevel = i;
				break;
			}
		}
		if (i == n)
			warning("couldn't find current level index for script %s", state.script.c_str());
	}
	// keep it alive until loading finishes.
	auto currentScript = Common::move(_script);
	assert(!_nextScript.empty());
	loadNextScript();

	_loadedState = state.state;
	{
		auto test = _script->getWarp(0)->getDefaultTest();
		Script::ExecutionContext ctx;
		test->scope.exec(ctx);
	}
	_loaded = false;

	return Common::kNoError;
}

Common::Error PhoenixVREngine::saveGameStream(Common::WriteStream *slot, bool isAutosave) {
	GameState state;
	state.script = _contextScript;
	state.game = _contextLabel;
	const bool isAmerzone = gameIdMatches("amerzone");
	Common::String amerzoneLevelLabel;
	if (isAmerzone) {
		amerzoneLevelLabel = getAmerzoneLevelLabel(state.script);
		state.game.clear();
	}

	TimeDate td = {};
	g_system->getTimeAndDate(td);
	state.info = formatSaveInfo(td, isAmerzone, amerzoneLevelLabel);

	state.thumbWidth = _thumbnail.w;
	state.thumbHeight = _thumbnail.h;
	auto *thumbnailPixels = static_cast<byte *>(_thumbnail.getPixels());
	auto thumbnailSize = _thumbnail.pitch * _thumbnail.h;

	Common::MemoryWriteStreamDynamic dib(DisposeAfterUse::YES);
	dib.writeUint32LE(0x28);
	dib.writeUint32LE(_thumbnail.w);
	dib.writeUint32LE(_thumbnail.h);
	dib.writeUint16LE(1); // planes
	dib.writeUint16LE(16);
	dib.writeUint32LE(3); // compression
	dib.writeUint32LE(thumbnailSize);
	dib.writeUint32LE(0);
	dib.writeUint32LE(0);
	dib.writeUint32LE(3);
	dib.writeUint32LE(0);

	// RGB masks
	dib.writeUint32LE(0xf800);
	dib.writeUint32LE(0x07e0);
	dib.writeUint32LE(0x001f);

	assert(dib.size() == 0x28 + 3 * 4);
	state.dibHeader.assign(dib.getData(), dib.getData() + dib.size());

	state.thumbnail.assign(thumbnailPixels, thumbnailPixels + thumbnailSize);
	state.state = Common::move(_capturedState);
	_capturedState.clear();

	state.save(*slot);
	return Common::kNoError;
}

void PhoenixVREngine::drawSaveCard(int idx) {
	if (!gameIdMatches("amerzone")) {
		static const int faces[] = {4, 3, 5, 1};
		const int face = faces[(idx - 1) / 2];
		const bool odd = (idx - 1) & 1;
		drawSlot(idx, face, odd ? 275 : 97, 200);
		return;
	}

	Common::ScopedPtr<Common::InSaveFile> slot(_saveFileMan->openForLoading(getSaveStateName(idx)));
	if (!slot)
		return;

	auto state = GameState::load(*slot);
	auto &dst = _vr.getSurface();
	Graphics::Surface *thumbnail = state.getThumbnail(dst.format, 232);
	const int cardW = thumbnail->w + 6;
	const int cardH = thumbnail->w + 30;

	Graphics::ManagedSurface card(cardW, cardH, dst.format);
	const uint32 white = dst.format.RGBToColor(0xff, 0xff, 0xff);
	const uint32 black = dst.format.RGBToColor(0, 0, 0);
	card.fillRect(Common::Rect(0, 0, cardW, cardH), white);
	card.fillRect(Common::Rect(0, 0, cardW, 1), black);
	card.fillRect(Common::Rect(0, cardH - 1, cardW, cardH), black);
	card.fillRect(Common::Rect(0, 0, 1, cardH), black);
	card.fillRect(Common::Rect(cardW - 1, 0, cardW, cardH), black);
	card.copyRectToSurface(*thumbnail, 3, 6, thumbnail->getRect());

	const Graphics::Font *font = getFont(16, true);
	if (font) {
		int textY = thumbnail->h + 18;
		textY = drawSaveTextBlock(*card.surfacePtr(), font, state.game, 0, textY, cardW, black, Graphics::kTextAlignCenter, 18, false, 0);
		drawSaveTextBlock(*card.surfacePtr(), font, state.info, 0, textY, cardW, black, Graphics::kTextAlignCenter, 18, false, 0);
	}

	static const int faces[] = {4, 3, 5, 1};
	const int face = faces[(idx - 1) / 2];
	const float angle = ((idx - 1) & 1) ? -kPi / 8.0f : kPi / 8.0f;
	Graphics::ManagedSurface faceSurface(512, 512, dst.format);
	copyCubeFaceToSurface(faceSurface, dst, face);
	projectSaveCard(faceSurface, card, angle);
	copySurfaceToCubeFace(dst, faceSurface, face);

	thumbnail->free();
	delete thumbnail;
}

void PhoenixVREngine::drawSlot(int idx, int face, int x, int y) {
	Common::ScopedPtr<Common::InSaveFile> slot(_saveFileMan->openForLoading(getSaveStateName(idx)));
	if (!slot)
		return;
	auto state = GameState::load(*slot);
	const bool isAmerzone = gameIdMatches("amerzone");

	y += face * 4 * 256;
	bool splitV = true;
	if (x > 256) {
		x -= 256;
		y += 256;
		splitV = false;
	}

	auto &dst = _vr.getSurface();
	auto *src = state.getThumbnail(dst.format, isAmerzone ? 232 : 0);
	int tileY = y / 256;
	if (isAmerzone) {
		const int cardX = x - 3;
		const int cardY = y - 6;
		const int cardW = src->w + 6;
		const int cardH = src->w + 30;
		uint32 white = dst.format.RGBToColor(0xff, 0xff, 0xff);
		uint32 black = dst.format.RGBToColor(0, 0, 0);
		fillSaveSlotRect(dst, Common::Rect(cardX, cardY, cardX + cardW, cardY + cardH), white, splitV, tileY);
		fillSaveSlotRect(dst, Common::Rect(cardX, cardY, cardX + cardW, cardY + 1), black, splitV, tileY);
		fillSaveSlotRect(dst, Common::Rect(cardX, cardY + cardH - 1, cardX + cardW, cardY + cardH), black, splitV, tileY);
		fillSaveSlotRect(dst, Common::Rect(cardX, cardY, cardX + 1, cardY + cardH), black, splitV, tileY);
		fillSaveSlotRect(dst, Common::Rect(cardX + cardW - 1, cardY, cardX + cardW, cardY + cardH), black, splitV, tileY);
		x = cardX + 3;
		y = cardY + 6;
		tileY = y / 256;
	}
	auto srcRect = src->getRect();
	short srcSplitY = MIN(y + src->h, (tileY + 1) * 256) - y;
	if (splitV)
		srcRect.bottom = srcSplitY;
	dst.copyRectToSurface(*src, x, y, srcRect);
	if (splitV) {
		srcRect.top = srcSplitY;
		srcRect.bottom = src->h;
		dst.copyRectToSurface(*src, x, (tileY + 3) * 256, srcRect);
	}
	auto *font = getFont(isAmerzone ? 10 : 12, isAmerzone);
	if (font) {
		auto color = dst.format.RGBToColor(0, 0, 0);
		int textX = x;
		int textW = src->w;
		Graphics::TextAlign textAlign = Graphics::kTextAlignLeft;
		int textY = y + 0x72;
		int lineHeight = 14;
		if (isAmerzone) {
			textX = x - 3;
			textW = src->w + 6;
			textY = y - 6 + src->h + 14;
			textAlign = Graphics::kTextAlignCenter;

			textY = drawSaveTextBlock(dst, font, state.game, textX, textY, textW, color, textAlign, lineHeight, splitV, tileY);
			drawSaveTextBlock(dst, font, state.info, textX, textY, textW, color, textAlign, lineHeight, splitV, tileY);
		} else {
			textY = drawSaveTextBlock(dst, font, state.game, textX, textY, textW, color, textAlign, lineHeight, splitV, tileY, true);
			drawSaveTextBlock(dst, font, state.info, textX, textY, textW, color, textAlign, lineHeight, splitV, tileY);
		}
	}

	src->free();
	delete src;
}

void PhoenixVREngine::setGlobalVolume(int volume) {
	ConfMan.setInt("music_volume", volume);
	ConfMan.setInt("sfx_volume", volume);
	syncSoundSettings();
}

void PhoenixVREngine::syncSoundSettings() {
	int musicVolume = ConfMan.getInt("music_volume");
	int sfxVolume = ConfMan.getInt("sfx_volume");
	debug("syncSoundSettings, music: %d, sfx: %d", musicVolume, sfxVolume);
	bool muted = false;
	if (ConfMan.hasKey("mute")) {
		muted = ConfMan.getBool("mute");
	}
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, muted ? 0 : musicVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, muted ? 0 : sfxVolume);
}

} // End of namespace PhoenixVR
