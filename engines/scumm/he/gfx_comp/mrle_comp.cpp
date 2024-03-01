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

static void MRLEFLIP_HorzFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect) {
	int dx = (baseRect->right - rectToAlign->right) - (rectToAlign->left - baseRect->left);
	rectToAlign->left += dx;
	rectToAlign->right += dx;
}

static void MRLEFLIP_VertFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect) {
	int dy = (baseRect->bottom - rectToAlign->bottom) - (rectToAlign->top - baseRect->top);
	rectToAlign->top += dy;
	rectToAlign->bottom += dy;
}

#define HANDLE_SKIP_PIXELS_MACRO() {                    \
		/* Decompress bytes to do simple clipping... */ \
		while (skipAmount > 0) {                        \
			if ((runCount = *dataStream++) & 1) {       \
                                                        \
				/* Handle the transparent color... */   \
				runCount >>= 1;                         \
				if (runCount > skipAmount) {            \
					runCount -= skipAmount;             \
					goto DoTransparentRun;              \
				} else {                                \
					skipAmount -= runCount;             \
				}                                       \
                                                        \
			} else { /* xxxxxxx0 */                     \
				/* Handle a run of color... */          \
				runCount >>= 1;                         \
				if (runCount > skipAmount) {            \
					runCount -= skipAmount;             \
					goto WriteRunData;                  \
				} else {                                \
					skipAmount -= runCount;             \
				}                                       \
			}                                           \
		}                                               \
	}

#define HANDLE_RUN_DECOMPRESS_MACRO(_TransparentCode_, _RunCode_) { \
		while (decompAmount > 0) {                                  \
			runCount = *dataStream++;                               \
                                                                    \
			if (runCount & 1) { /* xxxxxxx1 */                      \
				runCount >>= 1;                                     \
			DoTransparentRun:                                       \
				decompAmount -= runCount;                           \
				_TransparentCode_                                   \
                                                                    \
			} else { /* xxxxxxx0 */                                 \
                                                                    \
				runCount >>= 1;                                     \
			WriteRunData:                                           \
				decompAmount -= runCount;                           \
				if (decompAmount < 0) {                             \
					runCount += decompAmount;                       \
				}                                                   \
				_RunCode_                                           \
			}                                                       \
		}                                                           \
	}

