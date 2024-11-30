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

#define TRLEFLIP_MEMSET(_dPt, _Pv, _rC) \
	wiz->rawPixelMemset((_dPt), wiz->convert8BppToRawPixel(_Pv, conversionTable), (_rC))

#define TRLE_HANDLE_SKIP_PIXELS_STEP() {                       \
		/* Decompress bytes to do simple clipping... */        \
		while (skipAmount > 0) {                               \
			if ((runCount = *dataStream++) & 1) {              \
				/* Handle the transparent color... */          \
				runCount >>= 1;                                \
				if (runCount > skipAmount) {                   \
					runCount -= skipAmount;                    \
					goto DoTransparentRun;                     \
				} else {                                       \
					skipAmount -= runCount;                    \
				}                                              \
			} else if (runCount & 2) {                         \
				/* Handle a run of color... */                 \
				runCount = (runCount >> 2) + 1;                \
				if (runCount > skipAmount) {                   \
					runCount -= skipAmount;                    \
					goto WriteRunData;                         \
				} else {                                       \
					skipAmount -= runCount;                    \
					dataStream += sizeof(byte);                \
				}                                              \
			} else {                                           \
				/* Handle a literal run of pixels... */        \
				runCount = (runCount >> 2) + 1;                \
				if (runCount > skipAmount) {                   \
					runCount -= skipAmount;                    \
					dataStream += (skipAmount * sizeof(byte)); \
					goto WriteLiteralData;                     \
				} else {                                       \
					skipAmount -= runCount;                    \
					dataStream += (runCount * sizeof(byte));   \
				}                                              \
			}                                                  \
		}                                                      \
	}

#define TRLE_HANDLE_RUN_DECOMPRESS_STEP(_TransparentCode_, _RunCode_, _LiteralCode_) { \
		while (decompAmount > 0) {                                                     \
			runCount = *dataStream++;                                                  \
                                                                                       \
			if (runCount & 1) { /* xxxxxxx1 */                                         \
				runCount >>= 1;                                                        \
			DoTransparentRun:                                                          \
				decompAmount -= runCount;                                              \
				_TransparentCode_                                                      \
			} else if (runCount & 2) { /* xxxxxx10 */                                  \
				runCount = (runCount >> 2) + 1;                                        \
			WriteRunData:                                                              \
				decompAmount -= runCount;                                              \
				if (decompAmount < 0) {                                                \
					runCount += decompAmount;                                          \
				}                                                                      \
				_RunCode_                                                              \
					dataStream++;                                                      \
			} else { /* xxxxxx00 */                                                    \
				runCount = (runCount >> 2) + 1;                                        \
			WriteLiteralData:                                                          \
				decompAmount -= runCount;                                              \
				if (decompAmount < 0) {                                                \
					runCount += decompAmount;                                          \
				}                                                                      \
				_LiteralCode_                                                          \
					dataStream += runCount;                                            \
			}                                                                          \
		}                                                                              \
	}

#define TRLEFLIP_X_FORWARD_PASS(MEMSET_PARAM, MEMCPY_PARAM) {                                     \
		int runCount;                                                                             \
		TRLE_HANDLE_SKIP_PIXELS_STEP();                                                           \
		TRLE_HANDLE_RUN_DECOMPRESS_STEP(                                                          \
			{ destPtr += runCount; },                                                             \
			{                                                                                     \
				(MEMSET_PARAM)(                                                                   \
					destPtr, wiz->convert8BppToRawPixel(*dataStream, conversionTable), runCount); \
				destPtr += runCount;                                                              \
			},                                                                                    \
			{                                                                                     \
				(MEMCPY_PARAM)(destPtr, dataStream, runCount, conversionTable);                   \
				destPtr += runCount;                                                              \
			});                                                                                   \
	}

#define TRLEFLIP_X_BACKWARD_PASS(MEMSET_PARAM, MEMCPY_PARAM) {                                        \
		int runCount;                                                                                 \
		TRLE_HANDLE_SKIP_PIXELS_STEP();                                                               \
		TRLE_HANDLE_RUN_DECOMPRESS_STEP(                                                              \
			{ destPtr -= runCount; },                                                                 \
			{                                                                                         \
				destPtr -= runCount;                                                                  \
				(MEMSET_PARAM)(                                                                       \
					destPtr + 1, wiz->convert8BppToRawPixel(*dataStream, conversionTable), runCount); \
			},                                                                                        \
			{                                                                                         \
				(MEMCPY_PARAM)(destPtr, dataStream, runCount, conversionTable);                       \
				destPtr -= runCount;                                                                  \
			});                                                                                       \
	}

#define TRLEFLIP_X_ARBITRARY_DST_STEP(COLOROPERATION) {                                                  \
		int runCount;                                                                                    \
		TRLE_HANDLE_SKIP_PIXELS_STEP();                                                                  \
		TRLE_HANDLE_RUN_DECOMPRESS_STEP(                                                                 \
			{ destPtr += (destStepValue * runCount); },                                                  \
			{                                                                                            \
				WizRawPixel adjustedRunColor = wiz->convert8BppToRawPixel(*dataStream, conversionTable); \
				for (int counter = 0; counter < runCount; counter++) {                                   \
					WizRawPixel workColor = *destPtr;                                                    \
					*destPtr = (WizRawPixel)COLOROPERATION(workColor, adjustedRunColor);                 \
					destPtr += destStepValue;                                                            \
				}                                                                                        \
			},                                                                                           \
			{                                                                                            \
				const byte *src = (const byte *)dataStream;                                              \
				for (int counter = 0; counter < runCount; counter++) {                                   \
					WizRawPixel adjustedRunColor = wiz->convert8BppToRawPixel(*src++, conversionTable);  \
					WizRawPixel workColor = *destPtr;                                                    \
					*destPtr = (WizRawPixel)COLOROPERATION(workColor, adjustedRunColor);                 \
					destPtr += destStepValue;                                                            \
				}                                                                                        \
			});                                                                                          \
	}

#define TRLEFLIP_ALTX_FORWARD_PASS(X_MEMSET_PARAM, X_MEMCPY_PARAM) {                                      \
		const WizRawPixel *srcPtr = (const WizRawPixel *)altSourcePtr;                                    \
		int runCount;                                                                                     \
		TRLE_HANDLE_SKIP_PIXELS_STEP();                                                                   \
		TRLE_HANDLE_RUN_DECOMPRESS_STEP(                                                                  \
			{ destPtr += runCount; srcPtr += runCount; },                                                 \
			{                                                                                             \
				X_MEMSET_PARAM(wiz,                                                                       \
					destPtr, srcPtr, wiz->convert8BppToRawPixel(*dataStream, conversionTable), runCount); \
				destPtr += runCount;                                                                      \
				srcPtr += runCount;                                                                       \
			},                                                                                            \
			{                                                                                             \
				X_MEMCPY_PARAM(wiz, destPtr, srcPtr, dataStream, runCount, conversionTable);              \
				destPtr += runCount;                                                                      \
				srcPtr += runCount;                                                                       \
			});                                                                                           \
	}

#define TRLEFLIP_ALTX_BACKWARD_PASS(X_MEMSET_PARAM, X_MEMCPY_PARAM) {                                             \
		const WizRawPixel *srcPtr = (const WizRawPixel *)altSourcePtr;                                            \
		int runCount;                                                                                             \
		TRLE_HANDLE_SKIP_PIXELS_STEP();                                                                           \
		TRLE_HANDLE_RUN_DECOMPRESS_STEP(                                                                          \
			{destPtr -= runCount; srcPtr -= runCount; },                                                          \
			{                                                                                                     \
				destPtr -= runCount;                                                                              \
				srcPtr -= runCount;                                                                               \
				X_MEMSET_PARAM(wiz,                                                                               \
					destPtr + 1, srcPtr + 1, wiz->convert8BppToRawPixel(*dataStream, conversionTable), runCount); \
			},                                                                                                    \
			{                                                                                                     \
				X_MEMCPY_PARAM(wiz, destPtr, srcPtr, dataStream, runCount, conversionTable);                      \
				destPtr -= runCount;                                                                              \
				srcPtr -= runCount;                                                                               \
			});                                                                                                   \
	}

#define TRLE_DUMP   0
#define TRLE_RUN    1
#define TRLE_MINRUN 3
#define TRLE_MAXRUN 128
#define TRLE_MAXDAT 128

#define trle_putbyte(c) {   \
		if (dest) {         \
			*dest++ = (c);  \
		}                   \
		++_trlePutSize;     \
	}

void Wiz::trleFLIPCheckAlphaSetup() {
	if (_initializeAlphaTable && _uses16BitColor) {
		_initializeAlphaTable = false;

		// Create the int alpha to float table...
		for (int i = 1; i < 255; i++) {
			_alphaTable[i] = (float)i / 255.0f;
		}

		// Force 0 & 255 values just in case the float accuracy is bad...
		_alphaTable[0] = 0.0f;
		_alphaTable[255] = 1.0f;

		// Quantized pre-computed alpha results...
		int alpha_index = 0;

		for (int alpha = 0; alpha < 256; alpha += WIZ_QUANTIZED_ALPHA_DIV, alpha_index++) {
			float fa = (float)(alpha + WIZ_QUANTIZED_ALPHA_DIV - 1) / 255.0f;
			float ia = 1.0f - fa;

			for (int colorA = 0; colorA < WIZ_COLOR16_COMPONENT_COUNT; colorA++) {
				float ca = (float)colorA * fa;

				for (int colorB = 0; colorB < WIZ_COLOR16_COMPONENT_COUNT; colorB++) {
					int value = (int)(ca + ((float)colorB * ia));
					_precomputed16bppTable[alpha_index][colorA][colorB] = value;
				}
			}
		}
	}
}

