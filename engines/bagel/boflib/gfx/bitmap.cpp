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

#include "common/file.h"
#include "common/system.h"
#include "image/bmp.h"
#include "image/png.h"

#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/file.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

#define TEST_DEBUG 0

bool CBofBitmap::_bUseBackdrop = false;

extern bool g_bRealizePalette;

CBofBitmap::CBofBitmap() {
	_szFileName[0] = '\0';
}

CBofBitmap::CBofBitmap(int dx, int dy, CBofPalette *pPalette, bool bOwnPalette, byte *pPrivateBuff) {
	assert((dx > 0) && (dy > 0));

	// Use application's palette if not supplied
	//
	if (pPalette == nullptr) {
		bOwnPalette = false;
		pPalette = CBofApp::getApp()->getPalette();
	}

	// Inits
	//
	_nDX = dx;
	_nDY = dy;
	_nScanDX = ((dx + 3) & ~3);
	_bTopDown = true;
	_bOwnPalette = bOwnPalette;
	_bReadOnly = false;
	_szFileName[0] = '\0';
	_pPalette = nullptr;
	_bInitialized = true;

	// Allow privatization of the bitmap (used only on mac from displayTextEx).;
	_pBits = pPrivateBuff;
	if (pPrivateBuff != nullptr) {
		_bitmap.w = dx;
		_bitmap.h = dy;
		_bitmap.pitch = _nScanDX;
		_bitmap.format = Graphics::PixelFormat::createFormatCLUT8();
		_bitmap.setPixels(pPrivateBuff);

	}

	_pPalette = pPalette;
	load();
}

CBofBitmap::CBofBitmap(const char *pszFileName, CBofPalette *pPalette, bool bOwnPalette) {
	assert(pszFileName != nullptr);
	_bOwnPalette = bOwnPalette;
	_szFileName[0] = '\0';
	_bInitialized = true;

	if (pPalette == nullptr) {
		pPalette = new CBofPalette(pszFileName);
		_bOwnPalette = true;
	}

	// Init the info needed to load a bitmap from disk
	_pPalette = pPalette;

	if (fileGetFullPath(_szFileName, pszFileName) != nullptr) {
		// Load this bitmap into the cache
		load();
	} else {
		reportError(ERR_FFIND, "Could not build full path to %s", pszFileName);
	}
}

CBofBitmap::~CBofBitmap() {
	assert(isValidObject(this));

	release();

	if (_bOwnPalette && (_pPalette != nullptr)) {
		delete _pPalette;
		_bOwnPalette = false;
	}

	_pPalette = nullptr;
	_bInitialized = false;
}


ErrorCode CBofBitmap::buildBitmap(CBofPalette *pPalette) {
	assert(isValidObject(this));
	assert(pPalette != nullptr);

	if (_errCode == ERR_NONE) {
		_bitmap.create(_nDX, _nDY, Graphics::PixelFormat::createFormatCLUT8());
		_pBits = (byte *)_bitmap.getBasePtr(0, 0);
		_nScanDX = _bitmap.pitch;

		// Set this bitmap's palette
		setPalette(pPalette, _bOwnPalette);
	}

	return _errCode;
}

bool CBofBitmap::alloc() {
	if (_szFileName[0] != '\0') {
		loadBitmap(_szFileName, _pPalette);
	} else {
		buildBitmap(_pPalette);
	}

	return !errorOccurred();
}

void CBofBitmap::free() {
	releaseBitmap();
}

void CBofBitmap::releaseBitmap() {
	assert(isValidObject(this));

	_bitmap.clear();
}

ErrorCode CBofBitmap::loadBitmap(const char *pszFileName, CBofPalette *pPalette) {
	assert(isValidObject(this));
	assert(pszFileName != nullptr);
	assert(pPalette != nullptr);

	// Release any previous bitmap info
	releaseBitmap();

	if (_errCode == ERR_NONE) {
		CBofFile *pFile = new CBofFile(pszFileName, CBOFFILE_READONLY);

		// Open bitmap
		// filename must fit into our buffer
		assert(strlen(pszFileName) < MAX_FNAME);

		// Keep track of this filename
		Common::strcpy_s(_szFileName, pszFileName);

		// Decode the bitmap
		Image::BitmapDecoder decoder;
		Common::SeekableReadStream *rs = *pFile;
		if (!rs || !decoder.loadStream(*rs))
			error("Could not load bitmap %s", pszFileName);

		// Load up the decoded bitmap
		_bitmap.copyFrom(*decoder.getSurface());

		// Load the bitmap palette
		_bitmap.setPalette(decoder.getPalette(), 0, PALETTE_COUNT);

		_nDX = _bitmap.w;
		_nDY = _bitmap.h;
		_nScanDX = _bitmap.pitch;
		_pBits = (byte*)_bitmap.getBasePtr(0, 0);

		// Close bitmap file
		delete pFile;
	}

	return _errCode;
}

