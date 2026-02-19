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

#include "bolt/bolt.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

void XpLib::blit(byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height) {
	int16 srcSkip = srcStride - width;
	int16 dstSkip = dstStride - width;

	for (int16 row = 0; row < height; row++) {
		memcpy(dst, src, width);
		dst += dstSkip + width;
		src += srcSkip + width;
	}
}

void XpLib::dirtyBlit(byte *src, byte *dst, uint16 width, uint16 height, byte *dirtyFlags) {
	for (int16 row = 0; row < height; row++) {
		byte flag = dirtyFlags[row];

		// Clean row? Skip...
		if (flag == 3) {
			src += width;
			dst += width;
			continue;
		}

		// Mark row: 2 -> 3 (was dirty, now clean), else -> 0 (newly copied)
		dirtyFlags[row] = (flag == 2) ? 3 : 0;

		memcpy(dst, src, width);
	}
}

void XpLib::maskBlit(byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height) {
	int16 srcSkip = srcStride - width;
	int16 dstSkip = dstStride - width;

	for (int16 row = 0; row < height; row++) {
		for (int16 col = 0; col < width; col++) {
			byte pixel = *src++;
			if (pixel != 0)
				*dst = pixel;

			dst++;
		}

		dst += dstSkip;
		src += srcSkip;
	}
}

void XpLib::compositeBlit(byte *src, byte *background, byte *dst, uint16 stride, uint16 width, uint16 height) {
	int16 skip = stride - width;

	for (int16 row = 0; row < height; row++) {
		for (int16 col = 0; col < width; col++) {
			byte pixel = *src++;
			if (pixel == 0)
				pixel = *background;

			*dst++ = pixel;
			background++;
		}

		src += skip;
		background += skip;
		dst += skip;
	}
}

void XpLib::rleBlit(byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height) {
	int16 dstSkip = dstStride - width;

	for (int16 row = 0; row < height; row++) {
		byte *rowEnd = dst + width;

		while (true) {
			byte b = *src++;
			if ((b & 0x80) == 0) {
				// Literal pixel
				*dst++ = b;
			} else {
				// Run
				byte val = b & 0x7F;
				int16 count = *src++;
				if (count == 0) {
					count = (int16)(rowEnd - dst);
					rowEnd = 0;
				}

				memset(dst, val, count);
				dst += count;

				if (rowEnd == 0) {
					dst += dstSkip;
					break;
				}
			}
		}
	}
}

void XpLib::rleMaskBlit(byte *src, uint16 srcStride, byte *dst, uint16 dstStride, uint16 width, uint16 height) {
	int16 dstSkip = dstStride - width;

	for (int16 row = 0; row < height; row++) {
		byte *rowEnd = dst + width;

		while (true) {
			byte b = *src++;
			if ((b & 0x80) == 0) {
				// Literal pixel
				if (b != 0)
					*dst = b;

				dst++;
			} else {
				// Run
				byte val = b & 0x7F;
				int16 count = *src++;
				if (count == 0) {
					count = (int16)(rowEnd - dst);
					rowEnd = 0;
				}

				if (val == 0) {
					// Transparent run skip...
					dst += count;
				} else {
					memset(dst, val, count);
					dst += count;
				}

				if (rowEnd == 0) {
					dst += dstSkip;
					break;
				}
			}
		}
	}
}

void XpLib::rleCompositeBlit(byte *rle, byte *background, byte *dst, uint16 width, uint16 height, byte *dirtyFlags) {
	byte *rlePtr = rle;
	byte *bgPtr = background;
	byte *dstPtr = dst;

	for (int16 row = 0; row < height; row++) {
		byte *rowEnd = dstPtr + width;

		// Check if row is all transparent...
		if (rlePtr[0] == 0x80 && rlePtr[1] == 0x00) {
			if (dirtyFlags[row] & 2) {
				// Row was already transparent: mark clean and skip...
				dirtyFlags[row] = 3;
				rlePtr += 2;
				dstPtr = rowEnd;
				bgPtr += width;
				continue;
			}

			dirtyFlags[row] = 2;
		} else {
			dirtyFlags[row] = 0;
		}

		// Decode RLE tokens for this row...
		while (true) {
			byte b = *rlePtr++;
			if ((b & 0x80) == 0) {
				// Literal pixel
				if (b != 0) {
					*dstPtr++ = b; // opaque
					bgPtr++;
				} else {
					*dstPtr++ = *bgPtr++; // transparent (copy from background)
				}
			} else if (b != 0x80) {
				// Opaque run
				byte val = b & 0x7F;
				int16 count = *rlePtr++;
				if (count == 0) {
					count = rowEnd - dstPtr;
					rowEnd = nullptr;
				}

				bgPtr += count;
				memset(dstPtr, val, count);
				dstPtr += count;

				if (rowEnd == nullptr)
					break;
			} else {
				// Transparent run (copy from background)
				int16 count = *rlePtr++;
				if (count == 0) {
					count = rowEnd - dstPtr;
					rowEnd = nullptr;
				}

				memcpy(dstPtr, bgPtr, count);
				dstPtr += count;
				bgPtr += count;

				if (rowEnd == nullptr)
					break;
			}
		}
	}
}

uint16 XpLib::rleDataSize(byte *rleData, uint16 height) {
	byte *start = rleData;
	byte *ptr = start;

	for (int16 y = 0; y < height; y++) {
		while (true) {
			byte b = *ptr++;
			if (b & 0x80) {
				byte count = *ptr++;
				if (count == 0)
					break;
			}
		}
	}

	return (uint16)(ptr - start);
}

void XpLib::markCursorPixels(byte *buffer, uint32 count) {
	byte *ptr = buffer;
	for (uint32 i = 0; i < count; i++) {
		ptr[i] |= 0x80;
	}
}

} // End of namespace Bolt