WizRawPixel Wiz::trleFLIPAlphaMixPrim(WizRawPixel b, WizRawPixel a, int alpha) {
	int quantized = (alpha / WIZ_QUANTIZED_ALPHA_DIV);

	return (
		((_precomputed16bppTable[quantized]
								 [((a & WIZRAWPIXEL_R_MASK) >> WIZRAWPIXEL_R_SHIFT)]
								 [((b & WIZRAWPIXEL_R_MASK) >> WIZRAWPIXEL_R_SHIFT)])
		 << WIZRAWPIXEL_R_SHIFT) |
		((_precomputed16bppTable[quantized]
								 [((a & WIZRAWPIXEL_G_MASK) >> WIZRAWPIXEL_G_SHIFT)]
								 [((b & WIZRAWPIXEL_G_MASK) >> WIZRAWPIXEL_G_SHIFT)])
		 << WIZRAWPIXEL_G_SHIFT) |
		((_precomputed16bppTable[quantized]
								 [((a & WIZRAWPIXEL_B_MASK) >> WIZRAWPIXEL_B_SHIFT)]
								 [((b & WIZRAWPIXEL_B_MASK) >> WIZRAWPIXEL_B_SHIFT)])
		 << WIZRAWPIXEL_B_SHIFT));
}

void trleFLIPHorzFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect) {
	int dx = (baseRect->right - rectToAlign->right) - (rectToAlign->left - baseRect->left);
	rectToAlign->left += dx;
	rectToAlign->right += dx;
}

void trleFLIPVertFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect) {
	int dy = (baseRect->bottom - rectToAlign->bottom) - (rectToAlign->top - baseRect->top);
	rectToAlign->top += dy;
	rectToAlign->bottom += dy;
}

void Wiz::trleFLIPDecompImageHull(
	WizRawPixel *bufferPtr, int bufferWidth, const Common::Rect *destRect,
	const byte *compData, const Common::Rect *sourceRect, const byte *extraPtr,
	const WizRawPixel *conversionTable,
	void (*functionPtr)(Wiz *wiz,
		WizRawPixel *destPtr, const byte *dataStream, int skipAmount,
		int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable)) {
	int decompWidth, decompHeight, counter, sX1, lineSize;

	// General setup...
	sX1 = sourceRect->left;
	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	WizRawPixel8 *buf8 = (WizRawPixel8 *)bufferPtr;
	WizRawPixel16 *buf16 = (WizRawPixel16 *)bufferPtr;

	if (_uses16BitColor) {
		buf16 += bufferWidth * destRect->top + destRect->left;
		bufferPtr = (WizRawPixel *)buf16;
	} else {
		buf8 += bufferWidth * destRect->top + destRect->left;
		bufferPtr = (WizRawPixel *)buf8;
	}

	for (counter = sourceRect->top; counter > 0; counter--) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Flip the dest offset if vertical flipping...
	if (destRect->top > destRect->bottom) {
		bufferWidth = -bufferWidth;
	}

	// Decompress all the lines that are visible...
	while (decompHeight-- > 0) {
		lineSize = READ_LE_UINT16(compData);

		if (lineSize != 0) {
			(*functionPtr)(this, bufferPtr, compData + 2, sX1, decompWidth, extraPtr, conversionTable);
			compData += lineSize + 2;
		} else {
			// Handle a completely transparent line!
			compData += 2;
		}

		if (_uses16BitColor) {
			buf16 = (WizRawPixel16 *)bufferPtr;
			buf16 += bufferWidth;
			bufferPtr = (WizRawPixel *)buf16;
		} else {
			buf8 = (WizRawPixel8 *)bufferPtr;
			buf8 += bufferWidth;
			bufferPtr = (WizRawPixel *)buf8;
		}
	}
}

void Wiz::trleFLIPDecompressPrim(
	WizSimpleBitmap *bitmapPtr, const WizCompressedImage *imagePtr, int destX, int destY,
	const Common::Rect *sourceCoords, const Common::Rect *clipRectPtr, const byte *extraPtr,
	int32 flags, const WizRawPixel *conversionTable,
	void (*forwardFunctionPtr)(Wiz *wiz,
		WizRawPixel *destPtr, const byte *dataStream, int skipAmount,
		int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable),
	void (*backwardFunctionPtr)(Wiz *wiz,
		WizRawPixel *destPtr, const byte *dataStream, int skipAmount,
		int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable)) {

	Common::Rect sourceRect, destRect, clipRect, workRect, inSourceRect;
	int width, height, bufferWidth, bufferHeight;

	void (*functionPtr)(Wiz *wiz,
		WizRawPixel * destPtr, const byte *dataStream, int skipAmount,
		int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable);

	bufferWidth = bitmapPtr->bitmapWidth;
	bufferHeight = bitmapPtr->bitmapHeight;

	if (!sourceCoords) {
		width = imagePtr->width;
		height = imagePtr->height;

		makeSizedRect(&sourceRect, width, height);
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

	// Handle the flip coords source adjustment...
	if (flags & kWRFHFlip) {
		functionPtr = backwardFunctionPtr;
		trleFLIPHorzFlipAlignWithRect(&sourceRect, &inSourceRect);
		swapRectX(&destRect);
	} else {
		functionPtr = forwardFunctionPtr;
	}

	if (flags & kWRFVFlip) {
		trleFLIPVertFlipAlignWithRect(&sourceRect, &inSourceRect);
		swapRectY(&destRect);
	}

	// Call the primitive image renderer...
	trleFLIPDecompImageHull(
		bitmapPtr->bufferPtr(), bufferWidth, &destRect, imagePtr->data,
		&sourceRect, extraPtr, conversionTable, functionPtr);
}

void Wiz::trleFLIPFiftyFiftyMixPixelMemset(WizRawPixel *dstPtr, WizRawPixel mixColor, int size) {
	WizRawPixel adjustedColor = WIZRAWPIXEL_50_50_PREMIX_COLOR(mixColor);
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			*dst8 = WIZRAWPIXEL_50_50_MIX(
				adjustedColor, WIZRAWPIXEL_50_50_PREMIX_COLOR(*dst8));
			dst8++;
		} else {
			*dst16 = WIZRAWPIXEL_50_50_MIX(
				adjustedColor, WIZRAWPIXEL_50_50_PREMIX_COLOR(*dst16));
			dst16++;
		}
	}
}

void Wiz::trleFLIPFiftyFiftyMixForwardPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable) {
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			*dst8 = WIZRAWPIXEL_50_50_MIX(
				WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor),
				WIZRAWPIXEL_50_50_PREMIX_COLOR(*dst8));
			dst8++;
		} else {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			*dst16 = WIZRAWPIXEL_50_50_MIX(
				WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor),
				WIZRAWPIXEL_50_50_PREMIX_COLOR(*dst16));
			dst16++;
		}
	}
}

void Wiz::trleFLIPFiftyFiftyMixBackwardsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable) {
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			*dst8 = WIZRAWPIXEL_50_50_MIX(
				WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor),
				WIZRAWPIXEL_50_50_PREMIX_COLOR(*dst8));
			dst8--;
		} else {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			*dst16 = WIZRAWPIXEL_50_50_MIX(
				WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor),
				WIZRAWPIXEL_50_50_PREMIX_COLOR(*dst16));
			dst16--;
		}
	}
}

void Wiz::trleFLIPAdditivePixelMemset(WizRawPixel *dstPtr, WizRawPixel mixColor, int size) {
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			WizRawPixel workColor = *dst8;
			*dst8++ = (WizRawPixel8)WIZRAWPIXEL_ADDITIVE_MIX(workColor, FROM_LE_16(mixColor));
		} else {
			WizRawPixel workColor = *dst16;
			*dst16++ = (WizRawPixel16)WIZRAWPIXEL_ADDITIVE_MIX(workColor, FROM_LE_16(mixColor));
		}
	}
}

void Wiz::trleFLIPAdditiveForwardPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable) {
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			WizRawPixel workColor = *dst8;

			*dst8++ = (WizRawPixel8)WIZRAWPIXEL_ADDITIVE_MIX(workColor, srcColor);
		} else {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			WizRawPixel workColor = *dst16;

			*dst16++ = (WizRawPixel16)WIZRAWPIXEL_ADDITIVE_MIX(workColor, srcColor);
		}
	}
}

void Wiz::trleFLIPAdditiveBackwardsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable) {
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			WizRawPixel workColor = *dst8;

			*dst8-- = (WizRawPixel8)WIZRAWPIXEL_ADDITIVE_MIX(workColor, srcColor);
		} else {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			WizRawPixel workColor = *dst16;

			*dst16-- = (WizRawPixel16)WIZRAWPIXEL_ADDITIVE_MIX(workColor, srcColor);
		}
	}
}
void Wiz::trleFLIPSubtractivePixelMemset(WizRawPixel *dstPtr, WizRawPixel mixColor, int size) {
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			WizRawPixel workColor = *dst8;
			*dst8++ = (WizRawPixel8)WIZRAWPIXEL_SUBTRACTIVE_MIX(workColor, FROM_LE_16(mixColor));
		} else {
			WizRawPixel workColor = *dst16;
			*dst16++ = (WizRawPixel16)WIZRAWPIXEL_SUBTRACTIVE_MIX(workColor, FROM_LE_16(mixColor));
		}
	}
}

