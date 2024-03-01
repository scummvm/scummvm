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

#include "common/math.h"
#include "common/system.h"
#include "scumm/he/intern_he.h"
#include "scumm/he/wiz_he.h"

namespace Scumm {

#define WRLE_QUICK_CACHE					(0)
#define WRLE_CAN_STOP_EARLY					(0)
#define TRACK_COMPRESSION_STATS				(0)

#define WRLE_BIT_SIZE(zp)					(1 << (8-(zp)))

#define WRLE_TINY_RUN_BITS					0x01
#define WRLE_TINY_RUN_BIT_COUNT				1
#define WRLE_TINY_RUN_COUNT_BITS			2
#define WRLE_TINY_RUN_MIN_COUNT				3
#define WRLE_TINY_RUN_BITS_OVERHEAD			((WRLE_TINY_RUN_BIT_COUNT)+(WRLE_TINY_RUN_COUNT_BITS))
#define WRLE_TINY_RUN_MAX_COUNT				((WRLE_TINY_RUN_MIN_COUNT) + ((1 << (WRLE_TINY_RUN_COUNT_BITS)) - 1))

#define WRLE_TRUN_BITS						0x00
#define WRLE_TRUN_BIT_COUNT					2
#define WRLE_SMALL_TRUN_SIZE				(WRLE_BIT_SIZE(WRLE_TRUN_BIT_COUNT)-1)
#define WRLE_LARGE_TRUN_SIZE				65536

#define WRLE_SINGLE_COLOR_BITS				0x02
#define WRLE_SINGLE_COLOR_BIT_COUNT			3

#define WRLE_LESS_RUN_BITS					0x06
#define WRLE_LESS_RUN_BIT_COUNT				3
#define WRLE_LESS_SMALL_RUN_SIZE			(WRLE_BIT_SIZE(WRLE_LESS_RUN_BIT_COUNT)-1)
#define WRLE_LESS_LARGE_RUN_SIZE			256

#define AUX_IGNORE_ZPLANE_BITS(_dst, _mask, _count)         \
	{                                                       \
		 int _counter;                                      \
		for (_counter = 0; _counter < _count; _counter++) { \
			if (0 == (_mask >>= 1)) {                       \
				_mask = 0x80;                               \
				_dst++;                                     \
			}                                               \
		}                                                   \
	}

#define AUX_SET_ZPLANE_BITS(_dst, _mask, _count)            \
	{                                                       \
		int _counter;                                       \
		for (_counter = 0; _counter < _count; _counter++) { \
			*(_dst) |= _mask;                               \
			if (0 == (_mask >>= 1)) {                       \
				_mask = 0x80;                               \
				_dst++;                                     \
			}                                               \
		}                                                   \
	}

#define AUX_CLEAR_ZPLANE_BITS(_dst, _mask, _count)          \
	{                                                       \
		int _counter;                                       \
		for (_counter = 0; _counter < _count; _counter++) { \
			*(_dst) &= ~_mask;                              \
			if (0 == (_mask >>= 1)) {                       \
				_mask = 0x80;                               \
				_dst++;                                     \
			}                                               \
		}                                                   \
	}


void Wiz::auxWRLEUncompressPixelStream(WizRawPixel *destStream, const byte *singleColorTable, const byte *streamData, int streamSize, const WizRawPixel *conversionTable) {
	int value, runCount;

	while (streamSize > 0) {
		value = *streamData++;

		if (value & 0x01) { // xxxxxxx1
			// Tiny run handler
			runCount = WRLE_TINY_RUN_MIN_COUNT + ((value >> 1) & 0x03);
			streamSize -= runCount;

			memset8BppConversion(
				destStream, *(singleColorTable + (value >> 3)),
				runCount, conversionTable);

			destStream += runCount;

		} else if (!(value & 0x02)) { // xxxxxx00
			// Transparent run
			runCount = value >> 2;
			if (0 == runCount) {
				runCount = 1 + (*streamData + 256 * *(streamData + 1));
				streamData += 2;
			}

			streamSize -= runCount;
			destStream += runCount;

		} else if (value & 0x04) { // xxxxx110 ?
			// Run of some color
			value >>= 3;
			if (0 == value) {
				runCount = 1 + *streamData++;
			} else {
				runCount = value;
			}

			streamSize -= runCount;

			memset8BppConversion(
				destStream, *((byte *)streamData),
				runCount, conversionTable);

			streamData += sizeof(byte);
			destStream += runCount;

		} else { // xxxxx010
			// Single color!
			*destStream++ = convert8BppToRawPixel(*(singleColorTable + (value >> 3)), conversionTable);
			streamSize--;
		}
	}
}

void Wiz::auxWRLEUncompressAndCopyFromStreamOffset(WizRawPixel *destStream, const byte *singleColorTable, const byte *streamData, int streamSize, byte copyFromColor, int streamOffset, const WizRawPixel *conversionTable) {
	int value, runCount;
	byte color;

	while (streamSize > 0) {
		value = *streamData++;

		if (value & 0x01) { // xxxxxxx1
			// Tiny run handler
			runCount = WRLE_TINY_RUN_MIN_COUNT + ((value >> 1) & 0x03);
			streamSize -= runCount;

			color = *(singleColorTable + (value >> 3));

			if (copyFromColor != color) {
				memset8BppConversion(
					destStream, color, runCount, conversionTable);
			} else {
				memcpy(
					destStream, destStream + streamOffset,
					(runCount * sizeof(WizRawPixel)));
			}

			destStream += runCount;

		} else if (!(value & 0x02)) { // xxxxxx00
			// Transparent run
			runCount = value >> 2;
			if (runCount == 0) {
				runCount = 1 + (*streamData + 256 * *(streamData + 1));
				streamData += 2;
			}

			streamSize -= runCount;
			destStream += runCount;

		} else if (value & 0x04) { // xxxxx110 ?
			// Run of some color
			value >>= 3;
			if (value == 0) {
				runCount = 1 + *streamData++;
			} else {
				runCount = value;
			}

			streamSize -= runCount;

			color = *((byte *)streamData);

			if (color != copyFromColor) {
				memset8BppConversion(
					destStream, color,
					runCount, conversionTable);
			} else {
				memcpy(
					destStream, destStream + streamOffset,
					(runCount * sizeof(WizRawPixel)));
			}

			streamData += sizeof(byte);
			destStream += runCount;

		} else { // xxxxx010
			// Single color!
			color = *(singleColorTable + (value >> 3));

			if (color != copyFromColor) {
				*destStream++ = convert8BppToRawPixel(color, conversionTable);
			} else {
				*destStream++ = *(destStream + streamOffset);
			}

			streamSize--;
		}
	}
}

void Wiz::auxDecompSRLEStream(WizRawPixel *destStream, const WizRawPixel *backgroundStream, const byte *singleColorTable, const byte *streamData, int streamSize, const WizRawPixel *conversionTable) {
	int value, runCount;

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
			backgroundStream += runCount;
			destStream += runCount;

		} else if (!(value & 0x02)) { // xxxxxx00
			// Background run
			runCount = 1 + (value >> 2);
			streamSize -= runCount;
			memcpy(destStream, backgroundStream, runCount * sizeof(WizRawPixel));
			backgroundStream += runCount;
			destStream += runCount;

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

			backgroundStream += runCount;
			destStream += runCount;

		} else { // xxxxx010
			// Single color!

			*destStream++ = convert8BppToRawPixel(*(singleColorTable + (value >> 3)), conversionTable);
			backgroundStream++;
			streamSize--;
		}
	}
}

