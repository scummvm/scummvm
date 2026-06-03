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

// Rebel Assault 1 SMUSH video codecs

#include "scumm/smush/rebel/codec_ra1.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Scumm {

/**
 * RA1 codec 1: RLE with transparency on pixel 0.
 * Same BOMP encoding as smushDecodeRLE but pixel value 0 is not written,
 * allowing the background (restored via FTCH) to show through.
 */
void smushDecodeRA1Transparent(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize) {
	if (dst == nullptr || src == nullptr || width <= 0 || height <= 0 || pitch <= 0 || dataSize <= 0)
		return;

	const byte *srcEnd = src + dataSize;
	dst += top * pitch;
	while (height-- > 0 && srcEnd - src >= 2) {
		const int lineSize = READ_LE_UINT16(src);
		const byte *lineData = src + 2;
		const byte *lineEnd = lineData + MIN<int>(lineSize, srcEnd - lineData);
		byte *rowDst = dst + left;
		int remaining = width;

		while (remaining > 0 && lineData < lineEnd) {
			byte code = *lineData++;
			int num = (code >> 1) + 1;
			if (num > remaining)
				num = remaining;

			if (code & 1) {
				if (lineData >= lineEnd)
					break;
				byte color = *lineData++;
				if (color != 0)
					memset(rowDst, color, num);
			} else {
				const int readable = MIN<int>(num, lineEnd - lineData);
				for (int j = 0; j < readable; j++) {
					byte c = lineData[j];
					if (c != 0)
						rowDst[j] = c;
				}
				lineData += readable;
				if (readable < num)
					break;
			}
			rowDst += num;
			remaining -= num;
		}

		const int rowSize = 2 + lineSize;
		if (rowSize > srcEnd - src)
			break;
		src += rowSize;
		dst += pitch;
	}
}

/**
 * RA1 codec 21: Skip/copy line codec (FUN_10D41). Clip copy runs without
 * changing source X; stored cockpit patches can legitimately start offscreen.
 */
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

/**
 * RA1 codec 23: Additive line-update overlay (FUN_10B40).
 */
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

/**
 * RA1 codec 2: Scatter/point draw (FUN_110D7).
 */
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

// RA1 codec 4/5: block-based dithered codec with 4x4 tile lookup tables
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
	if (s_ra1C4Param != param) {
		ra1Codec4GenTiles(param);
		s_ra1C4Param = param;
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
			if (bit == 0 && idx == 0x80 && codec != 5)
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

			/* post processing of the hiquality implementations of codec4/5,
			 * see e.g. ASSAULT.EXE 121e8 - 12242
			 */
			if (x <= 0 || y <= 0 || (x + 4) >= mx || (y + 4) >= my)
				continue;		/* skip unreachable edges */
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
