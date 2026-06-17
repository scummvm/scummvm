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

// Rebel Assault 2 SMUSH video codecs

#include "scumm/smush/rebel/codec_ra2.h"

#include "common/endian.h"
#include "common/textconsole.h"

namespace Scumm {

static void bompDecodeLineOpaqueBounded(byte *dst, const byte *src, const byte *srcEnd, int len) {
	while (len > 0 && src < srcEnd) {
		byte code = *src++;
		int num = (code >> 1) + 1;
		if (num > len)
			num = len;

		if (code & 1) {
			if (src >= srcEnd)
				break;
			memset(dst, *src++, num);
			dst += num;
			len -= num;
		} else {
			int toCopy = num;
			if (toCopy > (int)(srcEnd - src))
				toCopy = (int)(srcEnd - src);
			memcpy(dst, src, toCopy);
			src += toCopy;
			dst += toCopy;
			len -= toCopy;
			if (toCopy < num)
				break;
		}
	}
}

const byte *smushSkipRLELines(const byte *src, int &dataSize, int lines) {
	for (int i = 0; i < lines; i++) {
		if (dataSize < 2) {
			src += dataSize;
			dataSize = 0;
			break;
		}

		int rowSize = READ_LE_UINT16(src) + 2;
		if (rowSize > dataSize) {
			src += dataSize;
			dataSize = 0;
			break;
		}

		src += rowSize;
		dataSize -= rowSize;
	}

	return src;
}

/**
 * Codec 3 RLE decoder that writes ALL colors including color 0 (black).
 * Use this for background images where color 0 should NOT be treated as transparent.
 * The standard smushDecodeRLE() treats color 0 as transparent, which is correct
 * for overlay sprites but wrong for background images.
 *
 * Used by: Rebel Assault 2 Level 2 background loading (IACT opcode 8, par4=5)
 */
void smushDecodeRLEOpaque(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize) {
	if (dataSize <= 0)
		return;

	const byte *srcEnd = src + dataSize;
	dst += top * pitch;
	while (height-- && srcEnd - src >= 2) {
		int lineSize = READ_LE_UINT16(src);
		src += 2;
		if (lineSize > srcEnd - src)
			lineSize = (int)(srcEnd - src);
		const byte *lineEnd = src + lineSize;

		dst += left;
		bompDecodeLineOpaqueBounded(dst, src, lineEnd, width);
		src = lineEnd;
		dst += pitch - left;
	}
}

/**
 * Codec 21/44: Line Update codec
 * Used for fonts (NUT files) and some embedded HUD frames.
 * Format: Each line has a 2-byte size header, then 2-byte skip, 2-byte count pairs with literal pixels.
 * The count value needs +1 to get the actual number of pixels to copy.
 * Note: Skip regions preserve previous frame content (delta compression).
 */
void smushDecodeLineUpdate(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize) {
	if (dataSize <= 0)
		return;

	const byte *srcEnd = src + dataSize;
	dst += top * pitch + left;

	while (height-- && srcEnd - src >= 2) {
		byte *dstPtrNext = dst + pitch;
		int lineDataSize = READ_LE_UINT16(src);
		src += 2;
		if (lineDataSize > srcEnd - src)
			lineDataSize = (int)(srcEnd - src);
		const byte *lineEnd = src + lineDataSize;
		int len = width;
		byte *lineDst = dst;

		while (len > 0 && lineEnd - src >= 2) {
			// Read 2-byte LE skip value
			int skip = READ_LE_UINT16(src);
			src += 2;
			if (skip >= len)
				break;
			lineDst += skip;
			len -= skip;

			// Read 2-byte LE copy count (+1 for actual count)
			if (lineEnd - src < 2)
				break;
			int count = READ_LE_UINT16(src) + 1;
			src += 2;
			if (count > len)
				count = len;

			// Copy literal pixels
			int toCopy = count;
			if (toCopy > (int)(lineEnd - src))
				toCopy = (int)(lineEnd - src);
			memcpy(lineDst, src, toCopy);
			lineDst += toCopy;
			src += toCopy;
			len -= toCopy;
			if (toCopy < count)
				break;
		}
		dst = dstPtrNext;
		src = lineEnd;
	}
}

/**
 * Codec 23: Skip/Copy with embedded RLE
 * Used for video frames with skip regions.
 * Format: Each line has 2-byte size, then (skip, runSize, RLE_data) triplets.
 * Note: Skip regions preserve previous frame content (delta compression).
 */
void smushDecodeSkipRLE(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize) {
	dst += top * pitch + left;
	const byte *srcEnd = src + dataSize;

	for (int row = 0; row < height; row++) {
		if (src + 2 > srcEnd)
			break;
		int lineDataSize = READ_LE_UINT16(src);
		src += 2;
		const byte *lineEnd = src + lineDataSize;
		if (lineEnd > srcEnd)
			lineEnd = srcEnd;
		byte *lineDst = dst;
		int x = 0;

		while (src + 2 <= lineEnd && x < width) {
			int skip = READ_LE_UINT16(src);
			src += 2;
			x += skip;  // Skip preserves previous frame content
			if (src + 2 > lineEnd || x >= width)
				break;

			int runSize = READ_LE_UINT16(src);
			src += 2;
			const byte *runEnd = src + runSize;
			if (runEnd > lineEnd)
				runEnd = lineEnd;

			// Decode RLE within this run - write ALL colors including 0
			while (src < runEnd && x < width) {
				byte code = *src++;
				int num = (code >> 1) + 1;
				if (num > width - x)
					num = width - x;

				if (code & 1) {
					// RLE run - repeat color
					byte color = (src < runEnd) ? *src++ : 0;
					memset(lineDst + x, color, num);
					x += num;
				} else {
					// Literal run - copy bytes
					int toCopy = num;
					if (toCopy > (int)(runEnd - src))
						toCopy = (int)(runEnd - src);
					memcpy(lineDst + x, src, toCopy);
					src += toCopy;
					x += toCopy;
				}
			}
			src = runEnd;
		}
		src = lineEnd;
		dst += pitch;
	}
}

// Codec 23: a skip/run RLE that tints the background in place (no pixel data). Per line,
// lineDataSize(2) then [skip(1), run(1)] pairs; each run pixel becomes remap[background]
// (translucency) or background+addColor.
void smushDecodeRA2SkipRemap(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize, const byte *remap, byte addColor) {
	dst += top * pitch + left;
	const byte *srcEnd = src + dataSize;

	for (int row = 0; row < height; row++) {
		if (src + 2 > srcEnd)
			break;
		int lineDataSize = READ_LE_UINT16(src);
		src += 2;
		const byte *lineStart = src;
		const byte *lineEnd = (lineStart + lineDataSize <= srcEnd) ? lineStart + lineDataSize : srcEnd;
		byte *lineDst = dst;
		int x = 0;

		while (x < width && src < lineEnd) {
			x += *src++;                 // skip (preserve background)
			if (x >= width || src >= lineEnd)
				break;
			int run = *src++;
			if (run > width - x)
				run = width - x;
			if (remap) {
				for (int i = 0; i < run; i++, x++)
					lineDst[x] = remap[lineDst[x]];
			} else {
				for (int i = 0; i < run; i++, x++)
					lineDst[x] = (byte)(lineDst[x] + addColor);
			}
		}

		src = lineStart + lineDataSize;
		dst += pitch;
	}
}

bool smushPrepareRA2BlurData(const byte *src, int dataSize, byte *palette, byte *lookup, const byte *&maskData, int &maskSize) {
	maskData = nullptr;
	maskSize = 0;

	if (src == nullptr || dataSize < 6 || palette == nullptr || lookup == nullptr)
		return false;

	// FUN_0042B530 only processes this codec body when byte +4 is 1.
	if (src[4] != 1)
		return false;

	const int tableMode = READ_LE_INT16(src + 2);
	maskData = src + 6;
	maskSize = dataSize - 6;

	if (tableMode == 0) {
		if (dataSize < 0x306)
			return false;

		memcpy(palette, src + 6, 0x300);

		const byte *p = src + 0x306;
		int remaining = dataSize - 0x306;
		int lookupIndex = 0;
		while (lookupIndex < 0x8000 && remaining >= 2) {
			int count = p[0];
			const byte color = p[1];
			p += 2;
			remaining -= 2;

			while (count-- > 0 && lookupIndex < 0x8000)
				lookup[lookupIndex++] = color;
		}

		if (lookupIndex < 0x8000)
			return false;

		maskData = p;
		maskSize = remaining;
	}

	return maskSize > 3;
}

/**
 * Codec 45: RA2 blur/wipe mask.
 * Original path: FUN_0042B460 -> FUN_0042B530 -> FUN_0042DDF0.
 */
void smushDecodeRA2Blur(byte *dst, const byte *src, int left, int top, int dstWidth, int dstHeight, int pitch, int dataSize, byte *palette, byte *lookup) {
	const byte *maskData = nullptr;
	int maskSize = 0;
	if (dst == nullptr || dstWidth <= 2 || dstHeight <= 2 || pitch <= 0 ||
			!smushPrepareRA2BlurData(src, dataSize, palette, lookup, maskData, maskSize))
		return;

	int x = left;
	int y = top;

	while (maskSize > 3) {
		const int dx = READ_LE_INT16(maskData);
		const int dy = maskData[2];
		const int count = maskData[3];
		maskData += 4;
		maskSize -= 4;

		x += dx;
		y += dy;

		for (int i = 0; i <= count; ++i) {
			if (x > 0 && y > 0 && x < dstWidth - 1) {
				if (y >= dstHeight - 1)
					return;

				byte *pixel = dst + y * pitch + x;
				const byte leftColor = pixel[-1];
				const byte rightColor = pixel[1];
				const byte topColor = pixel[-pitch];
				const byte bottomColor = pixel[pitch];

				const int red = palette[leftColor * 3] + palette[rightColor * 3] +
					palette[topColor * 3] + palette[bottomColor * 3];
				const int green = palette[leftColor * 3 + 1] + palette[rightColor * 3 + 1] +
					palette[topColor * 3 + 1] + palette[bottomColor * 3 + 1];
				const int blue = palette[leftColor * 3 + 2] + palette[rightColor * 3 + 2] +
					palette[topColor * 3 + 2] + palette[bottomColor * 3 + 2];

				const int lookupIndex = ((red << 5) & 0x7c00) + (green & 0x3e0) + (blue >> 5);
				*pixel = lookup[lookupIndex & 0x7fff];
			}

			++x;
		}
		--x;
	}
}

} // End of namespace Scumm
