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

#include "common/archive.h"
#include "common/system.h"
#include "common/math.h"
#include "graphics/cursorman.h"
#include "graphics/primitives.h"
#include "scumm/he/intern_he.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/util.h"
#include "scumm/he/wiz_he.h"
#include "scumm/he/moonbase/moonbase.h"

namespace Scumm {

int Wiz::pgReadPixel(const WizSimpleBitmap *srcBM, int x, int y, int defaultValue) {
	if ((x < 0) || (y < 0) || (x >= srcBM->bitmapWidth) || (y >= srcBM->bitmapHeight)) {
		return defaultValue;
	} else {
		if (_uses16BitColor) {
			return *(srcBM->bufferPtr + y * srcBM->bitmapWidth + x);
		} else {
			return *(((uint8 *)srcBM->bufferPtr) + y * srcBM->bitmapWidth + x);
		}
	}
}

void Wiz::pgWritePixel(WizSimpleBitmap *srcBM, int x, int y, WizRawPixel value) {
	if ((x >= 0) && (y >= 0) && (x < srcBM->bitmapWidth) && (y < srcBM->bitmapHeight)) {
		if (_uses16BitColor) {
			*(srcBM->bufferPtr + y * srcBM->bitmapWidth + x) = value;
		} else {
			*(((uint8 *)srcBM->bufferPtr) + y * srcBM->bitmapWidth + x) = value;
		}
	}
}

void Wiz::pgClippedWritePixel(WizSimpleBitmap *srcBM, int x, int y, const Common::Rect *clipRectPtr, WizRawPixel value) {
	if ((x >= clipRectPtr->left) && (y >= clipRectPtr->top) && (x <= clipRectPtr->right) && (y <= clipRectPtr->bottom)) {
		if (_uses16BitColor) {
			*(srcBM->bufferPtr + y * srcBM->bitmapWidth + x) = value;
		} else {
			*(((uint8 *)srcBM->bufferPtr) + y * srcBM->bitmapWidth + x) = value;
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

	// Get deltas
	iDX = iX2 - iX1;
	iDY = iY2 - iY1;

	// Protect against horizontal lines
	if (iDY <= 0) {
		return 0;
	}

	// Calculate inverse slope
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

	// if distance is 0, don't draw
	if (fDistance == 0) {
		return;
	}

	// The ratio of half the line thickness to the length of the line...
	double fScale = (double)(iLineThickness) / (2 * fDistance);

	// The x and y increments from an endpoint needed to create a rectangle...
	float fDDX = -fScale * (float)iDY;
	float fDDY = fScale * (float)iDX;

	if (fDDX > 0) { // round off
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

	// determine point order of polygons based on line orientation
	// topmost (and topleft in case of x or y = 0)
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

	int *xPoints = new int[4];
	int *yPoints = new int[4];
	int aLeftNeighborTable[4] = {1, 2, 3, 0};
	int aRightNeighborTable[4] = {3, 2, 1, 0};

	xPoints[aSortOrder[0]] = asx - iCompDX;
	yPoints[aSortOrder[0]] = asy - iCompDY;

	xPoints[aSortOrder[1]] = asx + iCompDX;
	yPoints[aSortOrder[1]] = asy + iCompDY;

	xPoints[aSortOrder[2]] = aex - iCompDX;
	yPoints[aSortOrder[2]] = aey - iCompDY;

	xPoints[aSortOrder[3]] = aex + iCompDX;
	yPoints[aSortOrder[3]] = aey + iCompDY;

	// determine how tall (how many scan lines) the polygon is
	int iNumScanLines = yPoints[2] - yPoints[0];

	// create scan line list
	WizScanLine *aScanLines = new WizScanLine[iNumScanLines];

	// scan left side
	int iNextScanLine = scanEdge(aScanLines, 0, true, xPoints[0], yPoints[0], xPoints[1], yPoints[1]);
	scanEdge(aScanLines, iNextScanLine, true, xPoints[1], yPoints[1], xPoints[2], yPoints[2]);

	// scan right side
	iNextScanLine = scanEdge(aScanLines, 0, false, xPoints[0], yPoints[0], xPoints[3], yPoints[3]);
	scanEdge(aScanLines, iNextScanLine, false, xPoints[3], yPoints[3], xPoints[2], yPoints[2]);

	// draw polygon
	int iCurX, iCurY;
	for (int iWhichScanLine = 0; iWhichScanLine < iNumScanLines; ++iWhichScanLine) {
		iCurY = iWhichScanLine + yPoints[0];
		for (iCurX = aScanLines[iWhichScanLine].iXMin; iCurX < aScanLines[iWhichScanLine].iXMax; ++iCurX) {
			pgClippedWritePixel(destBM, iCurX, iCurY, clipRectPtr, value);
		}
	}
}

int convertToFixed(int iNumber) {
	return (iNumber << kWECFixedSize);
}

int convertFromFixed(int iFixedNumber) {
	return (iFixedNumber >> kWECFixedSize);
}

void Wiz::pgDrawClippedEllipse(WizSimpleBitmap *pDestBitmap, int iPX, int iPY, int iQX, int iQY, int iKX, int iKY, int iLOD, const Common::Rect *pClipRectPtr, int iThickness, WizRawPixel aColor) {
	// since this is fixed point, limit the LOD to 14 bits precision
	if (iLOD > kWECFixedSize - 2) {
		iLOD = kWECFixedSize - 2;
	}

	// determine the rest of the rectangle verts using the midpoint formula
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

	// Loop through each quadrant
	for (int iSide = 0; iSide < 4; ++iSide) {
		int xP = 0, yP = 0, xQ = 0, yQ = 0, xK = 0, yK = 0;

		// Determine quadrant and convert input to fixed point
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

		vx = xK - xQ; // Displacements from center
		ux = xK - xP;
		vy = yK - yQ;
		uy = yK - yP;
		xJ = xP - vx + kWECHalf; // Center of ellipse J
		yJ = yP - vy + kWECHalf;

		ux -= (w = ux >> (2 * iLOD + 3)); // Cancel 2nd-order error
		ux -= (w >>= (2 * iLOD + 4));     // Cancel 4th-order error
		ux -= w >> (2 * iLOD + 3);        // Cancel 6th-order error
		ux += vx >> (iLOD + 1);           // Cancel 1st-order error
		uy -= (w = uy >> (2 * iLOD + 3)); // Cancel 2nd-order error
		uy -= (w >>= (2 * iLOD + 4));     // Cancel 4th-order error
		uy -= w >> (2 * iLOD + 3);        // Cancel 6th-order error
		uy += vy >> (iLOD + 1);           // Cancel 1st-order error

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
					// specific to print pack, should be moved to a thick pixel
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
	WizRawPixel *d;
	uint8 *d8bit;
	int cw, dw, ch;
	int x1 = rectPtr->left, y1 = rectPtr->top, x2 = rectPtr->right, y2 = rectPtr->bottom;

	// Common calcs...
	dw = destBM->bitmapWidth;
	cw = x2 - x1 + 1;
	ch = y2 - y1 + 1;
	d = destBM->bufferPtr + y1 * dw + x1;

	d8bit = ((uint8 *)destBM->bufferPtr) + y1 * dw + x1;

	if (cw > 1) {
		while (--ch >= 0) {
			if (_uses16BitColor) {
				for (int i = 0; i < dw; i++)
					WRITE_LE_UINT16(&d[i], color);
				d += dw;
			} else {
				memset(d8bit, color, cw);
				d8bit += dw;
			}
		}
	} else {
		while (--ch >= 0) {
			if (_uses16BitColor) {
				*d = color;
				d += dw;
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

	// Call the primitive
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

void Wiz::pgSimpleBitmapFromDrawBuffer(WizSimpleBitmap *bitmapPtr, bool background) {
	VirtScreen *vs = &_vm->_virtscr[kMainVirtScreen];

	bitmapPtr->bitmapWidth = vs->w;
	bitmapPtr->bitmapHeight = vs->h;

	if (background) {
		bitmapPtr->bufferPtr = (WizRawPixel *)vs->getBackPixels(0, vs->topline);
	} else {
		bitmapPtr->bufferPtr = (WizRawPixel *)vs->getPixels(0, vs->topline);
	}
}

bool Wiz::findRectOverlap(Common::Rect *destRectPtr, const Common::Rect *sourceRectPtr) {
	// Make sure only points inside the viewPort are being set...
	if ((destRectPtr->left > sourceRectPtr->right) || (destRectPtr->top > sourceRectPtr->bottom) ||
		(destRectPtr->right < sourceRectPtr->left) || (destRectPtr->bottom < sourceRectPtr->top)) {
		return false;
	}

	// Perform the actual clipping
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

int Wiz::getRawPixel(int color) {
	if (_uses16BitColor) {
		return color & 0xFFFF;
	} else {
		return color & 0xFF;
	}
}

} // End of namespace Scumm

#endif // ENABLE_HE
