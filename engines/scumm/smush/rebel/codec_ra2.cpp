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

#include "scumm/smush/rebel/codec_ra2.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

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

void copyRA2Handler7PerspectiveViewport(byte *dst, int dstPitch, int dstWidth, int dstHeight,
		const byte *src, int srcPitch, int srcWidth, int srcHeight,
		int perspectiveX, int perspectiveY, int viewShift) {
	if (!dst || !src || dstPitch <= 0 || srcPitch <= 0 || dstWidth <= 0 || dstHeight <= 0 || srcWidth <= 0 || srcHeight <= 0)
		return;

	const int viewportWidth = MIN(320, MIN(dstWidth, dstPitch));
	const int viewportHeight = MIN(170, dstHeight);
	if (viewportWidth <= 0 || viewportHeight <= 0)
		return;

	int tilt = (viewShift * 5) / 128;
	int xSkew = (tilt * 9) / 4;
	int ySkew = tilt * 5;
	int srcBaseX = 0x34 + perspectiveX + xSkew;
	int srcBaseY = 0x2d + perspectiveY - ySkew;

	const int tempPitch = viewportWidth;
	byte *temp = new byte[viewportWidth * viewportHeight];
	memset(temp, 0, viewportWidth * viewportHeight);

	const int absTilt = ABS(tilt);
	static const int chunkPatterns[6][7] = {
		{16, 16, 0, 0, 0, 0, 0},
		{8, 16, 8, 0, 0, 0, 0},
		{4, 12, 12, 4, 0, 0, 0},
		{4, 8, 8, 8, 4, 0, 0},
		{4, 8, 4, 8, 4, 4, 0},
		{4, 4, 4, 8, 4, 4, 4}
	};

	if (absTilt > 6)
		tilt = 0;

	if (tilt == 0) {
		for (int dy = 0; dy < viewportHeight; ++dy) {
			const int srcY = srcBaseY + dy;
			byte *dstRow = temp + dy * tempPitch;
			if (srcY < 0 || srcY >= srcHeight)
				continue;

			int copyX = srcBaseX;
			int dstX = 0;
			int copyWidth = viewportWidth;
			if (copyX < 0) {
				dstX = -copyX;
				copyWidth -= dstX;
				copyX = 0;
			}
			if (copyX + copyWidth > srcWidth)
				copyWidth = srcWidth - copyX;
			if (copyWidth > 0)
				memcpy(dstRow + dstX, src + srcY * srcPitch + copyX, copyWidth);
		}
	} else {
		const int yStep = (tilt > 0) ? 1 : -1;
		const int xStep = (tilt > 0) ? -1 : 1;
		const int rowShiftStep = ABS(xSkew) * 2;
		int rowX = srcBaseX;
		int rowY = srcBaseY;
		int rowError = 0;
		const int *pattern = chunkPatterns[absTilt - 1];
		const int patternCount = absTilt + 1;

		for (int dy = 0; dy < viewportHeight; ++dy) {
			byte *dstRow = temp + dy * tempPitch;
			int srcX = rowX;
			int srcY = rowY;
			int dstX = 0;
			while (dstX < viewportWidth) {
				for (int i = 0; i < patternCount && dstX < viewportWidth; ++i) {
					const int chunkWidth = MIN(pattern[i], viewportWidth - dstX);
					if (srcY >= 0 && srcY < srcHeight) {
						int copyX = srcX;
						int copyDstX = dstX;
						int copyWidth = chunkWidth;
						if (copyX < 0) {
							copyDstX -= copyX;
							copyWidth += copyX;
							copyX = 0;
						}
						if (copyX + copyWidth > srcWidth)
							copyWidth = srcWidth - copyX;
						if (copyWidth > 0)
							memcpy(dstRow + copyDstX, src + srcY * srcPitch + copyX, copyWidth);
					}

					srcX += chunkWidth;
					dstX += chunkWidth;
					if (i != patternCount - 1)
						srcY += yStep;
				}
			}

			rowY++;
			rowError += rowShiftStep;
			if (rowError > 0) {
				rowError -= viewportHeight;
				rowX += xStep;
			}
		}
	}

	for (int y = 0; y < viewportHeight; ++y)
		memcpy(dst + y * dstPitch, temp + y * tempPitch, viewportWidth);

	delete[] temp;
}

// RLE decoder for opaque backgrounds, including color 0.
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

// Codec 21/44 line update.
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
			int skip = READ_LE_UINT16(src);
			src += 2;
			if (skip >= len)
				break;
			lineDst += skip;
			len -= skip;

			if (lineEnd - src < 2)
				break;
			int count = READ_LE_UINT16(src) + 1;
			src += 2;
			if (count > len)
				count = len;

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

// Codec 23 skip/copy with embedded RLE.
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
			x += skip;
			if (src + 2 > lineEnd || x >= width)
				break;

			int runSize = READ_LE_UINT16(src);
			src += 2;
			const byte *runEnd = src + runSize;
			if (runEnd > lineEnd)
				runEnd = lineEnd;

			while (src < runEnd && x < width) {
				byte code = *src++;
				int num = (code >> 1) + 1;
				if (num > width - x)
					num = width - x;

				if (code & 1) {
					byte color = (src < runEnd) ? *src++ : 0;
					memset(lineDst + x, color, num);
					x += num;
				} else {
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

// Codec 23 skip/run remap that tints the background in place.
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

	// Byte 4 gates whether this codec body is valid.
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

static void smushDecodeRA2BlurImpl(byte *dst, const byte *src, int left, int top,
		int clipLeft, int clipTop, int clipRight, int clipBottom,
		int dstWidth, int dstHeight, int pitch, int dataSize, byte *palette, byte *lookup) {
	const byte *maskData = nullptr;
	int maskSize = 0;
	if (dst == nullptr || dstWidth <= 2 || dstHeight <= 2 || pitch <= 0 ||
			!smushPrepareRA2BlurData(src, dataSize, palette, lookup, maskData, maskSize))
		return;

	clipLeft = CLIP<int>(clipLeft + 1, 1, dstWidth - 1);
	clipTop = CLIP<int>(clipTop + 1, 1, dstHeight - 1);
	clipRight = CLIP<int>(clipRight - 1, 1, dstWidth - 1);
	clipBottom = CLIP<int>(clipBottom - 1, 1, dstHeight - 1);
	if (clipLeft >= clipRight || clipTop >= clipBottom)
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
			if (x >= clipLeft && y >= clipTop && x < clipRight) {
				if (y >= clipBottom)
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

void smushDecodeRA2Blur(byte *dst, const byte *src, int left, int top, int dstWidth, int dstHeight, int pitch, int dataSize, byte *palette, byte *lookup) {
	smushDecodeRA2BlurImpl(dst, src, left, top, 0, 0, dstWidth, dstHeight,
		dstWidth, dstHeight, pitch, dataSize, palette, lookup);
}

void smushDecodeRA2BlurClip(byte *dst, const byte *src, int left, int top,
		int clipLeft, int clipTop, int clipRight, int clipBottom,
		int dstWidth, int dstHeight, int pitch, int dataSize, byte *palette, byte *lookup) {
	smushDecodeRA2BlurImpl(dst, src, left, top, clipLeft, clipTop, clipRight, clipBottom,
		dstWidth, dstHeight, pitch, dataSize, palette, lookup);
}

} // End of namespace Scumm
