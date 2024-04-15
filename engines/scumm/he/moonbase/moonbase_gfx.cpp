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

#include "scumm/he/intern_he.h"
#include "scumm/he/wiz_he.h"
#include "scumm/he/moonbase/moonbase.h"

namespace Scumm {

#define MOONBASE_HANDLE_SKIP_PIXELS_STEP() {                            \
        /* Decompress bytes to do simple clipping... */                 \
        while (skipAmount > 0) {                                        \
            if ((runCount = *dataStream++) & 1) {                       \
                /* Handle the transparent color... */                   \
                runCount >>= 1;                                         \
                if (runCount > skipAmount) {                            \
                    runCount -= skipAmount;                             \
                    goto DoTransparentRun;                              \
                } else {                                                \
                    skipAmount -= runCount;                             \
                }                                                       \
        	} else if (runCount & 2) {                                  \
                /* Handle a run of color... */                          \
                runCount = (runCount >> 2) + 1;                         \
                if (runCount > skipAmount) {                            \
                    runCount -= skipAmount;                             \
                    goto WriteRunData;                                  \
                } else {                                                \
                    skipAmount -= runCount;                             \
                    dataStream += sizeof(WizRawPixel16);                \
                }                                                       \
        	} else {                                                    \
                /* Handle a literal run of pixels... */                 \
                runCount = (runCount >> 2) + 1;                         \
                if (runCount > skipAmount) {                            \
                    runCount -= skipAmount;                             \
                    dataStream += (skipAmount * sizeof(WizRawPixel16)); \
                    goto WriteLiteralData;                              \
                } else {                                                \
                    skipAmount -= runCount;                             \
                    dataStream += (runCount * sizeof(WizRawPixel16));   \
                }                                                       \
            }                                                           \
        }                                                               \
    }

#define MOONBASE_HANDLE_RUN_DECOMPRESS_STEP(_TransparentCode_, _RunCode_, _LiteralCode_) {  \
        while (decompAmount > 0) {                                                          \
            runCount = *dataStream++;                                                       \
            if (runCount & 1) { /* xxxxxxx1 */                                              \
                runCount >>= 1;                                                             \
            DoTransparentRun:                                                               \
                decompAmount -= runCount;                                                   \
                _TransparentCode_                                                           \
            } else if (runCount & 2) { /* xxxxxx10 */                                       \
                runCount = (runCount >> 2) + 1;                                             \
            WriteRunData:                                                                   \
                decompAmount -= runCount;                                                   \
                if (decompAmount < 0) {                                                     \
                    runCount += decompAmount;                                               \
                }                                                                           \
                _RunCode_                                                                   \
                    dataStream += sizeof(WizRawPixel16);                                    \
            } else { /* xxxxxx00 */                                                         \
            	runCount = (runCount >> 2) + 1;                                             \
            WriteLiteralData:                                                               \
                decompAmount -= runCount;                                                   \
                if (decompAmount < 0) {                                                     \
                    runCount += decompAmount;                                               \
                }                                                                           \
                _LiteralCode_                                                               \
                    dataStream += (runCount * sizeof(WizRawPixel16));                       \
            }                                                                               \
        }                                                                                   \
    }

static bool canMoonbaseDrawWizType(int compressionType) {
	return (kWCTTRLE16Bpp == compressionType) ||
			(kWCTComposite == compressionType) ||
			(kWCTDataBlockDependent == compressionType);
}

static bool getMoonbaseWizSizeAndType(ScummEngine_v71he *vm, WizImage *wizPtr, int state, int32 &sizeX, int32 &sizeY, int32 &compType) {
	byte *workPtr = vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), wizPtr->data, state, false);

	if (!workPtr) {
		sizeX    = 0;
		sizeX    = 0;
		compType = 0;

		return false;
	}

	compType = READ_LE_UINT32(workPtr + 0);
	sizeX    = READ_LE_UINT32(workPtr + 4);
	sizeY    = READ_LE_UINT32(workPtr + 8);

	return true;
}

