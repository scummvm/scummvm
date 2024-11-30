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

int Wiz::pgReadPixel(const WizSimpleBitmap *srcBM, int x, int y, int defaultValue) {
	if ((x < 0) || (y < 0) || (x >= srcBM->bitmapWidth) || (y >= srcBM->bitmapHeight)) {
		return defaultValue;
	} else {
		if (_uses16BitColor) {
			return FROM_LE_16(*(((WizRawPixel16 *)srcBM->bufferPtr()) + y * srcBM->bitmapWidth + x));
		} else {
			return *(((WizRawPixel8 *)srcBM->bufferPtr()) + y * srcBM->bitmapWidth + x);
		}
	}
}

void Wiz::pgWritePixel(WizSimpleBitmap *srcBM, int x, int y, WizRawPixel value) {
	if ((x >= 0) && (y >= 0) && (x < srcBM->bitmapWidth) && (y < srcBM->bitmapHeight)) {
		if (_uses16BitColor) {
			*(((WizRawPixel16 *)srcBM->bufferPtr()) + y * srcBM->bitmapWidth + x) = FROM_LE_16(value);
		} else {
			*(((WizRawPixel8 *)srcBM->bufferPtr()) + y * srcBM->bitmapWidth + x) = value;
		}
	}
}

void Wiz::pgClippedWritePixel(WizSimpleBitmap *srcBM, int x, int y, const Common::Rect *clipRectPtr, WizRawPixel value) {
	if ((x >= clipRectPtr->left) && (y >= clipRectPtr->top) && (x <= clipRectPtr->right) && (y <= clipRectPtr->bottom)) {
		if (_uses16BitColor) {
			*(((WizRawPixel16 *)srcBM->bufferPtr()) + y * srcBM->bitmapWidth + x) = FROM_LE_16(value);
		} else {
			*(((WizRawPixel8 *)srcBM->bufferPtr()) + y * srcBM->bitmapWidth + x) = value;
		}
	}
}

#define GET_SIGN(x) ((x) > 0 ? 1 : ((x) == 0 ? 0 : (-1)))

void Wiz::pgClippedLineDraw(WizSimpleBitmap *destBM, int asx, int asy, int aex, int aey, const Common::Rect *clipRectPtr, WizRawPixel value) {
	int x1, y1, x2, y2, d, dx, dy, sx, sy, incrA, incrB;

	x1 = asx;
	y1 = asy;
	x2 = aex;
	y2 = aey;
	sx = GET_SIGN(x2 - x1);
	sy = GET_SIGN(y2 - y1);
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	pgClippedWritePixel(destBM, x1, y1, clipRectPtr, value);

	if (dx >= dy) {
		d = (dy * 2) - dx;
		incrA = dy * 2;
		incrB = (dy - dx) * 2;

		while (x1 != x2) {
			if (d <= 0) {
				d += incrA;
			} else {
				d += incrB;
				y1 += sy;
			}

			x1 += sx;
			pgClippedWritePixel(destBM, x1, y1, clipRectPtr, value);
		}
	} else {
		d = (dx * 2) - dy;
		incrA = dx * 2;
		incrB = (dx - dy) * 2;

		while (y1 != y2) {
			if (d <= 0) {
				d += incrA;
			} else {
				d += incrB;
				x1 += sx;
			}

			y1 += sy;
			pgClippedWritePixel(destBM, x1, y1, clipRectPtr, value);
		}
	}
}

#undef GET_SIGN

struct WizScanLine {
	int iXMin;
	int iXMax;
};

int scanEdge(WizScanLine *aScanLines, int iScanLineStart, bool bLeftSide, int iX1, int iY1, int iX2, int iY2) {
	int iCurScanLine = iScanLineStart;
	int iDX, iDY;
	double fInverseSlope;

	// Get deltas...
	iDX = iX2 - iX1;
	iDY = iY2 - iY1;

	// Protect against horizontal lines...
	if (iDY <= 0) {
		return 0;
	}

	// Calculate inverse slope...
	fInverseSlope = (double)iDX / (double)iDY;

	for (int iY = iY1; iY < iY2; ++iY) {
		if (bLeftSide) {
			aScanLines[iCurScanLine].iXMin = iX1 + (int)(ceil((iY - iY1) * fInverseSlope));
		} else {
			aScanLines[iCurScanLine].iXMax = iX1 + (int)(ceil((iY - iY1) * fInverseSlope));
		}

		++iCurScanLine;
	}

	return iCurScanLine;
}

void Wiz::pgClippedThickLineDraw(WizSimpleBitmap *destBM, int asx, int asy, int aex, int aey, const Common::Rect *clipRectPtr, int iLineThickness, WizRawPixel value) {
	int iDX = aex - asx;
	int iDY = aey - asy;
	double fDistance = sqrt((iDX * iDX) + (iDY * iDY));

	// If distance is 0, don't draw...
	if (fDistance == 0) {
		return;
	}

	// The ratio of half the line thickness to the length of the line...
	double fScale = (double)(iLineThickness) / (2 * fDistance);

	// The x and y increments from an endpoint needed to create a rectangle...
	float fDDX = -fScale * (float)iDY;
	float fDDY = fScale * (float)iDX;

	if (fDDX > 0) { // Round off
		fDDX += 0.5;
	} else {
		fDDX -= 0.5;
	}

	if (fDDY > 0) {
		fDDY += 0.5;
	} else {
		fDDY -= 0.5;
	}

	int iCompDX = (int)fDDX;
	int iCompDY = (int)fDDY;

	int aSortOrder[4] = { 0, 0, 0, 0 };

	// Determine point order of polygons based on line orientation
	// topmost (and topleft in case of x or y = 0)...
	if ((iDX >= 0) && (iDY < 0)) {
		aSortOrder[0] = 1;
		aSortOrder[1] = 2;
		aSortOrder[2] = 0;
		aSortOrder[3] = 3;
	}

	if ((iDX > 0) && (iDY >= 0)) {
		aSortOrder[0] = 0;
		aSortOrder[1] = 1;
		aSortOrder[2] = 3;
		aSortOrder[3] = 2;
	}

	if ((iDX <= 0) && (iDY > 0)) {
		aSortOrder[0] = 3;
		aSortOrder[1] = 0;
		aSortOrder[2] = 2;
		aSortOrder[3] = 1;
	}

	if ((iDX < 0) && (iDY <= 0)) {
		aSortOrder[0] = 2;
		aSortOrder[1] = 3;
		aSortOrder[2] = 1;
		aSortOrder[3] = 0;
	}

	// Now we can compute the corner points...

	int xPoints[4];
	int yPoints[4];

	xPoints[aSortOrder[0]] = asx - iCompDX;
	yPoints[aSortOrder[0]] = asy - iCompDY;

	xPoints[aSortOrder[1]] = asx + iCompDX;
	yPoints[aSortOrder[1]] = asy + iCompDY;

	xPoints[aSortOrder[2]] = aex - iCompDX;
	yPoints[aSortOrder[2]] = aey - iCompDY;

	xPoints[aSortOrder[3]] = aex + iCompDX;
	yPoints[aSortOrder[3]] = aey + iCompDY;

	// Determine how tall (how many scan lines) the polygon is...
	int iNumScanLines = yPoints[2] - yPoints[0];

	// Create scan line list...
	WizScanLine *aScanLines = new WizScanLine[iNumScanLines];

	// Scan left side...
	int iNextScanLine = scanEdge(aScanLines, 0, true, xPoints[0], yPoints[0], xPoints[1], yPoints[1]);
	scanEdge(aScanLines, iNextScanLine, true, xPoints[1], yPoints[1], xPoints[2], yPoints[2]);

	// Scan right side...
	iNextScanLine = scanEdge(aScanLines, 0, false, xPoints[0], yPoints[0], xPoints[3], yPoints[3]);
	scanEdge(aScanLines, iNextScanLine, false, xPoints[3], yPoints[3], xPoints[2], yPoints[2]);

	// Draw polygon...
	int iCurX, iCurY;
	for (int iWhichScanLine = 0; iWhichScanLine < iNumScanLines; ++iWhichScanLine) {
		iCurY = iWhichScanLine + yPoints[0];
		for (iCurX = aScanLines[iWhichScanLine].iXMin; iCurX < aScanLines[iWhichScanLine].iXMax; ++iCurX) {
			pgClippedWritePixel(destBM, iCurX, iCurY, clipRectPtr, value);
		}
	}

	delete[] aScanLines;
}

int convertToFixed(int iNumber) {
	return (iNumber << kWECFixedSize);
}

int convertFromFixed(int iFixedNumber) {
	return (iFixedNumber >> kWECFixedSize);
}