void Wiz::auxDecompDRLEImage(WizRawPixel *foregroundBufferPtr, WizRawPixel *backgroundBufferPtr, byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, const WizRawPixel *conversionTable) {
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
	if (!clipRectPtr) {
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

void Wiz::auxDecompDRLEPrim(WizRawPixel *foregroundBufferPtr, WizRawPixel *backgroundBufferPtr, int bufferWidth, Common::Rect *destRect, byte *compData,  Common::Rect *sourceRect, const WizRawPixel *conversionTable) {
	int decompWidth, decompHeight, counter, sX1, dX1, dX2, lineSize;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;
	dX2 = destRect->right;
	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	foregroundBufferPtr += bufferWidth * destRect->top + dX1;
	backgroundBufferPtr += bufferWidth * destRect->top + dX1;

	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible!!!!
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxDecompDRLEStream(
				foregroundBufferPtr, compData + 2, backgroundBufferPtr,
				sX1, decompWidth, conversionTable);

			compData += lineSize + 2;
			foregroundBufferPtr += bufferWidth;
			backgroundBufferPtr += bufferWidth;

		} else {
			// Handle a completely transparent line!
			compData += 2;
			foregroundBufferPtr += bufferWidth;
			backgroundBufferPtr += bufferWidth;
		}
	}
}