ErrorCode CBofBitmap::paint(CBofWindow *pWnd, int x, int y, CBofRect *pSrcRect, int nMaskColor) {
	assert(isValidObject(this));
	assert(pWnd != nullptr);

	CBofRect cRect(x, y, x + _nDX - 1, y + _nDY - 1);

	if (pSrcRect != nullptr) {
		cRect.setRect(x, y, x + pSrcRect->width() - 1, y + pSrcRect->height() - 1);
	}

	return paint(pWnd, &cRect, pSrcRect, nMaskColor);
}

ErrorCode CBofBitmap::paint(CBofWindow *pWnd, CBofRect *pDstRect, CBofRect *pSrcRect, int nMaskColor) {
	assert(isValidObject(this));
	assert(pWnd != nullptr);

	Graphics::ManagedSurface *dstSurf = pWnd->getSurface();
	CBofRect cDestRect(0, 0, _nDX - 1, _nDY - 1), cSourceRect(0, 0, _nDX - 1, _nDY - 1);

	if (_errCode == ERR_NONE) {
		if (pDstRect != nullptr) {
			assert((pDstRect->width() > 0) && (pDstRect->height() > 0));
			cDestRect = *pDstRect;
		}

		if (pSrcRect != nullptr) {
			assert((pSrcRect->width() > 0) && (pSrcRect->height() > 0));
			cSourceRect = *pSrcRect;
		}

		if (_bitmap.format.bytesPerPixel == 1) {
			// Bitmap is paletted, so ensure it's palette is updated
			const HPALETTE &pal = _pPalette->getPalette();
			_bitmap.setPalette(pal._data, 0, pal._numColors);
		}

		// Handle the blitting
		if (nMaskColor == NOT_TRANSPARENT) {
			dstSurf->blitFrom(_bitmap, cSourceRect, cDestRect);
		} else {
			dstSurf->transBlitFrom(_bitmap, cSourceRect, cDestRect, nMaskColor);
		}
	}

	return _errCode;
}

ErrorCode CBofBitmap::paintMaskBackdrop(CBofWindow *pWnd, CBofRect *pDstRect, CBofRect *pSrcRect, int nMaskColor) {
	assert(isValidObject(this));
	assert(pWnd != nullptr);

	// This function requires the use of a backdrop
	assert(pWnd->getBackdrop() != nullptr);

	CBofRect cDestRect(0, 0, _nDX - 1, _nDY - 1), cSourceRect(0, 0, _nDX - 1, _nDY - 1);

	if (_errCode == ERR_NONE) {

		if (pDstRect != nullptr) {
			assert((pDstRect->width() > 0) && (pDstRect->height() > 0));
			cDestRect = *pDstRect;
		}

		if (pSrcRect != nullptr) {
			assert((pSrcRect->width() > 0) && (pSrcRect->height() > 0));

			cSourceRect = *pSrcRect;
		}

		CBofBitmap cTempBitmap(cSourceRect.width(), cSourceRect.height(), _pPalette);
		CBofBitmap *pBackdrop = pWnd->getBackdrop();

		// Use the backdrop to get the background instead of trying to
		// capture the screen
		if (pBackdrop != nullptr) {
			CBofRect cTempRect = cTempBitmap.getRect();
			pBackdrop->paint(&cTempBitmap, &cTempRect, &cDestRect);
		}

		paint(&cTempBitmap, 0, 0, &cSourceRect, nMaskColor);
		cTempBitmap.paint(pWnd, &cDestRect);

		unlock();
	}

	return _errCode;
}