void Wiz::trleFLIPSubtractiveForwardPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable) {
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			WizRawPixel workColor = *dst8;

			*dst8++ = (WizRawPixel8)WIZRAWPIXEL_SUBTRACTIVE_MIX(workColor, srcColor);
		} else {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			WizRawPixel workColor = *dst16;

			*dst16++ = (WizRawPixel16)WIZRAWPIXEL_SUBTRACTIVE_MIX(workColor, srcColor);
		}
	}
}

void Wiz::trleFLIPSubtractiveBackwardsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable) {
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			WizRawPixel workColor = *dst8;

			*dst8-- = (WizRawPixel8)WIZRAWPIXEL_SUBTRACTIVE_MIX(workColor, srcColor);
		} else {
			WizRawPixel srcColor = convert8BppToRawPixel(*srcPtr++, conversionTable);
			WizRawPixel workColor = *dst16;

			*dst16-- = (WizRawPixel16)WIZRAWPIXEL_SUBTRACTIVE_MIX(workColor, srcColor);
		}
	}
}

void Wiz::trleFLIPRemapDestPixels(WizRawPixel *dstPtr, int size, const byte *lookupTable) {
	if (!_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;

		while (size-- > 0) {
			*dst8 = *(lookupTable + *dst8);
			dst8++;
		}
	}
}

void Wiz::trleFLIPForwardPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable) {
	WizRawPixel8 *buf8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *buf16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (_uses16BitColor) {
			*buf16++ = (WizRawPixel16)convert8BppToRawPixel(*srcPtr++, conversionTable);
		} else {
			*buf8++ = (WizRawPixel8)convert8BppToRawPixel(*srcPtr++, conversionTable);
		}
	}

	if (_uses16BitColor) {
		dstPtr = (WizRawPixel *)buf16;
	} else {
		dstPtr = (WizRawPixel *)buf8;
	}
}

void Wiz::trleFLIPBackwardsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const WizRawPixel *conversionTable) {
	WizRawPixel8 *buf8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *buf16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (_uses16BitColor) {
			*buf16-- = (WizRawPixel16)convert8BppToRawPixel(*srcPtr++, conversionTable);
		} else {
			*buf8-- = (WizRawPixel8)convert8BppToRawPixel(*srcPtr++, conversionTable);
		}
	}

	if (_uses16BitColor) {
		dstPtr = (WizRawPixel *)buf16;
	} else {
		dstPtr = (WizRawPixel *)buf8;
	}
}

void Wiz::trleFLIPForwardLookupPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const byte *lookupTable, const WizRawPixel *conversionTable) {
	WizRawPixel8 *buf8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *buf16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (_uses16BitColor) {
			*buf16++ = (WizRawPixel16)convert8BppToRawPixel(*(lookupTable + *srcPtr++), conversionTable);
		} else {
			*buf8++ = (WizRawPixel8)convert8BppToRawPixel(*(lookupTable + *srcPtr++), conversionTable);
		}
	}

	if (_uses16BitColor) {
		dstPtr = (WizRawPixel *)buf16;
	} else {
		dstPtr = (WizRawPixel *)buf8;
	}
}

void Wiz::trleFLIPBackwardsLookupPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const byte *lookupTable, const WizRawPixel *conversionTable) {
	WizRawPixel8 *buf8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *buf16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (_uses16BitColor) {
			*buf16-- = (WizRawPixel16)convert8BppToRawPixel(*(lookupTable + *srcPtr++), conversionTable);
		} else {
			*buf8-- = (WizRawPixel8)convert8BppToRawPixel(*(lookupTable + *srcPtr++), conversionTable);
		}
	}

	if (_uses16BitColor) {
		dstPtr = (WizRawPixel *)buf16;
	} else {
		dstPtr = (WizRawPixel *)buf8;
	}
}

void Wiz::trleFLIPForwardMixColorsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const byte *lookupTable) {
	WizRawPixel8 *buf8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *buf16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			*buf8 = *(lookupTable + ((*srcPtr++) * 256) + *buf8);
			buf8++;
		} else {
			*buf16++ = *srcPtr++;
		}
	}

	if (_uses16BitColor) {
		dstPtr = (WizRawPixel *)buf16;
	} else {
		dstPtr = (WizRawPixel *)buf8;
	}
}

void Wiz::trleFLIPBackwardsMixColorsPixelCopy(WizRawPixel *dstPtr, const byte *srcPtr, int size, const byte *lookupTable) {
	WizRawPixel8 *buf8 = (WizRawPixel8 *)dstPtr;
	WizRawPixel16 *buf16 = (WizRawPixel16 *)dstPtr;

	while (size-- > 0) {
		if (!_uses16BitColor) {
			*buf8 = *(lookupTable + ((*srcPtr++) * 256) + *buf8);
			buf8--;
		} else {
			*buf16-- = *srcPtr++;
		}
	}

	if (_uses16BitColor) {
		dstPtr = (WizRawPixel *)buf16;
	} else {
		dstPtr = (WizRawPixel *)buf8;
	}
}

static void trleFLIPAdditiveDecompressLineForward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	TRLEFLIP_X_FORWARD_PASS(
		wiz->trleFLIPAdditivePixelMemset,
		wiz->trleFLIPAdditiveForwardPixelCopy);
}

static void trleFLIPAdditiveDecompressLineBackward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	TRLEFLIP_X_BACKWARD_PASS(
		wiz->trleFLIPAdditivePixelMemset,
		wiz->trleFLIPAdditiveBackwardsPixelCopy);
}

static void trleFLIPSubtractiveDecompressLineForward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	TRLEFLIP_X_FORWARD_PASS(
		wiz->trleFLIPSubtractivePixelMemset,
		wiz->trleFLIPSubtractiveForwardPixelCopy);
}

static void trleFLIPSubtractiveDecompressLineBackward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	TRLEFLIP_X_BACKWARD_PASS(
		wiz->trleFLIPSubtractivePixelMemset,
		wiz->trleFLIPSubtractiveBackwardsPixelCopy);
}

static void trleFLIPDecompressLineForward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (wiz->_uses16BitColor) {
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		},
		{
			TRLEFLIP_MEMSET(destPtr, *dataStream, runCount);
			if (wiz->_uses16BitColor) {
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		},
		{
			wiz->trleFLIPForwardPixelCopy(destPtr, dataStream, runCount, conversionTable);
			if (wiz->_uses16BitColor) {
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		});
}

static void trleFLIPDecompressLineBackward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (wiz->_uses16BitColor) {
				dest16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		},
		{
			if (wiz->_uses16BitColor) {
				dest16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
			}
			TRLEFLIP_MEMSET(destPtr + 1, *dataStream, runCount);
		},
		{
			wiz->trleFLIPBackwardsPixelCopy(destPtr, dataStream, runCount, conversionTable);
			if (wiz->_uses16BitColor) {
				dest16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		});
}

static void trleFLIPLookupDecompressLineForward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	const byte *lookupTable;
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	lookupTable = extraPtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (wiz->_uses16BitColor) {
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		},
		{
			TRLEFLIP_MEMSET(destPtr, *(lookupTable + *dataStream), runCount);
			if (wiz->_uses16BitColor) {
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		},
		{
			wiz->trleFLIPForwardLookupPixelCopy(destPtr, dataStream, runCount, lookupTable, conversionTable);
			if (wiz->_uses16BitColor) {
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		});
}

static void trleFLIPLookupDecompressLineBackward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	const byte *lookupTable;
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	lookupTable = extraPtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (wiz->_uses16BitColor) {
				dest16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		},
		{
			if (wiz->_uses16BitColor) {
				dest16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
			}
			TRLEFLIP_MEMSET(destPtr + 1, *(lookupTable + *dataStream), runCount);
		},
		{
			wiz->trleFLIPBackwardsLookupPixelCopy(destPtr, dataStream, runCount, lookupTable, conversionTable);
			if (wiz->_uses16BitColor) {
				dest16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		});
}

static void trleFLIPMixDecompressLineForward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	const byte *lookupTable;
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	lookupTable = extraPtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (wiz->_uses16BitColor) {
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				wiz->trleFLIPRemapDestPixels(destPtr, runCount, lookupTable + (*dataStream * 256));
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			} else {
				wiz->trleFLIPFiftyFiftyMixPixelMemset(
					destPtr, wiz->convert8BppToRawPixel(*dataStream, conversionTable), runCount);

				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				wiz->trleFLIPForwardMixColorsPixelCopy(destPtr, dataStream, runCount, lookupTable);
				dest8 += runCount;
				destPtr = (WizRawPixel *)dest8;
			} else {
				wiz->trleFLIPFiftyFiftyMixForwardPixelCopy(destPtr, dataStream, runCount, conversionTable);
				dest16 += runCount;
				destPtr = (WizRawPixel *)dest16;
			}
		});
}

