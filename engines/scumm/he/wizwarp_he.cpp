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

#define WARP_FRAC_SIZE        (_vm->_game.heversion > 98 ? 20 : 16)
#define WARP_FRAC_VALUE       (1 << (WARP_FRAC_SIZE))
#define WARP_TEXTURE_LIMIT    (1 << (31 - (WARP_FRAC_SIZE)))
#define WARP_TO_FRAC(_x_)     ((_x_) << (WARP_FRAC_SIZE))
#define WARP_FROM_FRAC(_x_)   ((_x_) >> (WARP_FRAC_SIZE))

bool Wiz::warpDrawWiz(int image, int state, int polygon, int32 flags, int transparentColor, WizSimpleBitmap *optionalDestBitmap, const WizRawPixel *optionalColorConversionTable, int shadowImage) {
	const byte *xmapColorTable;
	int polyIndex;

	// Parameters check...
	for (polyIndex = 0; polyIndex < ARRAYSIZE(_polygons); polyIndex++) {
		if (_polygons[polyIndex].id == polygon) {
			break;
		}
	}

	if (ARRAYSIZE(_polygons) <= polyIndex) {
		error("Wiz::warpDrawWiz(): Polygon %d not defined", polygon);
	}

	if (_polygons[polyIndex].numPoints != 5) {
		error("Wiz::warpDrawWiz(): Invalid point count");
	}

	if (shadowImage) {
		xmapColorTable = getColorMixBlockPtrForWiz(shadowImage);
		if (xmapColorTable) {
			xmapColorTable += _vm->_resourceHeaderSize;
		}

	} else {
		xmapColorTable = nullptr;
	}

	// Call the actual warping primitive...
	WarpWizPoint polypoints[5];
	for (int i = 0; i < 5; i++) {
		WarpWizPoint tmp(_polygons[polyIndex].points[i]);
		polypoints[i] = tmp;
	}

	return warpDrawWizTo4Points(
		image, state, polypoints, flags, transparentColor,
		nullptr, optionalDestBitmap, optionalColorConversionTable, xmapColorTable);
}