ErrorCode CBofBitmap::paint(CBofBitmap *pBmp, int x, int y, CBofRect *pSrcRect, int nMaskColor) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);

	CBofRect cRect(x, y, x + _nDX - 1, y + _nDY - 1);

	if (pSrcRect != nullptr) {
		cRect.setRect(x, y, x + pSrcRect->width() - 1, y + pSrcRect->height() - 1);
	}

	return paint(pBmp, &cRect, pSrcRect, nMaskColor);
}

ErrorCode CBofBitmap::paint(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect, int nMaskColor) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);

	CBofRect cDestRect(0, 0, _nDX - 1, _nDY - 1), cSourceRect(0, 0, _nDX - 1, _nDY - 1);
	CBofRect cClipRect;

	if (!errorOccurred() && !pBmp->errorOccurred()) {
		if (pDstRect != nullptr) {
			assert((pDstRect->width() > 0) && (pDstRect->height() > 0));
			cDestRect = *pDstRect;
		}

		if (pSrcRect != nullptr) {
			assert((pSrcRect->width() > 0) && (pSrcRect->height() > 0));
			cSourceRect = *pSrcRect;
		}

		// If painting entirely outside destination bitmap, then no need to paint.
		CBofRect cRect = pBmp->getRect();
		if (cClipRect.intersectRect(&cRect, &cDestRect) == 0) {
			return _errCode;
		}

		// Clip so we don't paint outside of the bitmap's memory buffer
		cSourceRect.left += cClipRect.left - cDestRect.left;
		cSourceRect.right += cClipRect.right - cDestRect.right;
		cSourceRect.top += cClipRect.top - cDestRect.top;
		cSourceRect.bottom += cClipRect.bottom - cDestRect.bottom;
		cDestRect = cClipRect;

		// If painting from entirely outside this bitmap, then don't paint at all.
		cRect = getRect();
		if (cClipRect.intersectRect(&cRect, &cSourceRect) == 0) {
			return _errCode;
		}

		// Clip so we don't paint outside of the bitmap's memory buffer
		cDestRect.left += cClipRect.left - cSourceRect.left;
		cDestRect.right += cClipRect.right - cSourceRect.right;
		cDestRect.top += cClipRect.top - cSourceRect.top;
		cDestRect.bottom += cClipRect.bottom - cSourceRect.bottom;
		cSourceRect = cClipRect;

		// Lock these bitmaps down so we can start painting
		lock();
		pBmp->lock();

		// Do the actual painting.
		// Since we are copying from bitmap to bitmap, using the
		// blit bitmap routine is a kosher thing to do, there is no screen drawing
		if (nMaskColor == NOT_TRANSPARENT) {
			pBmp->_bitmap.blitFrom(_bitmap, cSourceRect, cDestRect);
		} else {
			pBmp->_bitmap.transBlitFrom(_bitmap, cSourceRect, cDestRect, nMaskColor);
		}

		// Don't need a lock on these guys anymore
		pBmp->unlock();
		unlock();
	}

	return _errCode;
}

ErrorCode CBofBitmap::paint1To1(CBofBitmap *pBmp) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);

	if (!errorOccurred() && !pBmp->errorOccurred()) {

		lock();
		pBmp->lock();

		// Direct 1 to 1 copy
		memcpy(pBmp->_pBits, _pBits, _nScanDX * _nDY);

		pBmp->unlock();
		unlock();
	}

	return _errCode;
}

