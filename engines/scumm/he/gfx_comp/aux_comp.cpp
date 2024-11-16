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

#ifdef ENABLE_HE

#include "common/system.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/wiz_he.h"

namespace Scumm {

#define WRLE_BIT_SIZE(zp)					(1 << (8 - (zp)))

#define WRLE_TINY_RUN_BITS					0x01
#define WRLE_TINY_RUN_BIT_COUNT				1
#define WRLE_TINY_RUN_COUNT_BITS			2
#define WRLE_TINY_RUN_MIN_COUNT				3
#define WRLE_TINY_RUN_BITS_OVERHEAD			((WRLE_TINY_RUN_BIT_COUNT) + (WRLE_TINY_RUN_COUNT_BITS))
#define WRLE_TINY_RUN_MAX_COUNT				((WRLE_TINY_RUN_MIN_COUNT) + ((1 << (WRLE_TINY_RUN_COUNT_BITS)) - 1))

#define WRLE_TRUN_BITS						0x00
#define WRLE_TRUN_BIT_COUNT					2
#define WRLE_SMALL_TRUN_SIZE				(WRLE_BIT_SIZE(WRLE_TRUN_BIT_COUNT) - 1)
#define WRLE_LARGE_TRUN_SIZE				65536

#define WRLE_SINGLE_COLOR_BITS				0x02
#define WRLE_SINGLE_COLOR_BIT_COUNT			3

#define WRLE_LESS_RUN_BITS					0x06
#define WRLE_LESS_RUN_BIT_COUNT				3
#define WRLE_LESS_SMALL_RUN_SIZE			(WRLE_BIT_SIZE(WRLE_LESS_RUN_BIT_COUNT) - 1)
#define WRLE_LESS_LARGE_RUN_SIZE			256

#define AUX_IGNORE_ZPLANE_BITS(_dst, _mask, _count) {           \
		for (int i = 0; i < _count; i++) {                      \
			if ((_mask >>= 1) == 0) {                           \
				_mask = 0x80;                                   \
				_dst++;                                         \
			}                                                   \
		}                                                       \
	}

#define AUX_SET_ZPLANE_BITS(_dst, _mask, _count) {              \
		for (int i = 0; i < _count; i++) {                      \
			*(_dst) |= _mask;                                   \
			if ((_mask >>= 1) == 0) {                           \
				_mask = 0x80;                                   \
				_dst++;                                         \
			}                                                   \
		}                                                       \
	}

#define AUX_CLEAR_ZPLANE_BITS(_dst, _mask, _count) {            \
		for (int i = 0; i < _count; i++) {                      \
			*(_dst) &= ~_mask;                                  \
			if ((_mask >>= 1) == 0) {                           \
				_mask = 0x80;                                   \
				_dst++;                                         \
			}                                                   \
		}                                                       \
	}


void Wiz::auxWRLEUncompressPixelStream(WizRawPixel *destStream, const byte *singleColorTable, const byte *streamData, int streamSize, const WizRawPixel *conversionTable) {
	int value, runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destStream;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destStream;

	while (streamSize > 0) {
		value = *streamData++;

		if (value & 0x01) { // xxxxxxx1
			// Tiny run handler
			runCount = WRLE_TINY_RUN_MIN_COUNT + ((value >> 1) & 0x03);
			streamSize -= runCount;

			memset8BppConversion(destStream, *(singleColorTable + (value >> 3)), runCount, conversionTable);

			if (!_uses16BitColor) {
				dest8 += runCount;
				destStream = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				destStream = (WizRawPixel *)dest16;
			}

		} else if (!(value & 0x02)) { // xxxxxx00
			// Transparent run
			runCount = value >> 2;
			if (0 == runCount) {
				runCount = 1 + (*streamData + 256 * *(streamData + 1));
				streamData += 2;
			}

			streamSize -= runCount;

			if (!_uses16BitColor) {
				dest8 += runCount;
				destStream = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				destStream = (WizRawPixel *)dest16;
			}

		} else if (value & 0x04) { // xxxxx110 ?
			// Run of some color
			value >>= 3;
			if (0 == value) {
				runCount = 1 + *streamData++;
			} else {
				runCount = value;
			}

			streamSize -= runCount;

			memset8BppConversion(destStream, *streamData, runCount, conversionTable);

			streamData += sizeof(byte);

			if (!_uses16BitColor) {
				dest8 += runCount;
				destStream = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				destStream = (WizRawPixel *)dest16;
			}

		} else { // xxxxx010
			// Single color!
			if (!_uses16BitColor) {
				*dest8++ = (WizRawPixel8)convert8BppToRawPixel(*(singleColorTable + (value >> 3)), conversionTable);
				destStream = (WizRawPixel *)dest8;
			} else {
				*dest16++ = (WizRawPixel16)convert8BppToRawPixel(*(singleColorTable + (value >> 3)), conversionTable);
				destStream = (WizRawPixel *)dest16;
			}

			streamSize--;
		}
	}
}