static void trleFLIPMixDecompressLineBackward(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const byte *extraPtr, const WizRawPixel *conversionTable) {
	const byte *lookupTable;
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	lookupTable = extraPtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (wiz->_uses16BitColor) {
				dest16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
			} else {
				dest8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				dest8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
				wiz->trleFLIPRemapDestPixels(destPtr + 1, runCount, lookupTable + (*dataStream * 256));
			} else {
				dest16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
				wiz->trleFLIPFiftyFiftyMixPixelMemset(
					destPtr, wiz->convert8BppToRawPixel(*dataStream, conversionTable), runCount);
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				wiz->trleFLIPBackwardsMixColorsPixelCopy(destPtr, dataStream, runCount, lookupTable);
				dest8 -= runCount;
				destPtr = (WizRawPixel *)dest8;
			} else {
				wiz->trleFLIPFiftyFiftyMixBackwardsPixelCopy(destPtr, dataStream, runCount, conversionTable);
				dest16 -= runCount;
				destPtr = (WizRawPixel *)dest16;
			}
		});
}

static void trleFLIPArbitraryDstStepDecompressLine(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const void *userParam, int destStepValue, const WizRawPixel *conversionTable) {
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (!wiz->_uses16BitColor) {
				while (--runCount >= 0) {
					dest8 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest8;
			} else {
				while (--runCount >= 0) {
					dest16 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest16;
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				WizRawPixel8 val8 = (WizRawPixel8)wiz->convert8BppToRawPixel(*dataStream, conversionTable);

				for (int counter = 0; counter < runCount; counter++) {
					*dest8 = val8;
					dest8 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest8;
			} else {
				WizRawPixel16 val16 = (WizRawPixel16)wiz->convert8BppToRawPixel(*dataStream, conversionTable);

				for (int counter = 0; counter < runCount; counter++) {
					*dest16 = val16;
					dest16 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest16;
			}
		},
		{
			const byte *src = (const byte *)dataStream;

			if (!wiz->_uses16BitColor) {
				for (int counter = 0; counter < runCount; counter++) {
					*dest8 = (WizRawPixel8)wiz->convert8BppToRawPixel(*src++, conversionTable);
					dest8 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest8;
			} else {
				for (int counter = 0; counter < runCount; counter++) {
					*dest16 = (WizRawPixel16)wiz->convert8BppToRawPixel(*src++, conversionTable);
					dest16 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest16;
			}
		}
	);
}

static void trleFLIPLookupArbitraryDstStepDecompressLine(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const void *userParam, int destStepValue, const WizRawPixel *conversionTable) {
	const byte *lookupTable;
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	lookupTable = (const byte *)userParam;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (!wiz->_uses16BitColor) {
				while (--runCount >= 0) {
					dest8 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest8;
			} else {
				while (--runCount >= 0) {
					dest16 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest16;
			}
		},
		{
			if (!wiz->_uses16BitColor) {
				WizRawPixel8 val8 = (WizRawPixel8)wiz->convert8BppToRawPixel(*(lookupTable + *dataStream), conversionTable);

				for (int counter = 0; counter < runCount; counter++) {
					*dest8 = val8;
					dest8 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest8;
			} else {
				WizRawPixel16 val16 = (WizRawPixel16)wiz->convert8BppToRawPixel(*(lookupTable + *dataStream), conversionTable);

				for (int counter = 0; counter < runCount; counter++) {
					*dest16 = val16;
					dest16 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest16;
			}
		},
		{
			const byte *src = (const byte *)dataStream;
			if (!wiz->_uses16BitColor) {
				for (int counter = 0; counter < runCount; counter++) {
					*dest8 = (WizRawPixel8)wiz->convert8BppToRawPixel(*(lookupTable + *src++), conversionTable);
					dest8 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest8;
			} else {
				for (int counter = 0; counter < runCount; counter++) {
					*dest16 = (WizRawPixel16)wiz->convert8BppToRawPixel(*(lookupTable + *src++), conversionTable);
					dest16 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest16;
			}
		}
	);
}

static void trleFLIPMixArbitraryDstStepDecompressLine(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const void *userParam, int destStepValue, const WizRawPixel *conversionTable) {
	bool _uses16BitColor = wiz->_uses16BitColor;
	const byte *mixColorTable;
	const byte *lookupTable;
	int runCount;
	WizRawPixel8 *dest8 = (WizRawPixel8 *)destPtr;
	WizRawPixel16 *dest16 = (WizRawPixel16 *)destPtr;

	mixColorTable = (const byte *)userParam;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			if (!wiz->_uses16BitColor) {
				while (--runCount >= 0) {
					dest8 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest8;
			} else {
				while (--runCount >= 0) {
					dest16 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest16;
			}
		},
		{
			if (!_uses16BitColor) {
				WizRawPixel8 value = (WizRawPixel8)wiz->convert8BppToRawPixel(*dataStream, conversionTable);
				lookupTable = mixColorTable + (value * 256);

				for (int counter = 0; counter < runCount; counter++) {
					*dest8 = (WizRawPixel8)(*(lookupTable + *destPtr));
					dest8 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest8;
			} else {
				WizRawPixel16 adjustedRunColor = (WizRawPixel16)WIZRAWPIXEL_50_50_PREMIX_COLOR(
					wiz->convert8BppToRawPixel(*dataStream, conversionTable));

				for (int counter = 0; counter < runCount; counter++) {
					*dest16 = (WizRawPixel16)(WIZRAWPIXEL_50_50_MIX(
						adjustedRunColor, WIZRAWPIXEL_50_50_PREMIX_COLOR(*dest16)));
					dest16 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest16;
			}
		},
		{
			if (!_uses16BitColor) {
				const WizRawPixel8 *src = (const WizRawPixel8 *)dataStream;

				for (int counter = 0; counter < runCount; counter++) {
					*dest8 = (WizRawPixel8)wiz->convert8BppToRawPixel(
						(*(mixColorTable + ((*src++) * 256) + *dest8)), conversionTable);
					dest8 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest8;
			} else {
				const byte *src;

				src = (const byte *)dataStream;

				for (int counter = 0; counter < runCount; counter++) {
					WizRawPixel16 adjustedColor = (WizRawPixel16)(WIZRAWPIXEL_50_50_PREMIX_COLOR(
						wiz->convert8BppToRawPixel(*src++, conversionTable)));
					*dest16 = (WizRawPixel16)(WIZRAWPIXEL_50_50_MIX(
						adjustedColor, WIZRAWPIXEL_50_50_PREMIX_COLOR(*dest16)));

					dest16 += destStepValue;
				}

				destPtr = (WizRawPixel *)dest16;
			}
		}
	);
}

static void trleFLIPAdditiveArbitraryDstStepDecompressLine(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const void *userParam, int destStepValue, const WizRawPixel *conversionTable) {
	bool _uses16BitColor = wiz->_uses16BitColor;
	TRLEFLIP_X_ARBITRARY_DST_STEP(WIZRAWPIXEL_ADDITIVE_MIX);
}

static void trleFLIPSubtractiveArbitraryDstStepDecompressLine(Wiz *wiz, WizRawPixel *destPtr, const byte *dataStream, int skipAmount, int decompAmount, const void *userParam, int destStepValue, const WizRawPixel *conversionTable) {
	bool _uses16BitColor = wiz->_uses16BitColor;
	TRLEFLIP_X_ARBITRARY_DST_STEP(WIZRAWPIXEL_SUBTRACTIVE_MIX);
}

void Wiz::trleFLIP90DegreeRotateCore(WizSimpleBitmap *dstBitmap, int x, int y, const WizCompressedImage *imagePtr, const Common::Rect *optionalSrcRect,
	const Common::Rect *optionalClipRect, bool hFlip, bool vFlip, const void *userParam, const WizRawPixel *conversionTable,
	void(*functionPtr)(Wiz *wiz,
		WizRawPixel *destPtr, const byte *dataStream, int skipAmount,
		int decompAmount, const void *userParam, int destStepValue,
		const WizRawPixel *conversionTable)) {

	Common::Rect dstRect, srcRect, clipRect, clippedDstRect, clippedSrcRect;
	int dstOffset, dstStep, w, h, srcOffset, dstX, dstY, skipAmount, c;
	WizRawPixel *dstPtr;
	const byte *compData;

	// Do as much pre-clipping as possible
	makeSizedRect(&clipRect, dstBitmap->bitmapWidth, dstBitmap->bitmapHeight);

	if (optionalClipRect) {
		if (!findRectOverlap(&clipRect, optionalClipRect)) {
			return;
		}
	}

	// Clip the source rect against the actual src bitmap limits
	makeSizedRect(&srcRect, imagePtr->width, imagePtr->height);

	if (optionalSrcRect) {
		if (!findRectOverlap(&srcRect, optionalSrcRect)) {
			return;
		}
	}

	// Make the "dest" rect then clip it against the clip rect
	makeSizedRectAt(&dstRect, x, y, getRectHeight(&srcRect), getRectWidth(&srcRect));

	clippedDstRect = dstRect;

	if (!findRectOverlap(&clippedDstRect, &clipRect)) {
		return;
	}

	// Make the clipped src rect adjusted for the 90 degree rotation.
	clippedSrcRect.left   = srcRect.left   + (clippedDstRect.top  - dstRect.top);
	clippedSrcRect.top    = srcRect.top    + (dstRect.right       - clippedDstRect.right);
	clippedSrcRect.right  = srcRect.right  - (dstRect.bottom      - clippedDstRect.bottom);
	clippedSrcRect.bottom = srcRect.bottom - (clippedDstRect.left - dstRect.left);

	// Perform any flipping of the coords and setup the step variables
	if (hFlip) {
		horzFlipAlignWithRect(&clippedSrcRect, &srcRect);
		dstY = clippedDstRect.bottom;
		dstStep = -dstBitmap->bitmapWidth;
	} else {
		dstY = clippedDstRect.top;
		dstStep = dstBitmap->bitmapWidth;
	}

	if (vFlip) {
		vertFlipAlignWithRect(&clippedSrcRect, &srcRect);
		dstX = clippedDstRect.left;
		dstOffset = 1;
	} else {
		dstX = clippedDstRect.right;
		dstOffset = -1;
	}

	// Finally get down to business and do the blit!
	WizRawPixel8  *dst8  = ((WizRawPixel8  *)dstBitmap->bufferPtr()) + dstX + (dstY * dstBitmap->bitmapWidth);
	WizRawPixel16 *dst16 = ((WizRawPixel16 *)dstBitmap->bufferPtr()) + dstX + (dstY * dstBitmap->bitmapWidth);

	if (!_uses16BitColor) {
		dstPtr = (WizRawPixel *)dst8;
	} else {
		dstPtr = (WizRawPixel *)dst16;
	}

	w = getRectWidth(&clippedSrcRect);
	h = getRectHeight(&clippedSrcRect);

	// General setup / skip n lines
	skipAmount = clippedSrcRect.left;
	compData = imagePtr->data;

	for (c = clippedSrcRect.top; --c >= 0;) {
		compData += READ_LE_UINT16(compData) + 2;
	}

	// Transfer the src line to the dest line using the passed transfer prim.
	while (--h >= 0) {
		srcOffset = READ_LE_UINT16(compData);

		if (srcOffset) {
			(*functionPtr)(this, dstPtr, compData + 2, skipAmount, w, userParam, dstStep, conversionTable);
			compData += (srcOffset + 2);
		} else {
			compData += 2;
		}

		dstPtr += dstOffset;
	}
}

void Wiz::trleFLIPDecompressImage(
	WizRawPixel *bufferPtr, const byte *compData, int bufferWidth, int bufferHeight,
	int x, int y, int width, int height, Common::Rect *clipRectPtr,
	int32 wizFlags, const byte *extraTable, const WizRawPixel *conversionTable,
	const WizImageCommand *optionalICmdPtr) {
	WizCompressedImage fakeImage;
	WizSimpleBitmap fakeBitmap;

	if (_uses16BitColor)
		trleFLIPCheckAlphaSetup();

	// General setup for the primitives
	fakeBitmap.bufferPtr = WizPxShrdBuffer(bufferPtr, false);
	fakeBitmap.bitmapWidth = bufferWidth;
	fakeBitmap.bitmapHeight = bufferHeight;

	fakeImage.data = compData;
	fakeImage.width = width;
	fakeImage.height = height;

	if (!_uses16BitColor) {
		// Check to see if there is a remap to be performed on the TRLE data...
		if (conversionTable && (conversionTable != (WizRawPixel *)_vm->getHEPaletteSlot(1))) {
			extraTable = (const byte *)conversionTable;
			wizFlags &= ~kWRFUseShadow;
			wizFlags |= kWRFRemap;
		}
	} else {
		if (wizFlags & kWRFRemap) {
			extraTable = nullptr;
			wizFlags &= ~(kWRFUseShadow | kWRFRemap);
		}
	}

	if (_uses16BitColor) {
		// Check for new "special" rendering modes
		if (wizFlags & kWRFSpecialRenderBitMask) {
			if (wizFlags & kWRFAdditiveBlend) {
				trleFLIPDecompressPrim(
					&fakeBitmap, &fakeImage, x, y, nullptr, clipRectPtr, extraTable, wizFlags, conversionTable,
					trleFLIPAdditiveDecompressLineForward,
					trleFLIPAdditiveDecompressLineBackward);

				return;
			} else if (wizFlags & kWRFSubtractiveBlend) {
				trleFLIPDecompressPrim(
					&fakeBitmap, &fakeImage, x, y, nullptr, clipRectPtr, extraTable, wizFlags, conversionTable,
					trleFLIPSubtractiveDecompressLineForward,
					trleFLIPSubtractiveDecompressLineBackward);

				return;
			} else if (wizFlags & kWRF5050Blend) {
				trleFLIPDecompressPrim(
					&fakeBitmap, &fakeImage, x, y, nullptr, clipRectPtr, extraTable, wizFlags, conversionTable,
					trleFLIPMixDecompressLineForward,
					trleFLIPMixDecompressLineBackward);

				return;
			}
		}
	}

	// Find which draw primitives need to be called...
	if (!extraTable) {
		trleFLIPDecompressPrim(
			&fakeBitmap, &fakeImage, x, y, nullptr, clipRectPtr, extraTable, wizFlags, conversionTable,
			trleFLIPDecompressLineForward, trleFLIPDecompressLineBackward);
	} else if (wizFlags & kWRFRemap) {
		trleFLIPDecompressPrim(
			&fakeBitmap, &fakeImage, x, y, nullptr, clipRectPtr, extraTable, wizFlags, conversionTable,
			trleFLIPLookupDecompressLineForward, trleFLIPLookupDecompressLineBackward);
	} else if ((wizFlags & kWRFUseShadow) != 0 || _vm->_game.heversion > 98) {
		trleFLIPDecompressPrim(
			&fakeBitmap, &fakeImage, x, y, nullptr, clipRectPtr, extraTable, wizFlags, conversionTable,
			trleFLIPMixDecompressLineForward,
			trleFLIPMixDecompressLineBackward);
	}
}

void Wiz::trleFLIPRotate90DecompressImage(
	WizRawPixel *bufferPtr, const byte *compData, int bufferWidth, int bufferHeight,
	int x, int y, int width, int height, const Common::Rect *clipRectPtr,
	int32 wizFlags, const void *extraTable, const WizRawPixel *conversionTable,
	WizImageCommand *optionalICmdPtr) {
	WizCompressedImage fakeImage;
	WizSimpleBitmap fakeBitmap;

	if (_uses16BitColor)
		trleFLIPCheckAlphaSetup();

	// General setup for the primitives
	fakeBitmap.bufferPtr = WizPxShrdBuffer(bufferPtr, false);
	fakeBitmap.bitmapWidth = bufferWidth;
	fakeBitmap.bitmapHeight = bufferHeight;

	fakeImage.data = compData;
	fakeImage.width = width;
	fakeImage.height = height;

	if (!_uses16BitColor) {
		// Check to see if there is a remap to be performed on the TRLE data...
		if (conversionTable && (conversionTable != (WizRawPixel *)_vm->getHEPaletteSlot(1))) {
			extraTable = (const void *)conversionTable;
			wizFlags &= ~kWRFUseShadow;
			wizFlags |= kWRFRemap;
		}
	} else {
		if (_vm->_game.heversion < 100 || wizFlags & kWRFRemap) {
			extraTable = nullptr;
			wizFlags &= ~(kWRFUseShadow | kWRFRemap);
		}
	}

	if (_vm->_game.heversion >= 99 && _uses16BitColor) {
		// Check for new "special" rendering modes
		if (wizFlags & kWRFSpecialRenderBitMask) {
			if (wizFlags & kWRFAdditiveBlend) {
				trleFLIP90DegreeRotateCore(
					&fakeBitmap, x, y, &fakeImage, nullptr, clipRectPtr,
					(wizFlags & kWRFHFlip), (wizFlags & kWRFVFlip),
					extraTable, conversionTable,
					trleFLIPAdditiveArbitraryDstStepDecompressLine);

				return;
			} else if (wizFlags & kWRFSubtractiveBlend) {
				trleFLIP90DegreeRotateCore(
					&fakeBitmap, x, y, &fakeImage, nullptr, clipRectPtr,
					(wizFlags & kWRFHFlip), (wizFlags & kWRFVFlip),
					extraTable, conversionTable,
					trleFLIPSubtractiveArbitraryDstStepDecompressLine);

				return;
			} else if (wizFlags & kWRF5050Blend) {
				trleFLIP90DegreeRotateCore(
					&fakeBitmap, x, y, &fakeImage, nullptr, clipRectPtr,
					(wizFlags & kWRFHFlip), (wizFlags & kWRFVFlip),
					extraTable, conversionTable,
					trleFLIPMixArbitraryDstStepDecompressLine);

				return;
			}
		}
	}

	// Find which draw primitives need to be called...
	if (!extraTable) {
		trleFLIP90DegreeRotateCore(
			&fakeBitmap, x, y, &fakeImage, nullptr, clipRectPtr,
			(wizFlags & kWRFHFlip), (wizFlags & kWRFVFlip),
			extraTable, conversionTable,
			trleFLIPArbitraryDstStepDecompressLine);

	} else if (wizFlags & kWRFRemap) {
		trleFLIP90DegreeRotateCore(
			&fakeBitmap, x, y, &fakeImage, nullptr, clipRectPtr,
			(wizFlags & kWRFHFlip), (wizFlags & kWRFVFlip),
			extraTable, conversionTable,
			trleFLIPLookupArbitraryDstStepDecompressLine);
	} else if ((wizFlags & kWRFUseShadow) != 0 || _vm->_game.heversion > 98) {
		trleFLIP90DegreeRotateCore(
			&fakeBitmap, x, y, &fakeImage, nullptr, clipRectPtr,
			(wizFlags & kWRFHFlip), (wizFlags & kWRFVFlip),
			extraTable, conversionTable,
			trleFLIPMixArbitraryDstStepDecompressLine);
	}
}

static void trleFLIPX2XBoolAlphaMemset(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const WizRawPixel value, int size) {
	// 16-bit only
	while (size-- > 0) {
		if (*srcPtr++) {
			*dstPtr++ = value;
		} else {
			++dstPtr;
		}
	}
}

static void trleFLIPX2XBoolInvAlphaMemset(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const WizRawPixel value, int size) {
	// 16-bit only
	while (size-- > 0) {
		if (*srcPtr++) {
			++dstPtr;
		} else {
			*dstPtr++ = value;
		}
	}
}

static void trleFLIPX2XBoolAlphaForwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr,
	// 16-bit only
	int size, const WizRawPixel *conversionTable) {
	while (size-- > 0) {
		if ((*srcPtr)) {
			*dstPtr = wiz->convert8BppToRawPixel(*dataPtr, conversionTable);
		}

		++dstPtr;
		++srcPtr;
		++dataPtr;
	}
}

static void trleFLIPX2XBoolAlphaBackwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		if ((*srcPtr)) {
			*dstPtr = wiz->convert8BppToRawPixel(*dataPtr, conversionTable);
		}

		--dstPtr;
		--srcPtr;
		++dataPtr;
	}
}

static void trleFLIPX2XBoolInvAlphaForwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		if (!(*srcPtr)) {
			*dstPtr = wiz->convert8BppToRawPixel(*dataPtr, conversionTable);
		}

		++dstPtr;
		++srcPtr;
		++dataPtr;
	}
}

static void trleFLIPX2XBoolInvAlphaBackwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		if (!(*srcPtr)) {
			*dstPtr = wiz->convert8BppToRawPixel(*dataPtr, conversionTable);
		}

		--dstPtr;
		--srcPtr;
		++dataPtr;
	}
}

static void trleFLIPAltSourceForwardBoolAlphaX2X(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_FORWARD_PASS(
		trleFLIPX2XBoolAlphaMemset,
		trleFLIPX2XBoolAlphaForwardMemcpy);
}

static void trleFLIPAltSourceBackwardBoolAlphaX2X(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_BACKWARD_PASS(
		trleFLIPX2XBoolAlphaMemset,
		trleFLIPX2XBoolAlphaBackwardMemcpy);
}

static void trleFLIPAltSourceForwardBoolInvAlphaX2X(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_FORWARD_PASS(
		trleFLIPX2XBoolInvAlphaMemset,
		trleFLIPX2XBoolInvAlphaForwardMemcpy);
}

static void trleFLIPAltSourceBackwardBoolInvAlphaX2X(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_BACKWARD_PASS(
		trleFLIPX2XBoolInvAlphaMemset,
		trleFLIPX2XBoolInvAlphaBackwardMemcpy);
}

static void trleFLIPX2XAlphaMemset(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const WizRawPixel value, int size) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = ((*srcPtr++) & 255);

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(value, *dstPtr, (255 - alpha));
		}

		++dstPtr;
	}
}

static void trleFLIPX2XInvAlphaMemset(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const WizRawPixel value, int size) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = (255 - ((*srcPtr++) & 255));

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(value, *dstPtr, (255 - alpha));
		}

		++dstPtr;
	}
}

