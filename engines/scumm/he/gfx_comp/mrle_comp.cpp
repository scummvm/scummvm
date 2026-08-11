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

static void mrleFLIPHorzFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect) {
	int dx = (baseRect->right - rectToAlign->right) - (rectToAlign->left - baseRect->left);
	rectToAlign->left += dx;
	rectToAlign->right += dx;
}

static void mrleFLIPVertFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect) {
	int dy = (baseRect->bottom - rectToAlign->bottom) - (rectToAlign->top - baseRect->top);
	rectToAlign->top += dy;
	rectToAlign->bottom += dy;
}

#define MRLE_HANDLE_SKIP_PIXELS_STEP() {                \
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

#define MRLE_HANDLE_RUN_DECOMPRESS_STEP(_TransparentCode_, _RunCode_) {  \
		while (decompAmount > 0) {                                       \
			runCount = *dataStream++;                                    \
                                                                         \
			if (runCount & 1) { /* xxxxxxx1 */                           \
				runCount >>= 1;                                          \
			DoTransparentRun:                                            \
				decompAmount -= runCount;                                \
				_TransparentCode_                                        \
                                                                         \
			} else { /* xxxxxxx0 */                                      \
                                                                         \
				runCount >>= 1;                                          \
			WriteRunData:                                                \
				decompAmount -= runCount;                                \
				if (decompAmount < 0) {                                  \
					runCount += decompAmount;                            \
				}                                                        \
				_RunCode_                                                \
			}                                                            \
		}                                                                \
	}

static void mrleFLIPAltSourceForwardXBppToXBpp(Wiz *wiz,
	WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
	int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr = (const WizRawPixel *)altSourcePtr;

	const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;
	const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	int runCount;

	// Decompress bytes to do simple clipping...
	MRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	MRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (!wiz->_uses16BitColor) {
				dest8 += runCount;
				src8 += runCount;
			} else {
				dest16 += runCount;
				src16 += runCount;
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				memcpy(dest8, src8, (runCount * sizeof(WizRawPixel8)));
				dest8 += runCount;
				src8 += runCount;
			} else {
				// memcpy(dest16, src16, (runCount * sizeof(WizRawPixel16)));
				for (int i = 0; i < runCount; i++) {
					dest16[i] = FROM_LE_16(src16[i]);
				}

				dest16 += runCount;
				src16 += runCount;
			}
		}
	);
}

static void mrleFLIPAltSourceBackwardXBppToXBpp(Wiz *wiz,
	WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
	int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr = (const WizRawPixel *)altSourcePtr;

	const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;
	const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	int runCount;

	// Decompress bytes to do simple clipping...
	MRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	MRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (!wiz->_uses16BitColor) {
				dest8 -= runCount;
				src8 -= runCount;
			} else {
				dest16 -= runCount;
				src16 -= runCount;
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				dest8 -= runCount;
				src8 -= runCount;
				memcpy(dest8 + 1, src8 + 1, (runCount * sizeof(WizRawPixel8)));
			} else {
				dest16 -= runCount;
				src16 -= runCount;
				// memcpy(dest16 + 1, src16 + 1, (runCount * sizeof(WizRawPixel16)));
				for (int i = 1; i < runCount; i++) {
					dest16[i] = FROM_LE_16(src16[i]);
				}
			}
		}
	);
}

static void mrleFLIPAltSourceForward8BppToXBpp(Wiz *wiz,
	WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
	int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const byte *srcPtr = (const byte *)altSourcePtr;

	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	int runCount;

	// Decompress bytes to do simple clipping...
	MRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	MRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (!wiz->_uses16BitColor) {
				dest8 += runCount;
				srcPtr += runCount;
				destPtr = (WizRawPixel *)dest8;
			} else {
				dest16 += runCount;
				srcPtr += runCount;
				destPtr = (WizRawPixel *)dest16;
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				wiz->memcpy8BppConversion(destPtr, srcPtr, runCount, conversionTable);
				dest8 += runCount;
				srcPtr += runCount;
				destPtr = (WizRawPixel *)dest8;
			} else {
				wiz->memcpy8BppConversion(destPtr, srcPtr, runCount, conversionTable);
				dest16 += runCount;
				srcPtr += runCount;
				destPtr = (WizRawPixel *)dest16;
			}
		}
	);
}

static void mrleFLIPAltSourceBackward8BppToXBpp(Wiz *wiz,
	WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
	int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {

	// NOTE: This looks like it should be a const byte pointer, but the original
	// says it's a WizRawPixel pointer; I'm going to follow the original for now...
	const WizRawPixel *srcPtr = (const WizRawPixel *)altSourcePtr;

	const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;
	const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	int runCount;

	// Decompress bytes to do simple clipping...
	MRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	MRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (!wiz->_uses16BitColor) {
				dest8 -= runCount;
				src8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
				srcPtr = (const WizRawPixel *)src8;
			} else {
				dest16 -= runCount;
				src16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
				srcPtr = (const WizRawPixel *)src16;
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				wiz->memcpy8BppConversion(destPtr + 1, srcPtr + 1, runCount, conversionTable);
				dest8 -= runCount;
				src8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
				srcPtr = (const WizRawPixel *)src8;
			} else {
				wiz->memcpy8BppConversion(destPtr + 1, srcPtr + 1, runCount, conversionTable);
				dest16 -= runCount;
				src16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
				srcPtr = (const WizRawPixel *)src16;
			}
		}
	);
}