void Wiz::auxWRLEUncompressAndCopyFromStreamOffset(WizRawPixel *destStream, const byte *singleColorTable, const byte *streamData, int streamSize, byte copyFromColor, int streamOffset, const WizRawPixel *conversionTable) {
	int value, runCount;
	byte color;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destStream;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destStream;

	while (streamSize > 0) {
		value = *streamData++;

		if (value & 0x01) { // xxxxxxx1
			// Tiny run handler
			runCount = WRLE_TINY_RUN_MIN_COUNT + ((value >> 1) & 0x03);
			streamSize -= runCount;

			color = *(singleColorTable + (value >> 3));

			if (copyFromColor != color) {
				memset8BppConversion(destStream, color, runCount, conversionTable);
			} else {
				if (!_uses16BitColor) {
					memcpy(dest8, dest8 + streamOffset, (runCount * sizeof(WizRawPixel8)));
				} else {
					memcpy(dest16, dest16 + streamOffset, (runCount * sizeof(WizRawPixel16)));
				}
			}

			if (!_uses16BitColor) {
				dest8 += runCount;
				destStream = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				destStream = (WizRawPixel *)dest16;
			}

		} else if (!(value & 0x02)) { // xxxxxx00
			// Transparent run
			runCount = value >> 2;
			if (runCount == 0) {
				runCount = 1 + (*streamData + 256 * *(streamData + 1));
				streamData += 2;
			}

			streamSize -= runCount;

			if (!_uses16BitColor) {
				dest8 += runCount;
				destStream = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				destStream = (WizRawPixel *)dest16;
			}

		} else if (value & 0x04) { // xxxxx110 ?
			// Run of some color
			value >>= 3;
			if (value == 0) {
				runCount = 1 + *streamData++;
			} else {
				runCount = value;
			}

			streamSize -= runCount;

			color = *streamData;

			if (color != copyFromColor) {
				memset8BppConversion(destStream, color, runCount, conversionTable);
			} else {
				if (!_uses16BitColor) {
					memcpy(dest8, dest8 + streamOffset, (runCount * sizeof(WizRawPixel8)));
				} else {
					memcpy(dest16, dest16 + streamOffset, (runCount * sizeof(WizRawPixel16)));
				}
			}

			streamData += sizeof(byte);

			if (!_uses16BitColor) {
				dest8 += runCount;
				destStream = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				destStream = (WizRawPixel *)dest16;
			}

		} else { // xxxxx010
			// Single color!
			color = *(singleColorTable + (value >> 3));

			if (!_uses16BitColor) {
				if (color != copyFromColor) {
					*dest8++ = (WizRawPixel8)convert8BppToRawPixel(color, conversionTable);
				} else {
					*dest8 = *(dest8 + streamOffset);
					dest8++;
				}

				destStream = (WizRawPixel *)dest8;
			} else {
				if (color != copyFromColor) {
					*dest16++ = (WizRawPixel16)convert8BppToRawPixel(color, conversionTable);
				} else {
					*dest16 = *(dest16 + streamOffset);
					dest16++;
				}

				destStream = (WizRawPixel *)dest16;
			}

			streamSize--;
		}
	}
}

void Wiz::auxDecompSRLEStream(WizRawPixel *destStream, const WizRawPixel *backgroundStream, const byte *singleColorTable, const byte *streamData, int streamSize, const WizRawPixel *conversionTable) {
	int value, runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destStream;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destStream;
	const WizRawPixel8 *background8 = (const WizRawPixel8 *)backgroundStream;
	const WizRawPixel16 *background16 = (const WizRawPixel16 *)backgroundStream;

	while (streamSize > 0) {
		value = *streamData++;

		if (value & 0x01) { // xxxxxxx1
			// Transparent run
			runCount = value >> 1;
			if (runCount == 0) {
				runCount = 1 + (*streamData + 256 * *(streamData + 1));
				streamData += 2;
			}

			streamSize -= runCount;

			if (!_uses16BitColor) {
				background8 += runCount;
				dest8 += runCount;

				backgroundStream = (const WizRawPixel *)background8;
				destStream = (WizRawPixel *)dest8;
			} else {
				background16 += runCount;
				dest16 += runCount;

				backgroundStream = (const WizRawPixel *)background16;
				destStream = (WizRawPixel *)dest16;
			}

		} else if (!(value & 0x02)) { // xxxxxx00
			// Background run
			runCount = 1 + (value >> 2);
			streamSize -= runCount;

			if (!_uses16BitColor) {
				memcpy(dest8, background8, runCount * sizeof(WizRawPixel8));
				background8 += runCount;
				dest8 += runCount;

				backgroundStream = (const WizRawPixel *)background8;
				destStream = (WizRawPixel *)dest8;
			} else {
				memcpy(dest16, background16, runCount * sizeof(WizRawPixel16));
				background16 += runCount;
				dest16 += runCount;

				backgroundStream = (const WizRawPixel *)background16;
				destStream = (WizRawPixel *)dest16;
			}

		} else if (value & 0x04) { // xxxxx110
			// Run of some color
			value >>= 3;
			if (value == 0) {
				runCount = 1 + *streamData++;
			} else {
				runCount = value;
			}

			streamSize -= runCount;

			memset8BppConversion(
				destStream, *streamData++, runCount,
				conversionTable);

			if (!_uses16BitColor) {
				background8 += runCount;
				dest8 += runCount;

				backgroundStream = (const WizRawPixel *)background8;
				destStream = (WizRawPixel *)dest8;
			} else {
				background16 += runCount;
				dest16 += runCount;

				backgroundStream = (const WizRawPixel *)background16;
				destStream = (WizRawPixel *)dest16;
			}

		} else { // xxxxx010
			// Single color!
			if (!_uses16BitColor) {
				*dest8++ = (WizRawPixel8)convert8BppToRawPixel(*(singleColorTable + (value >> 3)), conversionTable);
				background8++;

				destStream = (WizRawPixel *)dest8;
				backgroundStream = (const WizRawPixel *)background8;
			} else {
				*dest16++ = (WizRawPixel16)convert8BppToRawPixel(*(singleColorTable + (value >> 3)), conversionTable);
				background16++;

				destStream = (WizRawPixel *)dest16;
				backgroundStream = (const WizRawPixel *)background16;
			}

			streamSize--;
		}
	}
}

