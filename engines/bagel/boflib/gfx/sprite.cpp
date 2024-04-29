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

#include "bagel/boflib/gfx/sprite.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

CBofRect *CBofSprite::m_cDirtyRect;
CBofSprite *CBofSprite::m_pSpriteChain = nullptr;          // Pointer to chain of linked sprites
CBofSprite *CBofSprite::m_pTouchedSprite = nullptr;        // Pointer to sprite overlapped during painting
CBofBitmap *CBofSprite::m_pWorkBmp = nullptr;              // Offscreen work area
CBofPalette *CBofSprite::m_pSharedPalette = nullptr;       // Shared palette for ALL sprites
int CBofSprite::m_nWorkDX = 0;
int CBofSprite::m_nWorkDY = 0;

void CBofSprite::initialize() {
	m_cDirtyRect = new CBofRect();
	m_pSpriteChain = nullptr;
	m_pTouchedSprite = nullptr;
	m_pWorkBmp = nullptr;
	m_pSharedPalette = nullptr;
	m_nWorkDX = 0;
	m_nWorkDY = 0;
}

void CBofSprite::shutdown() {
	delete m_cDirtyRect;
}

void CBofSprite::OpenLibrary(CBofPalette *pPal) {
	// Must have a valid palette to do any sprite related stuff
	Assert(pPal != nullptr);

	ClearDirtyRect();

	setSharedPalette(pPal);

	// Set up a default work area
	SetupWorkArea(200, 200);
}


void CBofSprite::CloseLibrary() {
	FlushSpriteChain();

	TearDownWorkArea();

	m_pSharedPalette = nullptr;
}


CBofSprite::CBofSprite() {
	m_pImage = nullptr;                                 // No initial bitmap image for the sprite

	m_cSize = CBofSize(0, 0);                           // There is no size to the sprite image
	_cRect.SetRectEmpty();                             // Rectangular bounds not yet defined

	m_cImageRect = _cRect;                             // Image rectangle starts same as display bounds
	m_cPosition = CBofPoint(0, 0);                      // Default position to upper left corner of display
	m_bPositioned = false;                              // Not yet positioned
	m_bDuplicated = false;                              // Not sharing resources with other sprites
	m_nZOrder = SPRITE_TOPMOST;                         // Default to top most in fore/back ground order
	m_nCelCount = 1;                                    // Number of frames in animated cel strip
	m_nCelID = m_nCelCount - 1;                         // Cel identifier not pointing at a cel
	m_bAnimated = false;                                // Not initially animated
	m_bLinked = false;                                  // Not initially linked into the sprite chain

	m_nMaskColor = NOT_TRANSPARENT;                     // Default to NO transparency
	m_bReadOnly = true;

	SetBlockAdvance(false);                             // Default always advance next sprite
}


CBofSprite::~CBofSprite() {
	Assert(IsValidObject(this));

	UnlinkSprite();

	ClearImage();   // Clear the sprite image bitmap and context
}


void CBofSprite::LinkSprite() {
	Assert(IsValidObject(this));

	if (!m_bLinked) {
		// Set for linked into chain
		m_bLinked = true;

		if (m_pSpriteChain != nullptr) {
			switch (m_nZOrder) {
			case SPRITE_TOPMOST:
				m_pSpriteChain->addToTail(this);
				break;

			case SPRITE_HINDMOST:
				m_pSpriteChain->addToHead(this);
				m_pSpriteChain = this;
				break;

			default: {
				CBofSprite *pSprite;
				CBofSprite *pLastSprite = pSprite = m_pSpriteChain;
				while (pSprite != nullptr && pSprite->m_nZOrder > m_nZOrder) {
					pLastSprite = pSprite;
					pSprite = (CBofSprite *)pSprite->m_pNext;
				}
				pLastSprite->Insert(this);
				break;
			}
			}

		} else {
			m_pSpriteChain = this;
		}

		// m_pSpriteChain must always point to the head of the linked list
		Assert(m_pSpriteChain == (CBofSprite *)m_pSpriteChain->GetHead());
	}
}


void CBofSprite::UnlinkSprite() {
	Assert(IsValidObject(this));

	if (m_bLinked) {
		// Set for not linked into chain
		m_bLinked = false;

		if (m_pSpriteChain == this)
			m_pSpriteChain = (CBofSprite *)m_pNext;

		Delete();
	}
}


