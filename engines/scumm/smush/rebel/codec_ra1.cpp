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

#include "scumm/smush/rebel/codec_ra1.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Scumm {

// Codecs 31/32 use the codec 1/3 line RLE over low-nibble-first packed 4bpp
// pixels. Codec 31 leaves color zero transparent.
void smushDecodeRA1SegaCDRLE(byte *dst, const byte *src, int left, int top, int width, int height,
		int pitch, int bufHeight, int dataSize, bool transparent, byte paletteBase) {
	if (dst == nullptr || src == nullptr || width <= 0 || height <= 0 || pitch <= 0 || dataSize <= 0)
		return;

	const byte *srcEnd = src + dataSize;
	for (int row = 0; row < height && srcEnd - src >= 2; row++) {
		const int lineSize = READ_LE_UINT16(src);
		const byte *lineData = src + 2;
		const byte *lineEnd = lineData + MIN<int>(lineSize, srcEnd - lineData);
		const int dstY = top + row;
		byte *rowDst = (dstY >= 0 && dstY < bufHeight) ? dst + dstY * pitch : nullptr;
		int x = 0;

		while (x < width && lineData < lineEnd) {
			const byte code = *lineData++;
			const int num = (code >> 1) + 1;
			const bool isRun = (code & 1) != 0;
			byte runByte = 0;
			if (isRun) {
				if (lineData >= lineEnd)
					break;
				runByte = *lineData++;
			}

			for (int k = 0; k < num && x < width; k++) {
				byte packed;
				if (isRun) {
					packed = runByte;
				} else {
					if (lineData >= lineEnd)
						break;
					packed = *lineData++;
				}
				const byte nibbles[2] = { (byte)(packed & 0x0F), (byte)(packed >> 4) };
				for (int d = 0; d < 2 && x < width; d++, x++) {
					const byte nibble = nibbles[d];
					if (rowDst && !(transparent && nibble == 0)) {
						const int dstX = left + x;
						if (dstX >= 0 && dstX < pitch)
							rowDst[dstX] = paletteBase + nibble;
					}
				}
			}
		}

		const int rowSize = 2 + lineSize;
		if (rowSize > srcEnd - src)
			break;
		src += rowSize;
	}
}

// RLE with transparency on pixel 0.
void smushDecodeRA1Transparent(byte *dst, const byte *src, int left, int top, int width, int height,
		int pitch, int dataSize, int sourceSkipX) {
	if (dst == nullptr || src == nullptr || width <= 0 || height <= 0 || pitch <= 0 || dataSize <= 0)
		return;

	const byte *srcEnd = src + dataSize;
	dst += top * pitch;
	while (height-- > 0 && srcEnd - src >= 2) {
		const int lineSize = READ_LE_UINT16(src);
		const byte *lineData = src + 2;
		const byte *lineEnd = lineData + MIN<int>(lineSize, srcEnd - lineData);
		byte *rowDst = dst + left;
		int skip = sourceSkipX;
		int remaining = width;

		while ((skip > 0 || remaining > 0) && lineData < lineEnd) {
			byte code = *lineData++;
			int num = (code >> 1) + 1;

			if (code & 1) {
				if (lineData >= lineEnd)
					break;
				byte color = *lineData++;

				const int skipped = MIN(num, skip);
				skip -= skipped;
				num -= skipped;
				if (num == 0)
					continue;

				const int count = MIN(num, remaining);
				if (color != 0)
					memset(rowDst, color, count);
				rowDst += count;
				remaining -= count;
			} else {
				const int readable = MIN<int>(num, lineEnd - lineData);
				const int skipped = MIN(num, skip);
				const int skippedBytes = MIN(readable, skipped);
				lineData += skippedBytes;
				skip -= skipped;
				if (skipped == num) {
					if (readable < num)
						break;
					continue;
				}

				const int count = MIN<int>(readable - skippedBytes, remaining);
				for (int j = 0; j < count; j++) {
					byte c = lineData[j];
					if (c != 0)
						rowDst[j] = c;
				}
				lineData += count;
				if (readable < num)
					break;
				rowDst += count;
				remaining -= count;
			}
		}

		const int rowSize = 2 + lineSize;
		if (rowSize > srcEnd - src)
			break;
		src += rowSize;
		dst += pitch;
	}
}