void Wiz::pgDrawClippedEllipse(WizSimpleBitmap *pDestBitmap, int iPX, int iPY, int iQX, int iQY, int iKX, int iKY, int iLOD, const Common::Rect *pClipRectPtr, int iThickness, WizRawPixel aColor) {
	// Since this is fixed point, limit the LOD to 14 bits precision...
	if (iLOD > kWECFixedSize - 2) {
		iLOD = kWECFixedSize - 2;
	}

	// Determine the rest of the rectangle verts using the midpoint formula...
	int iKX1 = (2 * iQX) - iKX;
	int iKY1 = (2 * iQY) - iKY;
	int iKX3 = (2 * iPX) - iKX;
	int iKY3 = (2 * iPY) - iKY;
	int iKX2 = iKX1 - (iKX - iKX3);
	int iKY2 = iKY1 + (iKY3 - iKY);

	int iOpPX = (iKX1 + iKX2) / 2;
	int iOpPY = (iKY1 + iKY2) / 2;
	int iOpQX = (iKX3 + iKX2) / 2;
	int iOpQY = (iKY3 + iKY2) / 2;

	bool bFirstPass = true;
	int x1 = 0, y1 = 0, x2 = 0, y2 = 0, iFirstX = 0, iFirstY = 0;

	// Loop through each quadrant...
	for (int iSide = 0; iSide < 4; ++iSide) {
		int xP = 0, yP = 0, xQ = 0, yQ = 0, xK = 0, yK = 0;

		// Determine quadrant and convert input to fixed point...
		switch (iSide) {
		case 0:
			xP = convertToFixed(iPX);
			yP = convertToFixed(iPY);
			xQ = convertToFixed(iQX);
			yQ = convertToFixed(iQY);
			xK = convertToFixed(iKX);
			yK = convertToFixed(iKY);
			break;
		case 1:
			xP = convertToFixed(iQX);
			yP = convertToFixed(iQY);
			xQ = convertToFixed(iOpPX);
			yQ = convertToFixed(iOpPY);
			xK = convertToFixed(iKX1);
			yK = convertToFixed(iKY1);
			break;
		case 2:
			xP = convertToFixed(iOpPX);
			yP = convertToFixed(iOpPY);
			xQ = convertToFixed(iOpQX);
			yQ = convertToFixed(iOpQY);
			xK = convertToFixed(iKX2);
			yK = convertToFixed(iKY2);
			break;
		case 3:
			xP = convertToFixed(iOpQX);
			yP = convertToFixed(iOpQY);
			xQ = convertToFixed(iPX);
			yQ = convertToFixed(iPY);
			xK = convertToFixed(iKX3);
			yK = convertToFixed(iKY3);
			break;
		}

		int vx, ux, vy, uy, w, xJ, yJ;

		vx = xK - xQ; // Displacements from center...
		ux = xK - xP;
		vy = yK - yQ;
		uy = yK - yP;
		xJ = xP - vx + kWECHalf; // Center of ellipse J...
		yJ = yP - vy + kWECHalf;

		ux -= (w = ux >> (2 * iLOD + 3)); // Cancel 2nd-order error...
		ux -= (w >>= (2 * iLOD + 4));     // Cancel 4th-order error...
		ux -= w >> (2 * iLOD + 3);        // Cancel 6th-order error...
		ux += vx >> (iLOD + 1);           // Cancel 1st-order error...
		uy -= (w = uy >> (2 * iLOD + 3)); // Cancel 2nd-order error...
		uy -= (w >>= (2 * iLOD + 4));     // Cancel 4th-order error...
		uy -= w >> (2 * iLOD + 3);        // Cancel 6th-order error...
		uy += vy >> (iLOD + 1);           // Cancel 1st-order error...

		for (int i = (kWECPiOver2 << iLOD) >> 16; i >= 0; --i) {
			x2 = (xJ + vx) >> 16;
			y2 = (yJ + vy) >> 16;

			if (bFirstPass) {
				x1 = x2;
				y1 = y2;
				iFirstX = x1;
				iFirstY = y1;
				bFirstPass = false;
			} else {
				if (iThickness > 0) {
					pgClippedThickLineDraw(pDestBitmap, x1, y1, x2, y2, pClipRectPtr, 5, aColor);
				} else {
					// Specific to print pack, should be moved to a thick pixel...
					pgClippedWritePixel(pDestBitmap, x2, y2, pClipRectPtr, aColor);
					pgClippedWritePixel(pDestBitmap, x2 + 1, y2, pClipRectPtr, aColor);
					pgClippedWritePixel(pDestBitmap, x2 - 1, y2, pClipRectPtr, aColor);
					pgClippedWritePixel(pDestBitmap, x2, y2 + 1, pClipRectPtr, aColor);
					pgClippedWritePixel(pDestBitmap, x2, y2 - 1, pClipRectPtr, aColor);
					pgClippedWritePixel(pDestBitmap, x2 + 1, y2 + 1, pClipRectPtr, aColor);
					pgClippedWritePixel(pDestBitmap, x2 + 1, y2 - 1, pClipRectPtr, aColor);
					pgClippedWritePixel(pDestBitmap, x2 - 1, y2 + 1, pClipRectPtr, aColor);
					pgClippedWritePixel(pDestBitmap, x2 - 1, y2 - 1, pClipRectPtr, aColor);
				}
				x1 = x2;
				y1 = y2;
			}

			ux -= vx >> iLOD;
			vx += ux >> iLOD;
			uy -= vy >> iLOD;
			vy += uy >> iLOD;
		}
	}

	if (iThickness > 0) {
		pgClippedThickLineDraw(pDestBitmap, x2, y2, iFirstX, iFirstY, pClipRectPtr, 5, aColor);
	} else {
		pgClippedLineDraw(pDestBitmap, x2, y2, iFirstX, iFirstY, pClipRectPtr, aColor);
	}
}

void Wiz::pgDrawSolidRect(WizSimpleBitmap *destBM, const Common::Rect *rectPtr, WizRawPixel color) {
	WizRawPixel16 *d16bit;
	WizRawPixel8 *d8bit;
	int cw, dw, ch;
	int x1 = rectPtr->left, y1 = rectPtr->top, x2 = rectPtr->right, y2 = rectPtr->bottom;

	// Common calcs...
	dw = destBM->bitmapWidth;
	cw = x2 - x1 + 1;
	ch = y2 - y1 + 1;

	d16bit = ((WizRawPixel16 *)destBM->bufferPtr()) + y1 * dw + x1;
	d8bit = ((WizRawPixel8 *)destBM->bufferPtr()) + y1 * dw + x1;

	if (cw > 1) {
		while (--ch >= 0) {
			if (_uses16BitColor) {
				for (int i = 0; i < cw; i++)
					WRITE_LE_UINT16(&d16bit[i], color);
				d16bit += dw;
			} else {
				memset(d8bit, color, cw);
				d8bit += dw;
			}
		}
	} else {
		while (--ch >= 0) {
			if (_uses16BitColor) {
				*d16bit = color;
				d16bit += dw;
			} else {
				*d8bit = color;
				d8bit += dw;
			}
		}
	}
}

void Wiz::pgFloodFillCmd(int x, int y, int color, const Common::Rect *optionalClipRect) {
	Common::Rect updateRect;
	WizSimpleBitmap fakeBitmap;
	int colorToWrite;

	// Setup the bitmap...
	int32 colorBackMask = _vm->_game.heversion > 99 ? 0x01000000 : 0x8000;
	pgSimpleBitmapFromDrawBuffer(&fakeBitmap, (color & colorBackMask) != 0);

	// Strip the color info down...
	colorToWrite = getRawPixel(color);

	// Call the primitive...
	updateRect.left = updateRect.top = 12345;
	updateRect.right = updateRect.bottom = -12345;

	_vm->VAR(_vm->VAR_OPERATION_FAILURE) = floodSimpleFill(
		&fakeBitmap, x, y, colorToWrite, optionalClipRect, &updateRect);

	if (isRectValid(updateRect)) {
		if (color & colorBackMask) {
			_vm->backgroundToForegroundBlit(updateRect);
		} else {
			updateRect.bottom++;
			_vm->markRectAsDirty(kMainVirtScreen, updateRect);
		}
	}
}

void Wiz::pgHistogramBitmapSubRect(int *tablePtr, const WizSimpleBitmap *bitmapPtr, const Common::Rect *sourceRect) {
	if (!_uses16BitColor) {
		Common::Rect rect, clipRect;
		WizRawPixel8 *srcPtr;
		int off, h, w, x;

		rect = *sourceRect;

		clipRect.left = 0;
		clipRect.top = 0;
		clipRect.right = (bitmapPtr->bitmapWidth - 1);
		clipRect.bottom = (bitmapPtr->bitmapHeight - 1);

		if (findRectOverlap(&rect, &clipRect)) {
			srcPtr = ((WizRawPixel8 *)bitmapPtr->bufferPtr()) + bitmapPtr->bitmapWidth * rect.top + rect.left;

			w = getRectWidth(&rect);
			h = getRectHeight(&rect);
			off = bitmapPtr->bitmapWidth - w;

			while (--h >= 0) {
				for (x = w; --x >= 0;) {
					*(tablePtr + *srcPtr++) += 1;
				}

				srcPtr += off;
			}
		}
	}
}