static void trleFLIPX2XAlphaForwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = ((*srcPtr++) & 255);

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(*dstPtr, wiz->convert8BppToRawPixel(*dataPtr, conversionTable), alpha);
		}

		++dstPtr;
		++dataPtr;
	}
}

static void trleFLIPX2XAlphaBackwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = ((*srcPtr--) & 255);

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(*dstPtr, wiz->convert8BppToRawPixel(*dataPtr, conversionTable), alpha);
		}

		--dstPtr;
		++dataPtr;
	}
}

static void trleFLIPX2XInvAlphaForwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = (255 - ((*srcPtr++) & 255));

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(*dstPtr, wiz->convert8BppToRawPixel(*dataPtr, conversionTable), alpha);
		}

		++dstPtr;
		++dataPtr;
	}
}

static void trleFLIPX2XInvAlphaBackwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = (255 - ((*srcPtr--) & 255));

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(
				*dstPtr, wiz->convert8BppToRawPixel(*dataPtr, conversionTable), alpha);
		}

		--dstPtr;
		++dataPtr;
	}
}

static void trleFLIPAltSourceForwardAlphaX2X(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_FORWARD_PASS(
		trleFLIPX2XAlphaMemset,
		trleFLIPX2XAlphaForwardMemcpy);
}

static void trleFLIPAltSourceBackwardAlphaX2X(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_BACKWARD_PASS(
		trleFLIPX2XAlphaMemset,
		trleFLIPX2XAlphaBackwardMemcpy);
}