bool Wiz::warpDrawWizTo4Points(int image, int state, const WarpWizPoint *dstPoints, int32 flags, int transparentColor, const Common::Rect *optionalClipRect, WizSimpleBitmap *optionalDestBitmap, const WizRawPixel *optionalColorConversionTable, const byte *colorMixTable) {
	WizSimpleBitmap dstBitmap, srcBitmap;
	bool rValue;
	Common::Rect updateRect;
	int x, y;
	WarpWizPoint srcPoints[4];
	byte *ptr;

	// Set the optional remap table up to the default if one isn't specified...
	if (!optionalColorConversionTable && _uses16BitColor) {
		optionalColorConversionTable = (WizRawPixel *)_vm->getHEPaletteSlot(1);
	}

	// Decompress the wiz into a WizSimpleBitmap... (Always, if using a remap table)...
	if ((getWizCompressionType(image, state) != kWCTNone) ||
		(optionalColorConversionTable != nullptr) || (flags & (kWRFHFlip | kWRFVFlip | kWRFRemap))) {

		srcBitmap.bufferPtr = drawAWizPrim(image, state, 0, 0, 0, 0, 0, 0, kWRFAlloc | flags, 0, optionalColorConversionTable);

		if (!srcBitmap.bufferPtr()) {
			return false;
		}
	} else {
		ptr = (byte *)getWizStateDataPrim(image, state);
		if (!ptr)
			error("Wiz::warpDrawWizTo4Points(): Image %d missing data", image);

		// Map the srcBitmap to the Wiz data...
		srcBitmap.bufferPtr = WizPxShrdBuffer(ptr + _vm->_resourceHeaderSize, false);
	}

	// Fill in the dest bitmap structure...
	if (optionalDestBitmap) {
		dstBitmap = *optionalDestBitmap;
	} else {
		VirtScreen *pvs = &_vm->_virtscr[kMainVirtScreen];
		dstBitmap.bitmapWidth = pvs->w;
		dstBitmap.bitmapHeight = pvs->h;

		if (flags & kWRFForeground) {
			dstBitmap.bufferPtr = WizPxShrdBuffer(pvs->getPixels(0, pvs->topline), false);
		} else {
			dstBitmap.bufferPtr = WizPxShrdBuffer(pvs->getBackPixels(0, pvs->topline), false);
		}

		if (!dstBitmap.bufferPtr()) {
			error("Wiz::warpDrawWizTo4Points(): Missing drawing buffer?");
		}

		dstBitmap.bufferPtr += (pvs->xstart * (_uses16BitColor ? 2 : 1));
	}

	// Find the bounding rect and double check the coords...
	updateRect.left   =  12345;
	updateRect.top    =  12345;
	updateRect.right  = -12345;
	updateRect.bottom = -12345;

	for (int i = 0; i < 4; i++) {
		x = dstPoints[i].x;
		y = dstPoints[i].y;
		updateRect.left =   MIN<int16>(updateRect.left, x);
		updateRect.top =    MIN<int16>(updateRect.top, y);
		updateRect.right =  MAX<int16>(updateRect.right, x);
		updateRect.bottom = MAX<int16>(updateRect.bottom, y);
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

	// Call the warping primitive...
	if (_vm->_game.heversion >= 95 && colorMixTable) {
		rValue = warpNPt2NPtClippedWarpMixColors(
			&dstBitmap, dstPoints, &srcBitmap, srcPoints, 4, transparentColor,
			optionalClipRect, colorMixTable);
	} else {
		if (_vm->_game.heversion >= 99) { // This shouldn't be a HE99 branch, but some later HE99 executables do this...
			rValue = warpNPt2NPtWarpCORE(
				&dstBitmap, dstPoints, &srcBitmap, srcPoints,
				4, transparentColor, optionalClipRect, flags);
		} else {
			if (optionalClipRect) {
				rValue = warpNPt2NPtClippedWarp(
					&dstBitmap, dstPoints, &srcBitmap, srcPoints, 4, transparentColor,
					optionalClipRect);
			} else {
				rValue = warpNPt2NPtNonClippedWarp(
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

	// Clean up...
	srcBitmap.bufferPtr = WizPxShrdBuffer();

	return rValue;
}

WarpWizOneSpanTable *Wiz::warpCreateSpanTable(int spanCount) {
	WarpWizOneSpanTable *spanTable;
	WarpWizOneSpan *spanPtr;
	int counter;

	spanTable = (WarpWizOneSpanTable *)malloc(sizeof(WarpWizOneSpanTable));
	if (!spanTable) {
		return nullptr;
	}

	// Clear spanTable structure
	spanTable->dstMinPt = WarpWizPoint();
	spanTable->dstMaxPt = WarpWizPoint();
	spanTable->srcMinPt = WarpWizPoint();
	spanTable->srcMaxPt = WarpWizPoint();
	spanTable->drawSpans = nullptr;
	spanTable->spans = nullptr;
	spanTable->drawSpanCount = 0;
	spanTable->spanCount = 0;

	spanTable->spanCount = spanCount;
	spanTable->spans = (WarpWizOneSpan *)malloc(spanCount * sizeof(WarpWizOneSpan));
	if (!spanTable->spans) {
		warpDestroySpanTable(spanTable);
		return nullptr;
	}

	// Clear spanTable spans structure
	for (counter = 0; counter < spanCount; counter++) {
		spanTable->spans[counter].dstLeft = 0;
		spanTable->spans[counter].dstRight = 0;
		spanTable->spans[counter].srcLeft = WarpWizPoint();
		spanTable->spans[counter].srcRight = WarpWizPoint();
	}

	spanPtr = spanTable->spans;
	for (counter = 0; counter < spanCount; counter++) {
		spanPtr->dstLeft = 0x7FFFFFFF;
		spanPtr->dstRight = (-0x7FFFFFFF - 1);
		spanPtr++;
	}

	spanTable->drawSpans = (WarpWizOneDrawSpan *)malloc(spanCount * sizeof(WarpWizOneDrawSpan));
	if (!spanTable->drawSpans) {
		warpDestroySpanTable(spanTable);
		return nullptr;
	}
	memset(spanTable->drawSpans, 0, spanCount * sizeof(WarpWizOneDrawSpan));

	return spanTable;
}

void Wiz::warpDestroySpanTable(WarpWizOneSpanTable *spanTable) {
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

WarpWizOneSpanTable *Wiz::warpBuildSpanTable(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *dstPts, const WarpWizPoint *srcPts, int npoints, const Common::Rect *clipRectPtr) {
	int offset, dw, nonClippedWidth, cl, cr, cy, dl, cw;
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
	warpFindMinMaxpoints(&dstMinPt, &dstMaxPt, dstPts, npoints);
	warpFindMinMaxpoints(&srcMinPt, &srcMaxPt, srcPts, npoints);

	st = warpCreateSpanTable((dstMaxPt.y - dstMinPt.y) + 1);
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
		warpFillSpanWithLine(st, dstPts, dstPts + 1, srcPts, srcPts + 1);
		dstPts++;
		srcPts++;
	}

	warpFillSpanWithLine(st, dstPts, &dstPt, srcPts, &srcPt);

	// Build the draw span table...
	drawSpan = st->drawSpans;
	dw = dstBitmap->bitmapWidth;
	span = st->spans;
	offset = st->dstMinPt.y * dw;
	st->drawSpanCount = 0;
	cy = st->dstMinPt.y;

	if (ignoreAll) {
		return st;
	}

	for (int i = st->spanCount; --i >= 0; ++cy, ++span, offset += dw) {
		// Clip vertical?
		if ((cy < clippingRect.top) || (cy > clippingRect.bottom)) {
			continue;
		}

		// Clip horizontal?
		cl = MAX<int>(clippingRect.left, span->dstLeft);
		cr = MIN<int>(clippingRect.right, span->dstRight);

		if ((cw = (cr - cl + 1)) <= 0) {
			continue;
		}

		// Calc the step values...
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

void Wiz::warpFindMinMaxpoints(WarpWizPoint *minPtr, WarpWizPoint *maxPtr, const WarpWizPoint *points, int npoints) {
	WarpWizPoint minPt, maxPt, pt;

	// Find the limits...
	maxPt.x = (-0x7FFFFFFF - 1);
	maxPt.y = (-0x7FFFFFFF - 1);
	minPt.x = 0x7FFFFFFF;
	minPt.y = 0x7FFFFFFF;

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

void Wiz::warpProcessDrawSpansA(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count) {
	int xStep, yStep, sw, xOffset, yOffset;

	const WizRawPixel8 *src8;
	const WizRawPixel16 *src16;

	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();

	sw = srcBitmap->bitmapWidth;

	src8 = (WizRawPixel8 *)srcBitmap->bufferPtr();
	src16 = (WizRawPixel16 *)srcBitmap->bufferPtr();

	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();
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

void Wiz::warpProcessDrawSpansTransparent(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor) {
	int xStep, yStep, sw, xOffset, yOffset;

	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr();
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr();

	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();

	WizRawPixel srcColor;

	sw = srcBitmap->bitmapWidth;

	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();
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

void Wiz::warpProcessDrawSpansTransparentFiltered(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor, const byte *pXmapColorTable, bool bIsHintColor, WizRawPixel hintColor) {
	int srcWidth = srcBitmap->bitmapWidth;

	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr();
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr();

	bool bSkipFilter = false;

	int xStep, yStep, xOffset, yOffset;
	WizRawPixel srcColor, srcColorN, srcColorS, srcColorE, srcColorW;
	int iCurrentX, iCurrentY, iXScan, iYScan;

	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();

	// Set up a rect for clipping if needed
	Common::Rect aSrcRect;  // Source rectangle for clipping...
	Common::Rect aScanRect; // Dest rectangle for clipping...
	aSrcRect.left = 0;
	aSrcRect.top = 0;
	aSrcRect.right = srcWidth;
	aSrcRect.bottom = srcBitmap->bitmapHeight;
	iYScan = aSrcRect.bottom / count;

	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();
			dst16 += drawSpans->dstOffset;
		}

		xOffset = drawSpans->xSrcOffset;
		yOffset = drawSpans->ySrcOffset;
		xStep = drawSpans->xSrcStep;
		yStep = drawSpans->ySrcStep;
		iXScan = WARP_FROM_FRAC(xStep); // The width of the search should be the x step size...

		for (int xCounter = drawSpans->dstWidth; --xCounter >= 0;) {
			iCurrentX = WARP_FROM_FRAC(xOffset);
			iCurrentY = WARP_FROM_FRAC(yOffset);

			// Get the current color and the surrounding colors...
			if (!_uses16BitColor) {
				srcColor = *(src8 + (srcWidth * iCurrentY) + iCurrentX);
			} else {
				srcColor = *(src16 + (srcWidth * iCurrentY) + iCurrentX);
			}

			bSkipFilter = false;
			if (bIsHintColor) {
				// Check if we need to clip our scan rectangle...
				aScanRect.left = iCurrentX - iXScan;
				aScanRect.top = iCurrentY - iYScan;
				aScanRect.right = iCurrentX + iXScan;
				aScanRect.bottom = iCurrentY + iYScan;
				findRectOverlap(&aScanRect, &aSrcRect);

				// Scan through rect looking for hint color...
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
				// Check if top...
				if (iCurrentY != 0) {
					if (!_uses16BitColor) {
						srcColorN = *(src8 + (srcWidth * (iCurrentY - 1)) + iCurrentX);
					} else {
						srcColorN = *(src16 + (srcWidth * (iCurrentY - 1)) + iCurrentX);
					}
				} else {
					srcColorN = transparentColor;
				}

				// Check if bottom...
				if (iCurrentY != aSrcRect.bottom) {
					if (!_uses16BitColor) {
						srcColorS = *(src8 + (srcWidth * (iCurrentY + 1)) + iCurrentX);
					} else {
						srcColorS = *(src16 + (srcWidth * (iCurrentY + 1)) + iCurrentX);
					}
				} else {
					srcColorS = transparentColor;
				}

				// Check for left edge...
				if (iCurrentX != 0) {
					if (!_uses16BitColor) {
						srcColorW = *(src8 + (srcWidth * iCurrentY) + (iCurrentX - 1));
					} else {
						srcColorW = *(src16 + (srcWidth * iCurrentY) + (iCurrentX - 1));
					}
				} else {
					srcColorW = transparentColor;
				}

				// Check for right edge...
				if (iCurrentX != aSrcRect.right) {
					if (!_uses16BitColor) {
						srcColorE = *(src8 + (srcWidth * iCurrentY) + (iCurrentX + 1));
					} else {
						srcColorE = *(src16 + (srcWidth * iCurrentY) + (iCurrentX + 1));
					}
				} else {
					srcColorE = transparentColor;
				}

				// Make transparent color black...
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
					// In 8 bit mode go around the pixel adding in colors (1/8 of the surrounding colors)
					// from the xmap color table; this ignores transparent pixels...
					srcColor = *(pXmapColorTable + (srcColorN * 256) + srcColor);
					srcColor = *(pXmapColorTable + (srcColorS * 256) + srcColor);
					srcColor = *(pXmapColorTable + (srcColorE * 256) + srcColor);
					srcColor = *(pXmapColorTable + (srcColorW * 256) + srcColor);

				} else {
					int rs = 0, gs = 0, bs = 0;
					float rd = 0, gd = 0, bd = 0;

					// Get 1/4 of each surrounding pixel and add into source pixel...
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

					// Average pixels...
					rd /= 4.0f;
					gd /= 4.0f;
					bd /= 4.0f;

					// Get source pixel colors...
					rawPixelExtractComponents(srcColor, rs, gs, bs);
					rd += rs;
					gd += gs;
					bd += bs;

					// Now average the two...
					rd /= 2.0f;
					gd /= 2.0f;
					bd /= 2.0f;
					
					// Pack the pixel back up...
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

void Wiz::warpProcessDrawSpansMixColors(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor, const byte *tablePtr) {
	int xStep, yStep, sw, xOffset, yOffset;
	WizRawPixel srcColor;

	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr();
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr();

	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();

	sw = srcBitmap->bitmapWidth;

	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();
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
					*dst8 = *(tablePtr + (srcColor * 256) + (*dst8));
					dst8++;
				} else {
					dst8++;
				}
			} else {
				srcColor = *(src16 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset));

				*dst16 = WIZRAWPIXEL_50_50_MIX(WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor), WIZRAWPIXEL_50_50_PREMIX_COLOR(*dst16));
				dst16++;
			}

			xOffset += xStep;
			yOffset += yStep;
		}

		drawSpans++;
	}
}

void Wiz::warpFillSpanWithLine(WarpWizOneSpanTable *st, const WarpWizPoint *dstA, const WarpWizPoint *dstB, const WarpWizPoint *srcA, const WarpWizPoint *srcB) {
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

void Wiz::warpProcessDrawSpansSampled(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count) {
	// Setup read pointer and clipping limits for the sampling rect...
	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr();
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr();
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();

	int sw = srcBitmap->bitmapWidth;
	int srcXLimit = (srcBitmap->bitmapWidth - 1);
	int srcYLimit = (srcBitmap->bitmapHeight - 1);

	// Process all of the spans in this span collection...
	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();
			dst16 += drawSpans->dstOffset;
		}

		int xOffset = drawSpans->xSrcOffset;
		int yOffset = drawSpans->ySrcOffset;
		int xStep = drawSpans->xSrcStep;
		int yStep = drawSpans->ySrcStep;

		// Figure out the sample rect for this "slope"...
		int sampleCx = (WARP_FROM_FRAC(xStep));
		int sampleCy = (WARP_FROM_FRAC(yStep));

		if (sampleCx < 0) {
			sampleCx = -sampleCx;
		}

		if (sampleCy < 0) {
			sampleCy = -sampleCy;
		}

		// Make the sampling area square using the largest delta...
		if (sampleCx > sampleCy) {
			sampleCy = sampleCx;
		} else {
			sampleCx = sampleCy;
		}

		int sampleXOffset = sampleCx;
		int sampleYOffset = sampleCy;

		// Process all pixels covered by this "span"...
		for (int xCounter = drawSpans->dstWidth; --xCounter >= 0;) {
			if (!_uses16BitColor) {
				*dst8++ = (*(src8 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset)));
			} else {
				// Do a really lame sampling of the potential source pixels...
				int sx = WARP_FROM_FRAC(xOffset);
				int sy = WARP_FROM_FRAC(yOffset);

				int sx1 = sx;
				int sy1 = sy;

				int sx2 = sx + sampleXOffset;
				int sy2 = sy + sampleYOffset;

				// Clip the source sample coords to the bitmap limits...
				sx1 = MAX<int>(0, MIN<int>(srcXLimit, sx1));
				sy1 = MAX<int>(0, MIN<int>(srcYLimit, sy1));
				sx2 = MAX<int>(0, MIN<int>(srcXLimit, sx2));
				sy2 = MAX<int>(0, MIN<int>(srcYLimit, sy2));

				// Now that the clipping is done figure out the sampling area...
				int sxc = ((sx2 - sx1) + 1);
				int syc = ((sy2 - sy1) + 1);
				int total = (sxc * syc);

				// Sample pixels from the source potential sampling area...
				if (total > 1) {
					int totalR, totalG, totalB;
					const WizRawPixel *samplePtr = (const WizRawPixel *)(src16 + ((sw * sy1) + sx1));

					rawPixelExtractComponents(*samplePtr, totalR, totalG, totalB);

					int sampleStep = sw - sxc;
					++total;

					for (sy = 0; sy < syc; sy++) {
						for (sx = 0; sx < sxc; sx++) {
							int r, g, b;

							WizRawPixel srcColor = *samplePtr++;
							rawPixelExtractComponents(srcColor, r, g, b);

							totalR += r;
							totalG += g;
							totalB += b;
						}

						samplePtr += sampleStep;
					}

					rawPixelPackComponents(*dst16, totalR / total, totalG / total, totalB / total);
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

void Wiz::warpProcessDrawSpansTransparentSampled(WizSimpleBitmap *dstBitmap, const WizSimpleBitmap *srcBitmap, const WarpWizOneDrawSpan *drawSpans, int count, WizRawPixel transparentColor) {
	// Setup read pointer and clipping limits for the sampling rect...
	const WizRawPixel8 *src8 = (WizRawPixel8 *)srcBitmap->bufferPtr();
	const WizRawPixel16 *src16 = (WizRawPixel16 *)srcBitmap->bufferPtr();
	WizRawPixel8 *dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
	WizRawPixel16 *dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();

	int sw = srcBitmap->bitmapWidth;
	int srcXLimit = (srcBitmap->bitmapWidth - 1);
	int srcYLimit = (srcBitmap->bitmapHeight - 1);
	
	// Process all of the spans in this span collection...
	for (int yCounter = count; --yCounter >= 0;) {
		if (!_uses16BitColor) {
			dst8 = (WizRawPixel8 *)dstBitmap->bufferPtr();
			dst8 += drawSpans->dstOffset;
		} else {
			dst16 = (WizRawPixel16 *)dstBitmap->bufferPtr();
			dst16 += drawSpans->dstOffset;
		}

		int xOffset = drawSpans->xSrcOffset;
		int yOffset = drawSpans->ySrcOffset;
		int xStep = drawSpans->xSrcStep;
		int yStep = drawSpans->ySrcStep;

		// Figure out the sample rect for this "slope"...
		int sampleCx = (WARP_FROM_FRAC(xStep));
		int sampleCy = (WARP_FROM_FRAC(yStep));

		if (sampleCx < 0) {
			sampleCx = -sampleCx;
		}

		if (sampleCy < 0) {
			sampleCy = -sampleCy;
		}

		// Make the sampling area square using the largest delta...
		if (sampleCx > sampleCy) {
			sampleCy = sampleCx;
		} else {
			sampleCx = sampleCy;
		}

		int sampleXOffset = sampleCx;
		int sampleYOffset = sampleCy;

		// Process all pixels covered by this "span"...
		for (int xCounter = drawSpans->dstWidth; --xCounter >= 0;) {
			if (!_uses16BitColor) {
				WizRawPixel srcColor = (*(src8 + (sw * WARP_FROM_FRAC(yOffset)) + WARP_FROM_FRAC(xOffset)));

				if (srcColor != transparentColor) {
					*dst8++ = srcColor;
				} else {
					dst8++;
				}

			} else {
				// Do a really lame sampling of the potential source pixels...
				int sx = WARP_FROM_FRAC(xOffset);
				int sy = WARP_FROM_FRAC(yOffset);

				int sx1 = sx;
				int sy1 = sy;

				int sx2 = sx + sampleXOffset;
				int sy2 = sy + sampleYOffset;

				// Clip the source sample coords to the bitmap limits...
				sx1 = MAX<int>(0, MIN<int>(srcXLimit, sx1));
				sy1 = MAX<int>(0, MIN<int>(srcYLimit, sy1));
				sx2 = MAX<int>(0, MIN<int>(srcXLimit, sx2));
				sy2 = MAX<int>(0, MIN<int>(srcYLimit, sy2));

				// Now that the clipping is done figure out the sampling area...
				int sxc = ((sx2 - sx1) + 1);
				int syc = ((sy2 - sy1) + 1);
				int total = (sxc * syc);

				// Sample pixels from the source potential sampling area...
				if (total > 1) {
					int totalR, totalG, totalB;

					const WizRawPixel *samplePtr = (const WizRawPixel *)(src16 + ((sw * sy1) + sx1));

					WizRawPixel firstColor = (*(src16 + (sw * sy1) + sx1));

					if (firstColor != transparentColor) {
						rawPixelExtractComponents(*samplePtr, totalR, totalG, totalB);
						++total;
					} else {
						totalR = 0;
						totalG = 0;
						totalB = 0;
					}

					int sampleStep = sw - sxc;

					for (sy = 0; sy < syc; sy++) {
						for (sx = 0; sx < sxc; sx++) {
							WizRawPixel srcColor = *samplePtr++;

							if (srcColor != transparentColor) {
								int r, g, b;

								rawPixelExtractComponents(srcColor, r, g, b);

								totalR += r;
								totalG += g;
								totalB += b;

							} else {
								--total;
							}
						}

						samplePtr += sampleStep;
					}

					if (total) {
						WizRawPixel srcColor;

						rawPixelPackComponents(srcColor, totalR / total, totalG / total, totalB / total);

						if (transparentColor != srcColor) {
							*dst16 = srcColor;
						} else {
							*dst16 = 4;
						}
					}

				} else {
					WizRawPixel srcColor = (*(src16 + (sw * sy1) + sx1));

					if (srcColor != transparentColor) {
						*dst16 = srcColor;
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

bool Wiz::warpNPt2NPtWarpCORE(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const Common::Rect *optionalClipRect, int32 wizFlags) {
	WarpWizOneSpanTable *st;

	if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
		error("Wiz::warpNPt2NPtWarpCORE(): Source bitmap (%d,%d) too big limit (%d,%d)",
			srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
			WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
	}

	st = warpBuildSpanTable(dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, optionalClipRect);

	if (st) {
		if (st->drawSpanCount) {
			if (transparentColor != -1) {
				if (wizFlags & kWRFAreaSampleDuringWarp) {
					warpProcessDrawSpansTransparentSampled(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
						(WizRawPixel)transparentColor);
				} else {
					warpProcessDrawSpansTransparent(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
						(WizRawPixel)transparentColor);
				}
			} else {
				if (wizFlags & kWRFAreaSampleDuringWarp) {
					warpProcessDrawSpansSampled(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount);
				} else {
					warpProcessDrawSpansA(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount);
				}
			}
		}

		warpDestroySpanTable(st);
		return true;
	}

	warning("Wiz::warpNPt2NPtWarpCORE(): Unable to warp bitmap");
	return false;
}

bool Wiz::warpNPt2NPtNonClippedWarp(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor) {
	if (_vm->_game.heversion > 99) {
		return warpNPt2NPtWarpCORE(
			dstBitmap, dstpoints, srcBitmap, srcpoints,
			npoints, transparentColor, nullptr, 0);
	} else {
		WarpWizOneSpanTable *st;

		if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
			error("Wiz::warpNPt2NPtNonClippedWarp(): Source bitmap (%d,%d) too big limit (%d,%d)",
				  srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
				  WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
		}

		st = warpBuildSpanTable(dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, nullptr);

		if (st) {
			if (st->drawSpanCount) {
				if (transparentColor != -1) {
					warpProcessDrawSpansTransparent(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
						(WizRawPixel)transparentColor);
				} else {
					warpProcessDrawSpansA(dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount);
				}
			}

			warpDestroySpanTable(st);
			return true;

		} else {
			warning("Wiz::warpNPt2NPtNonClippedWarp(): Unable to warp bitmap");
			return false;
		}
	}
}

bool Wiz::warpNPt2NPtClippedWarp(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const Common::Rect *optionalClipRect) {
	if (_vm->_game.heversion > 99) {
		return warpNPt2NPtWarpCORE(
			dstBitmap, dstpoints, srcBitmap, srcpoints,
			npoints, transparentColor, nullptr, 0);
	} else {
		WarpWizOneSpanTable *st;

		if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
			error("Wiz::warpNPt2NPtClippedWarp(): Source bitmap (%d,%d) too big limit (%d,%d)",
				  srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
				  WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
		}

		st = warpBuildSpanTable(dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, optionalClipRect);

		if (st) {
			if (st->drawSpanCount) {
				if (transparentColor != -1) {
					warpProcessDrawSpansTransparent(
						dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
						(WizRawPixel)transparentColor);
				} else {
					warpProcessDrawSpansA(dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount);
				}
			}

			warpDestroySpanTable(st);
			return true;
		} else {
			warning("Wiz::warpNPt2NPtClippedWarp(): Unable to warp bitmap");
			return false;
		}
	}
}

bool Wiz::warpNPt2NPtClippedWarpMixColors(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const Common::Rect *optionalClipRect, const byte *colorMixTable) {
	WarpWizOneSpanTable *st;

	if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
		error("Wiz::warpNPt2NPtClippedWarpMixColors(): Source bitmap (%d,%d) too big limit (%d,%d)",
			  srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
			  WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
	}

	st = warpBuildSpanTable(
		dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, optionalClipRect);

	if (st) {
		if (st->drawSpanCount) {
			warpProcessDrawSpansMixColors(
				dstBitmap, srcBitmap, st->drawSpans, st->drawSpanCount,
				transparentColor, colorMixTable);
		}

		warpDestroySpanTable(st);
		return true;
	} else {
		warning("Wiz::warpNPt2NPtClippedWarpMixColors(): Unable to warp bitmap");
		return false;
	}
}

bool Wiz::warpNPt2NPtNonClippedWarpFiltered(WizSimpleBitmap *dstBitmap, const WarpWizPoint *dstpoints, const WizSimpleBitmap *srcBitmap, const WarpWizPoint *srcpoints, int npoints, int transparentColor, const byte *pXmapColorTable, bool bIsHintColor, WizRawPixel hintColor) {
	WarpWizOneSpanTable *st;

	if ((srcBitmap->bitmapWidth >= WARP_TEXTURE_LIMIT) || (srcBitmap->bitmapHeight >= WARP_TEXTURE_LIMIT)) {
		error("Wiz::warpNPt2NPtNonClippedWarpFiltered(): Source bitmap (%d,%d) too big limit (%d,%d)",
			  srcBitmap->bitmapWidth, srcBitmap->bitmapHeight,
			  WARP_TEXTURE_LIMIT, WARP_TEXTURE_LIMIT);
	}

	st = warpBuildSpanTable(dstBitmap, srcBitmap, dstpoints, srcpoints, npoints, 0);

	if (st) {
		if (st->drawSpanCount) {
			warpProcessDrawSpansTransparentFiltered(dstBitmap, srcBitmap, st->drawSpans,
				st->drawSpanCount,(WizRawPixel)transparentColor, pXmapColorTable, bIsHintColor, hintColor);
		}

		warpDestroySpanTable(st);
		return true;

	} else {
		warning("Wiz::warpNPt2NPtNonClippedWarpFiltered(): Unable to warp bitmap");
		return false;
	}
}

} // End of namespace Scumm

#endif // ENABLE_HE