void Wiz::pgSimpleBitmapFromDrawBuffer(WizSimpleBitmap *bitmapPtr, bool background) {
	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];

	bitmapPtr->bitmapWidth = vs->w;
	bitmapPtr->bitmapHeight = vs->h;

	if (background) {
		bitmapPtr->bufferPtr = WizPxShrdBuffer(vs->getBackPixels(0, vs->topline), false);
	} else {
		bitmapPtr->bufferPtr = WizPxShrdBuffer(vs->getPixels(0, vs->topline), false);
	}
}

bool Wiz::pgGetMultiTypeBitmapFromDrawBuffer(WizMultiTypeBitmap *multiBM, bool background) {
	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];
	int pixelSize = _uses16BitColor ? sizeof(WizRawPixel16) : sizeof(WizRawPixel8);

	multiBM->width = vs->w;
	multiBM->height = vs->h;
	multiBM->stride = (multiBM->width * pixelSize);
	multiBM->format = _uses16BitColor ? 555 : 8;
	multiBM->bpp = (pixelSize * 8);

	if (background) {
		multiBM->data = (byte *)vs->getBackPixels(0, vs->topline);
	} else {
		multiBM->data = (byte *)vs->getPixels(0, vs->topline);
	}

	return true;
}

void Wiz::pgDrawRawDataFormatImage(WizRawPixel *bufferPtr, const WizRawPixel *rawData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, int32 wizFlags, const byte *extraTable, int transparentColor) {
	Common::Rect srcRect, dstRect, clipRect;
	WizSimpleBitmap srcBitmap, dstBitmap;
	int off, t;

	clipRect.left = 0;
	clipRect.top = 0;
	clipRect.right = bufferWidth - 1;
	clipRect.bottom = bufferHeight - 1;

	if (clipRectPtr) {
		if (!findRectOverlap(&clipRect, clipRectPtr)) {
			return;
		}
	}

	// Build the src and dest rects based on the passed in coords...
	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = (width - 1);
	srcRect.bottom = (height - 1);

	dstRect.left = x;
	dstRect.top = y;
	dstRect.right = (x + width - 1);
	dstRect.bottom = (y + height - 1);

	// Clip the src & dst coords to the clipping rect...
	clipRectCoords(&srcRect, &dstRect, &clipRect);

	if (dstRect.right < dstRect.left) {
		return;
	}

	if (dstRect.bottom < dstRect.top) {
		return;
	}

	if (srcRect.right < srcRect.left) {
		return;
	}

	if (srcRect.bottom < srcRect.top) {
		return;
	}

	if (_vm->_game.heversion > 90) {
		if (wizFlags & kWRFHFlip) {
			off = ((width - (srcRect.right + 1)) - srcRect.left);
			t = srcRect.left + off;
			srcRect.left = srcRect.right + off;
			srcRect.right = t;
		}

		if (wizFlags & kWRFVFlip) {
			off = ((height - (srcRect.bottom + 1)) - srcRect.top);
			t = srcRect.top + off;
			srcRect.top = srcRect.bottom + off;
			srcRect.bottom = t;
		}
	}

	// Setup the fake simple bitmaps...
	dstBitmap.bufferPtr = WizPxShrdBuffer(bufferPtr, false);
	dstBitmap.bitmapWidth = bufferWidth;
	dstBitmap.bitmapHeight = bufferHeight;

	srcBitmap.bufferPtr = WizPxShrdBuffer(const_cast<WizRawPixel *>(rawData), false);
	srcBitmap.bitmapWidth = width;
	srcBitmap.bitmapHeight = height;

	// Dispatch the drawing to the specific drawing function...
	if (_vm->_game.heversion <= 90) {
		pgSimpleBlit(&dstBitmap, &dstRect, &srcBitmap, &srcRect);
	} else {
		if (extraTable) {
			if (wizFlags & kWRFRemap) {
				if (transparentColor == -1) {
					pgSimpleBlitRemapColors(&dstBitmap, &dstRect, &srcBitmap, &srcRect, extraTable);
				} else {
					pgSimpleBlitTransparentRemapColors(&dstBitmap, &dstRect, &srcBitmap, &srcRect, (WizRawPixel)transparentColor, extraTable);
				}
			} else {
				if (transparentColor == -1) {
					pgSimpleBlitMixColors(&dstBitmap, &dstRect, &srcBitmap, &srcRect, extraTable);
				} else {
					pgSimpleBlitTransparentMixColors(&dstBitmap, &dstRect, &srcBitmap, &srcRect, (WizRawPixel)transparentColor, extraTable);
				}
			}
		} else {
			// Dispatch the drawing to the specific drawing function...
			if (transparentColor == -1) {
				pgSimpleBlit(&dstBitmap, &dstRect, &srcBitmap, &srcRect);
			} else {
				pgTransparentSimpleBlit(&dstBitmap, &dstRect, &srcBitmap, &srcRect, (WizRawPixel)transparentColor);
			}
		}
	}
}

