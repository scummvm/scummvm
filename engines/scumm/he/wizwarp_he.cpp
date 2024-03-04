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

#define WARP_FRAC_SIZE        (_vm->_game.heversion > 98 ? 20 : 16)
#define WARP_FRAC_VALUE       (1 << (WARP_FRAC_SIZE))
#define WARP_TEXTURE_LIMIT    (1 << (31 - (WARP_FRAC_SIZE)))
#define WARP_TO_FRAC(_x_)     ((_x_) << (WARP_FRAC_SIZE))
#define WARP_FROM_FRAC(_x_)   ((_x_) >> (WARP_FRAC_SIZE))

bool Wiz::WARPWIZ_DrawWiz(int image, int state, int polygon, int32 flags, int transparentColor, WizSimpleBitmap *optionalDestBitmap, const WizRawPixel *optionalColorConversionTable, int shadowImage) {
	byte *xmapColorTable;
	int polyIndex;

	// Error check the params.
	for (polyIndex = 0; polyIndex < ARRAYSIZE(_polygons); polyIndex++) {
		if (_polygons[polyIndex].id == polygon) {
			break;
		}
	}

	if (ARRAYSIZE(_polygons) <= polyIndex) {
		error("Polygon %d not defined", polygon);
	}

	if (_polygons[polyIndex].numPoints != 5) {
		error("Invalid point count");
	}

	// How to lock this bad boy down?
	if (shadowImage) {
		xmapColorTable = (byte *)getColorMixBlockPtrForWiz(shadowImage);
		if (xmapColorTable) {
			xmapColorTable += _vm->_resourceHeaderSize;
		}

	} else {
		xmapColorTable = nullptr;
	}

	// Call the actual warping primitive
	WarpWizPoint polypoints[5];
	for (int i = 0; i < 5; i++) {
		WarpWizPoint tmp(_polygons[polyIndex].points[i]);
		polypoints[i] = tmp;
	}

	return WARPWIZ_DrawWizTo4Points(
		image, state, polypoints, flags, transparentColor,
		nullptr, optionalDestBitmap, optionalColorConversionTable, xmapColorTable);
}

