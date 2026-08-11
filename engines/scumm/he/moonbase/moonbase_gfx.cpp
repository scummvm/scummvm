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

enum MoonbaseDistortionTypes {
	kMDTEdgeReflectionClipped = 0,
	kMDTNotClipped = 1,
	kMDTSpecializedNotClipped = 2
};

#define T14_MMX_REQUIRED              (0x8000)
#define T14_NOP                       (0x0000)
#define T14_COPY                      (0x0001)
#define T14_CHEAP_50_50               (0x0002)
#define T14_PREMULTIPLIED_5050        (0x0003)
#define T14_MMX_COPY                  (0x0004 | T14_MMX_REQUIRED)
#define T14_MMX_CHEAP_50_50           (0x0005 | T14_MMX_REQUIRED)
#define T14_MMX_PREMULTIPLIED_5050    (0x0006 | T14_MMX_REQUIRED)
#define T14_MMX_ADDITIVE              (0x0007 | T14_MMX_REQUIRED)
#define T14_MMX_SUBTRACTIVE           (0x0008 | T14_MMX_REQUIRED)
#define T14_MMX_CONSTANT_ALPHA        (0x0009 | T14_MMX_REQUIRED)
#define T14_MMX_SILHOUETTE_DARKEN     (0x000A | T14_MMX_REQUIRED)
#define T14_MMX_SILHOUETTE_BRIGHTEN   (0x000B | T14_MMX_REQUIRED)
#define T14_MMX_PREMUL_ALPHA_COPY     (0x000C | T14_MMX_REQUIRED)

#define SIZE_OF_BPTLLC_VALIDATION_HEADER    8
#define BPTLLC_CURRENT_DATA_REVISION        2
#define BPTLLC_CURRENT_REVISION_MASK        0x000000FF

#define BPTLLC_ID_T14_TYPE                  (uint32)(0x12340100 | (BPTLLC_CURRENT_DATA_REVISION))
#define BPTLLC_ID_RAW_555_DISTORT_TYPE      (uint32)(0x12340800 | (BPTLLC_CURRENT_DATA_REVISION))
#define BPTLLC_ID_TRLE_555_DISTORT_TYPE     (uint32)(0x12340900 | (BPTLLC_CURRENT_DATA_REVISION))

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
	return  (kWCTTRLE16Bpp          == compressionType) ||
			(kWCTComposite          == compressionType) ||
			(kWCTDataBlockDependent == compressionType);
}

static bool getMoonbaseWizSizeAndType(ScummEngine_v71he *vm, WizImage *wizPtr, int state, int32 &sizeX, int32 &sizeY, int32 &compType) {
	byte *workPtr = vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), wizPtr->data, state, false);

	if (!workPtr) {
		sizeX    = 0;
		sizeY    = 0;
		compType = 0;

		return false;
	}

	compType = READ_LE_UINT32(workPtr + 0);
	sizeX    = READ_LE_UINT32(workPtr + 4);
	sizeY    = READ_LE_UINT32(workPtr + 8);

	return true;
}

bool Wiz::drawMoonbaseLayeredWiz(
	byte *dstBitmapData, int dstWidth, int dstHeight, int dstPitch,
	int dstFormat, int dstBpp, byte *wizImageData,
	int x, const int y, int state, int clipX1, int clipY1, int clipX2, int clipY2,
	uint32 flags, uint32 conditionBits, byte *ptr8BppToXBppClut, byte *altSourceBuffer) {

	WizImage wiz;

	wiz.data = wizImageData;
	wiz.dataSize = READ_BE_UINT32(wiz.data + 4);

	// Check to see if this is a valid compression type...
	byte *data = _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), wiz.data, state, false);
	assert(data);

	int stateCompressionType = READ_LE_UINT32(data);

	if (!canMoonbaseDrawWizType(stateCompressionType)) {
		return false;
	}

	// Make sure we can map the multitype bitmap to a rawbitmap...
	WizRawBitmap mappedRawbitmap;

	mappedRawbitmap.data = (WizRawPixel16 *)dstBitmapData;
	mappedRawbitmap.width = dstWidth;
	mappedRawbitmap.height = dstHeight;
	mappedRawbitmap.dataSize = (mappedRawbitmap.width * sizeof(WizRawPixel16)) * mappedRawbitmap.height;

	if (dstPitch != (int)(mappedRawbitmap.width * sizeof(WizRawPixel16))) {
		return false;
	}

	Common::Rect clipRect;
	// We are not directly assigning the rectangle values
	// in the constructor: the game can (and will!) assign
	// values which will trigger the "invalid rectangle" assertion...
	clipRect.left = clipX1;
	clipRect.top = clipY1;
	clipRect.right = clipX2;
	clipRect.bottom = clipY2;

	// Dispatch to the image renderer...
	drawMoonbaseImageEx(&mappedRawbitmap, &wiz, x, y, state, &clipRect, flags, nullptr, conditionBits, (WizRawPixel16 *)ptr8BppToXBppClut, altSourceBuffer);

	// Assume if we're here that we did something...
	return true;
}