ErrorCode CBofBitmap::paintStretch4(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);
	assert(pDstRect != nullptr);
	assert(pSrcRect != nullptr);

	// These bitmaps MUST be locked down before hand
	assert(isLocked());
	assert(pBmp->isLocked());

	if (_errCode == ERR_NONE) {
		int dy1 = _nDY;
		int dx1 = _nScanDX;

		int dy2 = pBmp->_nDY;
		int dx2 = pBmp->_nScanDX;

		byte *pDestBits = pBmp->_pBits;
		byte *pSrcBits = _pBits;

		int32 dy = pSrcRect->height();

		int32 nDstHeight = pDstRect->height();

		int32 x1 = pSrcRect->left;
		int32 y1 = pSrcRect->top;

		int32 x2 = pDstRect->left;
		int32 y2 = pDstRect->top;

		if (_bTopDown) {
			pSrcBits += y1 * dx1 + x1;
		} else {
			pSrcBits += (dy1 - y1 - 1) * dx1 + x1;
			dx1 = -dx1;
		}

		if (pBmp->_bTopDown) {
			pDestBits += y2 * dx2 + x2;
		} else {
			pDestBits += (dy2 - y2 - 1) * dx2 + x2;
			dx2 = -dx2;
		}

		Fixed sourceStepY = fixedDivide(intToFixed(dy), intToFixed(nDstHeight));
		Fixed posY = 0;
		byte *pSrcEnd = pSrcBits + (dy - 1) * dx1;
		byte *pDestEnd = pDestBits + (nDstHeight - 1) * dx2;

		nDstHeight >>= 1;

		while (nDstHeight > 0) {
			nDstHeight -= 4;

			if (posY >= 0x00010000) {

				pSrcBits += dx1;
				pSrcEnd -= dx1;

				posY &= 0x0000FFFF;
			}
			*(uint32 *)pDestBits = *(uint32 *)pSrcBits;
			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;

			posY += sourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;

			if (posY >= 0x00010000) {

				pSrcBits += dx1;
				pSrcEnd -= dx1;

				posY &= 0x0000FFFF;
			}
			*(uint32 *)pDestBits = *(uint32 *)pSrcBits;
			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;

			posY += sourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;

			if (posY >= 0x00010000) {

				pSrcBits += dx1;
				pSrcEnd -= dx1;

				posY &= 0x0000FFFF;
			}
			*(uint32 *)pDestBits = *(uint32 *)pSrcBits;
			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;

			posY += sourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;

			if (posY >= 0x00010000) {

				pSrcBits += dx1;
				pSrcEnd -= dx1;

				posY &= 0x0000FFFF;
			}
			*(uint32 *)pDestBits = *(uint32 *)pSrcBits;
			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;

			posY += sourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;
		}
	}

	return _errCode;
}

ErrorCode CBofBitmap::paintStretchOpt(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect, int nOptSize) {
	assert(isValidObject(this));
	assert(pBmp != nullptr);
	assert(pDstRect != nullptr);
	assert(pSrcRect != nullptr);

	// These bitmaps MUST be locked down before hand
	assert(isLocked());
	assert(pBmp->isLocked());

	if (_errCode == ERR_NONE) {
		int dy1 = _nDY;
		int dx1 = _nScanDX;

		int dy2 = pBmp->_nDY;
		int dx2 = pBmp->_nScanDX;

		byte *pDestBits = pBmp->_pBits;
		byte *pSrcBits = _pBits;

		int32 dy = pSrcRect->height();

		int32 nDstHeight = pDstRect->height();

		int32 x1 = pSrcRect->left;
		int32 y1 = pSrcRect->top;

		int32 x2 = pDstRect->left;
		int32 y2 = pDstRect->top;

		if (_bTopDown) {
			pSrcBits += y1 * dx1 + x1;
		} else {
			pSrcBits += (dy1 - y1 - 1) * dx1 + x1;
			dx1 = -dx1;
		}

		if (pBmp->_bTopDown) {
			pDestBits += y2 * dx2 + x2;
		} else {
			pDestBits += (dy2 - y2 - 1) * dx2 + x2;
			dx2 = -dx2;
		}

		Fixed sourceStepY = fixedDivide(intToFixed(dy), intToFixed(nDstHeight));
		Fixed posY = 0;
		byte *pSrcEnd = pSrcBits + (dy - 1) * dx1;
		byte *pDestEnd = pDestBits + (nDstHeight - 1) * dx2;

		int nMod = (int)nDstHeight & 1;
		nDstHeight >>= 1;

		while (nDstHeight-- > 0) {

			if (posY >= 0x00010000) {

				int32 lInc = fixedToInt(posY) * dx1;
				pSrcBits += lInc;
				pSrcEnd -= lInc;

				posY &= 0x0000FFFF;
			}

			for (int i = 0; i < nOptSize; i += 4) {
				*(uint32 *)(pDestBits + i) = *(uint32 *)(pSrcBits + i);
				*(uint32 *)(pDestEnd + i) = *(uint32 *)(pSrcEnd + i);
			}

			posY += sourceStepY;

			pDestBits += dx2;
			pDestEnd -= dx2;
		}

		if (nMod != 0) {

			*(uint32 *)pDestEnd = *(uint32 *)pSrcEnd;
		}
	}

	return _errCode;
}