bool Wiz::WARPWIZ_DrawWizTo4Points(int image, int state, const WarpWizPoint *dstPoints, int32 flags, int transparentColor, const Common::Rect *optionalClipRect, WizSimpleBitmap *optionalDestBitmap, const WizRawPixel *optionalColorConversionTable, byte *colorMixTable) {
	WizSimpleBitmap dstBitmap, srcBitmap;
	bool rValue, freeBitmapBits;
	Common::Rect updateRect;
	int x, y;
	WarpWizPoint srcPoints[4];
	byte *ptr;

	// Set the optional remap table up to the default if one isn't specified
	if (!optionalColorConversionTable && _uses16BitColor) {
		optionalColorConversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(1);
	}

	// Decompress the wiz into a WizSimpleBitmap... (Always if using a remap table)
	if ((getWizCompressionType(image, state) != kWCTNone) ||
		(optionalColorConversionTable != nullptr) || (flags & (kWRFHFlip | kWRFVFlip | kWRFRemap))) {

		srcBitmap.bufferPtr = (WizRawPixel *)drawAWizPrim(image, state, 0, 0, 0, 0, 0, 0, kWRFAlloc | flags, 0, optionalColorConversionTable);

		if (!srcBitmap.bufferPtr) {
			return false;
		}

		freeBitmapBits = true;
	} else {
		// Get a pointer to the bits!
		ptr = (byte *)getWizStateDataPrim(image, state);
		if (!ptr)
			error("Image %d missing data", image);

		// Map the srcBitmap bits to the wiz data...
		srcBitmap.bufferPtr = (WizRawPixel *)(ptr + _vm->_resourceHeaderSize);
		freeBitmapBits = false;
	}

	// Fill in the dest bitmap structure.
	if (optionalDestBitmap) {
		dstBitmap = *optionalDestBitmap;
	} else {
		VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
		dstBitmap.bitmapWidth = pvs->w;
		dstBitmap.bitmapHeight = pvs->h;

		if (flags & kWRFForeground) {
			dstBitmap.bufferPtr = (WizRawPixel *)pvs->getPixels(0, pvs->topline);
		} else {
			dstBitmap.bufferPtr = (WizRawPixel *)pvs->getBackPixels(0, pvs->topline);
		}

		if (!dstBitmap.bufferPtr) {
			error("Missing drawing buffer?");
		}

		if (_uses16BitColor) {
			WizRawPixel16 *buf16 = (WizRawPixel16 *)dstBitmap.bufferPtr;
			buf16 += pvs->xstart;
			dstBitmap.bufferPtr = (WizRawPixel *)buf16;
		} else {
			WizRawPixel8 *buf8 = (WizRawPixel8 *)dstBitmap.bufferPtr;
			buf8 += pvs->xstart;
			dstBitmap.bufferPtr = (WizRawPixel *)buf8;
		}
	}

	// Find the bounding rect and double check the coords.
	updateRect.left   =  12345;
	updateRect.top    =  12345;
	updateRect.right  = -12345;
	updateRect.bottom = -12345;

	for (int i = 0; i < 4; i++) {
		x = dstPoints[i].x;
		y = dstPoints[i].y;
		updateRect.left =   MIN<int16>(updateRect.left, x);
		updateRect.top =    MIN<int16>(updateRect.top, y);
		updateRect.right =  MIN<int16>(updateRect.right, x);
		updateRect.bottom = MIN<int16>(updateRect.bottom, y);
	}

	updateRect.left =   MAX<int16>(0, MIN<int16>((dstBitmap.bitmapWidth - 1), updateRect.left));
	updateRect.top =    MAX<int16>(0, MIN<int16>((dstBitmap.bitmapHeight - 1), updateRect.top));
	updateRect.right =  MAX<int16>(0, MIN<int16>((dstBitmap.bitmapWidth - 1), updateRect.right));
	updateRect.bottom = MAX<int16>(0, MIN<int16>((dstBitmap.bitmapHeight - 1), updateRect.bottom));

	getWizImageDim(image, state, srcBitmap.bitmapWidth, srcBitmap.bitmapHeight);

	srcPoints[0].x = 0;
	srcPoints[0].y = 0;
	srcPoints[1].x = srcBitmap.bitmapWidth - 1;
	srcPoints[1].y = 0;
	srcPoints[2].x = srcBitmap.bitmapWidth - 1;
	srcPoints[2].y = srcBitmap.bitmapHeight - 1;
	srcPoints[3].x = 0;
	srcPoints[3].y = srcBitmap.bitmapHeight - 1;

	// Call the warping primitive!
	if (colorMixTable) {
		rValue = WARPWIZ_NPt2NPtClippedWarpMixColors(
			&dstBitmap, dstPoints, &srcBitmap, srcPoints, 4, transparentColor,
			optionalClipRect, colorMixTable);
	} else {
		if (_vm->_game.heversion > 99) {
			rValue = WARPWIZ_NPt2NPtWarp_CORE(
				&dstBitmap, dstPoints, &srcBitmap, srcPoints,
				4, transparentColor, optionalClipRect, flags);
		} else {
			if (optionalClipRect) {
				rValue = WARPWIZ_NPt2NPtClippedWarp(
					&dstBitmap, dstPoints, &srcBitmap, srcPoints, 4, transparentColor,
					optionalClipRect);
			} else {
				rValue = WARPWIZ_NPt2NPtNonClippedWarp(
					&dstBitmap, dstPoints, &srcBitmap, srcPoints, 4, transparentColor);
			}
		}
	}

	// Update the screen? (If not writing to another bitmap...)
	if (rValue && !optionalDestBitmap) {
		if (!(flags & kWRFForeground)) {
			_vm->backgroundToForegroundBlit(updateRect);
		} else {
			++updateRect.bottom;
			_vm->markRectAsDirty(kMainVirtScreen, updateRect);
		}
	}

	// Cleanup!
	if (freeBitmapBits) {
		free(srcBitmap.bufferPtr);
		srcBitmap.bufferPtr = nullptr;
	}

	return rValue;
}

WarpWizOneSpanTable *Wiz::WARPWIZ_CreateSpanTable(int spanCount) {
	WarpWizOneSpanTable *spanTable;
	WarpWizOneSpan *spanPtr;
	int counter;

	spanTable = (WarpWizOneSpanTable *)malloc(sizeof(WarpWizOneSpanTable));
	if (!spanTable) {
		return nullptr;
	}

	spanTable->spanCount = spanCount;
	spanTable->spans = (WarpWizOneSpan *)malloc(spanCount * sizeof(WarpWizOneSpan));
	if (!spanTable->spans) {
		WARPWIZ_DestroySpanTable(spanTable);
		return nullptr;
	}

	spanPtr = spanTable->spans;
	for (counter = 0; counter < spanCount; counter++) {
		spanPtr->dstLeft = INT_MAX;
		spanPtr->dstRight = INT_MIN;
		spanPtr++;
	}

	spanTable->drawSpans = (WarpWizOneDrawSpan *)malloc(spanCount * sizeof(WarpWizOneDrawSpan));
	if (!spanTable->drawSpans) {
		WARPWIZ_DestroySpanTable(spanTable);
		return nullptr;
	}

	return spanTable;
}

void Wiz::WARPWIZ_DestroySpanTable(WarpWizOneSpanTable *spanTable) {
	if (spanTable->drawSpans) {
		free(spanTable->drawSpans);
		spanTable->drawSpans = nullptr;
	}

	if (spanTable->spans) {
		free(spanTable->spans);
		spanTable->spans = nullptr;
	}

	free(spanTable);
	spanTable = nullptr;
}