void CBofSprite::FlushSpriteChain() {
	CBofSprite *pSprite = nullptr;

	// Cycle getting head of chain, un-linking it and then deleting it
	while ((pSprite = CBofSprite::GetSpriteChain()) != nullptr) {
		pSprite->UnlinkSprite();
		delete pSprite;
	}
}


bool CBofSprite::SetupWorkArea(int dx, int dy) {
	// Assume failure
	bool bSuccess = false;

	// Do we already have a work area?
	if (m_pWorkBmp != nullptr) {
		// Yes, so lets tear it down before we start a new one
		TearDownWorkArea();
	}

	// Create an offscreen bitmap where we do all the work;
	if ((m_pWorkBmp = new CBofBitmap(dx, dy, m_pSharedPalette)) != nullptr) {
		m_nWorkDX = dx;
		m_nWorkDY = dy;
		bSuccess = true;
	}

	return bSuccess;
}


void CBofSprite::TearDownWorkArea() {
	if (m_pWorkBmp != nullptr) {
		delete m_pWorkBmp;
		m_pWorkBmp = nullptr;
	}
}


CBofSprite *CBofSprite::DuplicateSprite() {
	Assert(IsValidObject(this));

	// Create an object for the sprite
	CBofSprite *pSprite = new CBofSprite;

	if (pSprite != nullptr) {
		DuplicateSprite(pSprite);
	}

	return pSprite;
}


bool CBofSprite::DuplicateSprite(CBofSprite *pSprite) {
	Assert(IsValidObject(this));

	// We require a valid sprite to copy
	Assert(pSprite != nullptr);

	if (pSprite != nullptr) {
		pSprite->m_pImage = m_pImage;

		pSprite->_cRect = _cRect;
		pSprite->m_cImageRect = m_cImageRect;
		pSprite->m_cSize = m_cSize;
		pSprite->m_cPosition = m_cPosition;
		pSprite->m_nZOrder = m_nZOrder;
		pSprite->m_nCelID = m_nCelID;
		pSprite->m_nCelCount = m_nCelCount;
		pSprite->m_bAnimated = m_bAnimated;
		pSprite->m_nMaskColor = m_nMaskColor;

		pSprite->m_bDuplicated = true;		// Mark it as a sprite with shared resources

		return true;
	}

	return false;
}


bool CBofSprite::LoadSprite(const char *pszPathName, int nCels) {
	Assert(IsValidObject(this));
	Assert(pszPathName != nullptr);
	Assert(nCels >= 1);

	// Create an object for the sprite's image
	CBofBitmap *pBitmap = new CBofBitmap(pszPathName, m_pSharedPalette);
	if (pBitmap != nullptr) {
		return LoadSprite(pBitmap, nCels);
	}

	return false;	// Return failure
}


bool CBofSprite::LoadSprite(CBofBitmap *pBitmap, int nCels) {
	Assert(IsValidObject(this));

	// Can't load an invalid bitmap
	Assert(pBitmap != nullptr);
	Assert(nCels >= 1);

	ClearImage();			// Clear out any/all existing bitmaps, palettes,

	m_pImage = pBitmap;		// Save pointer to bitmap

	pBitmap->setReadOnly(m_bReadOnly);

	m_cSize = pBitmap->getSize();

	_cRect.SetRect(0, 0, m_cSize.cx - 1, m_cSize.cy - 1);
	m_cImageRect.SetRect(0, 0, m_cSize.cx - 1, m_cSize.cy - 1);
	m_nCelCount = 1;
	m_nCelID = m_nCelCount - 1;

	if (nCels != 1) {
		SetupCels(nCels);

		// Assume it's animated
		m_bAnimated = true;
	}

	return true;			// Return success
}


bool CBofSprite::SetupCels(const int nCels) {
	Assert(IsValidObject(this));
	Assert(nCels > 0);

	m_nCelCount = nCels;                                        // Set cel count
	m_nCelID = m_nCelCount - 1;                                 // No current cel
	int nStripWidth = m_cSize.cx;                               // Temp place toRetain cell strip pixel length
	m_cSize.cx /= nCels;                                        // Calculate width of a cel

	if (m_cSize.cx * nCels == nStripWidth) {                    // Verify we have an even multiple
		_cRect.right = _cRect.left + m_cSize.cx;              // Reset sprite rectangular bounds
		_cRect.bottom = _cRect.top + m_cSize.cy;              // ... based on cel dimensions
		m_cImageRect.SetRect(0, 0, m_cSize.cx - 1, m_cSize.cy - 1); // Set bounds for first cel in strip
		return true;
	}

	return false;
}


