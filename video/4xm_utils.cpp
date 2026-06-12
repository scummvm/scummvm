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

#include "video/4xm_utils.h"
#include "common/bitstream.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include <math.h>

namespace Video {
namespace FourXM {

#define FIX_1_082392200 70936
#define FIX_1_414213562 92682
#define FIX_1_847759065 121095
#define FIX_2_613125930 171254

#define MULTIPLY(var, const) ((int)((var) * (unsigned)(const)) >> 16)

namespace {

struct RawTransform {
	bool swapAxes;
	bool flipX;
	bool flipY;
};

struct RawCoefficientToken {
	byte zeroes;
	int8 values[2];
	byte valueCount;
};

static const RawTransform kRawTransforms[8] = {
	{false, false, false},
	{false, true, false},
	{false, false, true},
	{false, true, true},
	{true, true, true},
	{true, false, true},
	{true, true, false},
	{true, false, false}};

static const int kRawCoefficientOrder[64] = {
	0, 1, 8, 9, 2, 3, 10, 11,
	16, 17, 24, 25, 18, 19, 26, 27,
	4, 5, 12, 20, 13, 6, 7, 14,
	21, 28, 29, 22, 15, 23, 30, 31,
	32, 33, 40, 48, 41, 34, 35, 42,
	49, 56, 57, 50, 43, 51, 58, 59,
	36, 37, 44, 52, 45, 38, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63};

static const RawCoefficientToken kRawCoefficientTokens[8] = {
	{64, {0, 0}, 0},
	{5, {0, 0}, 0},
	{1, {1, 0}, 1},
	{1, {-1, 0}, 1},
	{2, {1, 0}, 1},
	{1, {0, -1}, 2},
	{2, {0, 1}, 2},
	{3, {-1, 0}, 1}};

static const Graphics::PixelFormat kRawPixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);

int rawSign2(byte value) {
	return (value & 2) ? (value | ~3) : (value + 1);
}

int rawSign4(byte value) {
	return (value & 8) ? (value | ~15) : (value + 1);
}

void rawTransformPixel(int sx, int sy, int width, int height, uint32 mode, int &dx, int &dy) {
	const RawTransform &transform = kRawTransforms[mode & 7];
	dx = transform.swapAxes ? sy : sx;
	dy = transform.swapAxes ? sx : sy;
	if (transform.flipX)
		dx = width - 1 - dx;
	if (transform.flipY)
		dy = height - 1 - dy;
}

void readRawCoefficientToken(RawDeltaReader &reader, int coeff[64], int &index) {
	auto zero = [&](int count) {
		for (int i = 0; i < count && index < 64; ++i, ++index)
			coeff[kRawCoefficientOrder[index]] = 0;
	};
	auto write = [&](int value) {
		if (index < 64)
			coeff[kRawCoefficientOrder[index++]] = value;
	};
	auto writeSigned2 = [&](byte value) {
		write(rawSign2(value));
	};
	auto writeSigned4 = [&]() {
		write(rawSign4(reader.readNibble()));
	};

	byte code = reader.readNibble();
	if (code <= 7) {
		const RawCoefficientToken &token = kRawCoefficientTokens[code];

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

} // namespace

RawDeltaReader::RawDeltaReader(const byte *ptr, uint32 len) : data(ptr), size(len) {
	if (size < 0x18)
		return;

	mode = READ_LE_UINT32(data);
	pairOffset = READ_LE_UINT32(data + 8) + 0x18;
	byteOffset = READ_LE_UINT32(data + 12) + 0x18;
	nibbleOffset = READ_LE_UINT32(data + 16) + 0x18;
	controlWord = READ_LE_UINT32(data + 0x18);
	if (nibbleOffset + 4 <= size)
		nibbleWord = READ_LE_UINT32(data + nibbleOffset);
}

bool RawDeltaReader::valid() const {
	return size >= 0x18;
}

uint32 RawDeltaReader::readControl2() {
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

uint16 RawDeltaReader::readPair() {
	uint32 off = pairOffset + pairIndex * 2;
	++pairIndex;
	return off + 2 <= size ? READ_LE_UINT16(data + off) : 0;
}

byte RawDeltaReader::readByteIndex() {
	uint32 off = byteOffset + byteIndex;
	++byteIndex;
	return off < size ? data[off] : 0;
}

int8 RawDeltaReader::readSignedByte() {
	return (int8)readByteIndex();
}

uint32 RawDeltaReader::readNibble() {
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

void idct(int16_t block[64], int shift) {
	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	int tmp10, tmp11, tmp12, tmp13;
	int z5, z10, z11, z12, z13;
	int i;
	int temp[64];

	for (i = 0; i < 8; i++) {
		tmp10 = block[8 * 0 + i] + block[8 * 4 + i];
		tmp11 = block[8 * 0 + i] - block[8 * 4 + i];

		tmp13 = block[8 * 2 + i] + block[8 * 6 + i];
		tmp12 = MULTIPLY(block[8 * 2 + i] - block[8 * 6 + i], FIX_1_414213562) - tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		z13 = block[8 * 5 + i] + block[8 * 3 + i];
		z10 = block[8 * 5 + i] - block[8 * 3 + i];
		z11 = block[8 * 1 + i] + block[8 * 7 + i];
		z12 = block[8 * 1 + i] - block[8 * 7 + i];

		tmp7 = z11 + z13;
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562);

		z5 = MULTIPLY(z10 + z12, FIX_1_847759065);
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5;
		tmp12 = MULTIPLY(z10, -FIX_2_613125930) + z5;

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		temp[8 * 0 + i] = tmp0 + tmp7;
		temp[8 * 7 + i] = tmp0 - tmp7;
		temp[8 * 1 + i] = tmp1 + tmp6;
		temp[8 * 6 + i] = tmp1 - tmp6;
		temp[8 * 2 + i] = tmp2 + tmp5;
		temp[8 * 5 + i] = tmp2 - tmp5;
		temp[8 * 4 + i] = tmp3 + tmp4;
		temp[8 * 3 + i] = tmp3 - tmp4;
	}

	for (i = 0; i < 8 * 8; i += 8) {
		tmp10 = temp[0 + i] + temp[4 + i];
		tmp11 = temp[0 + i] - temp[4 + i];

		tmp13 = temp[2 + i] + temp[6 + i];
		tmp12 = MULTIPLY(temp[2 + i] - temp[6 + i], FIX_1_414213562) - tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		z13 = temp[5 + i] + temp[3 + i];
		z10 = temp[5 + i] - temp[3 + i];
		z11 = temp[1 + i] + temp[7 + i];
		z12 = temp[1 + i] - temp[7 + i];

		tmp7 = z11 + z13;
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562);

		z5 = MULTIPLY(z10 + z12, FIX_1_847759065);
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5;
		tmp12 = MULTIPLY(z10, -FIX_2_613125930) + z5;

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		block[0 + i] = (tmp0 + tmp7) >> shift;
		block[7 + i] = (tmp0 - tmp7) >> shift;
		block[1 + i] = (tmp1 + tmp6) >> shift;
		block[6 + i] = (tmp1 - tmp6) >> shift;
		block[2 + i] = (tmp2 + tmp5) >> shift;
		block[5 + i] = (tmp2 - tmp5) >> shift;
		block[4 + i] = (tmp3 + tmp4) >> shift;
		block[3 + i] = (tmp3 - tmp4) >> shift;
	}
}

void buildRawMotionTables(int width, Common::Array<int> &fullOffsets, Common::Array<int> &expOffsets) {
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

void copyRawBlock(uint16 *dst, const uint16 *src, int stride, int width, int height, uint32 mode, int add) {
	const int sourceWidth = (mode >= 4) ? height : width;
	const int sourceHeight = (mode >= 4) ? width : height;

	for (int sy = 0; sy < sourceHeight; ++sy) {
		for (int sx = 0; sx < sourceWidth; ++sx) {
			int dx, dy;
			rawTransformPixel(sx, sy, width, height, mode, dx, dy);
			if (dx >= 0 && dx < width && dy >= 0 && dy < height)
				dst[dy * stride + dx] = src[sy * stride + sx] + add;
		}
	}
}

void readRawCoefficients(RawDeltaReader &reader, int coeffs[3][64]) {
	for (int channel = 0; channel < 3; ++channel) {
		int *coeff = coeffs[channel];
		int index = 1;
		coeff[0] = reader.readSignedByte();

		while (index <= 63)
			readRawCoefficientToken(reader, coeff, index);
	}
}

void transformRawCoefficients(int coeff[64], int dst[64], int scaleCode, bool chroma) {
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

void writeRawDctBlock(const int yBlock[64], const int cbBlock[64], const int crBlock[64],
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
			dstPixel[x] = kRawPixelFormat.RGBToColor(red << 3, green << 3, blue << 3);
		}
	}
}

} // namespace FourXM
} // namespace Video