WarpWizOneSpanTable *Wiz::WARPWIZ_BuildSpanTable(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *dstPts, const WarpWizPoint *srcPts, int npoints, const Common::Rect *clipRectPtr) {
	int offset, dw, sw, nonClippedWidth, maxOffset, cl, cr, cy, dl, cw;
	WarpWizPoint srcPt, dstPt, dstMinPt, dstMaxPt, srcMinPt, srcMaxPt, clippedPt;
	WarpWizOneDrawSpan *drawSpan;
	Common::Rect clippingRect;
	WarpWizOneSpanTable *st;
	WarpWizOneSpan *span;
	bool ignoreAll;

	// Setup the clipping rect
	clippingRect.left = 0;
	clippingRect.top = 0;
	clippingRect.right = dstBitmap->bitmapWidth - 1;
	clippingRect.bottom = dstBitmap->bitmapHeight - 1;
	ignoreAll = false;

	if (clipRectPtr) {
		if (!findRectOverlap(&clippingRect, clipRectPtr)) {
			ignoreAll = true;
		}
	}

	// Find the span table fun parts...
	WARPWIZ_FindMinMaxpoints(&dstMinPt, &dstMaxPt, dstPts, npoints);
	WARPWIZ_FindMinMaxpoints(&srcMinPt, &srcMaxPt, srcPts, npoints);

	st = WARPWIZ_CreateSpanTable((dstMaxPt.y - dstMinPt.y) + 1);
	if (!st) {
		return nullptr;
	}

	st->dstMinPt = dstMinPt;
	st->dstMaxPt = dstMaxPt;
	st->srcMinPt = srcMinPt;
	st->srcMaxPt = srcMaxPt;

	dstPt = *dstPts;
	srcPt = *srcPts;

	// Scan convert the polygon...
	for (int i = 1; i < npoints; i++) {
		WARPWIZ_FillSpanWithLine(st, dstPts, dstPts + 1, srcPts, srcPts + 1);
		dstPts++;
		srcPts++;
	}

	WARPWIZ_FillSpanWithLine(st, dstPts, &dstPt, srcPts, &srcPt);

	// Build the draw span table!
	drawSpan = st->drawSpans;
	dw = dstBitmap->bitmapWidth;
	sw = srcBitmap->bitmapWidth;
	span = st->spans;
	offset = st->dstMinPt.y * dw;
	maxOffset = (dw * (dstBitmap->bitmapHeight - 1));
	st->drawSpanCount = 0;
	cy = st->dstMinPt.y;

	if (ignoreAll) {
		return st;
	}

	for (int i = st->spanCount; --i >= 0; ++cy, ++span, offset += dw) {
		// Clip vertical or all ?
		if ((cy < clippingRect.top) || (cy > clippingRect.bottom)) {
			continue;
		}

		// Clip horizontal
		cl = MAX<int>(clippingRect.left, span->dstLeft);
		cr = MAX<int>(clippingRect.right, span->dstRight);

		if ((cw = (cr - cl + 1)) <= 0) {
			continue;
		}

		// Calc the step values
		nonClippedWidth = (span->dstRight) - (span->dstLeft) + 1;
		drawSpan->xSrcStep = WARP_TO_FRAC((span->srcRight.x - span->srcLeft.x)) / nonClippedWidth;
		drawSpan->ySrcStep = WARP_TO_FRAC((span->srcRight.y - span->srcLeft.y)) / nonClippedWidth;

		// Calc the clipped values...
		drawSpan->dstOffset = offset + cl;
		drawSpan->dstWidth = cw;
		drawSpan->xSrcOffset = WARP_TO_FRAC(span->srcLeft.x);
		drawSpan->ySrcOffset = WARP_TO_FRAC(span->srcLeft.y);

		if ((dl = (cl - span->dstLeft)) != 0) {
			drawSpan->xSrcOffset += (dl * drawSpan->xSrcStep);
			drawSpan->ySrcOffset += (dl * drawSpan->ySrcStep);
		}

		st->drawSpanCount++;
		drawSpan++;
	}

	return st;
}

void Wiz::WARPWIZ_FindMinMaxpoints(WarpWizPoint *minPtr, WarpWizPoint *maxPtr, const WarpWizPoint *points, int npoints) {
	WarpWizPoint minPt, maxPt, pt;

	// Find the limits...
	maxPt.x = -0x8000;
	maxPt.y = -0x8000;
	minPt.x = 0x7FFF;
	minPt.y = 0x7FFF;

	for (int i = 0; i < npoints; i++) {
		pt = *points++;

		if (pt.x < minPt.x) {
			minPt.x = pt.x;
		}

		if (pt.y < minPt.y) {
			minPt.y = pt.y;
		}

		if (pt.x > maxPt.x) {
			maxPt.x = pt.x;
		}

		if (pt.y > maxPt.y) {
			maxPt.y = pt.y;
		}
	}

	*minPtr = minPt;
	*maxPtr = maxPt;
}