// Codec 21 clips copy runs without changing source X.
void smushDecodeRA1SkipCopy(byte *dst, const byte *src, int left, int top, int width, int height,
		int pitch, int bufWidth, int bufHeight, int dataSize) {
	if (dst == nullptr || src == nullptr || width <= 0 || height <= 0 || pitch <= 0 || dataSize <= 0)
		return;

	const byte *srcEnd = src + dataSize;
	for (int row = 0; row < height && srcEnd - src >= 2; row++) {
		const uint16 lineSize = READ_LE_UINT16(src);
		const byte *lineData = src + 2;
		const byte *lineEnd = lineData + MIN<int>(lineSize, srcEnd - lineData);
		const int dstY = top + row;
		int srcX = 0;

		while (srcX < width && lineData < lineEnd) {
			if (lineData + 2 > lineEnd)
				break;
			const uint16 skip = READ_LE_UINT16(lineData);
			lineData += 2;
			srcX += skip;
			if (srcX >= width)
				break;

			if (lineData + 2 > lineEnd)
				break;
			const int copyLen = READ_LE_UINT16(lineData) + 1;
			lineData += 2;

			const int readableLen = MIN<int>(copyLen, (int)(lineEnd - lineData));
			const int dstStartX = left + srcX;
			const int dstEndX = dstStartX + readableLen;
			if (readableLen > 0 && dstY >= 0 && dstY < bufHeight) {
				const int clippedStartX = MAX(dstStartX, 0);
				const int clippedEndX = MIN(dstEndX, bufWidth);
				if (clippedStartX < clippedEndX) {
					const int srcSkipX = clippedStartX - dstStartX;
					memcpy(dst + dstY * pitch + clippedStartX,
						lineData + srcSkipX, clippedEndX - clippedStartX);
				}
			}

			lineData += readableLen;
			srcX += copyLen;
			if (readableLen < copyLen)
				break;
		}

		const int rowSize = 2 + lineSize;
		if (rowSize > srcEnd - src)
			break;
		src += rowSize;
	}
}

// Codec 23 applies additive palette deltas to existing pixels.
void smushDecodeRA1AdditiveLineUpdate(byte *dst, const byte *src, int left, int top, int width, int height,
		int pitch, int bufWidth, int bufHeight, uint8 paletteBase, int dataSize) {
	if (dst == nullptr || src == nullptr || width <= 0 || height <= 0 || pitch <= 0 || dataSize <= 0)
		return;

	const uint8 colorDelta = (uint8)(paletteBase - 0x30);
	const byte *srcEnd = src + dataSize;
	for (int row = 0; row < height && srcEnd - src >= 2; row++) {
		const uint16 lineSize = READ_LE_UINT16(src);
		const byte *lineData = src + 2;
		const byte *lineEnd = lineData + MIN<int>(lineSize, srcEnd - lineData);
		const int dstY = top + row;
		int srcX = 0;
		while (srcX < width && lineData < lineEnd) {
			const int skip = *lineData++;
			srcX += skip;
			if (srcX >= width || lineData >= lineEnd)
				break;
			const int runLength = (int)(*lineData++);
			const int dstStartX = left + srcX;
			const int dstEndX = dstStartX + runLength;
			if (dstY >= 0 && dstY < bufHeight) {
				const int clippedStartX = MAX(dstStartX, 0);
				const int clippedEndX = MIN(dstEndX, bufWidth);
				if (clippedStartX < clippedEndX) {
					byte *dstPixel = dst + dstY * pitch + clippedStartX;
					for (int x = clippedStartX; x < clippedEndX; x++, dstPixel++)
						*dstPixel = (byte)(*dstPixel + colorDelta);
				}
			}
			srcX += runLength;
		}

		const int rowSize = 2 + lineSize;
		if (rowSize > srcEnd - src)
			break;
		src += rowSize;
	}
}

// Codec 2 scatter/point draw.
void smushDecodeRA1Scatter(byte *dst, const byte *src, int left, int top, int bufWidth, int bufHeight, int pitch, int dataSize) {
	if (dst == nullptr || src == nullptr || pitch <= 0 || dataSize <= 0)
		return;

	int curX = left;
	int curY = top;
	while (dataSize >= 4) {
		int16 dx = (int16)READ_LE_UINT16(src);
		uint8 dy = src[2];
		uint8 pixel = src[3];
		src += 4;
		dataSize -= 4;
		curX += dx;
		curY += dy;
		if (curX >= 0 && curY >= 0 && curX < bufWidth && curY < bufHeight)
			dst[curY * pitch + curX] = pixel;
	}
}

// Codecs 4/5/33/34 use 4x4 glyph tables.
static uint8 s_ra1C4Tbl[2][256][16];
static uint16 s_ra1C4Param = 0xFFFF;