ErrorCode CBofBitmap::captureScreen(CBofWindow *pWnd, CBofRect *pSrcRect, CBofRect *pDstRect) {
	assert(isValidObject(this));

	assert(pWnd != nullptr);
	assert(pSrcRect != nullptr);

	if (_errCode == ERR_NONE) {
		CBofRect cDestRect(0, 0, _nDX - 1, _nDY - 1);
		if (pDstRect != nullptr) {
			cDestRect = *pDstRect;
		}

		CBofRect cSrcRect = *pSrcRect;
		CBofBitmap *pBackdrop = pWnd->getBackdrop();

		// If we're capturing the screen, we have to convert the format first.
		if (!_bUseBackdrop || pBackdrop == nullptr) {
			Graphics::Surface *tmp = pWnd->getSurface()->rawSurface().convertTo(_bitmap.format, nullptr, 0, _pPalette->getData(), PALETTE_COUNT);
			_bitmap.blitFrom(*tmp, cSrcRect, cDestRect);
			tmp->free();
			delete tmp;
		} else {
			// Optimization to use the window's backdrop bitmap instead of doing
			// an actual screen capture.
			pBackdrop->paint(this, &cDestRect, &cSrcRect);
		}
	}

	return _errCode;
}

void CBofBitmap::setPalette(CBofPalette *pBofPalette, bool bOwnPalette) {
	assert(isValidObject(this));
	assert(pBofPalette != nullptr);

	if ((_errCode == ERR_NONE) && (pBofPalette != nullptr)) {
		if (_bOwnPalette && (_pPalette != nullptr) && (_pPalette != pBofPalette)) {
			delete _pPalette;
		}

		_bOwnPalette = bOwnPalette;
		_pPalette = pBofPalette;

		_bitmap.setPalette(_pPalette->getData(), 0, PALETTE_COUNT);
	}
}

byte *CBofBitmap::getPixelAddress(int x, int y) {
	assert(isValidObject(this));

	// You can not call this function unless you manually lock this bitmap
	assert(isLocked());

	// The pixel in question must be in the bitmap area
	assert(getRect().ptInRect(CBofPoint(x, y)));

	return (byte *)_bitmap.getBasePtr(x, y);
}

byte CBofBitmap::readPixel(int x, int y) {
	assert(isValidObject(this));

	lock();
	byte chPixel = *getPixelAddress(x, y);
	unlock();

	return chPixel;
}

void CBofBitmap::writePixel(int x, int y, byte iColor) {
	assert(isValidObject(this));

	lock();
	byte *pPixel = getPixelAddress(x, y);
	*pPixel = iColor;
	unlock();
}

void CBofBitmap::circle(int xCenter, int yCenter, uint16 nRadius, byte iColor) {
	assert(isValidObject(this));

	if (_errCode == ERR_NONE) {

		int x = 0;
		int y = nRadius;
		int i = 3 - 2 * y;

		while (x < y) {

			writePixel(xCenter + x, yCenter + y, iColor);
			writePixel(xCenter - x, yCenter + y, iColor);
			writePixel(xCenter + y, yCenter + x, iColor);
			writePixel(xCenter - y, yCenter + x, iColor);
			writePixel(xCenter - x, yCenter - y, iColor);
			writePixel(xCenter + x, yCenter - y, iColor);
			writePixel(xCenter - y, yCenter - x, iColor);
			writePixel(xCenter + y, yCenter - x, iColor);

			if (i <= 0) {
				i += 4 * x + 6;

			} else {

				i +=  4 * (x - y) + 10;
				y--;
			}
			x++;
		}

		if (x == y) {
			writePixel(xCenter + x, yCenter + y, iColor);
			writePixel(xCenter - x, yCenter + y, iColor);
			writePixel(xCenter + y, yCenter + x, iColor);
			writePixel(xCenter - y, yCenter + x, iColor);
			writePixel(xCenter - x, yCenter - y, iColor);
			writePixel(xCenter + x, yCenter - y, iColor);
			writePixel(xCenter - y, yCenter - x, iColor);
			writePixel(xCenter + y, yCenter - x, iColor);
		}
	}
}

