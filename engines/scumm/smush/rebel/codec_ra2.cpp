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

/**
 * Codec 45: RA2-specific BOMP RLE with variable header
 * Used for embedded ANIM frames, particularly small animation elements.
 * Has a variable-length header (commonly 6 bytes starting with 01 FE).
 * Note: For overlay sprites, color 0 is treated as transparent.
 */
void smushDecodeRA2Bomp(byte *dst, const byte *src, int left, int top, int width, int height, int pitch, int dataSize) {
	dst += top * pitch + left;

	// Detect header pattern and find RLE data start
	int headerSkip = 0;
	bool foundValidOffset = false;

	// Check for common 6-byte header pattern: 01 FE XX XX XX XX
	if (dataSize > 6 && src[0] == 0x01 && src[1] == 0xFE) {
		headerSkip = 6;
		foundValidOffset = true;
	} else {
		// Probe offsets to find valid RLE start
		// Valid start should have reasonable line size values
		for (int testOffset = 0; testOffset <= 6 && testOffset + 2 <= dataSize; testOffset += 2) {
			int testLineSize = READ_LE_INT16(src + testOffset);
			// A valid line size should be positive and reasonable for the width
			if (testLineSize > 0 && testLineSize <= width * 2 && testLineSize < dataSize - testOffset) {
				// Further validation: try to count valid line sizes
				int linesTest = 0;
				const byte *testPtr = src + testOffset;
				bool validSum = true;

				while (linesTest < height && testPtr + 2 <= src + dataSize) {
					int ls = READ_LE_INT16(testPtr);
					if (ls <= 0 || ls > width * 2) {
						validSum = false;
						break;
					}
					testPtr += ls + 2;
					linesTest++;
				}

				if (validSum && linesTest >= height - 1) {
					headerSkip = testOffset;
					foundValidOffset = true;
					break;
				}
			}
		}
	}

	if (!foundValidOffset) {
		warning("smushDecodeRA2Bomp: Codec 45 couldn't find valid RLE offset, using offset 0");
	}

	src += headerSkip;
	const byte *dataEnd = src + (dataSize - headerSkip);

	// Check first value to determine per-line vs continuous mode
	int firstVal = (src + 2 <= dataEnd) ? READ_LE_INT16(src) : 0;
	bool perLineMode = (firstVal > 0 && firstVal <= width * 2);

	if (perLineMode) {
		// Per-line RLE with 2-byte size headers
		for (int row = 0; row < height && src < dataEnd; row++) {
			int lineSize = READ_LE_INT16(src);
			src += 2;
			if (lineSize <= 0 || lineSize > (int)(dataEnd - src))
				break;

			const byte *lineEnd = src + lineSize;
			byte *rowDst = dst + row * pitch;
			int x = 0;

			while (src < lineEnd && x < width) {
				byte ctrl = *src++;
				int count = (ctrl >> 1) + 1;

				if (ctrl & 1) {
					// RLE fill - color 0 is transparent for overlay sprites
					byte color = (src < lineEnd) ? *src++ : 0;
					if (color != 0) {
						int num = (count > width - x) ? width - x : count;
						memset(rowDst + x, color, num);
					}
					x += count;
					if (x > width)
						x = width;
				} else {
					// Literal copy - color 0 is transparent for overlay sprites
					for (int i = 0; i < count && x < width && src < lineEnd; i++) {
						byte color = *src++;
						if (color != 0)
							rowDst[x] = color;
						x++;
					}
				}
			}
			src = lineEnd;
		}
	} else {
		// Continuous BOMP RLE (no per-line headers)
		for (int row = 0; row < height && src < dataEnd; row++) {
			byte *rowDst = dst + row * pitch;
			int x = 0;

			while (x < width && src < dataEnd) {
				byte ctrl = *src++;
				int count = (ctrl >> 1) + 1;

				if (ctrl & 1) {
					// RLE fill - color 0 is transparent for overlay sprites
					byte color = (src < dataEnd) ? *src++ : 0;
					if (color != 0) {
						int num = (count > width - x) ? width - x : count;
						memset(rowDst + x, color, num);
					}
					x += count;
					if (x > width)
						x = width;
				} else {
					// Literal copy - color 0 is transparent for overlay sprites
					for (int i = 0; i < count && x < width && src < dataEnd; i++) {
						byte color = *src++;
						if (color != 0)
							rowDst[x] = color;
						x++;
					}
				}
			}
		}
	}
}

} // End of namespace Scumm