static void mrleFLIPAltSourceDecompImageHull(Wiz *wiz,
	WizRawPixel *bufferPtr, int bufferWidth, const Common::Rect *destRect,
	const byte *altSourceBuffer, int altBytesPerLine,
	int altBytesPerPixel, const Common::Rect *altRect,
	const byte *compData, const Common::Rect *sourceRect,
	const WizRawPixel *conversionTable,
	void (*functionPtr)(Wiz *wiz,
		WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
		int skipAmount, int decompAmount, const WizRawPixel *conversionTable)) {

	int decompWidth, decompHeight, counter, sX1, lineSize;
	WizRawPixel8 *buffer8 = (WizRawPixel8 *)bufferPtr;
	WizRawPixel16 *buffer16 = (WizRawPixel16 *)bufferPtr;

	// Yet more general setup...
	sX1 = sourceRect->left;

	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	if (!wiz->_uses16BitColor) {
		buffer8 += bufferWidth * destRect->top + destRect->left;
		bufferPtr = (WizRawPixel *)buffer8;
	} else {
		buffer16 += bufferWidth * destRect->top + destRect->left;
		bufferPtr = (WizRawPixel *)buffer16;
	}

	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Calc the ALT buffer location...
	altSourceBuffer += (altBytesPerLine * altRect->top) + (altRect->left * altBytesPerPixel);

	// Flip the dest offset if vertical flipping...
	if (destRect->top > destRect->bottom) {
		bufferWidth = -bufferWidth;
		altBytesPerLine = -altBytesPerLine;
	}

	// Decompress all the lines that are visible...
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			(*functionPtr)(wiz,
				bufferPtr, altSourceBuffer, compData + 2, sX1,
				decompWidth, conversionTable);

			compData += lineSize + 2;
		} else {
			// Handle a completely transparent line!
			compData += 2;
		}

		if (!wiz->_uses16BitColor) {
			buffer8 += bufferWidth;
			bufferPtr = (WizRawPixel *)buffer8;
		} else {
			buffer16 += bufferWidth;
			bufferPtr = (WizRawPixel *)buffer16;
		}

		altSourceBuffer += altBytesPerLine;
	}
}

void Wiz::mrleFLIPAltSourceDecompressPrim(
	WizRawPixel *destBufferPtr, int destBufferWidth, int destBufferHeight,
	const void *altBufferPtr, int altBitsPerPixel,
	const WizCompressedImage *imagePtr, int destX, int destY,
	const Common::Rect *sourceCoords, const Common::Rect *clipRectPtr,
	int32 flags, const WizRawPixel *conversionTable,
	void (*forwardFunctionPtr)(Wiz *wiz,
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
	if (clipRectPtr) {
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
		mrleFLIPHorzFlipAlignWithRect(&sourceRect, &inSourceRect);
		SWAP<int16>(destRect.left, destRect.right);
	} else {
		functionPtr = forwardFunctionPtr;
	}

	if (flags & kWRFVFlip) {
		mrleFLIPVertFlipAlignWithRect(&sourceRect, &inSourceRect);
		SWAP<int16>(destRect.top, destRect.bottom);
	}

	// Call the primitive image renderer...
	mrleFLIPAltSourceDecompImageHull(this,
		destBufferPtr, destBufferWidth, &destRect,
		(const byte *)altBufferPtr, ((destBufferWidth * altBitsPerPixel) / 8),
		(altBitsPerPixel / 8), &destRect, imagePtr->data, &sourceRect,
		conversionTable, functionPtr);
}

void Wiz::mrleFLIPAltSourceDecompressImage(
	WizRawPixel *destBufferPtr, const byte *compData, int destBufferWidth, int destBufferHeight,
	const void *altBufferPtr, int altWidth, int altHeight, int altBitsPerPixel,
	int x, int y, int width, int height, Common::Rect *clipRectPtr,
	int32 wizFlags, const WizRawPixel *conversionTable) {

	Common::Rect srcRect, clipRect;
	WizCompressedImage fakeImage;

	// General setup
	fakeImage.data = compData;
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
		mrleFLIPAltSourceDecompressPrim(
			destBufferPtr, destBufferWidth, destBufferHeight,
			altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
			&srcRect, &clipRect, wizFlags, conversionTable,
			mrleFLIPAltSourceForwardXBppToXBpp,
			mrleFLIPAltSourceBackwardXBppToXBpp);
	} else if (altBitsPerPixel == 8) {
		mrleFLIPAltSourceDecompressPrim(
			destBufferPtr, destBufferWidth, destBufferHeight,
			altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
			&srcRect, &clipRect, wizFlags, conversionTable,
			mrleFLIPAltSourceForward8BppToXBpp,
			mrleFLIPAltSourceBackward8BppToXBpp);
	}
}

} // End of namespace Scumm

#endif // ENABLE_HE