void Wiz::auxDecompDRLEImage(WizRawPixel *foregroundBufferPtr, WizRawPixel *backgroundBufferPtr, const byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, const WizRawPixel *conversionTable) {
	Common::Rect sourceRect, destRect, clipRect, workRect;

	sourceRect.left = 0;
	sourceRect.top = 0;
	sourceRect.right = width - 1;
	sourceRect.bottom = height - 1;

	destRect.left = x;
	destRect.top = y;
	destRect.right = x + width - 1;
	destRect.bottom = y + height - 1;

	// Custom clip rect...
	if (clipRectPtr) {
		clipRect = *clipRectPtr;
		workRect.left = 0;
		workRect.top = 0;
		workRect.right = bufferWidth - 1;
		workRect.bottom = bufferHeight - 1;

		if (!findRectOverlap(&clipRect, &workRect)) {
			return;
		}
	} else {
		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = bufferWidth - 1;
		clipRect.bottom = bufferHeight - 1;
	}

	// Clip the source & dest coords to the clipping rectangle...
	clipRectCoords(&sourceRect, &destRect, &clipRect);

	if (destRect.right < destRect.left) {
		return;
	}

	if (destRect.bottom < destRect.top) {
		return;
	}

	if (sourceRect.right < sourceRect.left) {
		return;
	}

	if (sourceRect.bottom < sourceRect.top) {
		return;
	}

	auxDecompDRLEPrim(
		foregroundBufferPtr, backgroundBufferPtr, bufferWidth,
		&destRect, compData, &sourceRect, conversionTable);
}

void Wiz::auxDecompDRLEPrim(WizRawPixel *foregroundBufferPtr, WizRawPixel *backgroundBufferPtr, int bufferWidth, Common::Rect *destRect, const byte *compData,  Common::Rect *sourceRect, const WizRawPixel *conversionTable) {
	int decompWidth, decompHeight, counter, sX1, dX1, lineSize;
	WizRawPixel8 *foregroundBuffer8 = (WizRawPixel8 *)foregroundBufferPtr;
	WizRawPixel16 *foregroundBuffer16 = (WizRawPixel16 *)foregroundBufferPtr;
	WizRawPixel8 *backgroundBuffer8 = (WizRawPixel8 *)backgroundBufferPtr;
	WizRawPixel16 *backgroundBuffer16 = (WizRawPixel16 *)backgroundBufferPtr;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;

	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	if (!_uses16BitColor) {
		foregroundBuffer8 += bufferWidth * destRect->top + dX1;
		backgroundBuffer8 += bufferWidth * destRect->top + dX1;

		foregroundBufferPtr = (WizRawPixel *)foregroundBuffer8;
		backgroundBufferPtr = (WizRawPixel *)backgroundBuffer8;
	} else {
		foregroundBuffer16 += bufferWidth * destRect->top + dX1;
		backgroundBuffer16 += bufferWidth * destRect->top + dX1;

		foregroundBufferPtr = (WizRawPixel *)foregroundBuffer16;
		backgroundBufferPtr = (WizRawPixel *)backgroundBuffer16;
	}

	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible...
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxDecompDRLEStream(
				foregroundBufferPtr, compData + 2, backgroundBufferPtr,
				sX1, decompWidth, conversionTable);

			compData += lineSize + 2;
		} else {
			// Handle a completely transparent line!
			compData += 2;
		}

		if (!_uses16BitColor) {
			foregroundBuffer8 += bufferWidth;
			backgroundBuffer8 += bufferWidth;

			foregroundBufferPtr = (WizRawPixel *)foregroundBuffer8;
			backgroundBufferPtr = (WizRawPixel *)backgroundBuffer8;
		} else {
			foregroundBuffer16 += bufferWidth;
			backgroundBuffer16 += bufferWidth;

			foregroundBufferPtr = (WizRawPixel *)foregroundBuffer16;
			backgroundBufferPtr = (WizRawPixel *)backgroundBuffer16;
		}
	}
}

void Wiz::auxDecompTRLEImage(WizRawPixel *bufferPtr, const byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, const WizRawPixel *conversionTable) {
	Common::Rect sourceRect, destRect, clipRect, workRect;

	sourceRect.left = 0;
	sourceRect.top = 0;
	sourceRect.right = width - 1;
	sourceRect.bottom = height - 1;

	destRect.left = x;
	destRect.top = y;
	destRect.right = x + width - 1;
	destRect.bottom = y + height - 1;

	// Custom clip rect...
	if (clipRectPtr) {
		clipRect = *clipRectPtr;
		workRect.left = 0;
		workRect.top = 0;
		workRect.right = bufferWidth - 1;
		workRect.bottom = bufferHeight - 1;
		if (!findRectOverlap(&clipRect, &workRect)) {
			return;
		}
	} else {
		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = bufferWidth - 1;
		clipRect.bottom = bufferHeight - 1;
	}

	// Clip the source & dest coords to the clipping rectangle...
	clipRectCoords(&sourceRect, &destRect, &clipRect);

	if (destRect.right < destRect.left) {
		return;
	}

	if (destRect.bottom < destRect.top) {
		return;
	}

	if (sourceRect.right < sourceRect.left) {
		return;
	}

	if (sourceRect.bottom < sourceRect.top) {
		return;
	}

	// Call the primitive image renderer...
	auxDecompTRLEPrim(
		bufferPtr, bufferWidth, &destRect, compData,
		&sourceRect, conversionTable);
}