void Wiz::pgSimpleBlit(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect) {
	int cw, dw, sw, ch, cSize;
	WizRawPixel8 *s8, *d8;
	WizRawPixel16 *s16, *d16;

	// Common calcs...
	dw = destBM->bitmapWidth;
	sw = sourceBM->bitmapWidth;
	cw = abs(sourceRect->right - sourceRect->left) + 1;
	ch = abs(sourceRect->bottom - sourceRect->top) + 1;

	d8 = ((WizRawPixel8 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s8 = ((WizRawPixel8 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;
	d16 = ((WizRawPixel16 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s16 = ((WizRawPixel16 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;

	// Going up or down?
	if (sourceRect->top > sourceRect->bottom) {
		sw = -sw;
	}

	// Left or right?
	if (sourceRect->left <= sourceRect->right) {
		cSize = _uses16BitColor ? (cw * sizeof(WizRawPixel16)) : (cw * sizeof(WizRawPixel8));

		while (--ch >= 0) {
			if (!_uses16BitColor) {
				memcpy(d8, s8, cSize);

				d8 += dw;
				s8 += sw;
			} else {
				memcpy(d16, s16, cSize);

				d16 += dw;
				s16 += sw;
			}
		}
	} else {
		dw -= cw;
		sw += cw;

		while (--ch >= 0) {
			if (!_uses16BitColor) {
				for (int i = cw; --i >= 0;) {
					*d8++ = *s8--;
				}

				d8 += dw;
				s8 += sw;
			} else {
				for (int i = cw; --i >= 0;) {
					*d16++ = FROM_LE_16(*s16--);
				}

				d16 += dw;
				s16 += sw;
			}
		}
	}
}

void Wiz::pgSimpleBlitRemapColors(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, const byte *remapColorTable) {
	int cw, dw, sw, ch;
	WizRawPixel8 *s8, *d8;
	WizRawPixel16 *s16, *d16;

	// Common calcs...
	dw = destBM->bitmapWidth;
	sw = sourceBM->bitmapWidth;
	cw = abs(sourceRect->right - sourceRect->left) + 1;
	ch = abs(sourceRect->bottom - sourceRect->top) + 1;

	d8 = ((WizRawPixel8 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s8 = ((WizRawPixel8 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;
	d16 = ((WizRawPixel16 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s16 = ((WizRawPixel16 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;

	// Going up or down?
	if (sourceRect->top > sourceRect->bottom) {
		sw = -sw;
	}

	// Left or right?
	if (sourceRect->left <= sourceRect->right) {
		if (!_uses16BitColor) {
			while (--ch >= 0) {
				pgForwardRemapPixelCopy((WizRawPixel *)d8, (WizRawPixel *)s8, cw, remapColorTable);

				d8 += dw;
				s8 += sw;
			}
		} else {
			while (--ch >= 0) {
				pgForwardRemapPixelCopy((WizRawPixel *)d16, (WizRawPixel *)s16, cw, remapColorTable);

				d16 += dw;
				s16 += sw;
			}
		}
	} else {
		if (!_uses16BitColor) {
			d8 += (cw - 1);
			s8 -= (cw - 1);

			while (--ch >= 0) {
				pgBackwardsRemapPixelCopy((WizRawPixel *)d8, (WizRawPixel *)s8, cw, remapColorTable);

				d8 += dw;
				s8 += sw;
			}
		} else {
			d16 += (cw - 1);
			s16 -= (cw - 1);

			while (--ch >= 0) {
				pgBackwardsRemapPixelCopy((WizRawPixel *)d16, (WizRawPixel *)s16, cw, remapColorTable);

				d16 += dw;
				s16 += sw;
			}
		}
	}
}

void Wiz::pgSimpleBlitTransparentRemapColors(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, WizRawPixel transparentColor, const byte *remapColorTable) {
	int cw, dw, sw, ch;
	WizRawPixel8 *s8, *d8;
	WizRawPixel16 *s16, *d16;

	// Common calcs...
	dw = destBM->bitmapWidth;
	sw = sourceBM->bitmapWidth;
	cw = abs(sourceRect->right - sourceRect->left) + 1;
	ch = abs(sourceRect->bottom - sourceRect->top) + 1;

	d8 = ((WizRawPixel8 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s8 = ((WizRawPixel8 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;
	d16 = ((WizRawPixel16 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s16 = ((WizRawPixel16 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;

	// Going up or down?
	if (sourceRect->top > sourceRect->bottom) {
		sw = -sw;
	}

	// Left or right?
	if (sourceRect->left <= sourceRect->right) {
		if (!_uses16BitColor) {
			while (--ch >= 0) {
				pgTransparentForwardRemapPixelCopy((WizRawPixel *)d8, (WizRawPixel *)s8, cw, transparentColor, remapColorTable);

				d8 += dw;
				s8 += sw;
			}
		} else {
			while (--ch >= 0) {
				pgTransparentForwardRemapPixelCopy((WizRawPixel *)d16, (WizRawPixel *)s16, cw, transparentColor, remapColorTable);

				d16 += dw;
				s16 += sw;
			}
		}
	} else {
		if (!_uses16BitColor) {
			d8 += (cw - 1);
			s8 -= (cw - 1);

			while (--ch >= 0) {
				pgTransparentBackwardsRemapPixelCopy((WizRawPixel *)d8, (WizRawPixel *)s8, cw, transparentColor, remapColorTable);

				d8 += dw;
				s8 += sw;
			}
		} else {
			d16 += (cw - 1);
			s16 -= (cw - 1);

			while (--ch >= 0) {
				pgTransparentBackwardsRemapPixelCopy((WizRawPixel *)d16, (WizRawPixel *)s16, cw, transparentColor, remapColorTable);

				d16 += dw;
				s16 += sw;
			}
		}
	}
}

void Wiz::pgSimpleBlitMixColors(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, const byte *mixColorTable) {
	int cw, dw, sw, ch;
	WizRawPixel8 *s8, *d8;
	WizRawPixel16 *s16, *d16;

	// Common calcs...
	dw = destBM->bitmapWidth;
	sw = sourceBM->bitmapWidth;
	cw = abs(sourceRect->right - sourceRect->left) + 1;
	ch = abs(sourceRect->bottom - sourceRect->top) + 1;

	d8 = ((WizRawPixel8 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s8 = ((WizRawPixel8 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;
	d16 = ((WizRawPixel16 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s16 = ((WizRawPixel16 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;

	// Going up or down?
	if (sourceRect->top > sourceRect->bottom) {
		sw = -sw;
	}

	// Left or right?
	if (sourceRect->left <= sourceRect->right) {
		if (!_uses16BitColor) {
			while (--ch >= 0) {
				pgForwardMixColorsPixelCopy((WizRawPixel *)d8, (WizRawPixel *)s8, cw, mixColorTable);

				d8 += dw;
				s8 += sw;
			}
		} else {
			while (--ch >= 0) {
				pgForwardMixColorsPixelCopy((WizRawPixel *)d16, (WizRawPixel *)s16, cw, mixColorTable);

				d16 += dw;
				s16 += sw;
			}
		}
	} else {
		if (!_uses16BitColor) {
			d8 += (cw - 1);
			s8 -= (cw - 1);

			while (--ch >= 0) {
				pgBackwardsMixColorsPixelCopy((WizRawPixel *)d8, (WizRawPixel *)s8, cw, mixColorTable);

				d8 += dw;
				s8 += sw;
			}
		} else {
			d16 += (cw - 1);
			s16 -= (cw - 1);

			while (--ch >= 0) {
				pgBackwardsMixColorsPixelCopy((WizRawPixel *)d16, (WizRawPixel *)s16, cw, mixColorTable);

				d16 += dw;
				s16 += sw;
			}
		}
	}
}

void Wiz::pgSimpleBlitTransparentMixColors(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, WizRawPixel transparentColor, const byte *mixColorTable) {
	int cw, dw, sw, ch;
	WizRawPixel8 *s8, *d8;
	WizRawPixel16 *s16, *d16;

	// Common calcs...
	dw = destBM->bitmapWidth;
	sw = sourceBM->bitmapWidth;
	cw = abs(sourceRect->right - sourceRect->left) + 1;
	ch = abs(sourceRect->bottom - sourceRect->top) + 1;

	d8 = ((WizRawPixel8 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s8 = ((WizRawPixel8 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;
	d16 = ((WizRawPixel16 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s16 = ((WizRawPixel16 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;

	// Going up or down?
	if (sourceRect->top > sourceRect->bottom) {
		sw = -sw;
	}

	// Left or right?
	if (sourceRect->left <= sourceRect->right) {
		if (!_uses16BitColor) {
			while (--ch >= 0) {
				pgTransparentForwardMixColorsPixelCopy((WizRawPixel *)d8, (WizRawPixel *)s8, cw, transparentColor, mixColorTable);

				d8 += dw;
				s8 += sw;
			}
		} else {
			while (--ch >= 0) {
				pgTransparentForwardMixColorsPixelCopy((WizRawPixel *)d16, (WizRawPixel *)s16, cw, transparentColor, mixColorTable);

				d16 += dw;
				s16 += sw;
			}
		}
	} else {
		if (!_uses16BitColor) {
			d8 += (cw - 1);
			s8 -= (cw - 1);

			while (--ch >= 0) {
				pgTransparentBackwardsMixColorsPixelCopy((WizRawPixel *)d8, (WizRawPixel *)s8, cw, transparentColor, mixColorTable);

				d8 += dw;
				s8 += sw;
			}
		} else {
			d16 += (cw - 1);
			s16 -= (cw - 1);

			while (--ch >= 0) {
				pgTransparentBackwardsMixColorsPixelCopy((WizRawPixel *)d16, (WizRawPixel *)s16, cw, transparentColor, mixColorTable);

				d16 += dw;
				s16 += sw;
			}
		}
	}
}

void Wiz::pgTransparentSimpleBlit(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, WizRawPixel transparentColor) {
	int value, cw, dw, sw, ch, soff, doff, tColor;
	WizRawPixel8 *s8, *d8;
	WizRawPixel16 *s16, *d16;

	// Common calcs...
	dw = destBM->bitmapWidth;
	sw = sourceBM->bitmapWidth;
	cw = abs(sourceRect->right - sourceRect->left) + 1;
	ch = abs(sourceRect->bottom - sourceRect->top) + 1;

	d8 = ((WizRawPixel8 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s8 = ((WizRawPixel8 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;
	d16 = ((WizRawPixel16 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s16 = ((WizRawPixel16 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;

	tColor = (int)transparentColor;

	// Going up or down?
	if (sourceRect->top > sourceRect->bottom) {
		sw = -sw;
	}

	// Left or right?
	if (sourceRect->left <= sourceRect->right) {
		soff = sw - cw;
		doff = dw - cw;

		while (--ch >= 0) {
			if (!_uses16BitColor) {
				for (int x = cw; --x >= 0;) {
					value = *s8++;

					if (value != tColor) {
						*d8++ = (WizRawPixel8)value;
					} else {
						d8++;
					}
				}

				s8 += soff;
				d8 += doff;
			} else {
				for (int x = cw; --x >= 0;) {
					value = FROM_LE_16(*s16++);

					if (value != tColor) {
						*d16++ = (WizRawPixel16)value;
					} else {
						d16++;
					}
				}

				s16 += soff;
				d16 += doff;
			}

		}

	} else {
		soff = sw + cw;
		doff = dw - cw;

		while (--ch >= 0) {
			if (!_uses16BitColor) {
				for (int x = cw; --x >= 0;) {
					value = *s8--;

					if (value != tColor) {
						*d8++ = (WizRawPixel8)value;
					} else {
						d8++;
					}
				}

				s8 += soff;
				d8 += doff;
			} else {
				for (int x = cw; --x >= 0;) {
					value = FROM_LE_16(*s16--);

					if (value != tColor) {
						*d16++ = (WizRawPixel16)value;
					} else {
						d16++;
					}
				}

				s16 += soff;
				d16 += doff;
			}
		}
	}
}

void Wiz::pgDrawWarpDrawLetter(WizRawPixel *bitmapBuffer, int bitmapWidth, int bitmapHeight, const byte *charData, int x1, int y1, int width, int height, byte *colorLookupTable) {
	WizRawPixel *remapTable = (_vm->_game.heversion <= 90) ? nullptr : (WizRawPixel *)_vm->getHEPaletteSlot(1);

	if (colorLookupTable) {
		auxDecompRemappedTRLEImage(
			bitmapBuffer, charData, bitmapWidth, bitmapHeight, x1, y1, width, height, nullptr,
			colorLookupTable,
			remapTable
		);
	} else {
		auxDecompTRLEImage(
			bitmapBuffer, charData, bitmapWidth, bitmapHeight, x1, y1, width, height, nullptr,
			remapTable
		);
	}
}

void Wiz::pgDraw8BppFormatImage(WizRawPixel *bufferPtr, const byte *rawData, int bufferWidth, int bufferHeight, int x, int y, int width, int height, Common::Rect *clipRectPtr, int32 wizFlags, const byte *extraTable, int transparentColor, const WizRawPixel *conversionTable) {
	Common::Rect srcRect, dstRect, clipRect;
	WizSimpleBitmap srcBitmap, dstBitmap;

	clipRect.left = 0;
	clipRect.top = 0;
	clipRect.right = bufferWidth - 1;
	clipRect.bottom = bufferHeight - 1;

	if (clipRectPtr) {
		if (!findRectOverlap(&clipRect, clipRectPtr)) {
			return;
		}
	}

	// Build the src and dest rects based on the passed in coords...
	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = (width - 1);
	srcRect.bottom = (height - 1);

	dstRect.left = x;
	dstRect.top = y;
	dstRect.right = (x + width - 1);
	dstRect.bottom = (y + height - 1);

	// Clip the src & dst coords to the clipping rect...
	clipRectCoords(&srcRect, &dstRect, &clipRect);

	if (dstRect.right < dstRect.left) {
		return;
	}

	if (dstRect.bottom < dstRect.top) {
		return;
	}

	if (srcRect.right < srcRect.left) {
		return;
	}

	if (srcRect.bottom < srcRect.top) {
		return;
	}

	if (wizFlags & kWRFHFlip) {
		int off, t;
		off = ((width - (srcRect.right + 1)) - srcRect.left);
		t = srcRect.left + off;
		srcRect.left = srcRect.right + off;
		srcRect.right = t;
	}

	if (wizFlags & kWRFVFlip) {
		int off, t;
		off = ((height - (srcRect.bottom + 1)) - srcRect.top);
		t = srcRect.top + off;
		srcRect.top = srcRect.bottom + off;
		srcRect.bottom = t;
	}

	// Setup the fake simple bitmaps...
	dstBitmap.bufferPtr = WizPxShrdBuffer(bufferPtr, false);
	dstBitmap.bitmapWidth = bufferWidth;
	dstBitmap.bitmapHeight = bufferHeight;

	srcBitmap.bufferPtr = WizPxShrdBuffer(const_cast<byte *>(rawData), false);
	srcBitmap.bitmapWidth = width;
	srcBitmap.bitmapHeight = height;

	// Dispatch the drawing to the specific drawing function...
	if (transparentColor == -1) {
		pgDraw8BppSimpleBlit(&dstBitmap, &dstRect, &srcBitmap, &srcRect, conversionTable);
	} else {
		pgDraw8BppTransparentSimpleBlit(&dstBitmap, &dstRect, &srcBitmap, &srcRect, transparentColor, conversionTable);
	}
}

void Wiz::pgDraw8BppSimpleBlit(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, const WizRawPixel *conversionTable) {
	int x, cw, dw, sw, ch;
	const WizRawPixel8 *s8; // Source is always 8-bit
	WizRawPixel16 *d16; // Dest is always 16-bit

	// Common calcs...
	dw = destBM->bitmapWidth;
	sw = sourceBM->bitmapWidth;
	cw = abs(sourceRect->right - sourceRect->left) + 1;
	ch = abs(sourceRect->bottom - sourceRect->top) + 1;

	d16 = ((WizRawPixel16 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s8 = ((const WizRawPixel8 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;

	// Going up or down?
	if (sourceRect->top > sourceRect->bottom) {
		sw = -sw;
	}

	// Left or right?
	if (sourceRect->left <= sourceRect->right) {
		while (--ch >= 0) {
			memcpy8BppConversion(d16, s8, cw, conversionTable);

			d16 += dw;
			s8 += sw;
		}
	} else {
		dw -= cw;
		sw += cw;

		while (--ch >= 0) {
			for (x = cw; --x >= 0;) {
				*d16 = (WizRawPixel16)convert8BppToRawPixel((WizRawPixel)*s8, conversionTable);
				d16++;
				s8--;
			}

			d16 += dw;
			s8 += sw;
		}
	}
}

void Wiz::pgDraw8BppTransparentSimpleBlit(WizSimpleBitmap *destBM, Common::Rect *destRect, WizSimpleBitmap *sourceBM, Common::Rect *sourceRect, int transparentColor, const WizRawPixel *conversionTable) {
	int cw, dw, sw, ch, soff, doff, value;
	const WizRawPixel8 *s8; // Source is always 8-bit
	WizRawPixel16 *d16; // Dest is always 16-bit

	// Common calcs...
	dw = destBM->bitmapWidth;
	sw = sourceBM->bitmapWidth;
	cw = abs(sourceRect->right - sourceRect->left) + 1;
	ch = abs(sourceRect->bottom - sourceRect->top) + 1;
	d16 = ((WizRawPixel16 *)destBM->bufferPtr()) + destRect->top * dw + destRect->left;
	s8 = ((const WizRawPixel8 *)sourceBM->bufferPtr()) + sourceRect->top * sw + sourceRect->left;

	// Going up or down?
	if (sourceRect->top > sourceRect->bottom) {
		sw = -sw;
	}

	// Left or right?
	if (sourceRect->left <= sourceRect->right) {
		soff = sw - cw;
		doff = dw - cw;

		while (--ch >= 0) {
			for (int x = cw; --x >= 0;) {
				value = *s8++;

				if (value != transparentColor) {
					*d16++ = (WizRawPixel16)convert8BppToRawPixel((WizRawPixel)value, conversionTable);
				} else {
					d16++;
				}
			}

			s8 += soff;
			d16 += doff;
		}
	} else {
		soff = sw + cw;
		doff = dw - cw;

		while (--ch >= 0) {
			for (int x = cw; --x >= 0;) {
				value = *s8--;
				if (value != transparentColor) {
					*d16++ = (WizRawPixel16)convert8BppToRawPixel((WizRawPixel)value, conversionTable);
				} else {
					d16++;
				}
			}

			s8 += soff;
			d16 += doff;
		}
	}
}

void Wiz::pgDrawImageWith16BitZBuffer(WizSimpleBitmap *psbDst, const WizSimpleBitmap *psbZBuffer, const byte *imgData, int x, int y, int z, int width, int height, Common::Rect *prcClip) {
	// This is available only on HE100+, which hopefully means it should only be called for 16-bit games!
	assert(_uses16BitColor);

	// Validate parameters...
	assert(psbDst && psbZBuffer && imgData && prcClip);

	// Z-Buffer and destination buffer must have the same dimensions...
	assert(psbDst->bitmapWidth == psbZBuffer->bitmapWidth);
	assert(psbDst->bitmapHeight == psbZBuffer->bitmapHeight);

	// Make sure that clip rect is clipped to destination buffer...
	Common::Rect dstRect;

	makeSizedRect(&dstRect, psbDst->bitmapWidth, psbDst->bitmapHeight);

	if (!findRectOverlap(prcClip, &dstRect)) {
		// Rectangles don't intersect - no drawing necessary...
		return;
	}

	// Make sure that clip rect is clipped to source image...
	Common::Rect srcRect;

	srcRect.left = x;
	srcRect.top = y;
	srcRect.right = x + (width - 1);
	srcRect.bottom = y + (height - 1);

	if (!findRectOverlap(prcClip, &srcRect)) {
		// Rectangles don't intersect - no drawing necessary...
		return;
	}

	// Perform the drawing...
	int dstWidth = psbDst->bitmapWidth; // Same for destination and Z-Buffer...
	const int drawWidth = (prcClip->right - prcClip->left + 1);
	const int drawHeight = (prcClip->bottom - prcClip->top + 1);

	WizRawPixel *pSrc = (WizRawPixel *)const_cast<byte *>(imgData) + (prcClip->top - y) * width + (prcClip->left - x);
	WizRawPixel *pDst = psbDst->bufferPtr() + prcClip->top * dstWidth + prcClip->left;
	WizRawPixel *pZB = psbZBuffer->bufferPtr() + prcClip->top * dstWidth + prcClip->left;

	for (int row = 0; row < drawHeight; ++row) {
		for (int col = 0; col < drawWidth; ++col, ++pZB, ++pDst, ++pSrc) {
			// Left hand rule - don't draw unless we're closer than the Z-Buffer value...
			if (*pZB > z) {
				*pDst = *pSrc;
			}
		}

		// Move to the next line...
		pSrc += width - drawWidth;
		pDst += dstWidth - drawWidth;
		pZB += dstWidth - drawWidth;
	}
}

void Wiz::pgForwardRemapPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, const byte *lookupTable) {
	if (!_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		while (size-- > 0) {
			*dst8++ = *(lookupTable + *src8++);
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		while (size-- > 0) {
			*dst16++ = FROM_LE_16(*src16++);
		}
	}
}

void Wiz::pgTransparentForwardRemapPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, WizRawPixel transparentColor, const byte *lookupTable) {
	if (!_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		while (size-- > 0) {
			WizRawPixel8 srcColor = *src8++;

			if (transparentColor != srcColor) {
				WizRawPixel8 remappedColor = *(lookupTable + srcColor);

				if (transparentColor != remappedColor) {
					*dst8++ = remappedColor;
				} else {
					++dst8;
				}
			} else {
				++dst8;
			}
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		while (size-- > 0) {
			WizRawPixel16 srcColor = FROM_LE_16(*src16++);

			if (transparentColor != srcColor) {
				*dst16++ = srcColor;
			} else {
				++dst16;
			}
		}
	}
}

void Wiz::pgTransparentBackwardsRemapPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, WizRawPixel transparentColor, const byte *lookupTable) {
	if (!_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		while (size-- > 0) {
			WizRawPixel srcColor = *src8++;

			if (transparentColor != srcColor) {
				WizRawPixel8 remappedColor = *(lookupTable + srcColor);

				if (transparentColor != remappedColor) {
					*dst8-- = remappedColor;
				} else {
					--dst8;
				}
			} else {
				--dst8;
			}
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		while (size-- > 0) {
			WizRawPixel16 srcColor = FROM_LE_16(*src16++);

			if (transparentColor != srcColor) {
				*dst16-- = srcColor;
			} else {
				--dst16;
			}
		}
	}
}

void Wiz::pgBackwardsRemapPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, const byte *lookupTable) {
	if (!_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		while (size-- > 0) {
			*dst8-- = *(lookupTable + *src8++);
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		while (size-- > 0) {
			*dst16-- = FROM_LE_16(*src16++);
		}
	}
}

void Wiz::pgForwardMixColorsPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, const byte *lookupTable) {
	if (!_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		while (size-- > 0) {
			*dst8 = *(lookupTable + ((*src8++) * 256) + *dst8);
			dst8++;
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		while (size-- > 0) {
			if (_vm->_game.heversion >= 99) {
				WizRawPixel16 srcColor = FROM_LE_16(*src16++);
				WizRawPixel16 dstColor = *dst16;

				*dst16++ = WIZRAWPIXEL_50_50_MIX(
					WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor),
					WIZRAWPIXEL_50_50_PREMIX_COLOR(dstColor));
			} else {
				*dst16++ = FROM_LE_16(*src16++);
			}
		}
	}
}

void Wiz::pgBackwardsMixColorsPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, const byte *lookupTable) {
	if (!_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		while (size-- > 0) {
			*dst8 = *(lookupTable + ((*src8++) * 256) + *dst8);
			dst8--;
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		while (size-- > 0) {
			if (_vm->_game.heversion >= 99) {
				WizRawPixel16 srcColor = FROM_LE_16(*src16++);
				WizRawPixel16 dstColor = *dst16;

				*dst16-- = WIZRAWPIXEL_50_50_MIX(
					WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor),
					WIZRAWPIXEL_50_50_PREMIX_COLOR(dstColor));
			} else {
				*dst16-- = FROM_LE_16(*src16++);
			}
		}
	}
}

void Wiz::pgTransparentForwardMixColorsPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, WizRawPixel transparentColor, const byte *lookupTable) {
	if (!_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		while (size-- > 0) {
			WizRawPixel8 srcColor = *src8++;

			if (transparentColor != srcColor) {
				WizRawPixel8 resultColor = *(lookupTable + (srcColor * 256) + *dst8);

				if (transparentColor != resultColor) {
					*dst8++ = resultColor;
				} else {
					++dst8;
				}
			} else {
				++dst8;
			}
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		while (size-- > 0) {
			WizRawPixel16 srcColor = FROM_LE_16(*src16++);

			if (transparentColor != srcColor) {
				WizRawPixel16 dstColor = *dst16;

				*dst16++ = WIZRAWPIXEL_50_50_MIX(
					WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor),
					WIZRAWPIXEL_50_50_PREMIX_COLOR(dstColor));
			} else {
				++dst16;
			}
		}
	}
}

void Wiz::pgTransparentBackwardsMixColorsPixelCopy(WizRawPixel *dstPtr, const WizRawPixel *srcPtr, int size, WizRawPixel transparentColor, const byte *lookupTable) {
	if (!_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		while (size-- > 0) {
			WizRawPixel8 srcColor = *src8++;

			if (transparentColor != srcColor) {
				WizRawPixel8 resultColor = *(lookupTable + (srcColor * 256) + *dst8);

				if (transparentColor != resultColor) {
					*dst8-- = resultColor;
				} else {
					--dst8;
				}
			} else {
				--dst8;
			}
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		while (size-- > 0) {
			WizRawPixel16 srcColor = FROM_LE_16(*src16++);

			if (transparentColor != srcColor) {
				WizRawPixel16 dstColor = *dst16;

				*dst16-- = WIZRAWPIXEL_50_50_MIX(
					WIZRAWPIXEL_50_50_PREMIX_COLOR(srcColor),
					WIZRAWPIXEL_50_50_PREMIX_COLOR(dstColor));
			} else {
				--dst16;
			}
		}
	}
}

static void pgBlitForwardSrcArbitraryDstPixelTransfer(Wiz *wiz, WizRawPixel *dstPtr, int dstStep, const WizRawPixel *srcPtr, int count, const void *userParam, const void *userParam2) {
	if (!wiz->_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		for (int i = 0; i < count; i++) {
			*dst8 = *src8++;
			dst8 += dstStep;
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		for (int i = 0; i < count; i++) {
			*dst16 = FROM_LE_16(*src16++);
			dst16 += dstStep;
		}
	}
}

static void pgBlitForwardSrcArbitraryDstTransparentPixelTransfer(Wiz *wiz, WizRawPixel *dstPtr, int dstStep, const WizRawPixel *srcPtr, int count, const void *userParam, const void *userParam2) {
	if (!wiz->_uses16BitColor) {
		WizRawPixel8 *dst8 = (WizRawPixel8 *)dstPtr;
		const WizRawPixel8 *src8 = (const WizRawPixel8 *)srcPtr;

		WizRawPixel8 transparentColor, color;
		transparentColor = *((const WizRawPixel8 *)userParam);

		for (int i = 0; i < count; i++) {
			color = *src8++;

			if (transparentColor != color) {
				*dst8 = color;
			}

			dst8 += dstStep;
		}
	} else {
		WizRawPixel16 *dst16 = (WizRawPixel16 *)dstPtr;
		const WizRawPixel16 *src16 = (const WizRawPixel16 *)srcPtr;

		WizRawPixel16 transparentColor, color;
		transparentColor = *((const WizRawPixel16 *)userParam);

		for (int i = 0; i < count; i++) {
			color = FROM_LE_16(*src16++);

			if (transparentColor != color) {
				*dst16 = color;
			}

			dst16 += dstStep;
		}
	}
}

void Wiz::pgBlit90DegreeRotate(WizSimpleBitmap *dstBitmap, int x, int y, const WizSimpleBitmap *srcBitmap, const Common::Rect *optionalSrcRect, const Common::Rect *optionalClipRect, bool hFlip, bool vFlip) {
	pgBlit90DegreeRotateCore(
		dstBitmap, x, y, srcBitmap, optionalSrcRect, optionalClipRect,
		hFlip, vFlip, nullptr, nullptr, pgBlitForwardSrcArbitraryDstPixelTransfer);
}

void Wiz::pgBlit90DegreeRotateTransparent(WizSimpleBitmap *dstBitmap, int x, int y, const WizSimpleBitmap *srcBitmap, const Common::Rect *optionalSrcRect, const Common::Rect *optionalClipRect, bool hFlip, bool vFlip, WizRawPixel transparentColor) {
	pgBlit90DegreeRotateCore(
		dstBitmap, x, y, srcBitmap, optionalSrcRect, optionalClipRect,
		hFlip, vFlip, &transparentColor, nullptr, pgBlitForwardSrcArbitraryDstTransparentPixelTransfer);
}

void Wiz::pgBlit90DegreeRotateCore(WizSimpleBitmap *dstBitmap, int x, int y, const WizSimpleBitmap *srcBitmap, const Common::Rect *optionalSrcRect,
	const Common::Rect *optionalClipRect, bool hFlip, bool vFlip, const void *userParam, const void *userParam2,
	void (*srcTransferFP)(Wiz *wiz, WizRawPixel *dstPtr, int dstStep, const WizRawPixel *srcPtr, int count, const void *userParam, const void *userParam2)) {

	Common::Rect dstRect, srcRect, clipRect, clippedDstRect, clippedSrcRect;
	int dstOffset, dstStep, w, h, srcOffset, dstX, dstY;

	const WizRawPixel8 *src8;
	WizRawPixel8 *dst8;
	const WizRawPixel16 *src16;
	WizRawPixel16 *dst16;

	// Do as much pre-clipping as possible...
	makeSizedRect(&clipRect, dstBitmap->bitmapWidth, dstBitmap->bitmapHeight);

	if (optionalClipRect) {
		if (!findRectOverlap(&clipRect, optionalClipRect)) {
			return;
		}
	}

	// Clip the source rect against the actual src bitmap limits...
	makeSizedRect(&srcRect, srcBitmap->bitmapWidth, srcBitmap->bitmapHeight);

	if (optionalSrcRect) {
		if (!findRectOverlap(&srcRect, optionalSrcRect)) {
			return;
		}
	}

	// Make the dest rect, then clip it against the clip rect...
	makeSizedRectAt(&dstRect, x, y, getRectHeight(&srcRect), getRectWidth(&srcRect));

	clippedDstRect = dstRect;

	if (!findRectOverlap(&clippedDstRect, &clipRect)) {
		return;
	}

	// Make the clipped src rect adjusted for the 90 degree rotation...
	clippedSrcRect.left = srcRect.left + (clippedDstRect.top - dstRect.top);
	clippedSrcRect.top = srcRect.top + (dstRect.right - clippedDstRect.right);
	clippedSrcRect.right = srcRect.right - (dstRect.bottom - clippedDstRect.bottom);
	clippedSrcRect.bottom = srcRect.bottom - (clippedDstRect.left - dstRect.left);

	// Perform any flipping of the coords and setup the step variables...
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

	// Finally get down to business and do the blit...
	w = getRectWidth(&clippedSrcRect);
	h = getRectHeight(&clippedSrcRect);

	// Transfer the src line to the dest line using the passed transfer prim...
	srcOffset = srcBitmap->bitmapWidth;

	if (!_uses16BitColor) {
		dst8 = ((WizRawPixel8 *)dstBitmap->bufferPtr()) + dstX + (dstY * dstBitmap->bitmapWidth);
		src8 = ((WizRawPixel8 *)srcBitmap->bufferPtr()) + clippedSrcRect.left + (clippedSrcRect.top * srcBitmap->bitmapWidth);

		while (--h >= 0) {
			(*srcTransferFP)(this, (WizRawPixel *)dst8, dstStep, (const WizRawPixel *)src8, w, userParam, userParam2);
			dst8 += dstOffset;
			src8 += srcOffset;
		}
	} else {
		dst16 = ((WizRawPixel16 *)dstBitmap->bufferPtr()) + dstX + (dstY * dstBitmap->bitmapWidth);
		src16 = ((WizRawPixel16 *)srcBitmap->bufferPtr()) + clippedSrcRect.left + (clippedSrcRect.top * srcBitmap->bitmapWidth);

		while (--h >= 0) {
			(*srcTransferFP)(this, (WizRawPixel *)dst16, dstStep, (const WizRawPixel *)src16, w, userParam, userParam2);
			dst16 += dstOffset;
			src16 += srcOffset;
		}
	}
}

bool Wiz::findRectOverlap(Common::Rect *destRectPtr, const Common::Rect *sourceRectPtr) {
	// Make sure only points inside the viewPort are being set...
	if ((destRectPtr->left > sourceRectPtr->right) || (destRectPtr->top > sourceRectPtr->bottom) ||
		(destRectPtr->right < sourceRectPtr->left) || (destRectPtr->bottom < sourceRectPtr->top)) {
		return false;
	}

	// Perform the actual clipping...
	if (destRectPtr->left < sourceRectPtr->left) {
		destRectPtr->left = sourceRectPtr->left;
	}

	if (destRectPtr->top < sourceRectPtr->top) {
		destRectPtr->top = sourceRectPtr->top;
	}

	if (destRectPtr->right > sourceRectPtr->right) {
		destRectPtr->right = sourceRectPtr->right;
	}

	if (destRectPtr->bottom > sourceRectPtr->bottom) {
		destRectPtr->bottom = sourceRectPtr->bottom;
	}

	return true;
}

bool Wiz::isPointInRect(Common::Rect *r, Common::Point *pt) {
	return (pt->x >= r->left) && (pt->x <= r->right) && (pt->y >= r->top) && (pt->y <= r->bottom);
}

bool Wiz::isRectValid(Common::Rect r) {
	return r.left <= r.right && r.top <= r.bottom;
}

void Wiz::makeSizedRectAt(Common::Rect *rectPtr, int x, int y, int width, int height) {
	rectPtr->left = x;
	rectPtr->top = y;
	rectPtr->right = x + width - 1;
	rectPtr->bottom = y + height - 1;
}

void Wiz::makeSizedRect(Common::Rect *rectPtr, int width, int height) {
	makeSizedRectAt(rectPtr, 0, 0, width, height);
}

void Wiz::combineRects(Common::Rect *destRect, Common::Rect *ra, Common::Rect *rb) {
	destRect->left =   MIN<int16>(ra->left,   rb->left);
	destRect->top =    MIN<int16>(ra->top,    rb->top);
	destRect->right =  MAX<int16>(ra->right,  rb->right);
	destRect->bottom = MAX<int16>(ra->bottom, rb->bottom);
}

void Wiz::clipRectCoords(Common::Rect *sourceRectPtr, Common::Rect *destRectPtr, Common::Rect *clipRectPtr) {
	int16 value = destRectPtr->left - clipRectPtr->left;
	if (value < 0) {
		sourceRectPtr->left -= value;
		destRectPtr->left -= value;
	}

	value = destRectPtr->right - clipRectPtr->right;
	if (value > 0) {
		sourceRectPtr->right -= value;
		destRectPtr->right -= value;
	}

	value = destRectPtr->top - clipRectPtr->top;
	if (value < 0) {
		sourceRectPtr->top -= value;
		destRectPtr->top -= value;
	}

	value = destRectPtr->bottom - clipRectPtr->bottom;
	if (value > 0) {
		sourceRectPtr->bottom -= value;
		destRectPtr->bottom -= value;
	}
}

void Wiz::floodInitFloodState(WizFloodState *statePtr, WizSimpleBitmap *bitmapPtr, int colorToWrite, const Common::Rect *optionalClippingRect) {
	statePtr->colorToWrite = colorToWrite;
	statePtr->bitmapPtr = bitmapPtr;

	if (!optionalClippingRect) {
		if (bitmapPtr) {
			makeSizedRect(&statePtr->clipping, bitmapPtr->bitmapWidth, bitmapPtr->bitmapHeight);
		} else {
			makeSizedRect(&statePtr->clipping, 1, 1);
		}
	} else {
		statePtr->clipping = *optionalClippingRect;
	}

	statePtr->sp = statePtr->stack;
	statePtr->top = statePtr->stack + statePtr->numStackElements;
}

WizFloodState *Wiz::floodCreateFloodState(int numStackElements) {
	WizFloodState *statePtr;

	statePtr = (WizFloodState *)malloc(sizeof(WizFloodState));

	if (!statePtr) {
		return nullptr;
	}

	statePtr->numStackElements = numStackElements;
	statePtr->stack = (WizFloodSegment *)malloc(numStackElements * sizeof(WizFloodSegment));

	if (!statePtr->stack) {
		floodDestroyFloodState(statePtr);
		return nullptr;
	}

	floodInitFloodState(statePtr, nullptr, 0, nullptr);
	return statePtr;
}

void Wiz::floodDestroyFloodState(WizFloodState *statePtr) {
	if (statePtr->stack) {
		free(statePtr->stack);
		statePtr->stack = nullptr;
	}

	free(statePtr);
	statePtr = nullptr;
}

static bool floodSimpleFloodCheckPixel(Wiz *w, int x, int y, WizFloodState *state) {
	return (w->pgReadPixel(state->bitmapPtr, x, y, state->colorToWrite) == state->writeOverColor);
}

bool Wiz::floodBoundryColorFloodCheckPixel(int x, int y, WizFloodState *state) {
	int pixel = pgReadPixel(state->bitmapPtr, x, y, state->colorToWrite);
	return (pixel != state->boundryColor) && (pixel != state->colorToWrite);
}

#define FLOOD_PUSH(Y, XL, XR, DY, SS)                                                                  \
	if ((SS)->sp < (SS)->top && Y + (DY) >= (SS)->clipping.top && Y + (DY) <= (SS)->clipping.bottom) { \
		(SS)->sp->y = Y;                                                                               \
		(SS)->sp->xl = XL;                                                                             \
		(SS)->sp->xr = XR;                                                                             \
		(SS)->sp->dy = DY;                                                                             \
		(SS)->sp++;                                                                                    \
	}

#define FLOOD_POP(Y, XL, XR, DY, SS) {         \
		(SS)->sp--;                            \
		Y = (SS)->sp->y + (DY = (SS)->sp->dy); \
		XL = (SS)->sp->xl;                     \
		XR = (SS)->sp->xr;                     \
	}


void Wiz::floodFloodFillPrim(int x, int y, WizFloodState *statePtr, bool (*checkPixelFnPtr)(Wiz *w, int x, int y, WizFloodState *state)) {
	Common::Rect fillRect;

	int l, x1, x2, dy;

	// Make the initial update invalid...
	statePtr->updateRect.left = statePtr->updateRect.top = 12345;
	statePtr->updateRect.right = statePtr->updateRect.bottom = -12345;

	FLOOD_PUSH(y, x, x, 1, statePtr);
	FLOOD_PUSH(y + 1, x, x, -1, statePtr);

	while (statePtr->sp > statePtr->stack) {
		FLOOD_POP(y, x1, x2, dy, statePtr);

		// Fill the left span from this point!
		fillRect.left = x1 + 1;
		fillRect.top = y;
		fillRect.right = x1;
		fillRect.bottom = y;

		for (x = x1; (x >= statePtr->clipping.left) && (*checkPixelFnPtr)(this, x, y, statePtr); x--) {
			fillRect.left = x;
		}

		if (isRectValid(fillRect)) {
			floodPerformOpOnRect(statePtr, &fillRect);
		}

		// This goto, while ugly, allows me to avoid rewriting
		// a second version of the do-while loop below... Have mercy :-)
		if (x >= x1)
			goto skip;

		l = x + 1;

		if (l < x1) {
			FLOOD_PUSH(y, l, x1 - 1, -dy, statePtr);
		}

		x = x1 + 1;

		do {
			// Fill the right span from this point!
			fillRect.left = x;
			fillRect.top = y;
			fillRect.right = x - 1;
			fillRect.bottom = y;

			for (; (x <= statePtr->clipping.right) && (*checkPixelFnPtr)(this, x, y, statePtr); x++) {
				fillRect.right = x;
			}

			if (isRectValid(fillRect)) {
				floodPerformOpOnRect(statePtr, &fillRect);
			}

			FLOOD_PUSH(y, l, x - 1, dy, statePtr);

			if (x > (x2 + 1)) {
				FLOOD_PUSH(y, x2 + 1, x - 1, -dy, statePtr);
			}

		skip:
			for (x++; (x <= x2) && (!(*checkPixelFnPtr)(this, x, y, statePtr)); x++)
				;

			l = x;

		} while (x <= x2);
	}
}

#undef FLOOD_POP
#undef FLOOD_PUSH

void Wiz::floodPerformOpOnRect(WizFloodState *statePtr, Common::Rect *rectPtr) {
	pgDrawSolidRect(statePtr->bitmapPtr, rectPtr, (WizRawPixel)statePtr->colorToWrite);

	if (isRectValid(statePtr->updateRect)) {
		combineRects(&statePtr->updateRect, &statePtr->updateRect, rectPtr);
	} else {
		statePtr->updateRect = *rectPtr;
	}
}

bool Wiz::floodSimpleFill(WizSimpleBitmap *bitmapPtr, int x, int y, int colorToWrite, const Common::Rect *optionalClipRect, Common::Rect *updateRectPtr) {
	WizFloodState *statePtr;

	statePtr = floodCreateFloodState(bitmapPtr->bitmapHeight * 2);

	if (!statePtr) {
		return false;
	}

	floodInitFloodState(statePtr, bitmapPtr, colorToWrite, optionalClipRect);

	statePtr->writeOverColor = pgReadPixel(bitmapPtr, x, y, colorToWrite);

	if (colorToWrite != statePtr->writeOverColor) {
		floodFloodFillPrim(x, y, statePtr, floodSimpleFloodCheckPixel);
		*updateRectPtr = statePtr->updateRect;
	}

	floodDestroyFloodState(statePtr);
	return true;
}

int Wiz::getRectWidth(Common::Rect *rectPtr) {
	return abs(rectPtr->width()) + 1;
}

int Wiz::getRectHeight(Common::Rect *rectPtr) {
	return abs(rectPtr->height()) + 1;
}

void Wiz::moveRect(Common::Rect *rectPtr, int dx, int dy) {
	rectPtr->left += dx;
	rectPtr->right += dx;
	rectPtr->top += dy;
	rectPtr->bottom += dy;
}

void Wiz::horzFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect) {
	int dx = (baseRect->right - rectToAlign->right) - (rectToAlign->left - baseRect->left);
	moveRect(rectToAlign, dx, 0);
}

void Wiz::vertFlipAlignWithRect(Common::Rect *rectToAlign, const Common::Rect *baseRect) {
	int dy = (baseRect->bottom - rectToAlign->bottom) - (rectToAlign->top - baseRect->top);
	moveRect(rectToAlign, 0, dy);
}

void Wiz::swapRectX(Common::Rect *rectPtr) {
	int16 t = rectPtr->right;
	rectPtr->right = rectPtr->left;
	rectPtr->left = t;
}

void Wiz::swapRectY(Common::Rect *rectPtr) {
	int16 t = rectPtr->bottom;
	rectPtr->bottom = rectPtr->top;
	rectPtr->top = t;
}

int Wiz::getRawPixel(int color) {
	return color & WIZRAWPIXEL_MASK;
}

void Wiz::memset8BppConversion(void *dstPtr, int value, size_t count, const WizRawPixel *conversionTable) {
	if (_uses16BitColor) {
		rawPixelMemset((WizRawPixel16 *)dstPtr, (int)convert8BppToRawPixel((WizRawPixel16)value, conversionTable), count);
	} else {
		memset((WizRawPixel8 *)dstPtr, value, count);
	}
}

void Wiz::memcpy8BppConversion(void *dstPtr, const void *srcPtr, size_t count, const WizRawPixel *conversionTable) {
	if (_uses16BitColor) {
		WizRawPixel16 *dstWritePtr = (WizRawPixel16 *)(dstPtr);
		const byte *srcReadPtr = (const byte *)(srcPtr);
		int counter = count;
		while (0 <= --counter) {
			*dstWritePtr++ = convert8BppToRawPixel((WizRawPixel16)(*srcReadPtr++), conversionTable);
		}
	} else {
		memcpy((WizRawPixel8 *)dstPtr, (const byte *)srcPtr, count);
	}
}

void Wiz::rawPixelMemset(void *dstPtr, int value, size_t count) {
	if (_uses16BitColor) {
		WizRawPixel16 *dst16Bit = (WizRawPixel16 *)dstPtr;
		for (size_t i = 0; i < count; i++)
			WRITE_UINT16(&dst16Bit[i], value);
	} else {
		WizRawPixel8 *dst8Bit = (WizRawPixel8 *)dstPtr;
		memset(dst8Bit, value, count);
	}
}

WizRawPixel Wiz::convert8BppToRawPixel(WizRawPixel value, const WizRawPixel *conversionTable) {
	if (_uses16BitColor) {
		return FROM_LE_16(*(((const WizRawPixel16 *)conversionTable) + value));
	} else {
		return value;
	}
}

bool Wiz::compareDoPixelStreamsOverlap(const WizRawPixel *a, const WizRawPixel *b, int width, WizRawPixel transparentColor) {
	const WizRawPixel8 *a8 = (const WizRawPixel8 *)a;
	const WizRawPixel8 *b8 = (const WizRawPixel8 *)b;

	const WizRawPixel16 *a16 = (const WizRawPixel16 *)a;
	const WizRawPixel16 *b16 = (const WizRawPixel16 *)b;

	if (!_uses16BitColor) {
		for (int i = 0; i < width; i++) {
			if ((*a8++ != (WizRawPixel8)transparentColor) && (*b8++ != (WizRawPixel8)transparentColor)) {
				return true;
			}
		}
	} else {
		for (int i = 0; i < width; i++) {
			if ((FROM_LE_16(*a16++) != (WizRawPixel16)transparentColor) && (FROM_LE_16(*b16++) != (WizRawPixel16)transparentColor)) {
				return true;
			}
		}
	}

	return false;
}

void Wiz::rawPixelExtractComponents(WizRawPixel aPixel, int &r, int &g, int &b) {
	r = (aPixel & WIZRAWPIXEL_R_MASK) >> WIZRAWPIXEL_R_SHIFT;
	g = (aPixel & WIZRAWPIXEL_G_MASK) >> WIZRAWPIXEL_G_SHIFT;
	b = (aPixel & WIZRAWPIXEL_B_MASK) >> WIZRAWPIXEL_B_SHIFT;
}

void Wiz::rawPixelPackComponents(WizRawPixel &aPixel, int r, int g, int b) {
	aPixel = (r << WIZRAWPIXEL_R_SHIFT) | (g << WIZRAWPIXEL_G_SHIFT) | (b << WIZRAWPIXEL_B_SHIFT);
}

} // End of namespace Scumm

#endif // ENABLE_HE