void Wiz::WARPWIZ_ProcessDrawSpansA(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count) {
	int xStep, yStep, sw, xOffset, yOffset;

	const WizRawPixel8 *src8;
	const WizRawPixel16 *src16;

	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;

	sw = srcBitmap->bitmapWidth;

	src8 = (WizRawPixel8 *)srcBitmap->bufferPtr;
	src16 = (WizRawPixel16 *)srcBitmap->bufferPtr;

	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;
			dst16 += drawSpans->dstOffset;
		}

		xOffset = drawSpans->xSrcOffset;
		yOffset = drawSpans->ySrcOffset;
		xStep = drawSpans->xSrcStep;
		yStep = drawSpans->ySrcStep;

		for (int xCounter = drawSpans->dstWidth; --xCounter >= 0;) {
			if (!_uses16BitColor) {
				*dst8++ = *(src8 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset));
			} else {
				*dst16++ = *(src16 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset));
			}

			xOffset += xStep;
			yOffset += yStep;
		}

		drawSpans++;
	}
}

void Wiz::WARPWIZ_ProcessDrawSpansTransparent(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor) {
	int xStep, yStep, sw, xOffset, yOffset;

	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr;
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr;

	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;

	WizRawPixel srcColor;

	sw = srcBitmap->bitmapWidth;

	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;
			dst16 += drawSpans->dstOffset;
		}

		xOffset = drawSpans->xSrcOffset;
		yOffset = drawSpans->ySrcOffset;
		xStep = drawSpans->xSrcStep;
		yStep = drawSpans->ySrcStep;

		for (int xCounter = drawSpans->dstWidth; --xCounter >= 0;) {
			if (!_uses16BitColor) {
				srcColor = *(src8 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset));
				if (srcColor != transparentColor) {
					*dst8++ = (WizRawPixel8)srcColor;
				} else {
					dst8++;
				}
			} else {
				srcColor = *(src16 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset));
				if (srcColor != transparentColor) {
					*dst16++ = (WizRawPixel16)srcColor;
				} else {
					dst16++;
				}
			}

			xOffset += xStep;
			yOffset += yStep;
		}

		drawSpans++;
	}
}