void CBofSprite::NextCel() {
	Assert(IsValidObject(this));

	// verify old cel id
	Assert(m_nCelID >= 0 && m_nCelID < m_nCelCount);

	if (GetBlockAdvance() == false) {
		if (++m_nCelID >= m_nCelCount)
			m_nCelID = 0;

		SetCel(m_nCelID);
	}
}


void CBofSprite::PrevCel() {
	Assert(IsValidObject(this));

	// verify old cel id
	Assert(m_nCelID >= 0 && m_nCelID < m_nCelCount);

	if (--m_nCelID < 0)
		m_nCelID = m_nCelCount - 1;

	SetCel(m_nCelID);
}


bool CBofSprite::PaintSprite(CBofWindow *pWnd, const int x, const int y) {
	Assert(IsValidObject(this));

	// Can't paint to a non-existant window
	Assert(pWnd != nullptr);

	// The window MUST have a backdrop
	Assert(pWnd->getBackdrop() != nullptr);

	Batchpaint(x, y);

	UpdateDirtyRect(pWnd, this);

	return !ErrorOccurred();
}


bool CBofSprite::PaintSprite(CBofBitmap *pBmp, const int x, const int y) {
	Assert(IsValidObject(this));

	// Can't paint to a non-existent window
	Assert(pBmp != nullptr);

	Batchpaint(x, y);
	UpdateDirtyRect(pBmp, this);

	return !ErrorOccurred();
}


bool CBofSprite::PaintCel(CBofWindow *pWnd, int nCelId, const int x, const int y) {
	SetCel(nCelId - 1);
	return PaintSprite(pWnd, x, y);
}


bool CBofSprite::PaintCel(CBofBitmap *pBmp, int nCelId, const int x, const int y) {
	SetCel(nCelId - 1);
	return PaintSprite(pBmp, x, y);
}


void CBofSprite::Batchpaint(const int x, const int y) {
	Assert(IsValidObject(this));

	CBofRect cDstRect;

	// Default to no sprite being overlapped by this painting operation
	m_pTouchedSprite = nullptr;

	// Calculate destination rectangle
	cDstRect.SetRect(x, y, x + m_cSize.cx - 1, y + m_cSize.cy - 1);

	// Add the destination position to the dirty rectangle list
	AddToDirtyRect(&cDstRect);

	// If the sprite is already on screen, then we must also add it's old
	// current location to the dirty rect list so that it is erase properly
	if (m_bPositioned)  {
		AddToDirtyRect(&_cRect);
	}

	// Now establish the sprite's new position
	setPosition(x, y);

	if (m_bAnimated && (m_nCelCount > 1))
		// Advance to the next cel in the strip
		NextCel();
}

bool CBofSprite::UpdateDirtyRect(CBofWindow *pWnd, CBofSprite *pPrimarySprite) {
	Assert(pWnd != nullptr);

	// The window MUST have a backdrop associated with it.  If that's not feasible, then
	// use CSprites instead of CBofSprites
	Assert(pWnd->getBackdrop() != nullptr);

	//
	// Repaint the contents of the specified rectangle
	//

	CBofBitmap *pBackdrop = pWnd->getBackdrop();
	if (pBackdrop != nullptr) {

		CBofRect *pRect = m_cDirtyRect;
		if (pRect->width() != 0 && pRect->height() != 0) {
			// Need a work area
			CBofBitmap *pWork = m_pWorkBmp;
			int dx = pRect->width();
			int dy = pRect->height();

			bool bTempWorkArea = false;
			if ((pWork == nullptr) || (dx > m_nWorkDX) || (dy > m_nWorkDY)) {

				bTempWorkArea = true;
				pWork = new CBofBitmap(dx, dy, m_pSharedPalette);
			}
			pWork->lock();

			// Paint the background into the work area
			pBackdrop->paint(pWork, 0, 0, pRect);

			// Only need to search the sprite list if current sprite is linked
			CBofSprite *pSprite = pPrimarySprite;
			if (pPrimarySprite == nullptr || pPrimarySprite->m_bLinked) {
				pSprite = m_pSpriteChain;
			}

			CBofRect cRect, cSrcRect;
			// Run thru the sprite list
			while (pSprite != nullptr) {
				// and paint each partial sprite overlap to the work area
				if (pSprite->m_bPositioned && cRect.IntersectRect(&pSprite->_cRect, pRect)) {
					if (pPrimarySprite != pSprite)
						m_pTouchedSprite = pSprite;

					cSrcRect = cRect - pSprite->_cRect.TopLeft();
					cSrcRect += pSprite->m_cImageRect.TopLeft();
					cRect -= pRect->TopLeft();

					pSprite->m_pImage->paint(pWork, &cRect, &cSrcRect, pSprite->m_nMaskColor);
				}
				pSprite = (CBofSprite *)pSprite->m_pNext;
			}

			// Paint final outcome to the screen
			cSrcRect.SetRect(0, 0, pRect->width() - 1, pRect->height() - 1);
			pWork->paint(pWnd, pRect, &cSrcRect);

			pWork->unlock();

			if (bTempWorkArea) {
				delete pWork;
			}
		}
	}

	ClearDirtyRect();

	return true;
}