bool Wiz::drawLayeredWiz(
	byte *pDstBitmapData, int nDstWidth, int nDstHeight, int nDstPitch,
	int nDstFormat, int nDstBpp, byte *pWizImageData,
	int x, const int y, int state, int clip_x1, int clip_y1, int clip_x2, int clip_y2,
	uint32 dwFlags, uint32 dwConditionBits, byte *p8BppToXBppClut, byte *pAltSourceBuffer) {

	// Build a header to use with the WTOOLKIT functions
	WizImage wiz;

	wiz.data = pWizImageData;
	wiz.dataSize = READ_BE_UINT32(wiz.data + 4);

	// Check to see if this is a compression type we like
	byte *data = _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), wiz.data, state, false);
	assert(data);

	int stateCompressionType = READ_LE_UINT32(data);

	if (!canMoonbaseDrawWizType(stateCompressionType)) {
		return false;
	}

	// Make sure we can map the multitype bitmap to a rawbitmap...
	WizRawBitmap mappedRawbitmap;

	mappedRawbitmap.data = (WizRawPixel16 *)pDstBitmapData;
	mappedRawbitmap.width = nDstWidth;
	mappedRawbitmap.height = nDstHeight;
	mappedRawbitmap.dataSize = (mappedRawbitmap.width * sizeof(WizRawPixel16)) * mappedRawbitmap.height;

	if (nDstPitch != (mappedRawbitmap.width * sizeof(WizRawPixel16))) {
		return false;
	}

	Common::Rect clipRect(clip_x1, clip_y1, clip_x2, clip_y2);

	// Dispatch to the WToolkit image renderer
	drawImageEx(&mappedRawbitmap, &wiz, x, y, state, &clipRect, dwFlags, nullptr, dwConditionBits, (WizRawPixel16 *)p8BppToXBppClut, pAltSourceBuffer);

	// Assume if we're here that we did something
	return true;
}

void Wiz::drawImageEx(
	WizRawBitmap *bitmapPtr, WizImage *wizPtr, int x, int y, int state,
	Common::Rect *clipRectPtr, int32 flags, Common::Rect *optionalSrcRect,
	int32 conditionBits, WizRawPixel16 *p8BppToXBppClut, byte *pAltSourceBuffer) {
	WizMoonbaseCompressedImage fakedHeader;
	int nStateCompressionType;
	WizRawBitmap fakedBitmap;
	Common::Rect src, dstRect;
	int32 sizeX, sizeY;

	if (getMoonbaseWizSizeAndType(((ScummEngine_v71he *)_vm), wizPtr, state, sizeX, sizeY, nStateCompressionType)) {

		if (nStateCompressionType == kWCTDataBlockDependent) {
			// Find the data block
			byte *pCompressedDataPtr = _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), wizPtr->data, state, false);

			// If we have data (assume it's T14 for now...)
			if (pCompressedDataPtr) {
				// Massage the data to be 'U32' friendly
				pCompressedDataPtr += _vm->_resourceHeaderSize;

				Common::Rect clippingRect;
				Common::Rect *pClippingRect;

				if (clipRectPtr) {
					clippingRect.left = clipRectPtr->left;
					clippingRect.top = clipRectPtr->top;
					clippingRect.right = (clipRectPtr->right + 1);
					clippingRect.bottom = (clipRectPtr->bottom + 1);

					pClippingRect = &clippingRect;
				} else {
					pClippingRect = nullptr;
				}

				// Convert incoming condition bits to ROP's and params
				int rawROP = (conditionBits & kWMSBRopMask);

				int nROPParam = ((conditionBits & kWMSBReservedBits) >> kWMSBRopParamRShift);

				conditionBits &= ~kWMSBReservedBits;

				int nROP;

				switch (rawROP) {

				default:
				case 1: // MMX copy
					nROP = T14_MMX_PREMUL_ALPHA_COPY;
					break;

				case 2: // additive
					nROP = T14_MMX_ADDITIVE;
					break;

				case 3: // subtractive
					nROP = T14_MMX_SUBTRACTIVE;
					break;

				case 4: // over all alpha
					nROP = T14_MMX_CONSTANT_ALPHA;
					break;

				case 5: // MMX cheap 50:50 copy
					nROP = T14_MMX_CHEAP_50_50;
					break;

				case 6: // NON MMX copy
					nROP = T14_COPY;
					break;

				case 7: // NON MMX copy
					nROP = T14_CHEAP_50_50;
					break;
				}

				// Finally dispatch to the T14 handler
				//DISPATCH_Blit_RGB555(bitmapPtr->data, bitmapPtr->width, bitmapPtr->height,
				//	(bitmapPtr->width * sizeof(WizRawPixel16)), pClippingRect,
				//	pCompressedDataPtr, x, y, nROP, nROPParam, pAltSourceBuffer);
			}
		} else {
			byte *wizdBlockPtr = _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'D'), wizPtr->data, state, false) - _vm->_resourceHeaderSize;

			if (wizdBlockPtr) {
				// Fake up the compressed image header
				fakedHeader.data = wizdBlockPtr + _vm->_resourceHeaderSize;
				fakedHeader.transparentColor = 5;
				fakedHeader.width = sizeX;
				fakedHeader.height = sizeY;
				fakedHeader.size = READ_BE_UINT32(wizdBlockPtr + 4);
				fakedHeader.type = 0x100; // COMPRESSION_TYPE_SRLE

				// Clip the rects passed in...
				makeSizedRect(&src, sizeX, sizeY);

				if (optionalSrcRect) {
					if (!findRectOverlap(&src, optionalSrcRect)) {
						return;
					}
				}

				// Get down to business and draw this image :-)
				switch (nStateCompressionType) {
				case kWCTNone:
					break; // Explicitly unhandled

				case kWCTTRLE:
					if (p8BppToXBppClut) {
						error("Hey! This is actually used, implement it!");
						//TRLEFLIP_Decompress8BppToXBpp(
						//	bitmapPtr, &fakedHeader, x, y, &src, clipRectPtr, flags,
						//	p8BppToXBppClut);
					}
					
					break;
				case kWCTNone16Bpp:
					if (getRawBitmapInfoForState(&fakedBitmap, wizPtr, state)) {
						makeSizedRectAt(&dstRect, x, y, getRectWidth(&src), getRectHeight(&src));

						if (flags & kWRFHFlip) {
							swapRectX(&dstRect);
						}

						if (flags & kWRFVFlip) {
							swapRectY(&dstRect);
						}

						// How should transparency be managed?
						rawBitmapBlit(bitmapPtr, &dstRect, &fakedBitmap, &src);
					}
					break;

				case kWCTComposite:
					//_HandleCompositeDrawImage(
					//	bitmapPtr, wizPtr, wizdBlockPtr,
					//	x, y, &src, clipRectPtr, flags,
					//	conditionBits, &size,
					//	p8BppToXBppClut,
					//	pAltSourceBuffer);
					break;

				case kWCTTRLE16Bpp:
					trleFLIPDecompressMoonbaseImage(bitmapPtr, &fakedHeader, x, y, &src, clipRectPtr, flags);
					break;

				default:
					break;
				}
			}
		}
	}
}