void Wiz::WARPWIZ_ProcessDrawSpansTransparentFiltered(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor, byte *pXmapColorTable, bool bIsHintColor, WizRawPixel hintColor) {
	int srcWidth = srcBitmap->bitmapWidth;

	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr;
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr;

	bool bSkipFilter = false;

	int xStep, yStep, xOffset, yOffset;
	WizRawPixel srcColor, srcColorN, srcColorS, srcColorE, srcColorW;
	int iCurrentX, iCurrentY, iXScan, iYScan;

	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;

	// set up a rect for clipping if needed
	Common::Rect aSrcRect;  // the source rectangle for clipping
	Common::Rect aScanRect; // the dest rectangle for clipping
	aSrcRect.left = 0;
	aSrcRect.top = 0;
	aSrcRect.right = srcWidth;
	aSrcRect.bottom = srcBitmap->bitmapHeight;
	iYScan = aSrcRect.bottom / count;

	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;
			dst16 += drawSpans->dstOffset;
		}

		xOffset = drawSpans->xSrcOffset;
		yOffset = drawSpans->ySrcOffset;
		xStep = drawSpans->xSrcStep;
		yStep = drawSpans->ySrcStep;
		iXScan = WARP_FROM_FRAC(xStep); // the width of the search should be the x step size

		for (int xCounter = drawSpans->dstWidth; --xCounter >= 0;) {
			iCurrentX = WARP_FROM_FRAC(xOffset);
			iCurrentY = WARP_FROM_FRAC(yOffset);

			// get the current color and the surrounding colors
			if (!_uses16BitColor) {
				srcColor = *(src8 + (srcWidth * iCurrentY) + iCurrentX);
			} else {
				srcColor = *(src16 + (srcWidth * iCurrentY) + iCurrentX);
			}

			bSkipFilter = false;
			if (bIsHintColor) {
				// check if we need to clip our scan rectangle
				aScanRect.left = iCurrentX - iXScan;
				aScanRect.top = iCurrentY - iYScan;
				aScanRect.right = iCurrentX + iXScan;
				aScanRect.bottom = iCurrentY + iYScan;
				findRectOverlap(&aScanRect, &aSrcRect);

				// scan through rect looking for hint color
				for (int yScan = aScanRect.top; yScan < aScanRect.bottom; ++yScan) {
					for (int xScan = aScanRect.left; xScan < aScanRect.right; ++xScan) {
						if (!_uses16BitColor) {
							if ((*(src8 + (yScan * srcWidth) + xScan)) == hintColor) {
								srcColor = hintColor;
								bSkipFilter = true;
								break;
							}
						} else {
							if ((*(src16 + (yScan * srcWidth) + xScan)) == hintColor) {
								srcColor = hintColor;
								bSkipFilter = true;
								break;
							}
						}
					}

					if (bSkipFilter) {
						break;
					}
				}
			}

			if ((srcColor != transparentColor) && !bSkipFilter) {
				// check if top
				if (iCurrentY != 0) {
					if (!_uses16BitColor) {
						srcColorN = *(src8 + (srcWidth * (iCurrentY - 1)) + iCurrentX);
					} else {
						srcColorN = *(src16 + (srcWidth * (iCurrentY - 1)) + iCurrentX);
					}
				} else {
					srcColorN = transparentColor;
				}

				// check if bottom
				if (iCurrentY != aSrcRect.bottom) {
					if (!_uses16BitColor) {
						srcColorS = *(src8 + (srcWidth * (iCurrentY + 1)) + iCurrentX);
					} else {
						srcColorS = *(src16 + (srcWidth * (iCurrentY + 1)) + iCurrentX);
					}
				} else {
					srcColorS = transparentColor;
				}

				// check for left edge
				if (iCurrentX != 0) {
					if (!_uses16BitColor) {
						srcColorW = *(src8 + (srcWidth * iCurrentY) + (iCurrentX - 1));
					} else {
						srcColorW = *(src16 + (srcWidth * iCurrentY) + (iCurrentX - 1));
					}
				} else {
					srcColorW = transparentColor;
				}

				// check for right edge
				if (iCurrentX != aSrcRect.right) {
					if (!_uses16BitColor) {
						srcColorE = *(src8 + (srcWidth * iCurrentY) + (iCurrentX + 1));
					} else {
						srcColorE = *(src16 + (srcWidth * iCurrentY) + (iCurrentX + 1));
					}
				} else {
					srcColorE = transparentColor;
				}

				// make transparent color black
				if (srcColorN == transparentColor) {
					srcColorN = srcColor;
				}

				if (srcColorS == transparentColor) {
					srcColorS = srcColor;
				}

				if (srcColorW == transparentColor) {
					srcColorW = srcColor;
				}

				if (srcColorE == transparentColor) {
					srcColorE = srcColor;
				}

				if (!_uses16BitColor) {
					// in 8 bit mode go around the pixel adding in colors from the xmap color table
					// ignores transparent pixels
					// trying 1/8 of the surrounding colors
					srcColor = *(pXmapColorTable + (srcColorN * 256) + srcColor);
					srcColor = *(pXmapColorTable + (srcColorS * 256) + srcColor);
					srcColor = *(pXmapColorTable + (srcColorE * 256) + srcColor);
					srcColor = *(pXmapColorTable + (srcColorW * 256) + srcColor);

				} else {
					int rs = 0, gs = 0, bs = 0;
					float rd = 0, gd = 0, bd = 0;

					// get 1/4 of each surrounding pixel and add into source pixel
					rawPixelExtractComponents(srcColorN, rs, gs, bs);
					rd += rs;
					gd += gs;
					bd += bs;

					rawPixelExtractComponents(srcColorS, rs, gs, bs);
					rd += rs;
					gd += gs;
					bd += bs;

					rawPixelExtractComponents(srcColorW, rs, gs, bs);
					rd += rs;
					gd += gs;
					bd += bs;

					rawPixelExtractComponents(srcColorE, rs, gs, bs);
					rd += rs;
					gd += gs;
					bd += bs;

					// average pixels
					rd /= 4.0f;
					gd /= 4.0f;
					bd /= 4.0f;

					// get source pixel colors
					rawPixelExtractComponents(srcColor, rs, gs, bs);
					rd += rs;
					gd += gs;
					bd += bs;

					// now average the two
					rd /= 2.0f;
					gd /= 2.0f;
					bd /= 2.0f;

					// pack the pixel back up
					rawPixelPackComponents(srcColor, (int)rd, (int)gd, (int)bd);
				}
			}

			if (srcColor != transparentColor) {
				if (!_uses16BitColor) {
					*dst8++ = srcColor;
				} else {
					*dst16++ = srcColor;
				}
			} else {
				if (!_uses16BitColor) {
					dst8++;
				} else {
					dst16++;
				}
			}

			xOffset += xStep;
			yOffset += yStep;
		}

		drawSpans++;
	}
}

void Wiz::WARPWIZ_ProcessDrawSpansMixColors(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor, byte *tablePtr) {
	int xStep, yStep, sw, xOffset, yOffset;
	WizRawPixel srcColor;

	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr;
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr;

	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;

	sw = srcBitmap->bitmapWidth;

	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;
			dst16 += drawSpans->dstOffset;
		}

		xOffset = drawSpans->xSrcOffset;
		yOffset = drawSpans->ySrcOffset;
		xStep = drawSpans->xSrcStep;
		yStep = drawSpans->ySrcStep;

		for (int xCounter = drawSpans->dstWidth; --xCounter >= 0;) {
			if (!_uses16BitColor) {
				srcColor = (*(src8 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset)));
				if (srcColor != transparentColor) {
					*dst8++ = *(tablePtr + (srcColor * 256) + (*dst8));
				} else {
					dst8++;
				}
			} else {
				srcColor = *(src16 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset));

				*dst16++ = WIZRAWPIXEL_50_50_MIX(WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor), WIZRAWPIXEL_50_50_PREMIX_COLOR(*dst16));
			}

			xOffset += xStep;
			yOffset += yStep;
		}

		drawSpans++;
	}
}