void Wiz::auxDecompTRLEPrim(WizRawPixel *bufferPtr, int bufferWidth, Common::Rect *destRect, const byte *compData, Common::Rect *sourceRect, const WizRawPixel *conversionTable) {
	int decompWidth, decompHeight, counter, sX1, dX1, lineSize;
	WizRawPixel8 *buffer8 = (WizRawPixel8 *)bufferPtr;
	WizRawPixel16 *buffer16 = (WizRawPixel16 *)bufferPtr;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;

	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	if (!_uses16BitColor) {
		buffer8 += bufferWidth * destRect->top + dX1;
		bufferPtr = (WizRawPixel *)buffer8;
	} else {
		buffer16 += bufferWidth * destRect->top + dX1;
		bufferPtr = (WizRawPixel *)buffer16;
	}

	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible...
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxDecompTRLEStream(
				bufferPtr, compData + 2, sX1, decompWidth, conversionTable);

			compData += lineSize + 2;

		} else {
			// Handle a completely transparent line!
			compData += 2;
		}

		if (!_uses16BitColor) {
			buffer8 += bufferWidth;
			bufferPtr = (WizRawPixel *)buffer8;
		} else {
			buffer16 += bufferWidth;
			bufferPtr = (WizRawPixel *)buffer16;
		}
	}
}


void Wiz::auxDrawZplaneFromTRLEImage(byte *zplanePtr, const byte *compData, int zplanePixelWidth, int zplanePixelHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, int transOp, int solidOp) {
	Common::Rect sourceRect, destRect, clipRect, workRect;

	sourceRect.left = 0;
	sourceRect.top = 0;
	sourceRect.right = width - 1;
	sourceRect.bottom = height - 1;

	destRect.left = x;
	destRect.top = y;
	destRect.right = x + width - 1;
	destRect.bottom = y + height - 1;

	// Custom clip rect...
	if (clipRectPtr) {
		clipRect = *clipRectPtr;

		workRect.left = 0;
		workRect.top = 0;
		workRect.right = zplanePixelWidth - 1;
		workRect.bottom = zplanePixelHeight - 1;

		if (!findRectOverlap(&clipRect, &workRect)) {
			return;
		}
	} else {
		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = zplanePixelWidth - 1;
		clipRect.bottom = zplanePixelHeight - 1;
	}

	// Clip the source & dest coords to the clipping rectangle...
	clipRectCoords(&sourceRect, &destRect, &clipRect);

	if (destRect.right < destRect.left) {
		return;
	}

	if (destRect.bottom < destRect.top) {
		return;
	}

	if (sourceRect.right < sourceRect.left) {
		return;
	}

	if (sourceRect.bottom < sourceRect.top) {
		return;
	}

	// Call the primitive image renderer...
	auxDrawZplaneFromTRLEPrim(
		zplanePtr, zplanePixelWidth, &destRect, compData, &sourceRect, transOp, solidOp);
}

void Wiz::auxDrawZplaneFromTRLEPrim(byte *zplanePtr, int zplanePixelWidth, Common::Rect *destRect, const byte *compData, Common::Rect *sourceRect, int transOp, int solidOp) {
	int decompWidth, decompHeight, counter, sX1, dX1, lineSize, mask, zplaneWidth;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;

	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	zplaneWidth = zplanePixelWidth / 8;
	zplanePtr += zplaneWidth * destRect->top + (dX1 / 8);
	mask = 1 << (7 - (dX1 % 8));

	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible...
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxZplaneFromTRLEStream(
				zplanePtr, compData + 2, sX1, decompWidth, mask, transOp, solidOp);

			compData += lineSize + 2;
		} else {
			// Handle a completely transparent line!
			compData += 2;
		}

		zplanePtr += zplaneWidth;
	}
}

void Wiz::auxDecompRemappedTRLEImage(WizRawPixel *bufferPtr, const byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, byte *remapTable, const WizRawPixel *conversionTable) {
	Common::Rect sourceRect, destRect, clipRect, workRect;

	sourceRect.left = 0;
	sourceRect.top = 0;
	sourceRect.right = width - 1;
	sourceRect.bottom = height - 1;

	destRect.left = x;
	destRect.top = y;
	destRect.right = x + width - 1;
	destRect.bottom = y + height - 1;

	// Custom clip rect...
	if (clipRectPtr) {
		clipRect = *clipRectPtr;

		workRect.left = 0;
		workRect.top = 0;
		workRect.right = bufferWidth - 1;
		workRect.bottom = bufferHeight - 1;

		if (!findRectOverlap(&clipRect, &workRect)) {
			return;
		}
	} else {
		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = bufferWidth - 1;
		clipRect.bottom = bufferHeight - 1;
	}

	// Clip the source & dest coords to the clipping rectangle...
	clipRectCoords(&sourceRect, &destRect, &clipRect);

	if (destRect.right < destRect.left) {
		return;
	}

	if (destRect.bottom < destRect.top) {
		return;
	}

	if (sourceRect.right < sourceRect.left) {
		return;
	}

	if (sourceRect.bottom < sourceRect.top) {
		return;
	}

	// Call the primitive image renderer...
	auxDecompRemappedTRLEPrim(
		bufferPtr, bufferWidth, &destRect, compData, &sourceRect, remapTable,
		conversionTable);
}