static void MRLEFLIP_AltSource_F_XBppToXBpp(Wiz *wiz,
	WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
	int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr;
	int runCount;

	srcPtr = (const WizRawPixel *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	HANDLE_SKIP_PIXELS_MACRO();

	// Really decompress to the dest buffer...
	HANDLE_RUN_DECOMPRESS_MACRO(
		{
			destPtr += runCount;
			srcPtr += runCount;
		},
		{
			memcpy(destPtr, srcPtr, (runCount * sizeof(WizRawPixel)));
			destPtr += runCount;
			srcPtr += runCount;
		}
	);
}

static void MRLEFLIP_AltSource_B_XBppToXBpp(Wiz *wiz,
	WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
	int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr;
	int runCount;

	srcPtr = (const WizRawPixel *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	HANDLE_SKIP_PIXELS_MACRO();

	// Really decompress to the dest buffer...
	HANDLE_RUN_DECOMPRESS_MACRO(
		{
			destPtr -= runCount;
			srcPtr -= runCount;
		},
		{
			destPtr -= runCount;
			srcPtr -= runCount;
			memcpy(destPtr + 1, srcPtr + 1, (runCount * sizeof(WizRawPixel)));
		}
	);
}

static void MRLEFLIP_AltSource_F_8BppToXBpp(Wiz *wiz,
	WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
	int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const byte *srcPtr;
	int runCount;

	srcPtr = (const byte *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	HANDLE_SKIP_PIXELS_MACRO();

	// Really decompress to the dest buffer...
	HANDLE_RUN_DECOMPRESS_MACRO(
		{
			destPtr += runCount;
			srcPtr += runCount;
		},
		{
			wiz->memcpy8BppConversion(destPtr, srcPtr, runCount, conversionTable);
			destPtr += runCount;
			srcPtr += runCount;
		});
}

static void MRLEFLIP_AltSource_B_8BppToXBpp(Wiz *wiz,
	WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
	int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr;
	int runCount;

	srcPtr = (const WizRawPixel *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	HANDLE_SKIP_PIXELS_MACRO();

	// Really decompress to the dest buffer...
	HANDLE_RUN_DECOMPRESS_MACRO(
		{
			destPtr -= runCount;
			srcPtr -= runCount;
		},
		{
			destPtr -= runCount;
			srcPtr -= runCount;
			wiz->memcpy8BppConversion(destPtr + 1, srcPtr + 1, runCount, conversionTable);
		});
}

static void MRLEFLIP_AltSource_DecompImageHull(Wiz *wiz,
	WizRawPixel *bufferPtr, int bufferWidth, const Common::Rect *destRect,
	const byte *altSourceBuffer, int altBytesPerLine,
	int altBytesPerPixel, const Common::Rect *altRect,
	const byte *compData, const Common::Rect *sourceRect,
	const WizRawPixel *conversionTable,
	void (*functionPtr)(Wiz *wiz,
		WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
		int skipAmount, int decompAmount, const WizRawPixel *conversionTable)) {
	int decompWidth, decompHeight, counter, sX1, lineSize;

	// Yet more general setup...
	sX1 = sourceRect->left;

	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	bufferPtr += bufferWidth * destRect->top + destRect->left;

	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16((byte *)compData) + 2;
	}

	// Calc the ALT buffer location
	altSourceBuffer += (altBytesPerLine * altRect->top) + (altRect->left * altBytesPerPixel);

	// Flip the dest offset if vertical flipping...
	if (destRect->top > destRect->bottom) {
		bufferWidth = -bufferWidth;
		altBytesPerLine = -altBytesPerLine;
	}

	// Decompress all the lines that are visible!!!!
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16((byte *)compData);

		if (lineSize != 0) {
			(*functionPtr)(wiz,
				bufferPtr, altSourceBuffer, compData + 2, sX1,
				decompWidth, conversionTable);

			compData += lineSize + 2;
			bufferPtr += bufferWidth;
			altSourceBuffer += altBytesPerLine;

		} else {
			// Handle a completely transparent line!
			compData += 2;
			bufferPtr += bufferWidth;
			altSourceBuffer += altBytesPerLine;
		}
	}
}

void Wiz::MRLEFLIP_AltSource_DecompressPrim(
	WizRawPixel *destBufferPtr, int destBufferWidth, int destBufferHeight,
	const void *altBufferPtr, int altBitsPerPixel,
	const WizCompressedImage *imagePtr, int destX, int destY,
	const Common::Rect *sourceCoords, const Common::Rect *clipRectPtr,
	int32 flags, const WizRawPixel *conversionTable,
	void (*forewordFunctionPtr)(Wiz *wiz,
		WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
		int skipAmount, int decompAmount, const WizRawPixel *conversionTable),
	void (*backwardFunctionPtr)(Wiz *wiz,
		WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
		int skipAmount, int decompAmount, const WizRawPixel *conversionTable)) {

	Common::Rect sourceRect, destRect, clipRect, workRect, inSourceRect;
	int width, height;

	void (*functionPtr)(Wiz * wiz,
		WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
		int skipAmount, int decompAmount, const WizRawPixel *conversionTable);

	if (!sourceCoords) {
		width = imagePtr->width;
		height = imagePtr->height;

		sourceRect.left = 0;
		sourceRect.top = 0;
		sourceRect.right = width - 1;
		sourceRect.bottom = height - 1;
	} else {
		width = sourceCoords->right - sourceCoords->left + 1;
		height = sourceCoords->bottom - sourceCoords->top + 1;
		sourceRect = *sourceCoords;
	}

	inSourceRect = sourceRect;

	destRect.left = destX;
	destRect.top = destY;
	destRect.right = destX + width - 1;
	destRect.bottom = destY + height - 1;

	// Custom clip rect...
	if (!clipRectPtr) {
		clipRect = *clipRectPtr;

		workRect.left = 0;
		workRect.top = 0;
		workRect.right = destBufferWidth - 1;
		workRect.bottom = destBufferHeight - 1;

		if (!findRectOverlap(&clipRect, &workRect)) {
			return;
		}
	} else {
		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = destBufferWidth - 1;
		clipRect.bottom = destBufferHeight - 1;
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

	// Handle the flip coords source adjustment...
	if (flags & kWRFHFlip) {
		functionPtr = backwardFunctionPtr;
		MRLEFLIP_HorzFlipAlignWithRect(&sourceRect, &inSourceRect);
		SWAP<int16>(destRect.left, destRect.right);
	} else {
		functionPtr = forewordFunctionPtr;
	}

	if (flags & kWRFVFlip) {
		MRLEFLIP_VertFlipAlignWithRect(&sourceRect, &inSourceRect);
		SWAP<int16>(destRect.top, destRect.bottom);
	}

	// Call the primitive image renderer...
	MRLEFLIP_AltSource_DecompImageHull(this,
		destBufferPtr, destBufferWidth, &destRect,
		(const byte *)altBufferPtr, ((destBufferWidth * altBitsPerPixel) / 8),
		(altBitsPerPixel / 8), &destRect, imagePtr->data, &sourceRect,
		conversionTable, functionPtr);
}

void Wiz::MRLEFLIP_AltSource_DecompressImage(
	WizRawPixel *destBufferPtr, const byte *compData, int destBufferWidth, int destBufferHeight,
	const void *altBufferPtr, int altWidth, int altHeight, int altBitsPerPixel,
	int x, int y, int width, int height, Common::Rect *clipRectPtr,
	int32 wizFlags, const WizRawPixel *conversionTable) {

	Common::Rect srcRect, clipRect;
	WizCompressedImage fakeImage;

	// General setup
	fakeImage.data = (byte *)compData;
	fakeImage.width = width;
	fakeImage.height = height;

	makeSizedRect(&srcRect, width, height);
	makeSizedRect(&clipRect, destBufferWidth, destBufferHeight);

	if (clipRectPtr) {
		if (!findRectOverlap(&clipRect, clipRectPtr)) {
			return;
		}
	}

	// Check the assumptions about the alt src buffer matching the dest buffer
	if ((altWidth != destBufferWidth) || (altHeight != destBufferHeight)) {
		return;
	}

	// Dispatch to the next level of code. (8bpp or conversion)
	if (altBitsPerPixel == 16) {
		MRLEFLIP_AltSource_DecompressPrim(
			destBufferPtr, destBufferWidth, destBufferHeight,
			altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
			&srcRect, &clipRect, wizFlags, conversionTable,
			MRLEFLIP_AltSource_F_XBppToXBpp,
			MRLEFLIP_AltSource_B_XBppToXBpp);
	} else if (altBitsPerPixel == 8) {
		MRLEFLIP_AltSource_DecompressPrim(
			destBufferPtr, destBufferWidth, destBufferHeight,
			altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
			&srcRect, &clipRect, wizFlags, conversionTable,
			MRLEFLIP_AltSource_F_8BppToXBpp,
			MRLEFLIP_AltSource_B_8BppToXBpp);
	}
}

} // End of namespace Scumm

#endif // ENABLE_HE