bool Wiz::getRawBitmapInfoForState(WizRawBitmap *bitmapPtr, WizImage *wizPtr, int state) {
	byte *workPtr;
	int32 sizeX, sizeY, compType;

	bool wizValid = getMoonbaseWizSizeAndType(_vm, wizPtr, state, sizeX, sizeY, compType);

	// Make sure the compression type is RAW
	if (compType != kWCTNone16Bpp) {
		return false;
	}

	// Make sure that the wiz has size
	if (!wizValid) {
		return false;
	}

	// Get the data block for the state
	workPtr = _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), wizPtr->data, state, false);

	if (!workPtr) {
		return false;
	}

	// Build the bitmap struct for this state.
	bitmapPtr->data = (WizRawPixel16 *)workPtr;
	bitmapPtr->dataSize = sizeX * sizeY;
	bitmapPtr->width = sizeX;
	bitmapPtr->height = sizeY;

	return true;
}

void Wiz::rawBitmapBlit(WizRawBitmap *dstBitmap, Common::Rect *dstRectPtr, WizRawBitmap *srcBitmap, Common::Rect *srcRectPtr) {
	Common::Rect clipRect, dstRect, srcRect;
	int x, cw, dw, sw, ch, dxm, dym, ca;
	WizRawPixel16 *s;
	WizRawPixel16 *d;

	dstRect = *dstRectPtr;
	srcRect = *srcRectPtr;
	makeSizedRect(&clipRect, dstBitmap->width, dstBitmap->height);

	if (findRectOverlap(&dstRect, &clipRect)) {
		// Adjust the source coords to the clipped dest coords...
		dxm = (srcRect.left <= srcRect.right) ? 1 : -1;
		dym = (srcRect.top <= srcRect.bottom) ? 1 : -1;
		moveRect(&srcRect, (dstRect.left - dstRectPtr->left) * dxm, (dstRect.top - dstRectPtr->top) * dym);

		// Common calcs...
		dw = dstBitmap->width;
		sw = srcBitmap->width;
		cw = getRectWidth(&dstRect);
		ch = getRectHeight(&dstRect);
		d = dstBitmap->data + (((dstBitmap)->width * (dstRect.top) + (dstRect.left)));
		s = srcBitmap->data + (((srcBitmap)->width * (srcRect.top) + (srcRect.left)));
		ca = cw * sizeof(WizRawPixel16);

		// Going up or down?
		if (srcRect.top > srcRect.bottom) {
			sw = -sw;
		}

		// Left or right?
		if (srcRect.left <= srcRect.right) {
			while (--ch >= 0) {
				memcpy(d, s, ca);

				d += dw;
				s += sw;
			}

		} else {
			dw -= cw;
			sw += cw;

			while (--ch >= 0) {
				for (x = cw; --x >= 0;) {
					*d++ = *s--;
				}

				d += dw;
				s += sw;
			}
		}
	}
}

static void trleFLIPMoonbaseBackwardsPixelCopy(WizRawPixel16 *dstPtr, WizRawPixel16 *srcPtr, int size) {
	while (size-- > 0) {
		*dstPtr-- = *srcPtr++;
	}
}