void Wiz::auxDecompRemappedTRLEPrim(WizRawPixel *bufferPtr, int bufferWidth, Common::Rect *destRect, const byte *compData, Common::Rect *sourceRect, byte *remapTable, const WizRawPixel *conversionTable) {
	int decompWidth, decompHeight, sX1, dX1, lineSize;
	WizRawPixel8 *buffer8 = (WizRawPixel8 *)bufferPtr;
	WizRawPixel16 *buffer16 = (WizRawPixel16 *)bufferPtr;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;

	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	if (!_uses16BitColor) {
		buffer8 += bufferWidth * destRect->top + dX1;
		bufferPtr = (WizRawPixel *)buffer8;
	} else {
		buffer16 += bufferWidth * destRect->top + dX1;
		bufferPtr = (WizRawPixel *)buffer16;
	}

	for (int i = sourceRect->top; i > 0; i--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible...
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxDecompRemappedTRLEStream(
				bufferPtr, compData + 2, sX1, decompWidth,
				remapTable, conversionTable);

			compData += lineSize + 2;
		} else {
			// Handle a completely transparent line!
			compData += 2;
		}

		if (!_uses16BitColor) {
			buffer8 += bufferWidth;
			bufferPtr = (WizRawPixel *)buffer8;
		} else {
			buffer16 += bufferWidth;
			bufferPtr = (WizRawPixel *)buffer16;
		}
	}
}

int Wiz::auxHitTestTRLEXPos(const byte *dataStream, int skipAmount) {
	int runCount;

	// Decompress bytes to do simple clipping...
	while (skipAmount > 0) {

		if ((runCount = *dataStream++) & 1) {

			// Handle the transparent color...
			runCount >>= 1;
			if (runCount > skipAmount) {
				return 0;
			} else {
				skipAmount -= runCount;
			}

		} else if (runCount & 2) {

			// Handle a run of color...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				return 1;
			} else {
				skipAmount -= runCount;
				dataStream++;
			}

		} else {
			// Handle a literal run of pixels...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				return 1;
			} else {
				skipAmount -= runCount;
				dataStream += runCount;
			}
		}
	}

	// Transparent or not?
	runCount = *dataStream++;

	if (runCount & 1) {
		return 0;
	} else {
		return 1;
	}
}

int Wiz::auxHitTestTRLEImageRelPos(const byte *compData, int x, int y, int width, int height) {
	// Quickly reject points outside the image boundary.
	if ((x < 0) || (width <= x) || (y < 0) || (height <= y)) {
		return 0;
	}

	// Quickly skip down to the lines to be compressed & dest position...
	for (int i = 0; i < y; i++) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	if (READ_LE_UINT16(compData) != 0) {
		return auxHitTestTRLEXPos(compData + 2, x);
	} else {
		return 0;
	}
}

int Wiz::auxPixelHitTestTRLEXPos(byte *dataStream, int skipAmount, int transparentValue) {
	int runCount;

	// Decompress bytes to do simple clipping...
	while (skipAmount > 0) {
		if ((runCount = *dataStream++) & 1) {
			// Handle the transparent color...
			runCount >>= 1;

			if (runCount > skipAmount) {
				return transparentValue;
			} else {
				skipAmount -= runCount;
			}

		} else if (runCount & 2) {
			// Handle a run of color...
			runCount = (runCount >> 2) + 1;

			if (runCount > skipAmount) {
				return *dataStream;
			} else {
				skipAmount -= runCount;
				dataStream++;
			}
		} else {
			// Handle a literal run of pixels...
			runCount = (runCount >> 2) + 1;

			if (runCount > skipAmount) {
				runCount -= skipAmount;
				dataStream += skipAmount;
				return *dataStream;
			} else {
				skipAmount -= runCount;
				dataStream += runCount;
			}
		}
	}

	// Really decompress to the dest buffer...
	runCount = *dataStream++;

	if (runCount & 1) {
		return transparentValue;
	}

	return *dataStream;
}

int Wiz::auxPixelHitTestTRLEImageRelPos(byte *compData, int x, int y, int width, int height, int transparentValue) {
	// Quickly reject points outside the image boundary.
	if ((x < 0) || (width <= x) || (y < 0) || (height <= y)) {
		return transparentValue;
	}

	// Quickly skip down to the lines to be compressed & dest position...
	for (int i = 0; i < y; i++) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	if (READ_LE_UINT16(compData) != 0) {
		return auxPixelHitTestTRLEXPos(compData + 2, x, transparentValue);
	} else {
		return transparentValue;
	}
}

void Wiz::auxDecompMixColorsTRLEImage(WizRawPixel *bufferPtr, byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, const byte *colorMixTable, const WizRawPixel *conversionTable) {
	Common::Rect sourceRect, destRect, clipRect, workRect;

	sourceRect.left = 0;
	sourceRect.top = 0;
	sourceRect.right = width - 1;
	sourceRect.bottom = height - 1;

	destRect.left = x;
	destRect.top = y;
	destRect.right = x + width - 1;
	destRect.bottom = y + height - 1;

	// Custom clip rect...
	if (clipRectPtr) {
		clipRect = *clipRectPtr;

		workRect.left = 0;
		workRect.top = 0;
		workRect.right = bufferWidth - 1;
		workRect.bottom = bufferHeight - 1;

		if (!findRectOverlap(&clipRect, &workRect)) {
			return;
		}
	} else {
		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = bufferWidth - 1;
		clipRect.bottom = bufferHeight - 1;
	}

	// Clip the source & dest coords to the clipping rectangle...
	clipRectCoords(&sourceRect, &destRect, &clipRect);

	if (destRect.right < destRect.left) {
		return;
	}

	if (destRect.bottom < destRect.top) {
		return;
	}

	if (sourceRect.right < sourceRect.left) {
		return;
	}

	if (sourceRect.bottom < sourceRect.top) {
		return;
	}

	// Call the primitive image renderer...
	auxDecompMixColorsTRLEPrim(
		bufferPtr, bufferWidth, &destRect, compData, &sourceRect, colorMixTable,
		conversionTable);
}