void Wiz::drawMoonbaseImageEx(
	WizRawBitmap *bitmapPtr, WizImage *wizPtr, int x, int y, int state,
	Common::Rect *clipRectPtr, int32 flags, Common::Rect *optionalSrcRect,
	uint32 conditionBits, WizRawPixel16 *ptr8BppToXBppClut, byte *altSourceBuffer) {
	WizMoonbaseCompressedImage fakedHeader;
	int32 stateCompressionType;
	WizRawBitmap fakedBitmap;
	Common::Rect src, dstRect;
	int32 sizeX, sizeY;

	if (getMoonbaseWizSizeAndType(((ScummEngine_v71he *)_vm), wizPtr, state, sizeX, sizeY, stateCompressionType)) {
		if (stateCompressionType == kWCTDataBlockDependent) {
			// Find the data block
			byte *compressedDataPtr = _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'D'), wizPtr->data, state, false);

			// If we have data (assume it's T14 for now...)
			if (compressedDataPtr) {
				Common::Rect clippingRect;
				Common::Rect *targetClippingRect;

				if (clipRectPtr) {
					clippingRect.left = clipRectPtr->left;
					clippingRect.top = clipRectPtr->top;
					clippingRect.right = (clipRectPtr->right + 1);
					clippingRect.bottom = (clipRectPtr->bottom + 1);

					targetClippingRect = &clippingRect;
				} else {
					targetClippingRect = nullptr;
				}

				// Convert incoming condition bits to ROP's and params
				int rawROP = (conditionBits & kWMSBRopMask);
				int nROPParam = ((conditionBits & kWMSBReservedBits) >> kWMSBRopParamRShift);
				conditionBits &= ~kWMSBReservedBits;

				int nROP = T14_NOP;

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
				dispatchBlitRGB555((byte *)bitmapPtr->data, bitmapPtr->width, bitmapPtr->height,
					(bitmapPtr->width * sizeof(WizRawPixel16)), targetClippingRect,
					compressedDataPtr, x, y, nROP, nROPParam, altSourceBuffer);
			}
		} else {
			byte *wizdBlockPtr = _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'D'), wizPtr->data, state, false);

			if (wizdBlockPtr) {
				wizdBlockPtr -= _vm->_resourceHeaderSize;

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

				// Get down to business and draw the image...
				switch (stateCompressionType) {
				case kWCTNone:
					break; // Explicitly unhandled

				case kWCTTRLE:
					if (ptr8BppToXBppClut) {
						error("Wiz::drawMoonbaseImageEx(): unimplemented development path trleFLIPDecompress8BppToXBpp()!");
					}
					
					break;
				case kWCTNone16Bpp:
					if (getRawMoonbaseBitmapInfoForState(&fakedBitmap, wizPtr, state)) {
						makeSizedRectAt(&dstRect, x, y, getRectWidth(&src), getRectHeight(&src));

						if (flags & kWRFHFlip) {
							swapRectX(&dstRect);
						}

						if (flags & kWRFVFlip) {
							swapRectY(&dstRect);
						}

						// How should transparency be managed?
						rawMoonbaseBitmapBlit(bitmapPtr, &dstRect, &fakedBitmap, &src);
					}
					break;

				case kWCTComposite:
					handleCompositeDrawMoonbaseImage(
						bitmapPtr, wizPtr, wizdBlockPtr, x, y, &src, clipRectPtr, flags,
						conditionBits, sizeX, sizeY, ptr8BppToXBppClut, altSourceBuffer);

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

bool Wiz::getRawMoonbaseBitmapInfoForState(WizRawBitmap *bitmapPtr, WizImage *wizPtr, int state) {
	byte *workPtr;
	int32 sizeX, sizeY, compType;

	bool wizValid = getMoonbaseWizSizeAndType(_vm, wizPtr, state, sizeX, sizeY, compType);

	// Make sure this is a raw image...
	if (compType != kWCTNone16Bpp) {
		return false;
	}

	// Make sure that the Wiz is valid...
	if (!wizValid) {
		return false;
	}

	// Get the data block for the state...
	workPtr = _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), wizPtr->data, state, false);

	if (!workPtr) {
		return false;
	}

	// Build the bitmap struct for this state...
	bitmapPtr->data = (WizRawPixel16 *)workPtr;
	bitmapPtr->dataSize = sizeX * sizeY;
	bitmapPtr->width = sizeX;
	bitmapPtr->height = sizeY;

	return true;
}