static void trleFLIPDecompressMoonbaseLineForward(Wiz *wiz, WizRawPixel16 *destPtr, byte *dataStream, int skipAmount, int decompAmount, byte *extraPtr) {
	int runCount;

	// Decompress bytes to do simple clipping...
	MOONBASE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	MOONBASE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			destPtr += runCount;
		},
		{
			wiz->rawPixelMemset(destPtr, READ_LE_UINT16(dataStream), runCount);
			destPtr += runCount;
		},
		{
			memcpy(destPtr, dataStream, runCount * sizeof(WizRawPixel16));
			destPtr += runCount;
		}
	);
}

static void trleFLIPDecompressMoonbaseLineBackward(Wiz *wiz, WizRawPixel16 *destPtr, byte *dataStream, int skipAmount, int decompAmount, byte *extraPtr) {
	int runCount;

	// Decompress bytes to do simple clipping...
	MOONBASE_HANDLE_SKIP_PIXELS_STEP();

	// Really decompress to the dest buffer...
	MOONBASE_HANDLE_RUN_DECOMPRESS_STEP(
		{
			destPtr -= runCount;
		},
		{
			destPtr -= runCount;
			wiz->rawPixelMemset(destPtr + 1, READ_LE_UINT16(dataStream), runCount);
		},
		{
			trleFLIPMoonbaseBackwardsPixelCopy(destPtr, (WizRawPixel16 *)dataStream, runCount);
			destPtr -= runCount;
		}
	);
}

void Wiz::trleFLIPDecompressMoonbaseImage(
	WizRawBitmap *bitmapPtr, WizMoonbaseCompressedImage *imagePtr, int destX, int destY,
	Common::Rect *sourceCoords, Common::Rect *clipRectPtr, int32 flags) {

	Common::Rect sourceRect, destRect, clipRect, workRect, inSourceRect;
	int width, height, bufferWidth, bufferHeight;

	// Lame ass general setup...
	bufferWidth = bitmapPtr->width;
	bufferHeight = bitmapPtr->height;

	if (!sourceCoords) {
		width = imagePtr->width;
		height = imagePtr->height;
		makeSizedRect(&sourceRect, width, height);
	} else {
		width = getRectWidth(sourceCoords);
		height = getRectHeight(sourceCoords);
		sourceRect = *sourceCoords;
	}

	inSourceRect = sourceRect;
	makeSizedRectAt(&destRect, destX, destY, width, height);

	// Custom clip rect...
	if (clipRectPtr) {
		clipRect = *clipRectPtr;
		makeSizedRect(&workRect, bufferWidth, bufferHeight);

		if (!findRectOverlap(&clipRect, &workRect)) {
			return;
		}
	} else {
		makeSizedRect(&clipRect, bufferWidth, bufferHeight);
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
	bool useForwardFunction = true;

	if (flags & kWRFHFlip) {
		useForwardFunction = false;
		horzFlipAlignWithRect(&sourceRect, &inSourceRect);
		swapRectX(&destRect);
	}

	if (flags & kWRFVFlip) {
		vertFlipAlignWithRect(&sourceRect, &inSourceRect);
		swapRectY(&destRect);
	}

	// Call the primitive image renderer...
	if (useForwardFunction) {
		trleFLIPDecompMoonbaseImageHull(
			bitmapPtr->data, bufferWidth, &destRect, imagePtr->data,
			&sourceRect, nullptr, trleFLIPDecompressMoonbaseLineForward);
	} else {
		trleFLIPDecompMoonbaseImageHull(
			bitmapPtr->data, bufferWidth, &destRect, imagePtr->data,
			&sourceRect, nullptr, trleFLIPDecompressMoonbaseLineBackward);
	}
}

void Wiz::trleFLIPDecompMoonbaseImageHull(
	WizRawPixel16 *bufferPtr, int bufferWidth, Common::Rect *destRect, byte *compData, Common::Rect *sourceRect, byte *extraPtr,
	void (*functionPtr)(Wiz *wiz, WizRawPixel *destPtr, byte *dataStream, int skipAmount, int decompAmount, byte *extraPtr)) {

	int decompWidth, decompHeight, counter, x1, lineSize;

	// General setup...
	x1 = sourceRect->left;
	decompWidth = sourceRect->right - sourceRect->left + 1;
	decompHeight = sourceRect->bottom - sourceRect->top + 1;

	// Quickly skip down to the lines to be compressed & dest position...
	bufferPtr += bufferWidth * destRect->top + destRect->left;

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
			(*functionPtr)(this, bufferPtr, compData + 2, x1, decompWidth, extraPtr);
			compData += lineSize + 2;
			bufferPtr += bufferWidth;
		} else {
			// Handle a completely transparent line!
			compData += 2;
			bufferPtr += bufferWidth;
		}
	}
}

} // End of namespace Scumm