void Wiz::auxDecompMixColorsTRLEPrim(WizRawPixel *bufferPtr, int bufferWidth, Common::Rect *destRect, const byte *compData, Common::Rect *sourceRect, const byte *coloMixTable, const WizRawPixel *conversionTable) {
	int decompWidth, decompHeight, sX1, dX1, lineSize;
	WizRawPixel8 *buffer8 = (WizRawPixel8 *)bufferPtr;
	WizRawPixel16 *buffer16 = (WizRawPixel16 *)bufferPtr;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;

	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	if (!_uses16BitColor) {
		buffer8 += bufferWidth * destRect->top + dX1;
		bufferPtr = (WizRawPixel *)buffer8;
	} else {
		buffer16 += bufferWidth * destRect->top + dX1;
		bufferPtr = (WizRawPixel *)buffer16;
	}

	for (int i = sourceRect->top; i > 0; i--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible...
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxColorMixDecompressLine(
				bufferPtr, compData + 2, sX1, decompWidth, coloMixTable,
				conversionTable);

			compData += lineSize + 2;
		} else {
			// Handle a completely transparent line!
			compData += 2;
		}

		if (!_uses16BitColor) {
			buffer8 += bufferWidth;
			bufferPtr = (WizRawPixel *)buffer8;
		} else {
			buffer16 += bufferWidth;
			bufferPtr = (WizRawPixel *)buffer16;
		}
	}
}

void Wiz::auxDecompDRLEStream(WizRawPixel *destPtr, const byte *dataStream, WizRawPixel *backgroundPtr, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;
	WizRawPixel8 *background8 = (WizRawPixel8 *)backgroundPtr;
	WizRawPixel16 *background16 = (WizRawPixel16 *)backgroundPtr;

	// Decompress bytes to do simple clipping...
	while (skipAmount > 0) {
		if ((runCount = *dataStream++) & 1) {
			// Handle the transparent color...
			runCount >>= 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto DoTransparentRun;
			} else {
				skipAmount -= runCount;
			}

		} else if (runCount & 2) {
			// Handle a run of color...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto WriteRunData;
			} else {
				skipAmount -= runCount;
				dataStream++;
			}

		} else {
			// Handle a background run of pixels...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				dataStream += skipAmount;
				goto WriteBackgroundData;
			} else {
				skipAmount -= runCount;
				dataStream += runCount;
			}
		}
	}

	// Really decompress to the dest buffer...
	while (decompAmount > 0) {
		runCount = *dataStream++;

		if (runCount & 1) {
			/* xxxxxxx1 */
			runCount >>= 1;

		DoTransparentRun:
			if (!_uses16BitColor) {
				dest8 += runCount;
				background8 += runCount;

				destPtr = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				background16 += runCount;

				destPtr = (WizRawPixel *)dest16;
			}

			decompAmount -= runCount;

		} else if (runCount & 2) {
			/* xxxxxx10 */
			runCount = (runCount >> 2) + 1;

		WriteRunData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				memset8BppConversion(destPtr, *dataStream++, runCount, conversionTable);

				if (!_uses16BitColor) {
					dest8 += runCount;
					background8 += runCount;

					destPtr = (WizRawPixel *)dest8;
				} else {
					dest16 += runCount;
					background16 += runCount;

					destPtr = (WizRawPixel *)dest16;
				}
			} else {
				runCount += decompAmount;
				memset8BppConversion(destPtr, *dataStream++, runCount, conversionTable);
			}
		} else {
			/* xxxxxx00 */
			runCount = (runCount >> 2) + 1;
		WriteBackgroundData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				if (!_uses16BitColor) {
					memcpy(dest8, background8, runCount * sizeof(WizRawPixel8));
					dest8 += runCount;
					background8 += runCount;

					destPtr = (WizRawPixel *)dest8;
				} else {
					memcpy(dest16, background16, runCount * sizeof(WizRawPixel16));
					dest16 += runCount;
					background16 += runCount;

					destPtr = (WizRawPixel *)dest16;
				}
			} else {
				runCount += decompAmount;
				if (!_uses16BitColor) {
					memcpy(dest8, background8, runCount * sizeof(WizRawPixel8));
				} else {
					memcpy(dest16, background16, runCount * sizeof(WizRawPixel16));
				}
			}
		}
	}
}

void Wiz::auxDecompTRLEStream(WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	// Decompress bytes to do simple clipping...
	while (skipAmount > 0) {
		if ((runCount = *dataStream++) & 1) {
			// Handle the transparent color...
			runCount >>= 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto DoTransparentRun;
			} else {
				skipAmount -= runCount;
			}

		} else if (runCount & 2) {
			// Handle a run of color...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto WriteRunData;
			} else {
				skipAmount -= runCount;
				dataStream++;
			}

		} else {
			// Handle a literal run of pixels...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				dataStream += skipAmount;
				goto WriteLiteralData;
			} else {
				skipAmount -= runCount;
				dataStream += runCount;
			}
		}
	}

	// Really decompress to the dest buffer...
	while (decompAmount > 0) {
		runCount = *dataStream++;

		if (runCount & 1) {
			/* xxxxxxx1 */
			runCount >>= 1;

		DoTransparentRun:
			if (!_uses16BitColor) {
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			}

			decompAmount -= runCount;

		} else if (runCount & 2) {
			/* xxxxxx10 */
			runCount = (runCount >> 2) + 1;

		WriteRunData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				memset8BppConversion(destPtr, *dataStream++, runCount, conversionTable);

				if (!_uses16BitColor) {
					dest8 += runCount;
					destPtr = (WizRawPixel *)dest8;
				} else {
					dest16 += runCount;
					destPtr = (WizRawPixel *)dest16;
				}
			} else {
				runCount += decompAmount;
				memset8BppConversion(destPtr, *dataStream, runCount, conversionTable);
			}

		} else {
			/* xxxxxx00 */
			runCount = (runCount >> 2) + 1;

		WriteLiteralData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				memcpy8BppConversion(destPtr, dataStream, runCount, conversionTable);
				dataStream += runCount;

				if (!_uses16BitColor) {
					dest8 += runCount;
					destPtr = (WizRawPixel *)dest8;
				} else {
					dest16 += runCount;
					destPtr = (WizRawPixel *)dest16;
				}
			} else {
				runCount += decompAmount;
				memcpy8BppConversion(destPtr, dataStream, runCount, conversionTable);
			}
		}
	}
}

