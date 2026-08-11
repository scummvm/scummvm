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

#include "bagel/spacebar/baglib/pan_bitmap.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/baglib/paint_table.h"

namespace Bagel {
namespace SpaceBar {

const CBofRect CBagPanBitmap::getMaxView(CBofSize s) {
	CBofRect r;

	if (_bPanorama) {
		int h = 3 * (int)((double)width() / (MAX_DIV_VIEW * 4));
		if (h > height())
			h = height();

		r.setRect(0, 0, (int)(width() / MAX_DIV_VIEW - 2), h - 1);

	} else {
		r.setRect(0, 0, width() - 1, height() - 1);
	}

	if (s.cx > 0 && r.right > s.cx) {
		// r.left = s.cx;
		r.right = s.cx - 1;
	}

	if (s.cy > 0 && r.bottom > s.cy) {
		r.bottom = s.cy - 1;
	}

	return r;
}

CBagPanBitmap::CBagPanBitmap(const char *pszFileName, CBofPalette *pPalette, const CBofRect &xViewSize) :
	CBofBitmap(pszFileName, pPalette, true) {
	int nW = width();
	int nH = height();

	_bPanorama = false;
	_pCosineTable = nullptr;
	_bActiveScrolling = false; // The scrolling is not active
	_xDirection = kDirNONE;    // Direction is not moving
	_xFOVAngle = 0;
	_nCorrWidth = 0;
	_nNumDegrees = 0;


	if (nW && nH) {
		CBofRect xMaxViewSize(0, 0, nW - 1, nH - 1);
		if (nW > 1000) {
			xMaxViewSize.left = (long)(nW / MAX_DIV_VIEW);
			_bPanorama = true;
		}

		pPalette = getPalette();

		if (xViewSize.isRectEmpty())
			_xCurrView = xMaxViewSize;
		else
			_xCurrView = xViewSize;

		if (_xCurrView.width() > xMaxViewSize.width()) {
			_xCurrView.setRect(0, _xCurrView.top, xMaxViewSize.width() - 1, _xCurrView.bottom);
		}

		_xRotateRate.x = (nW - _xCurrView.width()) / 64 + 1;
		_xRotateRate.y = (nH - _xCurrView.height()) / 64 + 1;

		normalizeViewSize();

		// If FOV is set to 0 then unity FOV is assumed (faster redraws)
		// The update parameter is set to false to avoid to update the cosine table as _nCorrWidth isn't set yet
		setFOV(DEF_FOV, false);

		// Initialize _nCorrWidth and generate the cosine table.
		if (_bPanorama)
			setCorrWidth(4, true);
		else
			setCorrWidth(0, true);

		_bIsValid = true;

		return;
	}

	_bIsValid = false;
}

CBagPanBitmap::~CBagPanBitmap() {
	assert(isValidObject(this));

	delete[] _pCosineTable;
	_pCosineTable = nullptr;
}

// This must be updated whenever the size, view size, or correction width changes
void CBagPanBitmap::generateCosineTable() {
	int nWidth = 1 << _nCorrWidth;
	int offset = nWidth >> 1; // This is not really needed just more correction to move angle to center
	int viewWidth = _xCurrView.width();

	_nNumDegrees = (viewWidth >> _nCorrWidth) + 1;

	delete[] _pCosineTable;
	_pCosineTable = new CBofFixed[_nNumDegrees];

	for (int i = 0; i < _nNumDegrees; i++) {
		double inArea = (double)(offset + i * nWidth) / viewWidth;
		_pCosineTable[i] = (CBofFixed)(cos(_xFOVAngle * (-1.0 + 2.0 * inArea)));
	}
}

CBofRect CBagPanBitmap::getWarpSrcRect() {
	int nH2 = height() >> 1;

	return CBofRect(_xCurrView.left,
	                nH2 + (int)(*_pCosineTable * CBofFixed(_xCurrView.top - nH2)),
	                _xCurrView.right,
	                nH2 + (int)(*_pCosineTable * CBofFixed(_xCurrView.bottom - nH2)));
}

CBofPoint CBagPanBitmap::warpedPoint(CBofPoint &xPoint) {
	CBofRect r = getRect();
	int nH2 = height() >> 1;
	int nW = width();
	int nWidth = 1 << _nCorrWidth; // It may no longer be necessary to store corr width as a shift arg
	int nCenter = r.top + nH2;

	int nOffset = (xPoint.x - _xCurrView.left); // nWidth;
	if (nOffset < 0)
		nOffset += nW;
	nOffset /= nWidth;

	if ((nOffset < 0) || ((xPoint.x - _xCurrView.left) > _xCurrView.width()) || (nOffset >= _nNumDegrees))
		return CBofPoint(0, 0);

	CBofFixed srcHeight = _pCosineTable[nOffset];

	return CBofPoint(xPoint.x, nCenter + (int)(srcHeight * CBofFixed(xPoint.y - nH2)));
}

ErrorCode CBagPanBitmap::paintWarped(CBofBitmap *pBmp, const CBofRect &dstRect, const CBofRect &srcRect, const int offset, CBofBitmap *pSrcBmp, const CBofRect &preSrcRect) {
	int nH2 = _nDY >> 1;
	int nWidth = 1 << _nCorrWidth; // It may no longer be necessary to store corr width as a shift arg
	CBofFixed *pSrcHeight = &_pCosineTable[offset >> _nCorrWidth];
	CBofFixed srcTop = preSrcRect.top + srcRect.top - nH2;
	CBofFixed srcBottom = preSrcRect.top + srcRect.bottom - nH2;
	int nCenter = nH2;

	int nTop = nCenter - preSrcRect.top;
	int nRight = (srcRect.left + nWidth) - 1;

	CBofRect PanSrcRect;
	CBofRect WndDstRect(dstRect.left + 0, dstRect.top, (dstRect.left + 0 + nWidth) - 1, dstRect.bottom);

	pBmp->lock();
	pSrcBmp->lock();

	int nIncrement = 1;

	if (nWidth < 4) {
		for (int i = 0; i < dstRect.width(); i += nWidth) {
			// Set the source
			//
			PanSrcRect.setRect(srcRect.left + i,
			                   nTop + (int)(*pSrcHeight * srcTop),
			                   nRight + i,
			                   nTop + (int)(*pSrcHeight * srcBottom));

			pSrcBmp->paint(pBmp, &WndDstRect, &PanSrcRect);

			WndDstRect.left = WndDstRect.right + 1;
			WndDstRect.right = WndDstRect.right + nWidth;
			pSrcHeight += nIncrement;
		}
	} else if (nWidth == 4) {
		int tableSlot = srcRect.top + preSrcRect.top;
		int stripNumber = 0;

		for (int i = 0; i < dstRect.width(); i += nWidth, stripNumber++) {
			// Set the source
			PanSrcRect.setRect(srcRect.left + i,
			                   STRIP_POINTS[tableSlot][stripNumber]._top,
			                   nRight + i,
			                   STRIP_POINTS[tableSlot][stripNumber]._bottom);

			pSrcBmp->paintStretch4(pBmp, &WndDstRect, &PanSrcRect);

			WndDstRect.left = WndDstRect.right + 1;
			WndDstRect.right = WndDstRect.right + nWidth;
		}
	} else { // nWidth > 4
		for (int i = 0; i < dstRect.width(); i += nWidth) {
			// Set the source
			PanSrcRect.setRect(srcRect.left + i,
			                   nTop + (int)(*pSrcHeight * srcTop),
			                   nRight + i,
			                   nTop + (int)(*pSrcHeight * srcBottom));

			pSrcBmp->paintStretchOpt(pBmp, &WndDstRect, &PanSrcRect, nWidth);

			WndDstRect.left = WndDstRect.right + 1;
			WndDstRect.right = WndDstRect.right + nWidth;
			pSrcHeight += nIncrement;
		}
	}

	pSrcBmp->unlock();
	pBmp->unlock();

	return _errCode;
}

ErrorCode CBagPanBitmap::paintUncorrected(CBofBitmap *pBmp, CBofRect &dstRect) {
	int tmp = _nCorrWidth;
	_nCorrWidth = 0;

	CBofFixed fONE(1);
	CBofFixed fH2(height() / 2);
	CBofFixed fCos(_pCosineTable[0]);
	int nOffset = (int)((fONE - fCos) * fH2);

	dstRect = getCurrView();
	CBofRect tmpRect = dstRect;
	dstRect.top -= nOffset;
	dstRect.bottom += nOffset;

	if (dstRect.top < 0) {
		dstRect.bottom = dstRect.height() - 1;
		dstRect.top = 0;
	}
	if (dstRect.height() >= pBmp->height()) {
		dstRect.bottom = dstRect.top + pBmp->height() - 2;
	}
	_xCurrView = dstRect;

	paint(pBmp);
	_xCurrView = tmpRect;

	_nCorrWidth = tmp;

	return _errCode;
}

ErrorCode CBagPanBitmap::paint(CBofBitmap *pBmp, const CBofPoint xDstOffset) {
	CBofRect dstRect;
	CBofRect srcRect = _xCurrView;
	int nW = width();
	int viewWidth = _xCurrView.width();
	srcRect.right = _xCurrView.left + viewWidth - 1;
	int nOffset = srcRect.right - nW;

	dstRect.top = xDstOffset.y;
	dstRect.bottom = dstRect.top + srcRect.height() - 1;

	// If the right side of the view is the beginning of the panorama
	// paint the un-wrapped side (right) first.
	dstRect.left = xDstOffset.x;
	dstRect.right = xDstOffset.x + viewWidth - 1;

	if (nOffset > 0) {
		CBofRect srcRect2 = srcRect;
		srcRect2.left = 0;
		srcRect2.right = nOffset;
		dstRect.right = xDstOffset.x + viewWidth - 1;
		dstRect.left = dstRect.right - nOffset;

		CBofBitmap::paint(pBmp, &dstRect, &srcRect2);

		srcRect.right = nW - 1;
		dstRect.right = dstRect.left;
		dstRect.left = xDstOffset.x;
	}

	CBofBitmap::paint(pBmp, &dstRect, &srcRect);

	return _errCode;
}

CBagPanBitmap::Direction CBagPanBitmap::updateView() {
	if (_bActiveScrolling) {
		if (_xDirection & kDirLEFT)
			rotateRight();
		else if (_xDirection & kDirRIGHT)
			rotateLeft();

		if (_xDirection & kDirUP)
			rotateUp();
		else if (_xDirection & kDirDOWN)
			rotateDown();

		return _xDirection;
	}

	return kDirNONE;
}

void CBagPanBitmap::setCorrWidth(int nWidth, bool bUpdate) {
	int i = 0;

	while (nWidth >>= 1)
		++i;

	if (i < 6) {
		_nCorrWidth = i;
		if (bUpdate) {
			generateCosineTable();
		}
	}
}

void CBagPanBitmap::rotateRight(int nXRotRate) {
	if (nXRotRate > 0)
		offsetCurrView(CBofPoint(nXRotRate, 0));
	else
		offsetCurrView(CBofPoint(_xRotateRate.x, 0));
}

void CBagPanBitmap::rotateLeft(int nXRotRate) {
	if (nXRotRate > 0)
		offsetCurrView(CBofPoint(-1 * nXRotRate, 0));
	else
		offsetCurrView(CBofPoint(-1 * _xRotateRate.x, 0));
}

void CBagPanBitmap::rotateUp(int nYRotRate) {
	if (nYRotRate > 0)
		offsetCurrView(CBofPoint(0, nYRotRate));
	else
		offsetCurrView(CBofPoint(0, _xRotateRate.y));
}

void CBagPanBitmap::rotateDown(int nYRotRate) {
	if (nYRotRate > 0)
		offsetCurrView(CBofPoint(0, -1 * nYRotRate));
	else
		offsetCurrView(CBofPoint(0, -1 * _xRotateRate.y));
}

void CBagPanBitmap::normalizeViewSize() {
	int nW = width();
	int nH = height();

	if (_bPanorama) {
		// The CurrView can not be more than 0.25Width x Height of the Bitmap
		if ((_xCurrView.width() >= nW / MAX_DIV_VIEW) || (_xCurrView.width() <= 0))
			_xCurrView.right = (long)(_xCurrView.left + nW / MAX_DIV_VIEW - 1);
		if ((_xCurrView.height() >= nH) || (_xCurrView.height() <= 0))
			_xCurrView.bottom = _xCurrView.top + nH - 1;

		// The Base coords of CurrView must exist within the rectangle
		while (_xCurrView.left < 0)
			_xCurrView.offsetRect(nW, 0);
		while (_xCurrView.left >= nW)
			_xCurrView.offsetRect(-1 * nW, 0);
	} else { // Not a panorama
		// The Base coords of CurrView must exist within the rectangle
		if (_xCurrView.left < 0)
			_xCurrView.offsetRect(-_xCurrView.left, 0);
		else if (_xCurrView.right > nW)
			_xCurrView.offsetRect(nW - _xCurrView.right, 0);
	}

	// We never have up and down wrap around
	if (_xCurrView.top < 0)
		_xCurrView.offsetRect(0, -_xCurrView.top);
	if (_xCurrView.bottom >= nH)
		_xCurrView.offsetRect(0, (nH - _xCurrView.bottom) - 1);
}


void CBagPanBitmap::setCurrView(const CBofRect &xCurrView) {
	_xCurrView = xCurrView;
	normalizeViewSize();
}

void CBagPanBitmap::offsetCurrView(const CBofPoint &xOffset) {
	CBofRect xCurrView = _xCurrView;
	xCurrView.offsetRect(xOffset);
	setCurrView(xCurrView);
}

void CBagPanBitmap::setFOV(double degrees, bool bUpdate) {
	_xFOVAngle = degrees / 114.5916558176;
	if (bUpdate) {
		generateCosineTable();
	}
}

void CBagPanBitmap::setViewSize(const CBofSize &xViewSize, bool bUpdate) {
	_xCurrView.right = _xCurrView.left + xViewSize.cx;
	_xCurrView.bottom = _xCurrView.top + xViewSize.cy;
	normalizeViewSize();

	if (bUpdate) {
		generateCosineTable();
	}
}

CBofSize CBagPanBitmap::setUnityViewSize() {
	int w = (int)(width() * _xFOVAngle / 3.14159);
	_xCurrView.setRect(0, _xCurrView.top, w, _xCurrView.bottom);
	generateCosineTable();
	return getViewSize();
}

double CBagPanBitmap::setUnityFOV() {
	setFOV(360.0 * _xCurrView.width() / width(), false); // If FOV is set to 0 then unity FOV is assumed (faster redraws)
	generateCosineTable();
	return getFOV();
}

} // namespace SpaceBar
} // namespace Bagel