void CBofBitmap::drawRect(CBofRect *pRect, byte iColor) {
	assert(isValidObject(this));
	assert(pRect != nullptr);

	if (_errCode == ERR_NONE) {
		line(pRect->left, pRect->top, pRect->right, pRect->top, iColor);
		line(pRect->right, pRect->top, pRect->right, pRect->bottom, iColor);
		line(pRect->right, pRect->bottom, pRect->left, pRect->bottom, iColor);
		line(pRect->left, pRect->bottom, pRect->left, pRect->top, iColor);
	}
}

void CBofBitmap::fillRect(CBofRect *pRect, byte iColor) {
	assert(isValidObject(this));

	if (_errCode == ERR_NONE) {
		if (pRect == nullptr) {
			_bitmap.clear(iColor);
		} else {
			Common::Rect rect(pRect->left, pRect->top, pRect->right, pRect->bottom);
			_bitmap.fillRect(rect, iColor);
		}
	}
}

void CBofBitmap::line(int nSrcX, int nSrcY, int nDstX, int nDstY, byte iColor) {
	assert(isValidObject(this));

	// The source and destination points must be in the bitmap area
	assert(getRect().ptInRect(CBofPoint(nSrcX, nSrcY)));
	assert(getRect().ptInRect(CBofPoint(nDstX, nDstY)));

	if (_errCode == ERR_NONE) {
		// Horizontal lines are a special case that can be done optimally
		if (nSrcY == nDstY) {
			lock();
			memset(getPixelAddress(MIN(nSrcX, nDstX), nSrcY), iColor, ABS(nDstX - nSrcX));
			unlock();

		} else {
			// Otherwise use standard Bresenham Line algorithm
			int dx = nDstX - nSrcX;
			int dy = nDstY - nSrcY;

			int ix = dx < 0 ? (dx = -dx, -1) : !!dx;
			int iy = dy < 0 ? (dy = -dy, -1) : !!dy;

			int distance = MAX(dx, dy);

			int xerr = 0;
			int yerr = 0;
			for (int i = -2; i < distance; i++) {
				writePixel(nSrcX, nSrcY, iColor);

				xerr += dx;
				yerr += dy;

				if (xerr > distance) {
					xerr -= distance;
					nSrcX += ix;
				}

				if (yerr > distance) {
					yerr -= distance;
					nSrcY += iy;
				}
			}
		}
	}
}

void CBofBitmap::line(CBofPoint *pSrc, CBofPoint *pDest, byte iColor) {
	assert(isValidObject(this));
	assert(pSrc != nullptr);
	assert(pDest != nullptr);

	line(pSrc->x, pSrc->y, pDest->x, pDest->y, iColor);
}

ErrorCode CBofBitmap::scrollRight(int nPixels, CBofRect * /*pRect*/) {
	assert(isValidObject(this));

	if (_errCode == ERR_NONE) {
		if (nPixels != 0) {
			assert(_pBits != nullptr);
			byte *pTemp = (byte *)bofAlloc(abs(nPixels));

			int nBytes = _nDX - nPixels;
			if (nPixels < 0) {
				nBytes = _nDX + nPixels;
			}

			byte *p = _pBits;

			lock();

			if (nPixels > 0) {
				for (int i = 0; i < _nDY; i++) {
					memcpy(pTemp, p + nBytes, nPixels);
					memmove(p + nPixels, p, nBytes);
					memcpy(p, pTemp, nPixels);
					p += _nScanDX;
				}
			} else {
				nPixels = -nPixels;

				for (int i = 0; i < _nDY; i++) {
					memcpy(pTemp, p, nPixels);
					memmove(p, p + nPixels, nBytes);
					memcpy(p + nBytes, pTemp, nPixels);
					p += _nScanDX;
				}
			}

			unlock();

			bofFree(pTemp);
		}
	}

	return _errCode;
}