bool CBofSprite::UpdateDirtyRect(CBofBitmap *pBmp, CBofSprite *pPrimarySprite) {
	Assert(pBmp != nullptr);

	//
	// Repaint the contents of the specified rectangle
	//
	CBofRect *pRect = GetDirtyRect();

	// Only need to search the sprite list if current sprite is linked
	CBofSprite *pSprite = pPrimarySprite;
	if (pPrimarySprite == nullptr || pPrimarySprite->m_bLinked) {
		pSprite = m_pSpriteChain;
	}

	CBofRect cRect;
	// Run thru the sprite list
	while (pSprite != nullptr) {
		// and paint each partial sprite overlap to the work area
		if (pSprite->m_bPositioned && cRect.IntersectRect(&pSprite->_cRect, pRect)) {

			if (pPrimarySprite != pSprite)
				m_pTouchedSprite = pSprite;

			CBofRect cSrcRect = cRect - pSprite->_cRect.TopLeft();
			cSrcRect += pSprite->m_cImageRect.TopLeft();

			pSprite->m_pImage->paint(pBmp, &cRect, &cSrcRect, pSprite->m_nMaskColor);
		}
		pSprite = (CBofSprite *)pSprite->m_pNext;
	}

	ClearDirtyRect();

	return true;
}


void CBofSprite::AddToDirtyRect(CBofRect *pRect) {
	Assert(pRect != nullptr);

	CBofRect cRect;

	if (m_cDirtyRect->IsRectEmpty()) {
		cRect = *pRect;
	} else {
		cRect.UnionRect(m_cDirtyRect, pRect);
	}
	*m_cDirtyRect = cRect;
}


void CBofSprite::SetCel(const int nCelID) {
	Assert(IsValidObject(this));

	// All sprites must have at least 1 frame
	Assert(m_nCelCount > 0);

	if (m_nCelID != nCelID) {
		m_nCelID = nCelID % m_nCelCount;
		if ((m_nCelID != 0) && (nCelID < 0)) {
			m_nCelID = m_nCelCount + m_nCelID;
		}
	}

	// Verify new cel id
	Assert(m_nCelID >= 0 && m_nCelID < m_nCelCount);

	m_cImageRect.left = m_nCelID * m_cSize.cx;
	m_cImageRect.right = m_cImageRect.left + m_cSize.cx;
}

bool CBofSprite::EraseSprite(CBofWindow *pWnd) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	batchErase();
	UpdateDirtyRect(pWnd);

	return !ErrorOccurred();
}


void CBofSprite::batchErase() {
	if (m_bPositioned) {
		m_bPositioned = false;

		AddToDirtyRect(&_cRect);
	}
}


bool CBofSprite::TestInterception(CBofSprite *pTestSprite, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pTestSprite != nullptr);

	// Punt if no interception allowed
	if (pTestSprite != nullptr) {
		// be sure to not test against ourself
		if (this != pTestSprite) {

			CBofRect overlapRect; // Area of overlap between rectangles
			// Use simple rectangle screening first
			if (overlapRect.IntersectRect(&_cRect, &pTestSprite->_cRect)) {
				// ... and if that succeeds, see if we
				// ... have image masks that overlap
				if ((m_nMaskColor == NOT_TRANSPARENT) || (pTestSprite->m_nMaskColor == NOT_TRANSPARENT) || SpritesOverlap(pTestSprite, pPoint)) {
					return true;
				}
			}
		}
	}

	return false;
}


CBofSprite *CBofSprite::Interception(CBofRect *pNewRect, CBofSprite *pTestSprite) {
	Assert(IsValidObject(this));
	Assert(pNewRect != nullptr);

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
			if (overlapRect.IntersectRect(pNewRect, &pSprite->_cRect))
				// ... if so return a pointer to it
				return pSprite;
		}

		// Fetch next sprite in chain for testing
		pSprite = (CBofSprite *)pSprite->m_pNext;
	}

	return nullptr;
}