void Wiz::WARPWIZ_FillSpanWithLine(WarpWizOneSpanTable *st, const WarpWizPoint *dstA, const WarpWizPoint *dstB, const WarpWizPoint *srcA, const WarpWizPoint *srcB) {
	WarpWizPoint dstStep, srcStep, dstPt, srcPt, dpt, spt;
	int h, lx1, ly1, lx2, ly2;
	WarpWizOneSpan *spanPtr;

	dstPt.x = WARP_TO_FRAC(dstA->x);
	srcPt.x = WARP_TO_FRAC(srcA->x);
	srcPt.y = WARP_TO_FRAC(srcA->y);
	h = abs(dstB->y - dstA->y) + 1;
	dstStep.x = WARP_TO_FRAC((dstB->x - dstA->x)) / h;
	srcStep.x = WARP_TO_FRAC((srcB->x - srcA->x)) / h;
	srcStep.y = WARP_TO_FRAC((srcB->y - srcA->y)) / h;
	spanPtr = &st->spans[(dstA->y - st->dstMinPt.y)];

	if (srcA->x <= srcB->x) {
		lx1 = srcA->x;
		lx2 = srcB->x;
	} else {
		lx1 = srcB->x;
		lx2 = srcA->x;
	}

	if (srcA->y <= srcB->y) {
		ly1 = srcA->y;
		ly2 = srcB->y;
	} else {
		ly1 = srcB->y;
		ly2 = srcA->y;
	}

	for (int i = h; --i >= 0;) {
		dpt.x = WARP_FROM_FRAC(dstPt.x + (WARP_FRAC_VALUE / 2));
		spt.x = MAX<int>(lx1, MIN<int>(lx2, WARP_FROM_FRAC(srcPt.x + (WARP_FRAC_VALUE / 2))));
		spt.y = MAX<int>(ly1, MIN<int>(ly2, WARP_FROM_FRAC(srcPt.y + (WARP_FRAC_VALUE / 2))));

		if (dpt.x < spanPtr->dstLeft) {
			spanPtr->dstLeft = dpt.x;
			spanPtr->srcLeft = spt;
		}

		if (dpt.x > spanPtr->dstRight) {
			spanPtr->dstRight = dpt.x;
			spanPtr->srcRight = spt;
		}

		dstPt.x += dstStep.x;
		srcPt.x += srcStep.x;
		srcPt.y += srcStep.y;

		if (dstA->y < dstB->y) {
			spanPtr++;
		} else {
			spanPtr--;
		}
	}
}

void Wiz::WARPWIZ_ProcessDrawSpans_Sampled(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count) {
	// Setup read pointer and clipping limits for the sampling rect
	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr;
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr;
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;

	int sw = srcBitmap->bitmapWidth;
	int src_x_limit = (srcBitmap->bitmapWidth - 1);
	int src_y_limit = (srcBitmap->bitmapHeight - 1);

	// Process all of the spans in this span collection.
	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;
			dst16 += drawSpans->dstOffset;
		}

		int xOffset = drawSpans->xSrcOffset;
		int yOffset = drawSpans->ySrcOffset;
		int xStep = drawSpans->xSrcStep;
		int yStep = drawSpans->ySrcStep;

		// Figure out the sample rect for this "slope"
		int sample_cx = (WARP_FROM_FRAC(xStep));
		int sample_cy = (WARP_FROM_FRAC(yStep));

		if (sample_cx < 0) {
			sample_cx = -sample_cx;
		}

		if (sample_cy < 0) {
			sample_cy = -sample_cy;
		}

		// Make the sampling area square using the largest delta
		if (sample_cx > sample_cy) {
			sample_cy = sample_cx;
		} else {
			sample_cx = sample_cy;
		}

		int sample_x_offset = sample_cx;
		int sample_y_offset = sample_cy;

		// Process all pixels covered by this "span"
		for (int xCounter = drawSpans->dstWidth; --xCounter >= 0;) {
			if (!_uses16BitColor) {
				*dst8++ = (*(src8 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset)));
			} else {
				// Do a really lame sampling of the potential source pixels
				int sx = WARP_FROM_FRAC(xOffset);
				int sy = WARP_FROM_FRAC(yOffset);

				int sx1 = sx;
				int sy1 = sy;

				int sx2 = sx + sample_x_offset;
				int sy2 = sy + sample_y_offset;

				// Clip the source sample coords to the bitmap limits
				sx1 = MAX<int>(0, MIN<int>(src_x_limit, sx1));
				sy1 = MAX<int>(0, MIN<int>(src_y_limit, sy1));
				sx2 = MAX<int>(0, MIN<int>(src_x_limit, sx2));
				sy2 = MAX<int>(0, MIN<int>(src_y_limit, sy2));

				// Now that the clipping is done figure out the sampling area
				int sxc = ((sx2 - sx1) + 1);
				int syc = ((sy2 - sy1) + 1);
				int total = (sxc * syc);

				// Sample pixels from the source potential sampling area
				if (total > 1) {
					int total_R, total_G, total_B;
					const WizRawPixel *samplePtr = (const WizRawPixel *)(src16 + ((sw * sy1) + sx1));

					rawPixelExtractComponents(*samplePtr, total_R, total_G, total_B);

					int sampleStep = sw - sxc;
					++total;

					for (int i = 0; i < syc; sy++) {
						for (int j = 0; j < sxc; j++) {
							int r, g, b;

							WizRawPixel src_color = *samplePtr++;
							rawPixelExtractComponents(src_color, r, g, b);

							total_R += r;
							total_G += g;
							total_B += b;
						}

						samplePtr += sampleStep;
					}

					rawPixelPackComponents(*dst16, total_R / total, total_G / total, total_B / total);
				} else {
					*dst16 = (*(src16 + (sw * sy1) + sx1));
				}
			}

			xOffset += xStep;
			yOffset += yStep;
		}

		drawSpans++;
	}
}