ErrorCode CBofBitmap::scrollUp(int nPixels) {
	assert(isValidObject(this));

	if (_errCode == ERR_NONE) {
		lock();

		CBofRect cRect(0, 0, _nDX  - 1, _nDY  - 1);

		int32 x = cRect.left;
		int32 y = cRect.top;
		int32 dx = cRect.width();
		int32 dy = cRect.height();

		// Height must be valid or we're hosed
		assert(dy > 0);

		// We don't have to scroll more than the height of the bitmap, because
		// scrolling (bitmap height + 1) pixels is the same as scrolling 1 pixel,
		// and scrolling 1 pixel is obviously more efficient then scrolling
		// (bitmap height + 1) pixels.
		//
		// Also, we can handle down scrolling as scrolling up:
		// For example, if the bitmap-height is 10 pixels, and we want to
		// scroll down 6 pixels, that has the same effect as scrolling up
		// 4 pixels (10 - 6 = 4).  So, when we get negative nPixels, we will just
		// scroll (height + nPixels) in the opposite direction.
		if (nPixels >= 0) {
			nPixels = nPixels % dy;
		} else {
			nPixels = -(-nPixels % dy);
			if (nPixels < 0)
				nPixels = dy + nPixels;
			assert(nPixels >= 0 && nPixels < dy);
		}

		// Only scroll if we need to
		if (nPixels != 0) {
			// Allocate a buffer to hold one horizontal line
			byte *pRowBuf = (byte *)bofAlloc(dx);

			byte *pStart = _pBits;
			byte *pEnd = _pBits;

			int32 dx1 = _nScanDX;
			int32 dy1 = _nDY;

			// Is bitmap top-down or bottom up?
			if (_bTopDown) {
				pStart += y * dx1 + x;
				pEnd += (y + dy - 1) * dx1 + x;
			} else {
				pStart += (dy1 - y - 1) * dx1 + x;
				pEnd += (dy1 - (y + dy - 1) - 1) * dx1 + x;
				dx1 = -dx1;
			}
			byte *pCurRow = pStart;

			// Copy 1st row into temp row buffer
			memcpy(pRowBuf, pCurRow, dx);

			int32 lJump = dx1 * nPixels;

			byte *pLastRow = pCurRow;
			pCurRow += lJump;
			byte *p1stRow = pStart;

			// Working row by row
			for (int32 i = 1; i < dy; i++) {
				// Copy this row to row above it
				memcpy(pLastRow, pCurRow, dx);

				pLastRow = pCurRow;

				pCurRow += lJump;
				if (pCurRow < pEnd && !_bTopDown) {
					pCurRow = pStart - (pEnd - pCurRow) - dx1;

					if (pCurRow == p1stRow) {
						i++;

						// Copy 1st row into this row
						memcpy(pLastRow, pRowBuf, dx);

						pCurRow += dx1;
						p1stRow = pLastRow = pCurRow;

						// Copy this next row into temp row buffer
						memcpy(pRowBuf, p1stRow, dx);

						pCurRow += lJump;
					}
				}
			}

			// Copy 1st row into last row
			memcpy(pLastRow, pRowBuf, dx);

			bofFree(pRowBuf);
		}
		unlock();
	}

	return _errCode;
}

#define BIT0    0x00000001
#define BIT2    0x00000004
#define BIT15   0x00008000
#define BIT16   0x00010000
#define BIT17   0x00020000
#define SEQ     (BIT17)

ErrorCode CBofBitmap::fadeIn(CBofWindow *pWnd, int xStart, int yStart, int nMaskColor, int nBlockSize, int /*nSpeed*/) {
	assert(isValidObject(this));

	assert(pWnd != nullptr);
	assert(xStart >= 0);
	assert(yStart >= 0);

	if (_errCode == ERR_NONE) {

		CBofRect cSrcRect, cDstRect;
		uint32 mask = (BIT16 | BIT15 | BIT2 | BIT0);

		uint32 width = _nDX / nBlockSize;
		uint32 height = _nDY;

		uint32 maxvalue = height / nBlockSize * width;
		uint32 seed = SEQ - 1;
		uint32 value = seed;

		for (;;) {
			uint32 tmp = (value & mask);
			uint32 shft = BIT0;
			uint32 cnt = 0;
			while (shft < SEQ) {
				if (tmp & shft) {
					cnt++;
				}
				shft <<= 1;
			}
			value <<= 1;

			if (cnt & BIT0) {
				value |= BIT0;
			}

			value &= (SEQ - 1);
			if (value == seed)
				break;
			if (value > maxvalue)
				continue;

			uint32 y = (value / width) * nBlockSize;
			uint32 x = (value % width) * nBlockSize;

			cSrcRect.setRect((int)x, (int)y, (int)x + nBlockSize - 1, (int)y + nBlockSize - 1);
			x += xStart;
			y += yStart;
			cDstRect.setRect((int)x, (int)y, (int)x + nBlockSize - 1, (int)y + nBlockSize - 1);
			paint(pWnd, &cDstRect, &cSrcRect, nMaskColor);
		}

		cSrcRect.setRect(0, 0, nBlockSize - 1, nBlockSize - 1);
		paint(pWnd, &cSrcRect, &cSrcRect, nMaskColor);
	}

	return _errCode;
}