CBofSprite *CBofSprite::Interception(CBofSprite *pTestSprite) {
	Assert(IsValidObject(this));

	CBofSprite *pSprite = pTestSprite;				// Get first sprite to be tested

	while (pSprite != nullptr) {		// Thumb through the entire sprite collection

		if (TestInterception(pSprite, nullptr))		// ... testing against each sprite in turn
			return pSprite;							// found an interception

		pSprite = (CBofSprite *)pSprite->m_pNext;	// fetch next sprite in chain for testing
	}

	return nullptr;
}


bool CBofSprite::SpritesOverlap(CBofSprite *pSprite, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pSprite != nullptr);

	// Assume no overlap
	bool bHit = false;

	// If the sprite's rectangles overlap
	CBofRect overlapRect;
	if (overlapRect.IntersectRect(&_cRect, &pSprite->_cRect)) {
		int32 dx = overlapRect.width();
		int32 dy = overlapRect.height();

		int32 x1 = overlapRect.left - _cRect.left + m_cImageRect.left;
		int32 y1 = overlapRect.top - _cRect.top + m_cImageRect.top;

		int32 x2 = overlapRect.left - pSprite->_cRect.left + pSprite->m_cImageRect.left;
		int32 y2 = overlapRect.top - pSprite->_cRect.top + pSprite->m_cImageRect.top;

		int32 dx1 = m_pImage->widthBytes();
		int32 dx2 = pSprite->m_pImage->widthBytes();

		byte m1 = (byte)m_nMaskColor;
		byte m2 = (byte)pSprite->m_nMaskColor;

		// Lock down these bitmaps
		m_pImage->lock();
		pSprite->m_pImage->lock();

		byte *pDib1 = (byte *)m_pImage->getPixelAddress((int)x1, (int)y1);
		byte *pDib2 = (byte *)pSprite->m_pImage->getPixelAddress((int)x2, (int)y2);

		if (!m_pImage->isTopDown()) {
			dx1 = -dx1;
		}

		if (!pSprite->m_pImage->isTopDown()) {
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
	pSprite->m_pImage->unlock();
	m_pImage->unlock();

	return bHit;
}


void CBofSprite::setPosition(int x, int y) {
	Assert(IsValidObject(this));

	// Now have a real location establish the new location of the sprite
	// and setup the bitmap's bounding rectangle
	m_bPositioned = true;
	m_cPosition.x = x;
	m_cPosition.y = y;
	_cRect.SetRect(m_cPosition.x, m_cPosition.y, m_cPosition.x + m_cSize.cx - 1, m_cPosition.y + m_cSize.cy - 1);
}


bool CBofSprite::CropImage(CBofWindow *pWnd, CBofRect *pRect, bool bUpdateNow) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);
	Assert(pRect != nullptr);
	Assert(m_pImage != nullptr);

	if (m_nMaskColor != NOT_TRANSPARENT) {

		CBofRect myRect = *pRect; // Offset crop area by image rect
		myRect.left += m_cImageRect.left;
		myRect.right += m_cImageRect.left;
		CBofRect cDestRect = myRect + m_cPosition;

		m_pImage->fillRect(&myRect, (byte)m_nMaskColor);

		if (bUpdateNow) {
			CBofBitmap *pBackdrop = pWnd->getBackdrop();
			if (pBackdrop != nullptr) {
				pBackdrop->paint(pWnd, &cDestRect, &myRect);
			}
		}
	}

	return true;
}


void CBofSprite::ClearImage() {
	Assert(IsValidObject(this));

	if (!m_bDuplicated && (m_pImage != nullptr)) {
		delete m_pImage;
	}

	m_pImage = nullptr;
}


void CBofSprite::setSharedPalette(CBofPalette *pPal) {
	Assert(pPal != nullptr);

	m_pSharedPalette = pPal;
}

void CBofSprite::SetZOrder(int nValue) {
	Assert(IsValidObject(this));
	Assert(nValue >= SPRITE_TOPMOST && nValue <= SPRITE_HINDMOST);

	m_nZOrder = nValue;

	// Relinking this sprite after setting it's new Z-Order will
	// add the sprite to the correct Z-Order sorted location (Insertion Sort)
	if (m_bLinked) {
		UnlinkSprite();
		LinkSprite();
	}
}

} // namespace Bagel