static void trleFLIPAltSourceForwardInvAlphaX2X(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_FORWARD_PASS(
		trleFLIPX2XInvAlphaMemset,
		trleFLIPX2XInvAlphaForwardMemcpy);
}

static void trleFLIPAltSourceBackwardInvAlphaX2X(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_BACKWARD_PASS(
		trleFLIPX2XInvAlphaMemset,
		trleFLIPX2XInvAlphaBackwardMemcpy);
}

static void trleFLIPATRLEAlphaMemset(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const WizRawPixel value, int size) {
	// 16-bit only
	int alpha = (value & 255);

	if (alpha) {
		while (size-- > 0) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(*dstPtr, *srcPtr, alpha);

			++dstPtr;
			++srcPtr;
		}
	}
}

static void trleFLIPATRLEInvAlphaMemset(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const WizRawPixel value, int size) {
	// 16-bit only
	int alpha = (255 - (value & 255));

	if (alpha) {
		while (size-- > 0) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(*dstPtr, *srcPtr, alpha);

			++dstPtr;
			++srcPtr;
		}
	}
}

static void trleFLIPATRLEAlphaForwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = ((wiz->convert8BppToRawPixel(*dataPtr++, conversionTable)) & 255);

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(
				*dstPtr, *srcPtr, alpha);
		}

		++dstPtr;
		++srcPtr;
	}
}

static void trleFLIPATRLEAlphaBackwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = ((wiz->convert8BppToRawPixel(*dataPtr++, conversionTable)) & 255);

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(
				*dstPtr, *srcPtr, alpha);
		}

		--dstPtr;
		--srcPtr;
	}
}

static void trleFLIPATRLEInvAlphaForwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = (255 - ((wiz->convert8BppToRawPixel(*dataPtr++, conversionTable)) & 255));

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(
				*dstPtr, *srcPtr, alpha);
		}

		++dstPtr;
		++srcPtr;
	}
}

static void trleFLIPATRLEInvAlphaBackwardMemcpy(Wiz *wiz, WizRawPixel *dstPtr, const WizRawPixel *srcPtr, const byte *dataPtr, int size, const WizRawPixel *conversionTable) {
	// 16-bit only
	while (size-- > 0) {
		int alpha = (255 - ((wiz->convert8BppToRawPixel(*dataPtr++, conversionTable)) & 255));

		if (alpha) {
			*dstPtr = wiz->trleFLIPAlphaMixPrim(
				*dstPtr, *srcPtr, alpha);
		}

		--dstPtr;
		--srcPtr;
	}
}

static void trleFLIPAltSourceForwardAlphaATRLE(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_FORWARD_PASS(
		trleFLIPATRLEAlphaMemset,
		trleFLIPATRLEAlphaForwardMemcpy);
}

static void trleFLIPAltSourceBackwardAlphaATRLE(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_BACKWARD_PASS(
		trleFLIPATRLEAlphaMemset,
		trleFLIPATRLEAlphaBackwardMemcpy);
}

static void trleFLIPAltSourceForwardInvAlphaATRLE(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_FORWARD_PASS(
		trleFLIPATRLEInvAlphaMemset,
		trleFLIPATRLEInvAlphaForwardMemcpy);
}

static void trleFLIPAltSourceBackwardInvAlphaATRLE(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	// 16-bit only
	TRLEFLIP_ALTX_BACKWARD_PASS(
		trleFLIPATRLEInvAlphaMemset,
		trleFLIPATRLEInvAlphaBackwardMemcpy);
}

static void trleFLIPAltSourceForwardXBppToXBpp(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr;
	int runCount;

	srcPtr = (const WizRawPixel *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			destPtr += runCount;
			srcPtr += runCount;
		},
		{
			memcpy(destPtr, srcPtr, (runCount * sizeof(WizRawPixel)));
			destPtr += runCount;
			srcPtr += runCount;
		},
		{
			memcpy(destPtr, srcPtr, (runCount * sizeof(WizRawPixel)));
			destPtr += runCount;
			srcPtr += runCount;
		});
}

static void trleFLIPAltSourceBackwardXBppToXBpp(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr;
	int runCount;

	srcPtr = (const WizRawPixel *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			destPtr -= runCount;
			srcPtr -= runCount;
		},
		{
			destPtr -= runCount;
			srcPtr -= runCount;
			memcpy(destPtr + 1, srcPtr + 1, (runCount * sizeof(WizRawPixel)));
		},
		{
			destPtr -= runCount;
			srcPtr -= runCount;
			memcpy(destPtr + 1, srcPtr + 1, (runCount * sizeof(WizRawPixel)));
		});
}