void Wiz::WARPWIZ_ProcessDrawSpansTransparent_Sampled(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor) {
	// Setup read pointer and clipping limits for the sampling rect
	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr;
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr;
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;

	int sw = srcBitmap->bitmapWidth;
	int src_x_limit = (srcBitmap->bitmapWidth - 1);
	int src_y_limit = (srcBitmap->bitmapHeight - 1);

	// Process all of the spans in this span collection.
	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr;
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr;
			dst16 += drawSpans->dstOffset;
		}

		int xOffset = drawSpans->xSrcOffset;
		int yOffset = drawSpans->ySrcOffset;
		int xStep = drawSpans->xSrcStep;
		int yStep = drawSpans->ySrcStep;

		// Figure out the sample rect for this "slope"
		int sample_cx = (WARP_FROM_FRAC(xStep));
		int sample_cy = (WARP_FROM_FRAC(yStep));

		if (sample_cx < 0) {
			sample_cx = -sample_cx;
		}

		if (sample_cy < 0) {
			sample_cy = -sample_cy;
		}

		// Make the sampling area square using the largest delta
		if (sample_cx > sample_cy) {
			sample_cy = sample_cx;
		} else {
			sample_cx = sample_cy;
		}

		int sample_x_offset = sample_cx;
		int sample_y_offset = sample_cy;

		// Process all pixels covered by this "span"
		for (int xCounter = drawSpans->dstWidth; --xCounter >= 0;) {
			if (!_uses16BitColor) {
				WizRawPixel src_color = (*(src8 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset)));

				if (src_color != transparentColor) {
					*dst8++ = src_color;
				} else {
					dst8++;
				}

			} else {
				// Do a really lame sampling of the potential source pixels
				int sx = WARP_FROM_FRAC(xOffset);
				int sy = WARP_FROM_FRAC(yOffset);

				int sx1 = sx;
				int sy1 = sy;

				int sx2 = sx + sample_x_offset;
				int sy2 = sy + sample_y_offset;

				// Clip the source sample coords to the bitmap limits, this could
				// be moved outside the loop if efficiency was important!
				sx1 = MAX<int>(0, MIN<int>(src_x_limit, sx1));
				sy1 = MAX<int>(0, MIN<int>(src_y_limit, sy1));
				sx2 = MAX<int>(0, MIN<int>(src_x_limit, sx2));
				sy2 = MAX<int>(0, MIN<int>(src_y_limit, sy2));

				// Now that the clipping is done figure out the sampling area
				int sxc = ((sx2 - sx1) + 1);
				int syc = ((sy2 - sy1) + 1);
				int total = (sxc * syc);

				// Sample pixels from the source potential sampling area
				// For non-linear transformations this could be fairly incorrect!
				if (total > 1) {
					int total_R, total_G, total_B;

					const WizRawPixel *samplePtr = (const WizRawPixel *)(src16 + ((sw * sy1) + sx1));

					WizRawPixel first_color = (*(src16 + (sw * sy1) + sx1));

					if (first_color != transparentColor) {
						rawPixelExtractComponents(*samplePtr, total_R, total_G, total_B);
						++total;
					} else {
						total_R = 0;
						total_G = 0;
						total_B = 0;
					}

					int sampleStep = sw - sxc;

					for (int i = 0; i < syc; i++) {
						for (int j = 0; j < sxc; j++) {
							WizRawPixel src_color = *samplePtr++;

							if (src_color != transparentColor) {
								int r, g, b;

								rawPixelExtractComponents(src_color, r, g, b);

								total_R += r;
								total_G += g;
								total_B += b;

							} else {
								--total;
							}
						}

						samplePtr += sampleStep;
					}

					if (total) {
						WizRawPixel src_color;

						rawPixelPackComponents(src_color, total_R / total, total_G / total, total_B / total);

						if (transparentColor != src_color) {
							*dst16 = src_color;
						} else {
							*dst16 = 4;
						}
					}

				} else {
					WizRawPixel src_color = (*(src16 + (sw * sy1) + sx1));

					if (src_color != transparentColor) {
						*dst16 = src_color;
					}
				}

				dst16++;
			}

			xOffset += xStep;
			yOffset += yStep;
		}

		drawSpans++;
	}
}