void Wiz::auxDecompRemappedTRLEStream(WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, byte *remapTable, const WizRawPixel *conversionTable) {
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	// Decompress bytes to do simple clipping...
	while (skipAmount > 0) {
		if ((runCount = *dataStream++) & 1) {
			// Handle the transparent color...
			runCount >>= 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto DoTransparentRun;
			} else {
				skipAmount -= runCount;
			}

		} else if (runCount & 2) {
			// Handle a run of color...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto WriteRunData;
			} else {
				skipAmount -= runCount;
				dataStream++;
			}

		} else {
			// Handle a literal run of pixels...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				dataStream += skipAmount;
				goto WriteLiteralData;
			} else {
				skipAmount -= runCount;
				dataStream += runCount;
			}
		}
	}

	// Really decompress to the dest buffer...
	while (decompAmount > 0) {
		runCount = *dataStream++;

		if (runCount & 1) {
			/* xxxxxxx1 */
			runCount >>= 1;
		DoTransparentRun:
			if (!_uses16BitColor) {
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			}

			decompAmount -= runCount;

		} else if (runCount & 2) {
			/* xxxxxx10 */
			runCount = (runCount >> 2) + 1;
		WriteRunData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				memset8BppConversion(destPtr, *(remapTable + *dataStream++), runCount, conversionTable);

				if (!_uses16BitColor) {
					dest8 += runCount;
					destPtr = (WizRawPixel *)dest8;
				} else {
					dest16 += runCount;
					destPtr = (WizRawPixel *)dest16;
				}
			} else {
				runCount += decompAmount;
				memset8BppConversion(destPtr, *(remapTable + *dataStream), runCount, conversionTable);
			}

		} else {
			/* xxxxxx00 */
			runCount = (runCount >> 2) + 1;
		WriteLiteralData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				auxRemappedMemcpy(destPtr, dataStream, runCount, remapTable, conversionTable);
				dataStream += runCount;

				if (!_uses16BitColor) {
					dest8 += runCount;
					destPtr = (WizRawPixel *)dest8;
				} else {
					dest16 += runCount;
					destPtr = (WizRawPixel *)dest16;
				}
			} else {
				runCount += decompAmount;
				auxRemappedMemcpy(destPtr, dataStream, runCount, remapTable, conversionTable);
			}
		}
	}
}


void Wiz::auxZplaneFromTRLEStream(byte *destPtr, const byte *dataStream, int skipAmount, int decompAmount, int mask, int transOp, int solidOp) {
	int runCount;

	// Decompress bytes to do simple clipping...
	while (skipAmount > 0) {
		if ((runCount = *dataStream++) & 1) {
			// Handle the transparent color...
			runCount >>= 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto DoTransparentRun;
			} else {
				skipAmount -= runCount;
			}

		} else if (runCount & 2) {
			// Handle a run of color...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto WriteRunData;
			} else {
				skipAmount -= runCount;
				dataStream++;
			}

		} else {
			// Handle a literal run of pixels...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				dataStream += skipAmount;
				goto WriteLiteralData;
			} else {
				skipAmount -= runCount;
				dataStream += runCount;
			}
		}
	}

	// Really decompress to the dest buffer...
	while (decompAmount > 0) {
		runCount = *dataStream++;

		if (runCount & 1) {
			/* xxxxxxx1 */
			runCount >>= 1;
		DoTransparentRun:
			if (transOp == kWZOIgnore) {
				AUX_IGNORE_ZPLANE_BITS(destPtr, mask, runCount);
			} else if (transOp == kWZOClear) {
				AUX_CLEAR_ZPLANE_BITS(destPtr, mask, runCount);
			} else {
				AUX_SET_ZPLANE_BITS(destPtr, mask, runCount);
			}

			decompAmount -= runCount;

		} else if (runCount & 2) {
			/* xxxxxx10 */
			runCount = (runCount >> 2) + 1;
		WriteRunData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				if (solidOp == kWZOSet) {
					AUX_SET_ZPLANE_BITS(destPtr, mask, runCount);
				} else if (solidOp == kWZOClear) {
					AUX_CLEAR_ZPLANE_BITS(destPtr, mask, runCount);
				} else {
					AUX_IGNORE_ZPLANE_BITS(destPtr, mask, runCount);
				}
				dataStream++;
			} else {
				runCount += decompAmount;
				if (solidOp == kWZOSet) {
					AUX_SET_ZPLANE_BITS(destPtr, mask, runCount);
				} else if (solidOp == kWZOClear) {
					AUX_CLEAR_ZPLANE_BITS(destPtr, mask, runCount);
				} else {
					AUX_IGNORE_ZPLANE_BITS(destPtr, mask, runCount);
				}
			}

		} else {
			/* xxxxxx00 */
			runCount = (runCount >> 2) + 1;

		WriteLiteralData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				if (solidOp == kWZOSet) {
					AUX_SET_ZPLANE_BITS(destPtr, mask, runCount);
				} else if (solidOp == kWZOClear) {
					AUX_CLEAR_ZPLANE_BITS(destPtr, mask, runCount);
				} else {
					AUX_IGNORE_ZPLANE_BITS(destPtr, mask, runCount);
				}
				dataStream += runCount;
			} else {
				runCount += decompAmount;
				if (solidOp == kWZOSet) {
					AUX_SET_ZPLANE_BITS(destPtr, mask, runCount);
				} else if (solidOp == kWZOClear) {
					AUX_CLEAR_ZPLANE_BITS(destPtr, mask, runCount);
				} else {
					AUX_IGNORE_ZPLANE_BITS(destPtr, mask, runCount);
				}
			}
		}
	}
}