static void trleFLIPAltSourceForward8BppToXBpp(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const byte *srcPtr;
	int runCount;

	srcPtr = (const byte *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			destPtr += runCount;
			srcPtr += runCount;
		},
		{
			wiz->memcpy8BppConversion(destPtr, srcPtr, runCount, conversionTable);
			destPtr += runCount;
			srcPtr += runCount;
		},
		{
			wiz->memcpy8BppConversion(destPtr, srcPtr, runCount, conversionTable);
			destPtr += runCount;
			srcPtr += runCount;
		});
}

static void trleFLIPAltSourceBackward8BppToXBpp(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr;
	int runCount;

	srcPtr = (const WizRawPixel *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			destPtr -= runCount;
			srcPtr -= runCount;
		},
		{
			destPtr -= runCount;
			srcPtr -= runCount;
			wiz->memcpy8BppConversion(destPtr + 1, srcPtr + 1, runCount, conversionTable);
		},
		{
			destPtr -= runCount;
			srcPtr -= runCount;
			wiz->memcpy8BppConversion(destPtr + 1, srcPtr + 1, runCount, conversionTable);
		});
}

static void trleFLIPAltSourceInvForwardXBppToXBpp(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr;
	int runCount;

	srcPtr = (const WizRawPixel *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			memcpy(destPtr, srcPtr, (runCount * sizeof(WizRawPixel)));
			destPtr += runCount;
			srcPtr += runCount;
		},
		{
			destPtr += runCount;
			srcPtr += runCount;
		},
		{
			destPtr += runCount;
			srcPtr += runCount;
		});
}

static void trleFLIPAltSourceInvBackwardXBppToXBpp(Wiz *wiz, WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream, int skipAmount, int decompAmount, const WizRawPixel *conversionTable) {
	const WizRawPixel *srcPtr;
	int runCount;

	srcPtr = (const WizRawPixel *)altSourcePtr;

	// Decompress bytes to do simple clipping...
	TRLE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	TRLE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			destPtr -= runCount;
			srcPtr -= runCount;
			memcpy(destPtr + 1, srcPtr + 1, (runCount * sizeof(WizRawPixel)));
		},
		{
			destPtr -= runCount;
			srcPtr -= runCount;
		},
		{
			destPtr -= runCount;
			srcPtr -= runCount;
		});
}

void Wiz::trleFLIPAltSourceDecompImageHull(
	WizRawPixel *bufferPtr, int bufferWidth, const Common::Rect *destRect,
	const byte *altSourceBuffer, int altBytesPerLine,
	int altBytesPerPixel, const Common::Rect *altRect,
	const byte *compData, const Common::Rect *sourceRect,
	const WizRawPixel *conversionTable,
	void (*functionPtr)(Wiz *wiz,
		WizRawPixel *destPtr, const void *altSourcePtr, const byte *dataStream,
		int skipAmount, int decompAmount, const WizRawPixel *conversionTable)) {
	int decompWidth, decompHeight, counter, sX1, lineSize;

	// General setup...
	sX1 = sourceRect->left;

	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	WizRawPixel8 *buf8 = (WizRawPixel8 *)bufferPtr;
	WizRawPixel16 *buf16 = (WizRawPixel16 *)bufferPtr;

	if (_uses16BitColor) {
		buf16 += bufferWidth * destRect->top + destRect->left;
		bufferPtr = (WizRawPixel *)buf16;
	} else {
		buf8 += bufferWidth * destRect->top + destRect->left;
		bufferPtr = (WizRawPixel *)buf8;
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
			(*functionPtr)(this,
				bufferPtr, altSourceBuffer, compData + 2, sX1,
				decompWidth, conversionTable);

			compData += lineSize + 2;
		} else {
			// Handle a completely transparent line!
			compData += 2;
		}

		if (_uses16BitColor) {
			buf16 = (WizRawPixel16 *)bufferPtr;
			buf16 += bufferWidth;
			bufferPtr = (WizRawPixel *)buf16;
		} else {
			buf8 = (WizRawPixel8 *)bufferPtr;
			buf8 += bufferWidth;
			bufferPtr = (WizRawPixel *)buf8;
		}

		altSourceBuffer += altBytesPerLine;
	}
}

void Wiz::trleFLIPAltSourceDecompressPrim(
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

	void (*functionPtr)(Wiz *wiz,
		WizRawPixel * destPtr, const void *altSourcePtr, const byte *dataStream,
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
		trleFLIPHorzFlipAlignWithRect(&sourceRect, &inSourceRect);
		swapRectX(&destRect);
	} else {
		functionPtr = forwardFunctionPtr;
	}

	if (flags & kWRFVFlip) {
		trleFLIPVertFlipAlignWithRect(&sourceRect, &inSourceRect);
		swapRectY(&destRect);
	}

	// Call the primitive image renderer...
	trleFLIPAltSourceDecompImageHull(
		destBufferPtr, destBufferWidth, &destRect,
		(const byte *)altBufferPtr, ((destBufferWidth * altBitsPerPixel) / 8),
		(altBitsPerPixel / 8), &destRect, imagePtr->data, &sourceRect,
		conversionTable, functionPtr);
}

bool Wiz::trleFLIPAltSourceSpecialCaseDispatch(
	WizRawPixel *destBufferPtr, const byte *compData, int destBufferWidth, int destBufferHeight,
	const void *altBufferPtr, int altWidth, int altHeight, int altBitsPerPixel,
	int x, int y, int width, int height, Common::Rect *clipRectPtr,
	int32 wizFlags, const WizRawPixel *conversionTable,
	WizImageCommand *optionalICmdPtr) {

	if (!_uses16BitColor || altBitsPerPixel != 16) {
		return false;
	}

	// General setup...
	Common::Rect srcRect, clipRect;
	WizCompressedImage fakeImage;

	fakeImage.data = compData;
	fakeImage.width = width;
	fakeImage.height = height;

	makeSizedRect(&srcRect, width, height);
	makeSizedRect(&clipRect, destBufferWidth, destBufferHeight);

	if (clipRectPtr) {
		if (!findRectOverlap(&clipRect, clipRectPtr)) {
			return true;
		}
	}

	// Try to find a special case renderer!
	if (wizFlags & kWRFUseImageAsAlphaChannel) {
		if (wizFlags & kWRFBooleanAlpha) {
			if (wizFlags & kWRFInverseAlpha) {
				// Inverse alpha
				trleFLIPAltSourceDecompressPrim(
					destBufferPtr, destBufferWidth, destBufferHeight,
					altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
					&srcRect, &clipRect, wizFlags, conversionTable,
					trleFLIPAltSourceInvForwardXBppToXBpp,
					trleFLIPAltSourceInvBackwardXBppToXBpp);

				return true;
			} else {
				// Normal alpha
				trleFLIPAltSourceDecompressPrim(
					destBufferPtr, destBufferWidth, destBufferHeight,
					altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
					&srcRect, &clipRect, wizFlags, conversionTable,
					trleFLIPAltSourceForwardXBppToXBpp,
					trleFLIPAltSourceBackwardXBppToXBpp);

				return true;
			}
		} else {
			if (wizFlags & kWRFInverseAlpha) {
				// Inverse alpha
				trleFLIPAltSourceDecompressPrim(
					destBufferPtr, destBufferWidth, destBufferHeight,
					altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
					&srcRect, &clipRect, wizFlags, conversionTable,
					trleFLIPAltSourceForwardInvAlphaATRLE,
					trleFLIPAltSourceBackwardInvAlphaATRLE);

				return true;
			} else {
				// Normal alpha
				trleFLIPAltSourceDecompressPrim(
					destBufferPtr, destBufferWidth, destBufferHeight,
					altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
					&srcRect, &clipRect, wizFlags, conversionTable,
					trleFLIPAltSourceForwardAlphaATRLE,
					trleFLIPAltSourceBackwardAlphaATRLE);

				return true;
			}
		}
	} else if (wizFlags & kWRFUseSourceImageAsAlphaChannel) {
		if (wizFlags & kWRFBooleanAlpha) {
			if (wizFlags & kWRFInverseAlpha) {
				// Inverse alpha
				trleFLIPAltSourceDecompressPrim(
					destBufferPtr, destBufferWidth, destBufferHeight,
					altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
					&srcRect, &clipRect, wizFlags, conversionTable,
					trleFLIPAltSourceForwardBoolInvAlphaX2X,
					trleFLIPAltSourceBackwardBoolInvAlphaX2X);

				return true;
			} else {
				// Normal alpha
				trleFLIPAltSourceDecompressPrim(
					destBufferPtr, destBufferWidth, destBufferHeight,
					altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
					&srcRect, &clipRect, wizFlags, conversionTable,
					trleFLIPAltSourceForwardBoolAlphaX2X,
					trleFLIPAltSourceBackwardBoolAlphaX2X);

				return true;
			}
		} else {
			if (wizFlags & kWRFInverseAlpha) {
				// Inverse alpha
				trleFLIPAltSourceDecompressPrim(
					destBufferPtr, destBufferWidth, destBufferHeight,
					altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
					&srcRect, &clipRect, wizFlags, conversionTable,
					trleFLIPAltSourceForwardInvAlphaX2X,
					trleFLIPAltSourceBackwardInvAlphaX2X);

				return true;
			} else {
				// Normal alpha
				trleFLIPAltSourceDecompressPrim(
					destBufferPtr, destBufferWidth, destBufferHeight,
					altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
					&srcRect, &clipRect, wizFlags, conversionTable,
					trleFLIPAltSourceForwardAlphaX2X,
					trleFLIPAltSourceBackwardAlphaX2X);

				return true;
			}
		}
	}

	return false;
}