ErrorCode CBofBitmap::curtain(CBofWindow *pWnd, int nSpeed, int nMaskColor) {
	assert(isValidObject(this));
	assert(pWnd != nullptr);

	if (_errCode == ERR_NONE) {
		CBofRect cRect;

		int nWidth = pWnd->width();
		int nHeight = pWnd->height();

		for (int i = 0; i < nHeight; i += nSpeed) {
			cRect.setRect(0, i, nWidth - 1, i + nSpeed - 1);
			paint(pWnd, &cRect, &cRect, nMaskColor);
			bofSleep(1);
		}
	}

	return _errCode;
}

ErrorCode CBofBitmap::fadeLines(CBofWindow *pWnd,int nSpeed, int nMaskColor) {
	assert(isValidObject(this));
	assert(pWnd != nullptr);
	assert(nSpeed != 0);

	if (_errCode == ERR_NONE) {

		CBofRect cDstRect, cSrcRect, cWindowRect, cBmpRect;
		// Entire window?
		//
		cWindowRect = pWnd->getRect();
		CBofRect *pDstRect = &cWindowRect;

		cBmpRect = getRect();
		CBofRect *pSrcRect = &cBmpRect;

		int x1 = pDstRect->left;
		int y1 = pDstRect->top;
		int nWidth1 = pDstRect->width();
		int nHeight1 = pDstRect->height();

		int x2 = pSrcRect->left;
		int y2 = pSrcRect->top;
		int nWidth2 = pSrcRect->width();

		for (int j = 0; j < 4; j++) {
			for (int i = 0; i < nHeight1 ; i += 4) {

				cDstRect.setRect(x1, y1 + i + j, x1 + nWidth1 - 1, y1 + i + j);
				cSrcRect.setRect(x2, y2 + i + j, x2 + nWidth2 - 1, y2 + i + j);
				paint(pWnd, &cDstRect, &cSrcRect, nMaskColor);

				if (i % nSpeed == 0)
					bofSleep(1);
			}
		}
	}

	return _errCode;
}

const char *CBofBitmap::getFileName() {
	assert(isValidObject(this));

	const char *p = nullptr;
	if (_szFileName[0] != '\0')
		p = (const char *)&_szFileName[0];

	return p;
}

//////////////////////////////////////////////////////////////////////////////
//
// Misc global graphics routines
//
//////////////////////////////////////////////////////////////////////////////

CBofBitmap *loadBitmap(const char *pszFileName, CBofPalette *pPalette, bool bUseShared) {
	CBofPalette *pUsePal = pPalette;

	// If no palette was passed in and a shared palette was requested, then
	// use our default one established by "SHAREDPAL=" in the script
	if (bUseShared && pPalette == nullptr) {
		pUsePal = CBofPalette::getSharedPalette();
	}

	CBofBitmap *pBmp = new CBofBitmap(pszFileName, pUsePal);

	return pBmp;
}

ErrorCode paintBitmap(CBofWindow *pWindow, const char *pszFileName, CBofRect *pDstRect, CBofRect *pSrcRect, CBofPalette *pPalette, int nMaskColor) {
	assert(pWindow != nullptr);
	assert(pszFileName != nullptr);

	CBofBitmap *pBmp = new CBofBitmap(pszFileName, pPalette);
	CBofRect cRect = pBmp->getRect();

	if (pSrcRect == nullptr)
		pSrcRect = &cRect;

	if (pDstRect == nullptr)
		pDstRect = &cRect;

	ErrorCode errorCode = pBmp->paint(pWindow, pDstRect, pSrcRect, nMaskColor);

	delete pBmp;

	return errorCode;
}

Graphics::ManagedSurface CBofBitmap::getSurface() {
	Graphics::ManagedSurface s;
	s.w = _nDX;
	s.h = _nDY;
	s.pitch = _nScanDX;
	s.format = Graphics::PixelFormat::createFormatCLUT8();
	s.setPalette(_pPalette->getData(), 0, PALETTE_COUNT);
	s.setPixels(_pBits);

	return s;
}

} // namespace Bagel