static void ra1Codec4GenTiles(uint16 param1) {
	uint8 *dst = &s_ra1C4Tbl[0][0][0];
	for (int i = 1; i < 16; i += 2) {
		for (int k = 0; k < 16; k++) {
			int j = i + param1, l = k + param1;
			int m = (j + l) / 2, n = (j + m) / 2, o = (l + m) / 2;
			if (j == m || l == m) {
				*dst++ = l; *dst++ = j; *dst++ = l; *dst++ = j;
				*dst++ = j; *dst++ = l; *dst++ = j; *dst++ = j;
				*dst++ = l; *dst++ = j; *dst++ = l; *dst++ = j;
				*dst++ = l; *dst++ = l; *dst++ = j; *dst++ = l;
			} else {
				*dst++ = m; *dst++ = m; *dst++ = n; *dst++ = j;
				*dst++ = m; *dst++ = m; *dst++ = n; *dst++ = j;
				*dst++ = o; *dst++ = o; *dst++ = m; *dst++ = n;
				*dst++ = l; *dst++ = l; *dst++ = o; *dst++ = m;
			}
		}
	}
	for (int i = 0; i < 16; i += 2) {
		for (int k = 0; k < 16; k++) {
			int j = i + param1, l = k + param1;
			int m = (j + l) / 2, n = (j + m) / 2, o = (l + m) / 2;
			if (m == j || m == l) {
				*dst++ = j; *dst++ = j; *dst++ = l; *dst++ = j;
				*dst++ = j; *dst++ = j; *dst++ = j; *dst++ = l;
				*dst++ = l; *dst++ = j; *dst++ = l; *dst++ = l;
				*dst++ = j; *dst++ = l; *dst++ = j; *dst++ = l;
			} else {
				*dst++ = j; *dst++ = j; *dst++ = n; *dst++ = m;
				*dst++ = j; *dst++ = j; *dst++ = n; *dst++ = m;
				*dst++ = n; *dst++ = n; *dst++ = m; *dst++ = o;
				*dst++ = m; *dst++ = m; *dst++ = o; *dst++ = l;
			}
		}
	}
}

// Sega codecs use four 64-glyph families over an 8x8 color-pair grid.
static void ra1CodecSegaGenTiles(uint16 param1) {
	uint8 *dst = &s_ra1C4Tbl[0][0][0];
	for (int family = 0; family < 4; family++) {
		for (int i = 0; i < 8; i++) {
			for (int k = 0; k < 8; k++) {
				const int cy = i + param1;
				const int cx = k + param1;
				const int c0 = param1 + ((i + k) >> 1);
				const int c1 = (c0 + cy) >> 1;
				const int c2 = (c0 + cx) >> 1;

				switch (family) {
				case 0:
					*dst++ = c0; *dst++ = c0; *dst++ = c1; *dst++ = cy;
					*dst++ = c0; *dst++ = c0; *dst++ = c1; *dst++ = cy;
					*dst++ = c2; *dst++ = c2; *dst++ = c0; *dst++ = c1;
					*dst++ = cx; *dst++ = cx; *dst++ = c2; *dst++ = c0;
					break;
				case 1:
					*dst++ = cy; *dst++ = cy; *dst++ = cy; *dst++ = cy;
					*dst++ = c0; *dst++ = c0; *dst++ = c0; *dst++ = c0;
					*dst++ = c2; *dst++ = c2; *dst++ = c2; *dst++ = c2;
					*dst++ = cx; *dst++ = cx; *dst++ = cx; *dst++ = cx;
					break;
				case 2:
					*dst++ = cy; *dst++ = cy; *dst++ = c1; *dst++ = c0;
					*dst++ = cy; *dst++ = cy; *dst++ = c1; *dst++ = c0;
					*dst++ = c1; *dst++ = c1; *dst++ = c0; *dst++ = c2;
					*dst++ = c0; *dst++ = c0; *dst++ = c2; *dst++ = cx;
					break;
				case 3:
					*dst++ = cy; *dst++ = c0; *dst++ = c2; *dst++ = cx;
					*dst++ = cy; *dst++ = c0; *dst++ = c2; *dst++ = cx;
					*dst++ = cy; *dst++ = c0; *dst++ = c2; *dst++ = cx;
					*dst++ = cy; *dst++ = c0; *dst++ = c2; *dst++ = cx;
					break;
				}
			}
		}
	}
}

static bool ra1Codec4LoadTiles(const byte *&src, int &remaining, uint16 param2, uint8 clr) {
	uint8 *dst = &s_ra1C4Tbl[1][0][0];
	int loop = param2 * 8;
	if (param2 > 256 || remaining < loop)
		return false;
	for (int i = 0; i < loop; i++) {
		byte c = *src++;
		remaining--;
		*dst++ = (c >> 4) + clr;
		*dst++ = (c & 0xF) + clr;
	}
	return true;
}