void Wiz::auxDecompTRLEImage(WizRawPixel *bufferPtr, byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, const WizRawPixel *conversionTable) {
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
	if (!clipRectPtr) {
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

void Wiz::auxDecompTRLEPrim(WizRawPixel *bufferPtr, int bufferWidth, Common::Rect *destRect, byte *compData, Common::Rect *sourceRect, const WizRawPixel *conversionTable) {
	int decompWidth, decompHeight, counter, sX1, dX1, dX2, lineSize;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;
	dX2 = destRect->right;
	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	bufferPtr += bufferWidth * destRect->top + dX1;
	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible!!!!
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxDecompTRLEStream(
				bufferPtr, compData + 2, sX1, decompWidth, conversionTable);

			compData += lineSize + 2;
			bufferPtr += bufferWidth;
		} else {
			// Handle a completely transparent line!
			compData += 2;
			bufferPtr += bufferWidth;
		}
	}
}


void Wiz::auxDrawZplaneFromTRLEImage(byte *zplanePtr, byte *compData, int zplanePixelWidth, int zplanePixelHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, int transOp, int solidOp) {
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
	if (!clipRectPtr) {
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

void Wiz::auxDrawZplaneFromTRLEPrim(byte *zplanePtr, int zplanePixelWidth, Common::Rect *destRect, byte *compData, Common::Rect *sourceRect, int transOp, int solidOp) {
	int decompWidth, decompHeight, counter, sX1, dX1, dX2, lineSize, mask, zplaneWidth;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;
	dX2 = destRect->right;
	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	zplaneWidth = zplanePixelWidth / 8;
	zplanePtr += zplaneWidth * destRect->top + (dX1 / 8);
	mask = 1 << (7 - (dX1 % 8));

	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible!!!!
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxZplaneFromTRLEStream(
				zplanePtr, compData + 2, sX1, decompWidth, mask, transOp, solidOp);

			compData += lineSize + 2;
			zplanePtr += zplaneWidth;

		} else {
			// Handle a completely transparent line!
			compData += 2;
			zplanePtr += zplaneWidth;
		}
	}
}

void Wiz::auxDecompRemappedTRLEImage(WizRawPixel *bufferPtr, byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, byte *remapTable, const WizRawPixel *conversionTable) {
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
	if (!clipRectPtr) {
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

void Wiz::auxDecompRemappedTRLEPrim(WizRawPixel *bufferPtr, int bufferWidth, Common::Rect *destRect, byte *compData, Common::Rect *sourceRect, byte *remapTable, const WizRawPixel *conversionTable) {
	int decompWidth, decompHeight, counter, sX1, dX1, dX2, lineSize;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;
	dX2 = destRect->right;
	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	bufferPtr += bufferWidth * destRect->top + dX1;
	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible!!!!
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxDecompRemappedTRLEStream(
				bufferPtr, compData + 2, sX1, decompWidth,
				remapTable, conversionTable);

			compData += lineSize + 2;
			bufferPtr += bufferWidth;
		} else {
			// Handle a completely transparent line!
			compData += 2;
			bufferPtr += bufferWidth;
		}
	}
}

bool Wiz::auxHitTestTRLEXPos(byte *dataStream, int skipAmount) {
	int runCount;

	// Decompress bytes to do simple clipping...
	while (skipAmount > 0) {

		if ((runCount = *dataStream++) & 1) {

			// Handle the transparent color...
			runCount >>= 1;
			if (runCount > skipAmount) {
				return false;
			} else {
				skipAmount -= runCount;
			}

		} else if (runCount & 2) {

			// Handle a run of color...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				return true;
			} else {
				skipAmount -= runCount;
				dataStream++;
			}

		} else {
			// Handle a literal run of pixels...
			runCount = (runCount >> 2) + 1;
			if (runCount > skipAmount) {
				return true;
			} else {
				skipAmount -= runCount;
				dataStream += runCount;
			}
		}
	}

	// Transparent or not?
	runCount = *dataStream++;

	if (runCount & 1) {
		return false;
	} else {
		return true;
	}
}

bool Wiz::auxHitTestTRLEImageRelPos(byte *compData, int x, int y, int width, int height) {
	int counter;

	// Quickly reject points outside the image boundry.
	if ((x < 0) || (width <= x) || (y < 0) || (height <= y)) {
		return false;
	}

	// Quickly skip down to the lines to be compressed & dest position...
	for (counter = 0; counter < y; counter++) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	if (READ_LE_UINT16(compData) != 0) {
		return auxHitTestTRLEXPos(compData + 2, x);
	} else {
		return false;
	}
}

bool Wiz::auxPixelHitTestTRLEXPos(byte *dataStream, int skipAmount, int transparentValue) {
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
	int counter;

	// Quickly reject points outside the image boundry.
	if ((x < 0) || (width <= x) || (y < 0) || (height <= y)) {
		return transparentValue;
	}

	// Quickly skip down to the lines to be compressed & dest position...
	for (counter = 0; counter < y; counter++) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	if (READ_LE_UINT16(compData) != 0) {
		return auxPixelHitTestTRLEXPos(compData + 2, x, transparentValue);
	} else {
		return transparentValue;
	}
}

