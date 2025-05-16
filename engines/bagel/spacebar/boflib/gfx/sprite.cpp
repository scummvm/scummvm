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

#include "bagel/spacebar/boflib/gfx/sprite.h"
#include "bagel/boflib/misc.h"

namespace Bagel {
namespace SpaceBar {

CBofRect *CBofSprite::_cDirtyRect;
CBofSprite *CBofSprite::_pSpriteChain = nullptr;          // Pointer to chain of linked sprites
CBofSprite *CBofSprite::_pTouchedSprite = nullptr;        // Pointer to sprite overlapped during painting
CBofBitmap *CBofSprite::_pWorkBmp = nullptr;              // Offscreen work area
CBofPalette *CBofSprite::_pSharedPalette = nullptr;       // Shared palette for ALL sprites
int CBofSprite::_nWorkDX = 0;
int CBofSprite::_nWorkDY = 0;

void CBofSprite::initialize() {
	_cDirtyRect = new CBofRect();
	_pSpriteChain = nullptr;
	_pTouchedSprite = nullptr;
	_pWorkBmp = nullptr;
	_pSharedPalette = nullptr;
	_nWorkDX = 0;
	_nWorkDY = 0;
}

void CBofSprite::shutdown() {
	delete _cDirtyRect;
}

void CBofSprite::openLibrary(CBofPalette *pPal) {
	// Must have a valid palette to do any sprite related stuff
	assert(pPal != nullptr);

	clearDirtyRect();
	setSharedPalette(pPal);

	// Set up a default work area
	setupWorkArea(200, 200);
}


void CBofSprite::closeLibrary() {
	flushSpriteChain();
	tearDownWorkArea();

	_pSharedPalette = nullptr;
}


CBofSprite::CBofSprite() {
	_pImage = nullptr;                                 // No initial bitmap image for the sprite

	_cSize = CBofSize(0, 0);                // There is no size to the sprite image
	_cRect.setRectEmpty();                             // Rectangular bounds not yet defined

	_cImageRect = _cRect;                              // Image rectangle starts same as display bounds
	_cPosition = CBofPoint(0, 0);             // Default position to upper left corner of display
	_bPositioned = false;                              // Not yet positioned
	_bDuplicated = false;                              // Not sharing resources with other sprites
	_nZOrder = SPRITE_TOPMOST;                         // Default to top most in fore/back ground order
	_nCelCount = 1;                                    // Number of frames in animated cel strip
	_nCelID = _nCelCount - 1;                          // Cel identifier not pointing at a cel
	_bAnimated = false;                                // Not initially animated
	_bLinked = false;                                  // Not initially linked into the sprite chain

	_nMaskColor = NOT_TRANSPARENT;                     // Default to NO transparency
	_bReadOnly = true;

	setBlockAdvance(false);                          // Default always advance next sprite
}


CBofSprite::~CBofSprite() {
	assert(isValidObject(this));

	unlinkSprite();
	clearImage();   // Clear the sprite image bitmap and context
}


void CBofSprite::linkSprite() {
	assert(isValidObject(this));

	if (!_bLinked) {
		// Set for linked into chain
		_bLinked = true;

		if (_pSpriteChain != nullptr) {
			switch (_nZOrder) {
			case SPRITE_TOPMOST:
				_pSpriteChain->addToTail(this);
				break;

			case SPRITE_HINDMOST:
				_pSpriteChain->addToHead(this);
				_pSpriteChain = this;
				break;

			default: {
				CBofSprite *pSprite;
				CBofSprite *pLastSprite = pSprite = _pSpriteChain;
				while (pSprite != nullptr && pSprite->_nZOrder > _nZOrder) {
					pLastSprite = pSprite;
					pSprite = (CBofSprite *)pSprite->_pNext;
				}
				pLastSprite->Insert(this);
				break;
			}
			}

		} else {
			_pSpriteChain = this;
		}

		// _pSpriteChain must always point to the head of the linked list
		assert(_pSpriteChain == (CBofSprite *)_pSpriteChain->getHead());
	}
}


void CBofSprite::unlinkSprite() {
	assert(isValidObject(this));

	if (_bLinked) {
		// Set for not linked into chain
		_bLinked = false;

		if (_pSpriteChain == this)
			_pSpriteChain = (CBofSprite *)_pNext;

		Delete();
	}
}


void CBofSprite::flushSpriteChain() {
	CBofSprite *pSprite = getSpriteChain();

	// Cycle getting head of chain, un-linking it and then deleting it
	while (pSprite != nullptr) {
		pSprite->unlinkSprite();
		delete pSprite;
		pSprite = getSpriteChain();
	}
}


void CBofSprite::setupWorkArea(int dx, int dy) {
	// Do we already have a work area?
	if (_pWorkBmp != nullptr) {
		// Yes, so lets tear it down before we start a new one
		tearDownWorkArea();
	}

	// Create an offscreen bitmap where we do all the work;
	_pWorkBmp = new CBofBitmap(dx, dy, _pSharedPalette);
	_nWorkDX = dx;
	_nWorkDY = dy;
}


void CBofSprite::tearDownWorkArea() {
	delete _pWorkBmp;
	_pWorkBmp = nullptr;
}


CBofSprite *CBofSprite::duplicateSprite() {
	assert(isValidObject(this));

	// Create an object for the sprite
	CBofSprite *pSprite = new CBofSprite;
	duplicateSprite(pSprite);

	return pSprite;
}


void CBofSprite::duplicateSprite(CBofSprite *pSprite) {
	if (!isValidObject(this) || (pSprite == nullptr))
		error("duplicateSprite - Invalid source or destination sprite");

	pSprite->_pImage = _pImage;
	pSprite->_cRect = _cRect;
	pSprite->_cImageRect = _cImageRect;
	pSprite->_cSize = _cSize;
	pSprite->_cPosition = _cPosition;
	pSprite->_nZOrder = _nZOrder;
	pSprite->_nCelID = _nCelID;
	pSprite->_nCelCount = _nCelCount;
	pSprite->_bAnimated = _bAnimated;
	pSprite->_nMaskColor = _nMaskColor;

	pSprite->_bDuplicated = true;       // Mark it as a sprite with shared resources
}


bool CBofSprite::loadSprite(const char *pszPathName, int nCels) {
	assert(isValidObject(this));
	assert(pszPathName != nullptr);
	assert(nCels >= 1);

	// Create an object for the sprite's image
	CBofBitmap *pBitmap = new CBofBitmap(pszPathName, _pSharedPalette);
	return loadSprite(pBitmap, nCels);
}


bool CBofSprite::loadSprite(CBofBitmap *pBitmap, int nCels) {
	assert(isValidObject(this));

	// Can't load an invalid bitmap
	assert(pBitmap != nullptr);
	assert(nCels >= 1);

	clearImage();           // Clear out any/all existing bitmaps, palettes,

	_pImage = pBitmap;      // Save pointer to bitmap

	pBitmap->setReadOnly(_bReadOnly);

	_cSize = pBitmap->getSize();

	_cRect.setRect(0, 0, _cSize.cx - 1, _cSize.cy - 1);
	_cImageRect.setRect(0, 0, _cSize.cx - 1, _cSize.cy - 1);
	_nCelCount = 1;
	_nCelID = _nCelCount - 1;

	if (nCels != 1) {
		setupCels(nCels);

		// Assume it's animated
		_bAnimated = true;
	}

	return true;            // Return success
}


bool CBofSprite::setupCels(const int nCels) {
	assert(isValidObject(this));
	assert(nCels > 0);

	_nCelCount = nCels;                                        // Set cel count
	_nCelID = _nCelCount - 1;                                 // No current cel
	int nStripWidth = _cSize.cx;                               // Temp place toRetain cell strip pixel length
	_cSize.cx /= nCels;                                        // Calculate width of a cel

	if (_cSize.cx * nCels == nStripWidth) {                    // Verify we have an even multiple
		_cRect.right = _cRect.left + _cSize.cx;              // Reset sprite rectangular bounds
		_cRect.bottom = _cRect.top + _cSize.cy;              // ... based on cel dimensions
		_cImageRect.setRect(0, 0, _cSize.cx - 1, _cSize.cy - 1); // Set bounds for first cel in strip
		return true;
	}

	return false;
}


void CBofSprite::nextCel() {
	assert(isValidObject(this));

	// verify old cel id
	assert(_nCelID >= 0 && _nCelID < _nCelCount);

	if (getBlockAdvance() == false) {
		if (++_nCelID >= _nCelCount)
			_nCelID = 0;

		setCel(_nCelID);
	}
}


void CBofSprite::prevCel() {
	assert(isValidObject(this));

	// verify old cel id
	assert(_nCelID >= 0 && _nCelID < _nCelCount);

	if (--_nCelID < 0)
		_nCelID = _nCelCount - 1;

	setCel(_nCelID);
}


bool CBofSprite::paintSprite(CBofWindow *pWnd, const int x, const int y) {
	assert(isValidObject(this));

	// Can't paint to a non-existent window
	assert(pWnd != nullptr);

	// The window MUST have a backdrop
	assert(pWnd->getBackdrop() != nullptr);

	batchPaint(x, y);

	updateDirtyRect(pWnd, this);

	return !errorOccurred();
}


bool CBofSprite::paintSprite(CBofBitmap *pBmp, const int x, const int y) {
	assert(isValidObject(this));

	// Can't paint to a non-existent window
	assert(pBmp != nullptr);

	batchPaint(x, y);
	updateDirtyRect(pBmp, this);

	return !errorOccurred();
}


bool CBofSprite::paintCel(CBofWindow *pWnd, int nCelId, const int x, const int y) {
	setCel(nCelId - 1);
	return paintSprite(pWnd, x, y);
}


bool CBofSprite::paintCel(CBofBitmap *pBmp, int nCelId, const int x, const int y) {
	setCel(nCelId - 1);
	return paintSprite(pBmp, x, y);
}


void CBofSprite::batchPaint(const int x, const int y) {
	assert(isValidObject(this));

	CBofRect cDstRect;

	// Default to no sprite being overlapped by this painting operation
	_pTouchedSprite = nullptr;

	// Calculate destination rectangle
	cDstRect.setRect(x, y, x + _cSize.cx - 1, y + _cSize.cy - 1);

	// Add the destination position to the dirty rectangle list
	addToDirtyRect(&cDstRect);

	// If the sprite is already on screen, then we must also add it's old
	// current location to the dirty rect list so that it is erase properly
	if (_bPositioned) {
		addToDirtyRect(&_cRect);
	}

	// Now establish the sprite's new position
	setPosition(x, y);

	if (_bAnimated && (_nCelCount > 1))
		// Advance to the next cel in the strip
		nextCel();
}

bool CBofSprite::updateDirtyRect(CBofWindow *pWnd, CBofSprite *pPrimarySprite) {
	assert(pWnd != nullptr);

	// The window MUST have a backdrop associated with it.  If that's not feasible, then
	// use CSprites instead of CBofSprites
	assert(pWnd->getBackdrop() != nullptr);

	//
	// Repaint the contents of the specified rectangle
	//

	CBofBitmap *pBackdrop = pWnd->getBackdrop();
	if (pBackdrop != nullptr) {

		CBofRect *pRect = _cDirtyRect;
		if (pRect->width() != 0 && pRect->height() != 0) {
			// Need a work area
			CBofBitmap *pWork = _pWorkBmp;
			int dx = pRect->width();
			int dy = pRect->height();

			bool bTempWorkArea = false;
			if ((pWork == nullptr) || (dx > _nWorkDX) || (dy > _nWorkDY)) {

				bTempWorkArea = true;
				pWork = new CBofBitmap(dx, dy, _pSharedPalette);
			}
			pWork->lock();

			// Paint the background into the work area
			pBackdrop->paint(pWork, 0, 0, pRect);

			// Only need to search the sprite list if current sprite is linked
			CBofSprite *pSprite = pPrimarySprite;
			if (pPrimarySprite == nullptr || pPrimarySprite->_bLinked) {
				pSprite = _pSpriteChain;
			}

			CBofRect cRect, cSrcRect;
			// Run through the sprite list
			while (pSprite != nullptr) {
				// and paint each partial sprite overlap to the work area
				if (pSprite->_bPositioned && cRect.intersectRect(&pSprite->_cRect, pRect)) {
					if (pPrimarySprite != pSprite)
						_pTouchedSprite = pSprite;

					cSrcRect = cRect - pSprite->_cRect.topLeft();
					cSrcRect += pSprite->_cImageRect.topLeft();
					cRect -= pRect->topLeft();

					pSprite->_pImage->paint(pWork, &cRect, &cSrcRect, pSprite->_nMaskColor);
				}
				pSprite = (CBofSprite *)pSprite->_pNext;
			}

			// Paint final outcome to the screen
			cSrcRect.setRect(0, 0, pRect->width() - 1, pRect->height() - 1);
			pWork->paint(pWnd, pRect, &cSrcRect);

			pWork->unlock();

			if (bTempWorkArea) {
				delete pWork;
			}
		}
	}

	clearDirtyRect();

	return true;
}


bool CBofSprite::updateDirtyRect(CBofBitmap *pBmp, CBofSprite *pPrimarySprite) {
	assert(pBmp != nullptr);

	//
	// Repaint the contents of the specified rectangle
	//
	CBofRect *pRect = getDirtyRect();

	// Only need to search the sprite list if current sprite is linked
	CBofSprite *pSprite = pPrimarySprite;
	if (pPrimarySprite == nullptr || pPrimarySprite->_bLinked) {
		pSprite = _pSpriteChain;
	}

	CBofRect cRect;
	// Run through the sprite list
	while (pSprite != nullptr) {
		// and paint each partial sprite overlap to the work area
		if (pSprite->_bPositioned && cRect.intersectRect(&pSprite->_cRect, pRect)) {

			if (pPrimarySprite != pSprite)
				_pTouchedSprite = pSprite;

			CBofRect cSrcRect = cRect - pSprite->_cRect.topLeft();
			cSrcRect += pSprite->_cImageRect.topLeft();

			pSprite->_pImage->paint(pBmp, &cRect, &cSrcRect, pSprite->_nMaskColor);
		}
		pSprite = (CBofSprite *)pSprite->_pNext;
	}

	clearDirtyRect();

	return true;
}


void CBofSprite::addToDirtyRect(CBofRect *pRect) {
	assert(pRect != nullptr);

	CBofRect cRect;

	if (_cDirtyRect->isRectEmpty()) {
		cRect = *pRect;
	} else {
		cRect.unionRect(_cDirtyRect, pRect);
	}
	*_cDirtyRect = cRect;
}


void CBofSprite::setCel(const int nCelID) {
	assert(isValidObject(this));

	// All sprites must have at least 1 frame
	assert(_nCelCount > 0);

	if (_nCelID != nCelID) {
		_nCelID = nCelID % _nCelCount;
		if ((_nCelID != 0) && (nCelID < 0)) {
			_nCelID = _nCelCount + _nCelID;
		}
	}

	// Verify new cel id
	assert(_nCelID >= 0 && _nCelID < _nCelCount);

	_cImageRect.left = _nCelID * _cSize.cx;
	_cImageRect.right = _cImageRect.left + _cSize.cx;
}

bool CBofSprite::eraseSprite(CBofWindow *pWnd) {
	assert(isValidObject(this));
	assert(pWnd != nullptr);

	batchErase();
	updateDirtyRect(pWnd);

	return !errorOccurred();
}


void CBofSprite::batchErase() {
	if (_bPositioned) {
		_bPositioned = false;

		addToDirtyRect(&_cRect);
	}
}


bool CBofSprite::testInterception(CBofSprite *pTestSprite, CBofPoint *pPoint) {
	assert(isValidObject(this));
	assert(pTestSprite != nullptr);

	// Punt if no interception allowed
	if (pTestSprite != nullptr) {
		// be sure to not test against ourself
		if (this != pTestSprite) {

			CBofRect overlapRect; // Area of overlap between rectangles
			// Use simple rectangle screening first
			if (overlapRect.intersectRect(&_cRect, &pTestSprite->_cRect)) {
				// ... and if that succeeds, see if we
				// ... have image masks that overlap
				if ((_nMaskColor == NOT_TRANSPARENT) || (pTestSprite->_nMaskColor == NOT_TRANSPARENT) || spritesOverlap(pTestSprite, pPoint)) {
					return true;
				}
			}
		}
	}

	return false;
}


CBofSprite *CBofSprite::interception(CBofRect *pNewRect, CBofSprite *pTestSprite) {
	assert(isValidObject(this));
	assert(pNewRect != nullptr);

	// Get first sprite to be tested
	CBofSprite *pSprite = pTestSprite;

	// Thumb through the sprite chain
	while (pSprite != nullptr) {
		// be sure to not test against ourself
		// ... and only test against overlapping sprites
		if (this != pSprite) {
			CBofRect overlapRect; // Area of overlap between rectangles
			// Sprites touch if their rectangles intersect.
			// does our sprite overlap another?
			if (overlapRect.intersectRect(pNewRect, &pSprite->_cRect))
				// ... if so return a pointer to it
				return pSprite;
		}

		// Fetch next sprite in chain for testing
		pSprite = (CBofSprite *)pSprite->_pNext;
	}

	return nullptr;
}


CBofSprite *CBofSprite::interception(CBofSprite *pTestSprite) {
	assert(isValidObject(this));

	CBofSprite *pSprite = pTestSprite;              // Get first sprite to be tested

	while (pSprite != nullptr) {        // Thumb through the entire sprite collection

		if (testInterception(pSprite, nullptr))     // ... testing against each sprite in turn
			return pSprite;                         // found an interception

		pSprite = (CBofSprite *)pSprite->_pNext;    // fetch next sprite in chain for testing
	}

	return nullptr;
}


bool CBofSprite::spritesOverlap(CBofSprite *pSprite, CBofPoint *pPoint) {
	assert(isValidObject(this));
	assert(pSprite != nullptr);

	// Assume no overlap
	bool bHit = false;

	// If the sprite's rectangles overlap
	CBofRect overlapRect;
	if (overlapRect.intersectRect(&_cRect, &pSprite->_cRect)) {
		int32 dx = overlapRect.width();
		int32 dy = overlapRect.height();

		int32 x1 = overlapRect.left - _cRect.left + _cImageRect.left;
		int32 y1 = overlapRect.top - _cRect.top + _cImageRect.top;

		int32 x2 = overlapRect.left - pSprite->_cRect.left + pSprite->_cImageRect.left;
		int32 y2 = overlapRect.top - pSprite->_cRect.top + pSprite->_cImageRect.top;

		int32 dx1 = _pImage->widthBytes();
		int32 dx2 = pSprite->_pImage->widthBytes();

		byte m1 = (byte)_nMaskColor;
		byte m2 = (byte)pSprite->_nMaskColor;

		// Lock down these bitmaps
		_pImage->lock();
		pSprite->_pImage->lock();

		byte *pDib1 = (byte *)_pImage->getPixelAddress((int)x1, (int)y1);
		byte *pDib2 = (byte *)pSprite->_pImage->getPixelAddress((int)x2, (int)y2);

		if (!_pImage->isTopDown()) {
			dx1 = -dx1;
		}

		if (!pSprite->_pImage->isTopDown()) {
			dx2 = -dx2;
		}

		for (int32 y = 0; y < dy; y++) {
			byte *pPtr1 = pDib1;
			byte *pPtr2 = pDib2;

			for (int32 x = 0; x < dx; x++) {
				if ((*pPtr1 != m1) && (*pPtr2 != m2)) {
					if (pPoint != nullptr) {
						pPoint->x = (int)x;
						pPoint->y = (int)y;
					}

					bHit = true;
					goto endroutine;
				}

				pPtr1++;
				pPtr2++;
			}

			pDib1 += dx1;
			pDib2 += dx2;
		}
	}

endroutine:
	// Don't need access to these bitmaps any more
	pSprite->_pImage->unlock();
	_pImage->unlock();

	return bHit;
}


void CBofSprite::setPosition(int x, int y) {
	assert(isValidObject(this));

	// Now have a real location establish the new location of the sprite
	// and setup the bitmap's bounding rectangle
	_bPositioned = true;
	_cPosition.x = x;
	_cPosition.y = y;
	_cRect.setRect(_cPosition.x, _cPosition.y, _cPosition.x + _cSize.cx - 1, _cPosition.y + _cSize.cy - 1);
}


void CBofSprite::clearImage() {
	assert(isValidObject(this));

	if (!_bDuplicated && (_pImage != nullptr)) {
		delete _pImage;
	}

	_pImage = nullptr;
}


void CBofSprite::setSharedPalette(CBofPalette *pPal) {
	assert(pPal != nullptr);

	_pSharedPalette = pPal;
}

void CBofSprite::setZOrder(int nValue) {
	assert(isValidObject(this));
	assert(nValue >= SPRITE_TOPMOST && nValue <= SPRITE_HINDMOST);

	_nZOrder = nValue;

	// Relinking this sprite after setting it's new Z-Order will
	// add the sprite to the correct Z-Order sorted location (Insertion Sort)
	if (_bLinked) {
		unlinkSprite();
		linkSprite();
	}
}

} // namespace SpaceBar
} // namespace Bagel