void smushDecodeRA1Block(byte *dst, const byte *src, int left, int top, int width, int height,
		int pitch, int bufHeight, int dataSize, uint8 param, uint16 parm2, int codec) {
	if (dst == nullptr || src == nullptr || width <= 0 || height <= 0 || pitch <= 0 || bufHeight <= 0 || dataSize <= 0)
		return;

	const int mx = pitch;
	const int my = bufHeight;
	const bool segaGlyphs = codec == 33 || codec == 34;
	const bool hasSkipGlyph = codec == 4 || codec == 33;
	const uint16 tableParam = param | (segaGlyphs ? 0x100 : 0);
	if (s_ra1C4Param != tableParam) {
		if (segaGlyphs)
			ra1CodecSegaGenTiles(param);
		else
			ra1Codec4GenTiles(param);
		s_ra1C4Param = tableParam;
	}
	int remaining = dataSize;
	const byte *data = src;
	if (parm2 > 0) {
		if (!ra1Codec4LoadTiles(data, remaining, parm2, param)) {
			warning("smushDecodeRA1Block: not enough data for tile load (parm2=%d)", parm2);
			return;
		}
	}
	for (int j = 0; j < width; j += 4) {
		byte mask = 0, bits = 0;
		int x = left + j;
		for (int i = 0; i < height; i += 4) {
			int y = top + i;
			int bit = 0;
			if (parm2 > 0) {
				if (bits == 0) {
					if (remaining < 1)
						return;
					mask = *data++;
					remaining--;
					bits = 8;
				}
				bit = !!(mask & 0x80);
				mask <<= 1;
				bits--;
			}
			if (remaining < 1)
				return;
			byte idx = *data++;
			remaining--;
			if (bit == 0 && idx == 0x80 && hasSkipGlyph)
				continue;
			if (y >= my || (y + 4) < 0 || (x + 4) < 0 || x >= mx)
				continue;
			const byte *gs = &s_ra1C4Tbl[bit][idx][0];
			if (y >= 0 && x >= 0 && (y + 4) <= my && (x + 4) <= mx) {
				for (int k = 0; k < 4; k++, gs += 4)
					memcpy(dst + x + (y + k) * pitch, gs, 4);
			} else {
				for (int k = 0; k < 4; k++)
					for (int l = 0; l < 4; l++, gs++) {
						int yo = y + k, xo = x + l;
						if (yo >= 0 && yo < my && xo >= 0 && xo < mx)
							*(dst + yo * pitch + xo) = *gs;
					}
			}

			// High-quality block modes blend tile edges after each 4x4 block.
			if (x <= 0 || y <= 0 || (x + 4) >= mx || (y + 4) >= my)
				continue;
			const uint32 dstoff = y * pitch + x;
			if (s_ra1C4Param & 0x80) {
				for (int k = 0; k < 4; k++)
					*(dst + dstoff + k) = ((*(dst + dstoff + k) + *(dst + dstoff + k - pitch)) >> 1) | 0x80;
				*(dst + dstoff + 1 * pitch) = ((*(dst + dstoff + 1 * pitch) + *(dst + dstoff + 1 * pitch - 1)) >> 1) | 0x80;
				*(dst + dstoff + 2 * pitch) = ((*(dst + dstoff + 2 * pitch) + *(dst + dstoff + 2 * pitch - 1)) >> 1) | 0x80;
				*(dst + dstoff + 3 * pitch) = ((*(dst + dstoff + 3 * pitch) + *(dst + dstoff + 3 * pitch - 1)) >> 1) | 0x80;
			} else {
				for (int k = 0; k < 4; k++)
					*(dst + dstoff + k) = ((*(dst + dstoff + k) + *(dst + dstoff + k - pitch)) >> 1) & 0x7f;
				*(dst + dstoff + 1 * pitch) = ((*(dst + dstoff + 1 * pitch) + *(dst + dstoff + 1 * pitch - 1)) >> 1) & 0x7f;
				*(dst + dstoff + 2 * pitch) = ((*(dst + dstoff + 2 * pitch) + *(dst + dstoff + 2 * pitch - 1)) >> 1) & 0x7f;
				*(dst + dstoff + 3 * pitch) = ((*(dst + dstoff + 3 * pitch) + *(dst + dstoff + 3 * pitch - 1)) >> 1) & 0x7f;
			}
		}
	}
}

} // End of namespace Scumm