bool Wiz::WARPWIZ_NPt2NPtWarp_CORE(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const Common::Rect *optionalClipRect, int32 wizFlags) {
	WarpWizOneSpanTable *st;

	if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
		error("Source bitmap (%d,%d) too big limit (%d,%d)",
			srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
			WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
	}

	st = WARPWIZ_BuildSpanTable(dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, optionalClipRect);

	if (st) {
		if (st->drawSpanCount) {
			if (transparentColor != -1) {
				if (wizFlags & kWRFAreaSampleDuringWarp) {
					WARPWIZ_ProcessDrawSpansTransparent_Sampled(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
						(WizRawPixel)transparentColor);
				} else {
					WARPWIZ_ProcessDrawSpansTransparent(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
						(WizRawPixel)transparentColor);
				}
			} else {
				if (wizFlags & kWRFAreaSampleDuringWarp) {
					WARPWIZ_ProcessDrawSpans_Sampled(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount);
				} else {
					WARPWIZ_ProcessDrawSpansA(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount);
				}
			}
		}

		WARPWIZ_DestroySpanTable(st);
		return true;
	}

	warning("Unable to warp bitmap");
	return false;
}

bool Wiz::WARPWIZ_NPt2NPtNonClippedWarp(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor) {
	if (_vm->_game.heversion > 99) {
		return WARPWIZ_NPt2NPtWarp_CORE(
			dstBitmap, dstpoints, srcBitmap, srcpoints,
			npoints, transparentColor, nullptr, 0);
	} else {
		WarpWizOneSpanTable *st;

		if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
			error("Source bitmap (%d,%d) too big limit (%d,%d)",
				  srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
				  WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
		}

		st = WARPWIZ_BuildSpanTable(dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, nullptr);

		if (st) {
			if (st->drawSpanCount) {
				if (transparentColor != -1) {
					WARPWIZ_ProcessDrawSpansTransparent(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
						(WizRawPixel)transparentColor);
				} else {
					WARPWIZ_ProcessDrawSpansA(dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount);
				}
			}

			WARPWIZ_DestroySpanTable(st);
			return true;

		} else {
			warning("Unable to warp bitmap");
			return false;
		}
	}
}

bool Wiz::WARPWIZ_NPt2NPtClippedWarp(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const Common::Rect *optionalClipRect) {
	if (_vm->_game.heversion > 99) {
		return WARPWIZ_NPt2NPtWarp_CORE(
			dstBitmap, dstpoints, srcBitmap, srcpoints,
			npoints, transparentColor, nullptr, 0);
	} else {
		WarpWizOneSpanTable *st;

		if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
			error("Source bitmap (%d,%d) too big limit (%d,%d)",
				  srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
				  WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
		}

		st = WARPWIZ_BuildSpanTable(dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, optionalClipRect);

		if (st) {
			if (st->drawSpanCount) {
				if (transparentColor != -1) {
					WARPWIZ_ProcessDrawSpansTransparent(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
						(WizRawPixel)transparentColor);
				} else {
					WARPWIZ_ProcessDrawSpansA(dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount);
				}
			}

			WARPWIZ_DestroySpanTable(st);
			return true;
		} else {
			warning("Unable to warp bitmap");
			return false;
		}
	}
}

bool Wiz::WARPWIZ_NPt2NPtClippedWarpMixColors(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const Common::Rect *optionalClipRect, byte *colorMixTable) {
	WarpWizOneSpanTable *st;

	if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
		error("Source bitmap (%d,%d) too big limit (%d,%d)",
			  srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
			  WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
	}

	st = WARPWIZ_BuildSpanTable(
		dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, optionalClipRect);

	if (st) {
		if (st->drawSpanCount) {
			WARPWIZ_ProcessDrawSpansMixColors(
				dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
				transparentColor, colorMixTable);
		}

		WARPWIZ_DestroySpanTable(st);
		return true;
	} else {
		warning("Unable to warp bitmap");
		return false;
	}
}

bool Wiz::WARPWIZ_NPt2NPtNonClippedWarpFiltered(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, byte *pXmapColorTable, bool bIsHintColor, WizRawPixel hintColor) {
	WarpWizOneSpanTable *st;

	if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
		error("Source bitmap (%d,%d) too big limit (%d,%d)",
			  srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
			  WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
	}

	st = WARPWIZ_BuildSpanTable(dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, 0);

	if (st) {
		if (st->drawSpanCount) {
			WARPWIZ_ProcessDrawSpansTransparentFiltered(dstBitmap, srcBitmap, st->drawSpans,
				st->drawSpanCount,(WizRawPixel)transparentColor, pXmapColorTable, bIsHintColor, hintColor);
		}

		WARPWIZ_DestroySpanTable(st);
		return true;

	} else {
		warning("Unable to warp bitmap");
		return false;
	}
}

} // End of namespace Scumm

#endif // ENABLE_HE