void Wiz::auxColorMixDecompressLine(WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *colorMixTable, const WizRawPixel *conversionTable) {
	if (_uses16BitColor)
		return;

	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	const byte *remapTable;
	int runCount;

	// Decompress bytes to do simple clipping...
	while (skipAmount > 0) {
		if ((runCount = *dataStream++) & 1) {
			// Handle the transparent color...
			runCount >>= 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto DoTransparentRun;
			} else {
				skipAmount -= runCount;
			}
		} else if (runCount & 2) {
			// Handle a run of color...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto WriteRunData;
			} else {
				skipAmount -= runCount;
				dataStream++;
			}

		} else {
			// Handle a literal run of pixels...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				dataStream += skipAmount;
				goto WriteLiteralData;
			} else {
				skipAmount -= runCount;
				dataStream += runCount;
			}
		}
	}

	// Really decompress to the dest buffer...
	while (decompAmount > 0) {
		runCount = *dataStream++;

		if (runCount & 1) {
			/* xxxxxxx1 */
			runCount >>= 1;
		DoTransparentRun:
			dest8 += runCount;
			decompAmount -= runCount;

		} else if (runCount & 2) {
			/* xxxxxx10 */
			runCount = (runCount >> 2) + 1;
		WriteRunData:
			decompAmount -= runCount;
			if (decompAmount < 0) {
				runCount += decompAmount;
			}
			remapTable = colorMixTable + ((*((const byte *)dataStream)) << 8);
			dataStream++;
			while (--runCount >= 0) {
				*dest8 = *(remapTable + *dest8);
				dest8++;
			}

		} else {
			/* xxxxxx00 */
			runCount = (runCount >> 2) + 1;
		WriteLiteralData:
			decompAmount -= runCount;
			if (decompAmount < 0) {
				runCount += decompAmount;
			}
			while (--runCount >= 0) {
				*dest8 = *(colorMixTable + ((*((const byte *)dataStream)) << 8) + *dest8);
				dest8++;
				dataStream++;
			}
		}
	}
}

void Wiz::auxHistogramTRLELine(int *tablePtr, const byte *dataStream, int skipAmount, int decompAmount) {
	int runCount;

	// Decompress bytes to do simple clipping...
	while (skipAmount > 0) {
		if ((runCount = *dataStream++) & 1) {
			// Handle the transparent color...
			runCount >>= 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto DoTransparentRun;
			} else {
				skipAmount -= runCount;
			}

		} else if (runCount & 2) {
			// Handle a run of color...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				goto WriteRunData;
			} else {
				skipAmount -= runCount;
				dataStream += sizeof(byte);
			}

		} else {
			// Handle a literal run of pixels...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				runCount -= skipAmount;
				dataStream += (skipAmount * sizeof(byte));
				goto WriteLiteralData;
			} else {
				skipAmount -= runCount;
				dataStream += (runCount * sizeof(byte));
			}
		}
	}

	// Really histogram the non-skipped data...
	while (decompAmount > 0) {
		runCount = *dataStream++;

		if (runCount & 1) {
			/* xxxxxxx1 */
			runCount >>= 1;

		DoTransparentRun:
			decompAmount -= runCount;

		} else if (runCount & 2) {
			/* xxxxxx10 */
			runCount = (runCount >> 2) + 1;

		WriteRunData:
			decompAmount -= runCount;
			if (decompAmount < 0) {
				runCount += decompAmount;
			}

			*(tablePtr + *((const byte *)dataStream)) += runCount;
			dataStream += sizeof(byte);
		} else {
			/* xxxxxx00 */
			runCount = (runCount >> 2) + 1;

		WriteLiteralData:
			decompAmount -= runCount;
			if (decompAmount < 0) {
				runCount += decompAmount;
			}

			while (--runCount >= 0) {
				(*(tablePtr + *((const byte *)dataStream)))++;
				dataStream += sizeof(byte);
			}
		}
	}
}

void Wiz::auxHistogramTRLEPrim(int *histogramTablePtr, byte *compData, Common::Rect *sourceRect) {
	int decompWidth, decompHeight, counter, sX1, lineSize;

	// Yet more general setup...
	sX1 = sourceRect->left;
	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible...
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxHistogramTRLELine(histogramTablePtr, compData + 2, sX1, decompWidth);
			compData += lineSize + 2;
		} else {
			// Handle a completely transparent line!
			compData += 2;
		}
	}
}

void Wiz::auxRemappedMemcpy(WizRawPixel *dstPtr, const byte *srcPtr, int count, byte *remapTable, const WizRawPixel *conversionTable) {
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	do {
		if (!_uses16BitColor) {
			*dst8++ = (WizRawPixel8)convert8BppToRawPixel(*(remapTable + *srcPtr++), conversionTable);
		} else {
			*dst16++ = (WizRawPixel16)convert8BppToRawPixel(*(remapTable + *srcPtr++), conversionTable);
		}

	} while (--count > 0);
}

} // End of namespace Scumm

#endif // ENABLE_HE