void Wiz::auxDecompMixColorsTRLEImage(WizRawPixel *bufferPtr, byte *compData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, byte *coloMixTable, const WizRawPixel *conversionTable) {
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
	if (!clipRectPtr) {
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
		bufferPtr, bufferWidth, &destRect, compData, &sourceRect, coloMixTable,
		conversionTable);
}

void Wiz::auxDecompMixColorsTRLEPrim(WizRawPixel *bufferPtr, int bufferWidth, Common::Rect *destRect, byte *compData, Common::Rect *sourceRect, byte *coloMixTable, const WizRawPixel *conversionTable) {
	int decompWidth, decompHeight, counter, sX1, dX1, dX2, lineSize;

	// General setup...
	sX1 = sourceRect->left;
	dX1 = destRect->left;
	dX2 = destRect->right;
	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	bufferPtr += bufferWidth * destRect->top + dX1;
	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Decompress all the lines that are visible!!!!
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			auxColorMixDecompressLine(
				bufferPtr, compData + 2, sX1, decompWidth, coloMixTable,
				conversionTable);

			compData += lineSize + 2;
			bufferPtr += bufferWidth;

		} else {
			// Handle a completely transparent line!
			compData += 2;
			bufferPtr += bufferWidth;
		}
	}
}

void Wiz::auxDecompDRLEStream(WizRawPixel *destPtr, byte *dataStream, WizRawPixel *backgroundPtr, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
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
			destPtr += runCount;
			backgroundPtr += runCount;
			decompAmount -= runCount;

		} else if (runCount & 2) {
			/* xxxxxx10 */
			runCount = (runCount >> 2) + 1;

		WriteRunData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				memset8BppConversion(destPtr, *dataStream++, runCount, conversionTable);
				destPtr += runCount;
				backgroundPtr += runCount;
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
				memcpy(destPtr, backgroundPtr, runCount * sizeof(WizRawPixel));

				destPtr += runCount;
				backgroundPtr += runCount;
			} else {
				runCount += decompAmount;
				memcpy(destPtr, backgroundPtr, runCount * sizeof(WizRawPixel));

			}
		}
	}
}

void Wiz::auxDecompTRLEStream(WizRawPixel *destPtr, byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
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
			destPtr += runCount;
			decompAmount -= runCount;

		} else if (runCount & 2) {
			/* xxxxxx10 */
			runCount = (runCount >> 2) + 1;

		WriteRunData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				memset8BppConversion(destPtr, *dataStream++, runCount, conversionTable);
				destPtr += runCount;
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
				destPtr += runCount;
			} else {
				runCount += decompAmount;
				memcpy8BppConversion(destPtr, dataStream, runCount, conversionTable);
			}
		}
	}
}

void Wiz::auxDecompRemappedTRLEStream(WizRawPixel *destPtr, byte *dataStream, int skipAmount, int decompAmount, byte *remapTable, const WizRawPixel *conversionTable) {
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

			destPtr += runCount;
			decompAmount -= runCount;

		} else if (runCount & 2) {
			/* xxxxxx10 */
			runCount = (runCount >> 2) + 1;
		WriteRunData:
			decompAmount -= runCount;
			if (decompAmount >= 0) {
				memset8BppConversion(destPtr, *(remapTable + *dataStream++), runCount, conversionTable);
				destPtr += runCount;
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
				destPtr += runCount;
			} else {
				runCount += decompAmount;
				auxRemappedMemcpy(destPtr, dataStream, runCount, remapTable, conversionTable);
			}
		}
	}
}


void Wiz::auxZplaneFromTRLEStream(byte *destPtr, byte *dataStream, int skipAmount, int decompAmount, int mask, int transOp, int solidOp) {
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
			destPtr += runCount;
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
				*destPtr++ = *(remapTable + *destPtr);
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
				*destPtr++ = *(colorMixTable + ((*((const byte *)dataStream)) << 8) + *destPtr);
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

	// Decompress all the lines that are visible!!!!
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

void Wiz::auxRemappedMemcpy(WizRawPixel *dstPtr, byte *srcPtr, int count, byte *remapTable, const WizRawPixel *conversionTable) {
	do {
		*dstPtr++ = convert8BppToRawPixel(*(remapTable + *srcPtr++), conversionTable);
	} while (--count > 0);
}

} // End of namespace Scumm

#endif // ENABLE_HE