void Wiz::trleFLIPAltSourceDecompressImage(
	WizRawPixel *destBufferPtr, const byte *compData, int destBufferWidth, int destBufferHeight,
	const void *altBufferPtr, int altWidth, int altHeight, int altBitsPerPixel,
	int x, int y, int width, int height, Common::Rect *clipRectPtr,
	int32 wizFlags, const WizRawPixel *conversionTable,
	WizImageCommand *optionalICmdPtr) {
	Common::Rect srcRect, clipRect;
	WizCompressedImage fakeImage;
	int curBpp = _uses16BitColor ? 16 : 8;

	if (_uses16BitColor)
		trleFLIPCheckAlphaSetup();

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

	if (_uses16BitColor) {
		// Check for new "special" rendering modes
		if (wizFlags & kWRFSpecialRenderBitMask) {
			if (!trleFLIPAltSourceSpecialCaseDispatch(
					destBufferPtr, compData, destBufferWidth, destBufferHeight,
					altBufferPtr, altWidth, altHeight, altBitsPerPixel,
					x, y, width, height, clipRectPtr, wizFlags, conversionTable,
					optionalICmdPtr)) {
			} else {
				return;
			}
		}
	}

	// Dispatch to the next level of code. (8bpp or conversion)
	if (altBitsPerPixel == curBpp) {
		if (!_uses16BitColor)
			error("Wiz::trleFLIPAltSourceDecompressImage(): Yeah, it's used, fix it! 1");
		trleFLIPAltSourceDecompressPrim(
			destBufferPtr, destBufferWidth, destBufferHeight,
			altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
			&srcRect, &clipRect, wizFlags, conversionTable,
			trleFLIPAltSourceForwardXBppToXBpp,
			trleFLIPAltSourceBackwardXBppToXBpp);
	} else if (altBitsPerPixel == 8) {
		if (!_uses16BitColor)
			error("Wiz::trleFLIPAltSourceDecompressImage(): Yeah, it's used, fix it! 2");
		trleFLIPAltSourceDecompressPrim(
			destBufferPtr, destBufferWidth, destBufferHeight,
			altBufferPtr, altBitsPerPixel, &fakeImage, x, y,
			&srcRect, &clipRect, wizFlags, conversionTable,
			trleFLIPAltSourceForward8BppToXBpp,
			trleFLIPAltSourceBackward8BppToXBpp);
	}
}

int Wiz::trleCompressImageArea(byte *destBuffer, const WizRawPixel *sourceBuffer, int sourceBufferWidth, int x1, int y1, int x2, int y2, WizRawPixel transparentColor) {
	int width, height, totalSize, lineSize;
	byte *sizeWordPtr = nullptr;

	width = x2 - x1 + 1;
	height = y2 - y1 + 1;

	const WizRawPixel8 *buf8 = (const WizRawPixel8 *)sourceBuffer;
	const WizRawPixel16 *buf16 = (const WizRawPixel16 *)sourceBuffer;

	if (_uses16BitColor) {
		buf16 += sourceBufferWidth * y1 + x1;
		sourceBuffer = (const WizRawPixel *)buf16;
	} else {
		buf8 += sourceBufferWidth * y1 + x1;
		sourceBuffer = (const WizRawPixel *)buf8;
	}

	totalSize = 0;

	while (height-- > 0) {
		if (destBuffer) {
			sizeWordPtr = destBuffer;
			destBuffer += 2;
		}

		totalSize += 2;
		lineSize = trleRLECompression(
			destBuffer, sourceBuffer, width, transparentColor);

		// Refresh pointers, you never know...
		buf8 = (const WizRawPixel8 *)sourceBuffer;
		buf16 = (const WizRawPixel16 *)sourceBuffer;

		if (_uses16BitColor) {
			buf16 += sourceBufferWidth;
			sourceBuffer = (const WizRawPixel *)buf16;
		} else {
			buf8 += sourceBufferWidth;
			sourceBuffer = (const WizRawPixel *)buf8;
		}

		totalSize += lineSize;
		if (destBuffer) {
			WRITE_LE_UINT16(sizeWordPtr, (uint16)lineSize);
			destBuffer += lineSize;
		}
	}

	return totalSize;
}

byte *Wiz::trlePutDump(byte *dest, int nn) {
	int count, index;
	byte value;

	index = 0;

	do {
		count = nn;
		if (count > 64) {
			count = 64;
		}

		nn -= count;

		value = count - 1;
		value <<= 2;
		trle_putbyte(value);

		for (int i = 0; i < count; i++) {
			trle_putbyte(_trleBuf[index++]);
		}

	} while (nn > 0);

	return dest;
}

byte *Wiz::trlePutRun(byte *dest, int nn, int cc, int tColor) {
	byte value;
	int count;

	if (cc == tColor) {
		do {
			count = nn;
			if (count > 127) {
				count = 127;
			}

			nn -= count;

			value = count;
			value <<= 1;
			value |= 1;

			trle_putbyte(value);
		} while (nn > 0);
	} else {
		do {
			count = nn;
			if (count > 64) {
				count = 64;
			}

			nn -= count;

			value = count - 1;
			value <<= 2;
			value |= 2;

			trle_putbyte(value);
			trle_putbyte(cc);
		} while (nn > 0);
	}

	return dest;
}

int Wiz::trleRLECompression(byte *pdest, const WizRawPixel *psource, int rowsize, WizRawPixel tcolor) {
	byte *dest;
	WizRawPixel c;
	WizRawPixel lastc = 0;
	int mode = TRLE_DUMP;
	int nbuf = 0;
	int rstart = 0;
	int counter;

	const WizRawPixel8 *source8 = (const WizRawPixel8 *)psource;
	const WizRawPixel16 *source16 = (const WizRawPixel16 *)psource;

	// Check for completely transparent
	for (counter = 0; counter < rowsize; counter++) {
		if (_uses16BitColor) {
			if (*(source16 + counter) != tcolor) {
				break;
			}
		} else {
			if (*(source8 + counter) != tcolor) {
				break;
			}
		}
	}

	if (counter >= rowsize) {
		return 0;
	}

	dest = pdest;
	_trlePutSize = 0;

	if (_uses16BitColor) {
		lastc = c = (*source16++);
		_trleBuf[0] = (byte)c;
	} else {
		lastc = c = (*source8++);
		_trleBuf[0] = (byte)c;
	}

	nbuf = 1;
	rowsize--;
	mode = ((c == tcolor) ? TRLE_RUN : TRLE_DUMP);

	for (; rowsize; --rowsize) {
		if (_uses16BitColor) {
			c = (*source16++);
			_trleBuf[nbuf++] = (byte)c;
		} else {
			c = (*source8++);
			_trleBuf[nbuf++] = (byte)c;
		}

		switch (mode) {
		case TRLE_DUMP:
			// Don't allow transparent colors in a literal run!
			if (c == tcolor) {
				dest = trlePutDump(dest, nbuf - 1);
				_trleBuf[0] = c;
				nbuf = 1;
				rstart = 0;
				mode = TRLE_RUN;
				break;
			}

			// Check to see if too literal run too big if so dump it...
			if (nbuf > TRLE_MAXDAT) {
				dest = trlePutDump(dest, nbuf - 1);
				_trleBuf[0] = c;
				nbuf = 1;
				rstart = 0;
				break;
			}

			// Check to see if you can start a run of data...
			if (c == lastc) {
				if (nbuf - rstart >= TRLE_MINRUN) {
					if (rstart > 0) {
						dest = trlePutDump(dest, rstart);
					}
					mode = TRLE_RUN;
				} else {
					if (rstart == 0) {
						mode = TRLE_RUN;
					}
				}

				// Note, in this case the original falls through!
				// Let's replicate that in a way which doesn't trigger warnings...
				//
				// case TRLE_RUN:
				// Check run to see if still going...
				if ((c != lastc) || (nbuf - rstart > TRLE_MAXRUN)) {
					dest = trlePutRun(dest, nbuf - 1 - rstart, lastc, tcolor);
					_trleBuf[0] = c;
					nbuf = 1;
					rstart = 0;
					// Don't allow transparent colors in a literal run!
					mode = ((c == tcolor) ? TRLE_RUN : TRLE_DUMP);
				}
				break;
			} else {
				rstart = nbuf - 1;
				break;
			}

		case TRLE_RUN:
			// Check run to see if still going...
			if ((c != lastc) || (nbuf - rstart > TRLE_MAXRUN)) {
				dest = trlePutRun(dest, nbuf - 1 - rstart, lastc, tcolor);
				_trleBuf[0] = c;
				nbuf = 1;
				rstart = 0;
				// Don't allow transparent colors in a literal run!
				mode = ((c == tcolor) ? TRLE_RUN : TRLE_DUMP);
			}
			break;
		}

		lastc = c;
	}

	switch (mode) {
	case TRLE_DUMP:
		dest = trlePutDump(dest, nbuf);
		break;
	case TRLE_RUN:
		dest = trlePutRun(dest, nbuf - rstart, lastc, tcolor);
		break;
	}

	return _trlePutSize;
}

} // End of namespace Scumm

#endif // ENABLE_HE