void Wiz::rawMoonbaseBitmapBlit(WizRawBitmap *dstBitmap, Common::Rect *dstRectPtr, WizRawBitmap *srcBitmap, Common::Rect *srcRectPtr) {
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
				for (x = 0; x < ca; x++) {
					d[x] = FROM_LE_16(s[x]);
				}

				d += dw;
				s += sw;
			}

		} else {
			dw -= cw;
			sw += cw;

			while (--ch >= 0) {
				for (x = cw; --x >= 0;) {
					*d++ = FROM_LE_16(*s--);
				}

				d += dw;
				s += sw;
			}
		}
	}
}

static void trleFLIPMoonbaseBackwardsPixelCopy(WizRawPixel16 *dstPtr, WizRawPixel16 *srcPtr, int size) {
	while (size-- > 0) {
		*dstPtr-- = FROM_LE_16(*srcPtr++);
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
			wiz->rawPixelMemset(destPtr, FROM_LE_16(*((uint16 *)dataStream)), runCount);
			destPtr += runCount;
		},
		{
			for (int i = 0; i < runCount; i++) {
				destPtr[i] = FROM_LE_16(((WizRawPixel16 *)dataStream)[i]);
			}
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
			wiz->rawPixelMemset(destPtr + 1, FROM_LE_16(*((uint16 *)dataStream)), runCount);
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

bool Wiz::moonbaseLayeredWizHitTest(int32 *outValue, int32 *optionalOutActualValue, byte *globPtr, int state, int x, int y, int32 flags, uint32 conditionBits) {
	WizImage wiz;

	wiz.data = globPtr;
	wiz.dataSize = READ_BE_UINT32(wiz.data + 4);

	// Check to see if this is a compression type we like
	byte *data = _vm->findWrappedBlock(MKTAG('W', 'I', 'Z', 'H'), wiz.data, state, false);
	assert(data);

	int stateCompressionType = READ_LE_UINT32(data);

	if (!canMoonbaseDrawWizType(stateCompressionType)) {
		return false;
	}

	// Use the layered renderer to do hit-tests, so that all compression
	// types supported by the U32 are supported...
	int32 chroma = ~0;

	int32 pixel = chroma;

	drawMoonbaseLayeredWiz((byte *)&pixel, 1, 1, sizeof(WizRawPixel16), 555, 16,
		globPtr, -x, -y, state, 0, 0, 0, 0, flags, conditionBits, 0, 0);

	if (chroma != pixel) {
		*outValue = 1;
	} else {
		*outValue = 0;
	}

	if (optionalOutActualValue) {
		*optionalOutActualValue = (int32)((uint16)pixel);
	}

	return true;
}

void Wiz::handleCompositeDrawMoonbaseImage(
	WizRawBitmap *bitmapPtr, WizImage *wizPtr, byte *compositeInfoBlockPtr,
	int x, int y, Common::Rect *srcRect, Common::Rect *clipRect,
	int32 flags, uint32 conditionBits, int32 outerSizeX, int32 outerSizeY,
	WizRawPixel16 *ptr8BppToXBppClut, byte *altSourceBuffer) {

	int layerCount, xPos, yPos, subState, cmdSize;
	uint32 layerConditionBits, layerCmdDataBits;
	uint32 subConditionBits, drawFlags;
	WizImage nestedMultiStateWiz;
	byte *nestedBlockHeader;
	byte *nestedWizHeader;
	byte *cmdPtr;
	uint32 conditionType;
	int32 stateSizeX = 0, stateSizeY = 0;

	// Get the nested block...	
	nestedBlockHeader = _vm->heFindResource(MKTAG('N', 'E', 'S', 'T'), wizPtr->data);

	if (!nestedBlockHeader) {
		return;
	}

	// Get the real nested wiz...
	nestedWizHeader = _vm->heFindResource(MKTAG('M', 'U', 'L', 'T'), nestedBlockHeader);

	if (!nestedWizHeader) {
		return;
	}
	
	// Build a fake wiz header...
	nestedMultiStateWiz.data = nestedWizHeader;
	nestedMultiStateWiz.dataSize = READ_BE_UINT32(nestedWizHeader + 4);

	// Process the composite command block...
	compositeInfoBlockPtr += _vm->_resourceHeaderSize;

	// Get the number of layers in this command block...
	layerCount = READ_LE_UINT16(compositeInfoBlockPtr);
	compositeInfoBlockPtr += 2;


	// Deal with the moonbase 'system' bits...
	int32 defaultSubConditionBits = (conditionBits & kWMSBReservedBits);
	conditionBits &= ~kWMSBReservedBits;

	// Process each layer...
	for (int layerCounter = 0; layerCounter < layerCount; layerCounter++) {
		// Get the command and move down by the cmd size
		cmdSize = READ_LE_UINT16(compositeInfoBlockPtr);
		cmdPtr = compositeInfoBlockPtr + 2;
		compositeInfoBlockPtr += (cmdSize + 2);

		// Get the cmd flags
		layerCmdDataBits = READ_LE_UINT32(cmdPtr);
		cmdPtr += 4;

		// Check for condition bits
		if (layerCmdDataBits & kWCFConditionBits) {
			layerConditionBits = READ_LE_UINT32(cmdPtr);
			cmdPtr += 4;

			// Check to see if the layer overrides the default system bits
			subConditionBits = (layerConditionBits & kWMSBReservedBits);
			layerConditionBits &= ~kWMSBReservedBits;

			if (subConditionBits == 0) {
				subConditionBits = defaultSubConditionBits;
			}

			// Get the condition bits and strip them from the bits...
			conditionType = (layerConditionBits & kWSPCCTBits);
			layerConditionBits &= ~kWSPCCTBits;

			// Perform the actual compare for the bits...
			switch (conditionType) {

			default:
			case kWSPCCTOr:
				if (!(layerConditionBits & conditionBits)) {
					continue;
				}

				break;

			case kWSPCCTAnd:
				if (layerConditionBits != (layerConditionBits & conditionBits)) {
					continue;
				}

				break;

			case kWSPCCTNot:
				if (layerConditionBits & conditionBits) {
					continue;
				}

				break;
			}
		} else {
			subConditionBits = defaultSubConditionBits;
		}

		// Get the sub state
		if (layerCmdDataBits & kWCFSubState) {
			subState = READ_LE_UINT16(cmdPtr);
			cmdPtr += 2;
		} else {
			subState = 0;
		}

		// Get the X delta
		if (layerCmdDataBits & kWCFXDelta) {
			xPos = (int16)READ_LE_UINT16(cmdPtr);
			cmdPtr += 2;
		} else {
			xPos = 0;
		}

		// Get the Y delta
		if (layerCmdDataBits & kWCFYDelta) {
			yPos = (int16)READ_LE_UINT16(cmdPtr);
			cmdPtr += 2;
		} else {
			yPos = 0;
		}

		// Get the drawing flags
		if (layerCmdDataBits & kWCFDrawFlags) {
			drawFlags = READ_LE_UINT32(cmdPtr);
			cmdPtr += 4;
		} else {
			drawFlags = flags;
		}

		// Based on the drawing flags adjust the blit position
		if (drawFlags & (kWRFHFlip | kWRFVFlip)) {
			int32 compressionType = 0;
			if (!getMoonbaseWizSizeAndType(_vm, &nestedMultiStateWiz, subState, stateSizeX, stateSizeY, compressionType)) {
				return;
			}
		}

		if (drawFlags & kWRFHFlip) {
			xPos = (outerSizeX - (xPos + stateSizeX));
		}

		if (drawFlags & kWRFVFlip) {
			yPos = (outerSizeY - (yPos + stateSizeY));
		}

		// Are there any sub condition bits?
		if (layerCmdDataBits & kWCFSubConditionBits) {
			subConditionBits = READ_LE_UINT32(cmdPtr);
			cmdPtr += 4;
		}

		// Finally do the actual command...
		drawMoonbaseImageEx(
			bitmapPtr, &nestedMultiStateWiz, (x + xPos), (y + yPos), subState, clipRect,
			drawFlags, nullptr, subConditionBits, ptr8BppToXBppClut, altSourceBuffer);

	}
}

void Wiz::dispatchBlitRGB555(
	byte *bufferData, int bufferWidth, int bufferHeight, int bufferPitch,
	Common::Rect *optionalClippingRect, byte *compressedDataStream,
	int x, int y, int nROP, int nROPParam, byte *altSourceBuffer) {

	if (compressedDataStream) {
		uint32 id = READ_LE_UINT32(compressedDataStream);

		if (id == BPTLLC_ID_T14_TYPE) {
			blitT14CodecImage(
				bufferData, bufferWidth, bufferHeight, bufferPitch, optionalClippingRect, compressedDataStream, x, y, nROP, nROPParam);
		} else if (id == BPTLLC_ID_RAW_555_DISTORT_TYPE) {
			blitDistortion(
				bufferData, bufferWidth, bufferHeight, bufferPitch, optionalClippingRect, compressedDataStream, x, y, altSourceBuffer);
		} else if (id == BPTLLC_ID_TRLE_555_DISTORT_TYPE) {
			error("Unimplemented development path!");
		}
	}
}

void Wiz::blitT14CodecImage(byte *dst, int dstw, int dsth, int dstPitch, const Common::Rect *clipBox, byte *wizd, int x, int y, int rawROP, int paramROP) {
	bool premulAlpha = false;

	if (rawROP == T14_MMX_PREMUL_ALPHA_COPY)
		premulAlpha = true;

	Common::Rect clippedDstRect(dstw, dsth);
	if (clipBox) {
		Common::Rect clip(clipBox->left, clipBox->top, clipBox->right, clipBox->bottom);
		if (clippedDstRect.intersects(clip)) {
			clippedDstRect.clip(clip);
		} else {
			return;
		}
	}

	int width = READ_LE_UINT16(wizd + 0x8 + 0);
	int height = READ_LE_UINT16(wizd + 0x8 + 2);

	Common::Rect srcLimitsRect(width, height);
	Common::Rect dstOperation(x, y, x + width, y + height);
	if (!clippedDstRect.intersects(dstOperation))
		return;
	Common::Rect clippedRect = clippedDstRect.findIntersectingRect(dstOperation);

	int cx = clippedRect.right - clippedRect.left;
	int cy = clippedRect.bottom - clippedRect.top;

	int sx = ((clippedRect.left - x) + srcLimitsRect.left);
	int sy = ((clippedRect.top - y) + srcLimitsRect.top);

	dst += clippedRect.top * dstPitch + clippedRect.left * 2;

	int headerSize = READ_LE_UINT32(wizd + 0x4);
	uint8 *dataPointer = wizd + 0x8 + headerSize;

	for (int i = 0; i < sy; i++) {
		uint16 lineSize = READ_LE_UINT16(dataPointer + 0);

		dataPointer += lineSize;
	}

	for (int i = 0; i < cy; i++) {
		uint16 lineSize = READ_LE_UINT16(dataPointer + 0);
		uint8 *singlesOffset = READ_LE_UINT16(dataPointer + 2) + dataPointer;
		uint8 *quadsOffset = READ_LE_UINT16(dataPointer + 4) + dataPointer;

		int pixels = 0;
		byte *dst1 = dst;
		byte *codes = dataPointer + 6;

		while (1) {
			int code = *codes - 2;
			codes++;

			if (code <= 0) { // quad or single
				uint8 *src;
				int cnt;
				if (code == 0) { // quad
					src = quadsOffset;
					quadsOffset += 8;
					cnt = 4; // 4 pixels
				} else {     // single
					src = singlesOffset;
					singlesOffset += 2;
					cnt = 1;
				}

				for (int c = 0; c < cnt; c++) {
					if (pixels >= sx) {
						if (rawROP == T14_MMX_PREMUL_ALPHA_COPY) { // MMX_PREMUL_ALPHA_COPY
							WRITE_UINT16(dst1, READ_LE_UINT16(src));
						} else if (rawROP == T14_MMX_ADDITIVE) { // MMX_ADDITIVE
							uint16 color = READ_LE_UINT16(src);
							uint16 orig = READ_UINT16(dst1);

							uint32 r = MIN<uint32>(0x7c00, (orig & 0x7c00) + (color & 0x7c00));
							uint32 g = MIN<uint32>(0x03e0, (orig & 0x03e0) + (color & 0x03e0));
							uint32 b = MIN<uint32>(0x001f, (orig & 0x001f) + (color & 0x001f));
							WRITE_UINT16(dst1, (r | g | b));
						} else if (rawROP == T14_MMX_CHEAP_50_50) { // MMX_CHEAP_50_50
							uint16 color = (READ_LE_UINT16(src) >> 1) & 0x3DEF;
							uint16 orig = (READ_UINT16(dst1) >> 1) & 0x3DEF;
							WRITE_UINT16(dst1, (color + orig));
						}
						dst1 += 2;
					}
					src += 2;
					pixels++;
					if (pixels >= cx + sx)
						break;
				}
			} else { // skip
				if ((code & 1) == 0) {
					code >>= 1;

					for (int j = 0; j < code; j++) {
						if (pixels >= sx)
							dst1 += 2;
						pixels++;
						if (pixels >= cx + sx)
							break;
					}
				} else { // special case
					if (pixels >= sx) {
						int alpha = code >> 1;
						uint16 color = READ_LE_UINT16(singlesOffset);
						uint32 orig = READ_UINT16(dst1);

						if (!premulAlpha) {
							WRITE_UINT16(dst1, color); // ENABLE_PREMUL_ALPHA = 0
						} else {
							if (alpha > 32) {
								alpha -= 32;

								uint32 oR = orig & 0x7c00;
								uint32 oG = orig & 0x03e0;
								uint32 oB = orig & 0x1f;
								uint32 dR = ((((color & 0x7c00) - oR) * alpha) >> 5) + oR;
								uint32 dG = ((((color & 0x3e0) - oG) * alpha) >> 5) + oG;
								uint32 dB = ((((color & 0x1f) - oB) * alpha) >> 5) + oB;

								WRITE_UINT16(dst1, (dR & 0x7c00) | (dG & 0x3e0) | (dB & 0x1f));
							} else {
								uint32 pix = ((orig << 16) | orig) & 0x3e07c1f;
								pix = (((pix * alpha) & 0xffffffff) >> 5) & 0x3e07c1f;
								pix = ((pix >> 16) + pix + color) & 0xffff;
								WRITE_UINT16(dst1, pix);
							}
						}

						dst1 += 2;
					}

					singlesOffset += 2;
					pixels++;
				}
			}

			if (pixels >= cx + sx)
				break;
		}

		dataPointer += lineSize;
		dst += dstPitch;
	}
}

void Wiz::blitDistortion(
	byte *bufferData, int bufferWidth, int bufferHeight, int bufferPitch,
	Common::Rect *optionalClippingRect, byte *compressedDataStream,
	int x, int y, byte *altSourceBuffer) {

	int format = 555;
	int bpp = 16;
	int distortionFormat = 555;
	int distortionBpp = 16;

	// Setup the source pointer...
	byte *sourcePixels = (altSourceBuffer) ? altSourceBuffer : bufferData;

	// Clip the optional clipping rect to the dest bitmap limits...
	Common::Rect dstLimitsRect(0, 0, bufferWidth, bufferHeight);

	Common::Rect clippedDstRect;

	if (optionalClippingRect) {
		if (!dstLimitsRect.intersects(*optionalClippingRect)) {
			return;
		}

		clippedDstRect = dstLimitsRect;
		clippedDstRect.clip(*optionalClippingRect);
	} else {
		clippedDstRect = dstLimitsRect;
	}

	// Get the source operation size...
	int w = READ_LE_UINT16(compressedDataStream + SIZE_OF_BPTLLC_VALIDATION_HEADER + 0);
	int h = READ_LE_UINT16(compressedDataStream + SIZE_OF_BPTLLC_VALIDATION_HEADER + 2);

	Common::Rect srcLimitsRect(0, 0, w, h);
	Common::Rect clippedSrcRect = srcLimitsRect;

	// Perform a simple clipping operation to detect if we have to bail out early...
	Common::Rect dstOperation(x, y,
							  x + (clippedSrcRect.right - clippedSrcRect.left),
							  y + (clippedSrcRect.bottom - clippedSrcRect.top));

	Common::Rect clippedRect;

	if (!clippedDstRect.intersects(dstOperation)) {
		return;
	}

	clippedRect = clippedDstRect;
	clippedRect.clip(dstOperation);

	// Get the important header information...
	int subBlockCount = READ_LE_UINT16(compressedDataStream + SIZE_OF_BPTLLC_VALIDATION_HEADER + 4);
	byte *subBlockStream = compressedDataStream + SIZE_OF_BPTLLC_VALIDATION_HEADER + READ_LE_UINT32(compressedDataStream + 4);

	// Get our clip rect information...
	int cx1 = clippedDstRect.left;
	int cy1 = clippedDstRect.top;
	int cx2 = clippedDstRect.right - 1;
	int cy2 = clippedDstRect.bottom - 1;

	// Process each sub-block...
	for (int i = 0; i < subBlockCount; i++) {
		// Read the sub-block header...
		byte *blockData = subBlockStream;

		uint32 blockSize = READ_LE_UINT32(blockData); blockData += 4;
		subBlockStream += blockSize;

		int xOffset = READ_LE_UINT16(blockData); blockData += 2;
		int yOffset = READ_LE_UINT16(blockData); blockData += 2;
		int width   = READ_LE_UINT16(blockData); blockData += 2;
		int height  = READ_LE_UINT16(blockData); blockData += 2;
		int lReach  = READ_LE_UINT16(blockData); blockData += 2;
		int rReach  = READ_LE_UINT16(blockData); blockData += 2;
		int tReach  = READ_LE_UINT16(blockData); blockData += 2;
		int bReach  = READ_LE_UINT16(blockData); blockData += 2;
		int distortionPitch = ((width * distortionBpp + 7) / 8);

		// Check for special case...
		if ((width == 0) && (height == 0)) {
			continue;
		}

		// Call the blitter for the sub block...
		blitUncompressedDistortionBitmap(
			bufferData, bufferWidth, bufferHeight, bufferPitch, format, bpp,
			sourcePixels, bufferWidth, bufferHeight, bufferPitch, format, bpp,
			blockData, width, height, distortionPitch, distortionFormat, distortionBpp,
			(x + xOffset), (y + yOffset), (x + xOffset), (y + yOffset),
			lReach, rReach, tReach, bReach,
			cx1, cy1, cx2, cy2, cx1, cy1, cx2, cy2
		);
	}
}

void Wiz::blitUncompressedDistortionBitmap(
	byte *dstBitmapData, int dstWidth, int dstHeight, int dstPitch, int dstFormat, int dstBpp,
	byte *srcBitmapData, int srcWidth, int srcHeight, int srcPitch, int srcFormat, int srcBpp,
	byte *distortionBitmapData, int distortionWidth, int distortionHeight, int distortionPitch, int distortionFormat, int distortionBpp,
	int dstX, int dstY, int srcX, int srcY, int lReach, int rReach, int tReach, int bReach,
	int srcClipX1, int srcClipY1, int srcClipX2, int srcClipY2,
	int dstClipX1, int dstClipY1, int dstClipX2, int dstClipY2) {

	MoonbaseDistortionInfo distInfo;

	int gMax = ((1 << 5) - 1);
	int bMax = ((1 << 5) - 1);
	
	int xSignedAdjust = -(gMax / 2);
	int ySignedAdjust = -(bMax / 2);
	
	Graphics::Surface mappedDstBitmap;
	mappedDstBitmap.init(dstWidth, dstHeight, dstPitch, dstBitmapData, Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

	Graphics::Surface mappedSrcBitmap;
	mappedSrcBitmap.init(srcWidth, srcHeight, srcPitch, srcBitmapData, Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

	Graphics::Surface mappedDistortionBitmap;
	mappedDistortionBitmap.init(distortionWidth, distortionHeight, distortionPitch, distortionBitmapData, Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0));

	Common::Rect srcClipRect(srcClipX1, srcClipY1, srcClipX2, srcClipY2);
	Common::Rect dstClipRect(dstClipX1, dstClipY1, dstClipX2, dstClipY2);
	Common::Rect srcReach((srcX - lReach), (srcY - tReach), (srcX + rReach), (srcY + bReach));

	Common::Rect srcLimits(mappedSrcBitmap.w, mappedSrcBitmap.h);

	if (!findRectOverlap(&srcLimits, &srcClipRect)) {
		return;
	}

	if (!srcReach.intersects(srcLimits)) {
		return;
	}

	if (srcReach.contains(srcLimits)) {
		if (mappedSrcBitmap.pitch == 1280) {
			distInfo.baseX = 0;
			distInfo.baseY = 0;
			distInfo.srcPitch = mappedSrcBitmap.pitch;
			distInfo.clipRect = nullptr;
			distInfo.srcData = (byte *)mappedSrcBitmap.getBasePtr(
				(dstX - srcX) + xSignedAdjust, (dstY - srcY) + ySignedAdjust);

			distortionBlitCore(&mappedDstBitmap, dstX, dstY, &mappedDistortionBitmap, 0, &dstClipRect, kMDTSpecializedNotClipped, &distInfo);
		} else {
			distInfo.baseX = (dstX - srcX) + xSignedAdjust;
			distInfo.baseY = (dstY - srcY) + ySignedAdjust;
			distInfo.srcPitch = mappedSrcBitmap.pitch;
			distInfo.clipRect = nullptr;
			distInfo.srcData = (byte *)mappedSrcBitmap.getBasePtr(0, 0);

			distortionBlitCore(&mappedDstBitmap, dstX, dstY, &mappedDistortionBitmap, 0, &dstClipRect, kMDTNotClipped, &distInfo);
		}
	} else {
		distInfo.baseX = (dstX - srcX) + xSignedAdjust;
		distInfo.baseY = (dstY - srcY) + ySignedAdjust;
		distInfo.srcPitch = mappedSrcBitmap.pitch;
		distInfo.clipRect = &srcLimits;
		distInfo.srcData = (byte *)mappedSrcBitmap.getBasePtr(0, 0);

		distortionBlitCore(&mappedDstBitmap, dstX, dstY, &mappedDistortionBitmap, 0, &dstClipRect, kMDTEdgeReflectionClipped, &distInfo);
	}
}

static void distortionGetCoordinates(int s, int &x, int &y) {
	int G_Mask = (((1 << 5) - 1) << 5);
	int B_Mask = (((1 << 5) - 1) << 0);
	int G_LShift = 5;
	int B_LShift = 0;

	x = (s & G_Mask) >> G_LShift;
	y = (s & B_Mask) >> B_LShift;
}

static void distortionTransferOp(int transferOp, MoonbaseDistortionInfo *mdi, uint16 *d, uint16 *s, int dx, int dy) {
	int xx, yy, sx, sy;
	switch (transferOp) {
	case kMDTEdgeReflectionClipped:
		// Get the 'coordinates' from the color channels...
		distortionGetCoordinates(READ_LE_UINT16(s), xx, yy);

		// Find the read X location
		sx = mdi->baseX + dx + xx;

		if (sx < mdi->clipRect->left) {
			sx -= (mdi->clipRect->left - sx);
		}

		if (sx > mdi->clipRect->right) {
			sx -= (sx - mdi->clipRect->right);
		}

		// Cap the coordinates just incase it's a really tight src clip...
		sx = MAX<int>(mdi->clipRect->left, MIN<int>(sx, mdi->clipRect->right));

		// Find the read Y location...
		sy = mdi->baseY + dy + yy;

		if (sy < mdi->clipRect->top) {
			sy -= (mdi->clipRect->top - sy);
		}

		if (sy > mdi->clipRect->bottom) {
			sy -= (sy - mdi->clipRect->bottom);
		}

		// Cap the coordinates just in case it's a really tight src clip...
		sy = MAX<int>(mdi->clipRect->top, MIN<int>(sy, mdi->clipRect->bottom));

		// Transfer the pixel...
		*d = *((uint16 *)(mdi->srcData + sy * mdi->srcPitch + sx * sizeof(uint16)));

		break;
	case kMDTNotClipped:
		// Get the 'coordinates' from the color channels...
		distortionGetCoordinates(READ_BE_UINT16(s), xx, yy);

		// Find the read X location...
		sx = mdi->baseX + dx + xx;

		// Find the read Y location...
		sy = mdi->baseY + dy + yy;

		// Transfer the pixel...
		*d = *((uint16 *)(mdi->srcData + sy * mdi->srcPitch + sx * sizeof(uint16)));
		break;
	case kMDTSpecializedNotClipped:
		// Get the 'coordinates' from the color channels...
		distortionGetCoordinates(READ_BE_UINT16(s), xx, yy);

		// Transfer the source pixel...
		*d = *((uint16 *)(mdi->srcData + ((dy + yy) * 640 * sizeof(uint16)) + ((dx + xx) * sizeof(uint16))));

		break;
	default:
		error("distortionTransferOp(): Invalid transfer operation %d", transferOp);
	}

}

void Wiz::distortionBlitCore(
	Graphics::Surface *dstBitmap, int x, int y, Graphics::Surface *srcBitmap,
	Common::Rect *optionalSrcRectPtr, Common::Rect *optionalclipRectPtr, int transferOp, MoonbaseDistortionInfo *mdi) {

	// Clip the clip rect if one with the dest bitmap limits...
	Common::Rect clipRect;
	makeSizedRect(&clipRect, dstBitmap->w, dstBitmap->h);

	if (optionalclipRectPtr) {
		if (!findRectOverlap(&clipRect, optionalclipRectPtr))
			return;
	}

	// Clip the source coords to the source bitmap limits...
	Common::Rect srcRect;
	makeSizedRect(&srcRect, srcBitmap->w, srcBitmap->h);

	if (optionalSrcRectPtr) {
		if (!findRectOverlap(&srcRect, optionalSrcRectPtr))
			return;
	}

	// Build/clip the dest rect...
	Common::Rect dstRect;
	makeSizedRectAt(&dstRect, x, y, getRectWidth(&srcRect), getRectHeight(&srcRect));

	if (!findRectOverlap(&dstRect, &clipRect))
		return;

	// Adjust the source coords to the clipped dest coords...
	moveRect(&srcRect, (dstRect.left - x), (dstRect.top - y));

	// Calc the source pointer/get the source stride...
	byte *srcPtr = (byte *)srcBitmap->getBasePtr(srcRect.left, srcRect.top);
	int sStride = srcBitmap->pitch;

	// Calc the destination pointer / get the destination stride...
	byte *dstPtr = (byte *)dstBitmap->getBasePtr(dstRect.left, dstRect.top);
	int dStride = dstBitmap->pitch;

	// Setup operation amounts...
	int cw = getRectWidth(&dstRect);
	int ch = getRectHeight(&dstRect);

	// Left or right?
	int idx = dstRect.left;
	int dy = dstRect.top;

	while (--ch >= 0) {
		// Get the working pointers and step the line pointers...
		uint16 *d = (uint16 *)dstPtr;
		uint16 *s = (uint16 *)srcPtr;

		// Transfer the pixels...
		int dx = idx;

		for (int i = cw; --i >= 0;) {
			distortionTransferOp(transferOp, mdi, d, s, dx, dy);

			++d;
			++s;
			++dx;
		}

		// Move the line pointers...
		dstPtr += dStride;
		srcPtr += sStride;

		++dy;
	}
}

} // End of namespace Scumm
