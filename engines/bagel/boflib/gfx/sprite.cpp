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
#include "bagel/boflib/debug.h"
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

	SetSharedPalette(pPal);

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
	m_cRect.SetRectEmpty();                             // Rectangular bounds not yet defined

	m_cImageRect = m_cRect;                             // Image rectangle starts same as display bounds
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

	CBofSprite *pSprite, *pLastSprite;

	if (!m_bLinked) {
		// Set for linked into chain
		m_bLinked = true;

		if (m_pSpriteChain != nullptr) {
			switch (m_nZOrder) {
			case SPRITE_TOPMOST:
				m_pSpriteChain->AddToTail(this);
				break;

			case SPRITE_HINDMOST:
				m_pSpriteChain->AddToHead(this);
				m_pSpriteChain = this;
				break;

			default:
				pLastSprite = pSprite = m_pSpriteChain;
				while (pSprite != nullptr && pSprite->m_nZOrder > m_nZOrder) {
					pLastSprite = pSprite;
					pSprite = (CBofSprite *)pSprite->m_pNext;
				}
				pLastSprite->Insert(this);
				break;
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

	// Cycle getting head of chain unlinking it and then deleting it
	while ((pSprite = CBofSprite::GetSpriteChain()) != nullptr) {
		pSprite->UnlinkSprite();
		delete pSprite;
	}
}


bool CBofSprite::SetupWorkArea(int dx, int dy) {
	bool bSuccess;

	// Assume failure
	bSuccess = false;

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

	CBofSprite *pSprite;

	// Create an object for the sprite
	if ((pSprite = new CBofSprite) != nullptr) {
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

		pSprite->m_cRect = m_cRect;
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
	CBofBitmap *pBitmap;
	if ((pBitmap = new CBofBitmap(pszPathName, m_pSharedPalette)) != nullptr) {
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

	pBitmap->SetReadOnly(m_bReadOnly);

	m_cSize = pBitmap->GetSize();

	m_cRect.SetRect(0, 0, m_cSize.cx - 1, m_cSize.cy - 1);
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

	int     nStripWidth;                                        // Temp place to hold cel strip width

	m_nCelCount = nCels;                                        // Set cel count
	m_nCelID = m_nCelCount - 1;                                 // No current cel
	nStripWidth = m_cSize.cx;                                   // Retain cell strip pixel length
	m_cSize.cx /= nCels;                                        // Calculate width of a cel

	if (m_cSize.cx * nCels == nStripWidth) {                    // Verify we have an even multiple
		m_cRect.right = m_cRect.left + m_cSize.cx;              // Reset sprite rectangular bounds
		m_cRect.bottom = m_cRect.top + m_cSize.cy;              // ... based on cel dimensions
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
	Assert(pWnd->GetBackdrop() != nullptr);

	BatchPaint(x, y);

	UpdateDirtyRect(pWnd, this);

	return !ErrorOccurred();
}


bool CBofSprite::PaintSprite(CBofBitmap *pBmp, const int x, const int y) {
	Assert(IsValidObject(this));

	// Can't paint to a non-existant window
	Assert(pBmp != nullptr);

	BatchPaint(x, y);

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


void CBofSprite::BatchPaint(const int x, const int y) {
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
		AddToDirtyRect(&m_cRect);
	}

	// Now establish the sprite's new position
	SetPosition(x, y);

	if (m_bAnimated && (m_nCelCount > 1))
		// Advance to the next cel in the strip
		NextCel();
}

bool CBofSprite::UpdateDirtyRect(CBofWindow *pWnd, CBofSprite *pPrimarySprite) {
	Assert(pWnd != nullptr);

	CBofRect cRect, cSrcRect, *pRect;
	CBofBitmap *pBackdrop, *pWork;
	CBofSprite *pSprite;
	int dx, dy;
	bool bTempWorkArea;

	// The window MUST have a backdrop associated with it.  If that's not feasable, then
	// use CSprites instead of CBofSprites
	Assert(pWnd->GetBackdrop() != nullptr);

	//
	// Repaint the contents of the specified rectangle
	//

	if ((pBackdrop = pWnd->GetBackdrop()) != nullptr) {

		pRect = m_cDirtyRect;
		if (pRect->Width() != 0 && pRect->Height() != 0) {
			// Need a work area
			pWork = m_pWorkBmp;
			dx = pRect->Width();
			dy = pRect->Height();

			bTempWorkArea = false;
			if ((pWork == nullptr) || (dx > m_nWorkDX) || (dy > m_nWorkDY)) {

				bTempWorkArea = true;
				pWork = new CBofBitmap(dx, dy, m_pSharedPalette);
			}
			pWork->Lock();

			// Paint the background into the work area
			pBackdrop->Paint(pWork, 0, 0, pRect);

			// Only need to search the sprite list if current sprite is linked
			pSprite = pPrimarySprite;
			if (pPrimarySprite == nullptr || pPrimarySprite->m_bLinked) {
				pSprite = m_pSpriteChain;
			}

			// Run thru the sprite list
			while (pSprite != nullptr) {
				// and paint each partial sprite overlap to the work area
				if (pSprite->m_bPositioned && cRect.IntersectRect(&pSprite->m_cRect, pRect)) {
					if (pPrimarySprite != pSprite)
						m_pTouchedSprite = pSprite;

					cSrcRect = cRect - pSprite->m_cRect.TopLeft();
					cSrcRect += pSprite->m_cImageRect.TopLeft();
					cRect -= pRect->TopLeft();

					pSprite->m_pImage->Paint(pWork, &cRect, &cSrcRect, pSprite->m_nMaskColor);
				}
				pSprite = (CBofSprite *)pSprite->m_pNext;
			}

			// Paint final outcome to the screen
			cSrcRect.SetRect(0, 0, pRect->Width() - 1, pRect->Height() - 1);
			pWork->Paint(pWnd, pRect, &cSrcRect);

			pWork->UnLock();

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

	CBofRect cRect, cSrcRect, *pRect;
	CBofSprite *pSprite;

	//
	// Repaint the contents of the specified rectangle
	//
	pRect = GetDirtyRect();

	// Only need to search the sprite list if current sprite is linked
	pSprite = pPrimarySprite;
	if (pPrimarySprite == nullptr || pPrimarySprite->m_bLinked) {
		pSprite = m_pSpriteChain;
	}

	// Run thru the sprite list
	while (pSprite != nullptr) {
		// and paint each partial sprite overlap to the work area
		if (pSprite->m_bPositioned && cRect.IntersectRect(&pSprite->m_cRect, pRect)) {

			if (pPrimarySprite != pSprite)
				m_pTouchedSprite = pSprite;

			cSrcRect = cRect - pSprite->m_cRect.TopLeft();
			cSrcRect += pSprite->m_cImageRect.TopLeft();

			pSprite->m_pImage->Paint(pBmp, &cRect, &cSrcRect, pSprite->m_nMaskColor);
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

	// All sprites must have atleast 1 frame
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


bool CBofSprite::EraseSprites(CBofWindow *pWnd) {
	CBofSprite *pSprite;

	Assert(pWnd != nullptr);

	pSprite = m_pSpriteChain;

	while (pSprite != nullptr) {
		pSprite->BatchErase();
		pSprite = (CBofSprite *)pSprite->m_pNext;
	}
	UpdateDirtyRect(pWnd);

	return true;
}


bool CBofSprite::EraseSprite(CBofWindow *pWnd) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);

	BatchErase();
	UpdateDirtyRect(pWnd);

	return !ErrorOccurred();
}


void CBofSprite::BatchErase() {
	if (m_bPositioned) {
		m_bPositioned = false;

		AddToDirtyRect(&m_cRect);
	}
}


bool CBofSprite::TestInterception(CBofSprite *pTestSprite, CBofPoint *pPoint) {
	Assert(IsValidObject(this));

	CBofRect overlapRect;		// Area of overlap between rectangles

	Assert(pTestSprite != nullptr);

	// Punt if no interception allowed
	if (pTestSprite != nullptr) {
		// be sure to not test against ourself
		if (this != pTestSprite) {

			// Use simple rectangle screening first
			if (overlapRect.IntersectRect(&m_cRect, &pTestSprite->m_cRect)) {
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

	CBofSprite *pSprite;	// Pointer to current sprite
	CBofRect overlapRect;	// Area of overlap between rectangles

	Assert(pNewRect != nullptr);

	// Get first sprite to be tested
	pSprite = pTestSprite;

	// Thumb through the sprite chain
	while (pSprite != nullptr) {
		// be sure to not test against ourself
		// ... and only test against overlapping sprites
		if (this != pSprite) {
			// Sprites touch if their rectangles intersect.
			// does our sprite overlap another?
			if (overlapRect.IntersectRect(pNewRect, &pSprite->m_cRect))
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

	CBofSprite *pSprite;				// Pointer to current sprite

	pSprite = pTestSprite;				// Get first sprite to be tested

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

	CBofRect overlapRect;
	int32 dx, dy, x, y, x1, y1, x2, y2, dx1, dx2;
	byte *pDib1, *pDib2, *pPtr1, *pPtr2;
	byte m1, m2;
	bool bHit;

	// Assume no overlap
	bHit = false;

	// If the sprite's rectangles overlap
	if (overlapRect.IntersectRect(&m_cRect, &pSprite->m_cRect)) {
		dx = overlapRect.Width();
		dy = overlapRect.Height();

		x1 = overlapRect.left - m_cRect.left + m_cImageRect.left;
		y1 = overlapRect.top - m_cRect.top + m_cImageRect.top;

		x2 = overlapRect.left - pSprite->m_cRect.left + pSprite->m_cImageRect.left;
		y2 = overlapRect.top - pSprite->m_cRect.top + pSprite->m_cImageRect.top;

		dx1 = m_pImage->WidthBytes();
		dx2 = pSprite->m_pImage->WidthBytes();

		m1 = (byte)m_nMaskColor;
		m2 = (byte)pSprite->m_nMaskColor;

		// Lock down these bitmaps
		m_pImage->Lock();
		pSprite->m_pImage->Lock();

		pDib1 = (byte *)m_pImage->GetPixelAddress((int)x1, (int)y1);
		pDib2 = (byte *)pSprite->m_pImage->GetPixelAddress((int)x2, (int)y2);

		if (!m_pImage->IsTopDown()) {
			dx1 = -dx1;
		}

		if (!pSprite->m_pImage->IsTopDown()) {
			dx2 = -dx2;
		}

		for (y = 0; y < dy; y++) {
			pPtr1 = pDib1;
			pPtr2 = pDib2;

			for (x = 0; x < dx; x++) {
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
	pSprite->m_pImage->UnLock();
	m_pImage->UnLock();

	return bHit;
}


bool CBofSprite::Touching(CBofPoint myPoint) {
	Assert(IsValidObject(this));

	// Ignoring sprites that don't intercept
	if (m_cRect.PtInRect(myPoint))   // See if the point is in the sprite's rectangle
		return true;                 // ... and if so, return success

	return false;
}


CBofSprite *CBofSprite::Touched(CBofPoint myPoint, CBofSprite *pSprite) {
	while (pSprite != nullptr) {                    // Thumb through the entire sprite collection
		if (pSprite->m_cRect.PtInRect(myPoint))     // See if the point is in the sprite's rectangle
			return pSprite;                         // ... and if so, return a pointer to it

		pSprite = (CBofSprite *)pSprite->m_pNext;	// Fetch next sprite for testing
	}

	return nullptr;
}


void CBofSprite::SetPosition(int x, int y) {
	Assert(IsValidObject(this));

	// Now have a real location establish the new location of the sprite
	// and setup the bitmap's bounding rectangle
	m_bPositioned = true;
	m_cPosition.x = x;
	m_cPosition.y = y;
	m_cRect.SetRect(m_cPosition.x, m_cPosition.y, m_cPosition.x + m_cSize.cx - 1, m_cPosition.y + m_cSize.cy - 1);
}


bool CBofSprite::CropImage(CBofWindow *pWnd, CBofRect *pRect, bool bUpdateNow) {
	Assert(IsValidObject(this));
	Assert(pWnd != nullptr);
	Assert(pRect != nullptr);
	Assert(m_pImage != nullptr);

	if (m_nMaskColor != NOT_TRANSPARENT) {

		CBofRect       myRect, cDestRect;
		CBofBitmap *pBackdrop;

		myRect = *pRect;                                // Offset crop area by image rect
		myRect.left += m_cImageRect.left;
		myRect.right += m_cImageRect.left;

		cDestRect = myRect + m_cPosition;

		m_pImage->FillRect(&myRect, (byte)m_nMaskColor);

		if (bUpdateNow) {
			if ((pBackdrop = pWnd->GetBackdrop()) != nullptr) {
				pBackdrop->Paint(pWnd, &cDestRect, &myRect);
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


void CBofSprite::SetReadOnly(bool bReadOnly) {
	Assert(IsValidObject(this));
	Assert(m_pImage != nullptr);

	m_bReadOnly = bReadOnly;

	m_pImage->SetReadOnly(bReadOnly);
}


void CBofSprite::SetSharedPalette(CBofPalette *pPal) {
	Assert(pPal != nullptr);

	m_pSharedPalette = pPal;
}


void CBofSprite::SetMasked(bool bMasked) {
	Assert(IsValidObject(this));

	// Default to white as transparent color
	m_nMaskColor = COLOR_WHITE;

	if (!bMasked) {
		m_nMaskColor = NOT_TRANSPARENT;
	}
}


bool CBofSprite::IsSpriteInSprite(CBofSprite *pSprite) {
	CBofRect cUnionRect;
	int32 x, y, dx, dy, x1, y1, x2, y2, dx1, dy1, dx2, dy2;
	int32 nDxBytes1, nDxBytes2;
	bool bOk1, bOk2, bGood1, bGood2;
	byte *pDib1, *pDib2, *pPtr1, *pPtr2;
	byte m1, m2;
	byte c1, c2;

	// Can't access null pointer
	Assert(pSprite != nullptr);

	if (pSprite == this)
		return true;

	if (!cUnionRect.UnionRect(&m_cRect, &pSprite->m_cRect))
		return false;

	m_pImage->Lock();
	pSprite->m_pImage->Lock();

	pDib1 = (byte *)m_pImage->GetPixelAddress(0, 0);
	pDib2 = (byte *)pSprite->m_pImage->GetPixelAddress(0, 0);

	if (m_nCelCount != 0) {
		pDib1 += ((int32)m_cSize.cx * m_nCelID);
	}

	if (pSprite->m_nCelCount != 0) {
		pDib2 += ((int32)pSprite->m_cSize.cx * pSprite->m_nCelID);
	}

	dx = cUnionRect.Width();
	dy = cUnionRect.Height();

	x1 = m_cRect.left - cUnionRect.left;
	y1 = m_cRect.top - cUnionRect.top;

	x2 = pSprite->m_cRect.left - cUnionRect.left;
	y2 = pSprite->m_cRect.top - cUnionRect.top;

	dx1 = x1 + m_cSize.cx;
	dy1 = y1 + m_cSize.cy;

	dx2 = x2 + pSprite->m_cSize.cx;
	dy2 = y2 + pSprite->m_cSize.cy;

	nDxBytes1 = m_pImage->WidthBytes();

	nDxBytes2 = pSprite->m_pImage->WidthBytes();

	m1 = (byte)m_nMaskColor;
	m2 = (byte)pSprite->m_nMaskColor;

	if (!m_pImage->IsTopDown())
		nDxBytes1 = -nDxBytes1;

	if (!pSprite->m_pImage->IsTopDown())
		nDxBytes2 = -nDxBytes2;

	for (y = 0; y < dy; y++) {
		pPtr1 = pDib1;
		pPtr2 = pDib2;

		bOk1 = false;
		if ((y >= y1) && (y < dy1)) {
			bOk1 = true;
		}

		bOk2 = false;
		if ((y >= y2) && (y < dy2)) {
			bOk2 = true;
		}

		for (x = 0; x < dx; x++) {
			bGood1 = false;
			c1 = m1;
			if (bOk1 && (x >= x1) && (x < dx1)) {
				bGood1 = true;
				c1 = *pPtr1++;
			}

			bGood2 = false;
			c2 = m2;

			if (bOk2 && (x >= x2) && (x < dx2)) {
				bGood2 = true;
				c2 = *pPtr2++;
			}

			if (bGood1 && (c1 != m1) && (!bGood2 || (c2 == m2))) {
				goto endroutine;
			}
		}

		if (bOk1) {
			pDib1 += nDxBytes1;
		}

		if (bOk2) {
			pDib2 += nDxBytes2;
		}
	}

endroutine:

	pSprite->m_pImage->UnLock();
	m_pImage->UnLock();

	return true;
}


bool CBofSprite::IsSpriteHidden() {
	CBofRect rect, cOverlapRect;
	byte *pBuf;
	CBofSprite *pSprite;
	int32 i, lBufSize;
	int32 dx, dy, x, y, x1, y1, x2, y2, dx1, dx2, dy1, dy2;
	byte *pDib1, *pDib2, *pPtr1, *pPtr2;
	byte m1, m2;
	bool bHidden;

	rect = m_cRect;

	// Assume sprite is visible
	bHidden = false;

	// Create a copy of the DibBytes for this sprite
	if ((pBuf = (byte *)BofAlloc(m_pImage->WidthBytes() * m_pImage->Height())) != nullptr) {
		m_pImage->Lock();

		pDib1 = pBuf;
		pDib2 = (byte *)m_pImage->GetPixelAddress(0, 0);

		lBufSize = m_pImage->WidthBytes() * m_pImage->Height();
		for (i = 0; i < lBufSize; i++) {
			*pDib1++ = *pDib2++;
		}

		m1 = (byte)m_nMaskColor;

		// Walk thru the sprite chain
		pSprite = m_pSpriteChain;
		while (pSprite != nullptr) {

			// For all visible sprites, mask out those areas of our copy
			// that are covered by other sprites
			if (pSprite != this) {
				// Sprites must atleast intersect
				if (cOverlapRect.IntersectRect(&rect, &pSprite->m_cRect)) {
					if ((pSprite->m_nZOrder < m_nZOrder) || ((pSprite->m_nZOrder == m_nZOrder))) {
						pSprite->m_pImage->Lock();

						pDib1 = pBuf;
						pDib2 = (byte *) pSprite->m_pImage->GetPixelAddress(0, 0);

						dx = cOverlapRect.Width();
						dy = cOverlapRect.Height();

						x1 = cOverlapRect.left - m_cRect.left + m_cImageRect.left;
						y1 = cOverlapRect.top - m_cRect.top + m_cImageRect.top;

						x2 = cOverlapRect.left - pSprite->m_cRect.left + pSprite->m_cImageRect.left;
						y2 = cOverlapRect.top - pSprite->m_cRect.top + pSprite->m_cImageRect.top;

						dx1 = m_pImage->WidthBytes();
						dx2 = pSprite->m_pImage->WidthBytes();

						dy1 = m_pImage->Height();
						dy2 = pSprite->m_pImage->Height();

						m2 = (byte)pSprite->m_nMaskColor;

						if (m_pImage->IsTopDown()) {
							pDib1 += y1 * dx1 + x1;
						} else {
							pDib1 += (dy1 - y1 - 1) * dx1 + x1;
							dx1 = -dx1;
						}

						if (pSprite->m_pImage->IsTopDown()) {
							pDib2 += y2 * dx2 + x2;
						} else {
							pDib2 += (dy2 - y2 - 1) * dx2 + x2;
							dx2 = -dx2;
						}

						for (y = 0; y < dy; y++) {
							pPtr1 = pDib1;
							pPtr2 = pDib2;

							for (x = 0; x < dx; x++) {
								if ((*pPtr1 != m1) && (*pPtr2 != m2)) {
									*pPtr1 = m1;
								}

								pPtr1++;
								pPtr2++;
							}

							pDib1 += dx1;
							pDib2 += dx2;
						}

						pSprite->m_pImage->UnLock();
					}
				}
			}

			pSprite = (CBofSprite *)pSprite->m_pNext;
		}

		bHidden = true;
		pDib1 = pBuf;

		for (y = 0; y < m_pImage->Height(); y++) {
			for (x = 0; x < m_pImage->Width(); x++) {
				if (*pBuf++ != m1) {
					bHidden = false;
					break;
				}
			}

			if (!bHidden)
				break;

			pBuf += m_pImage->WidthBytes() - m_pImage->Width();
		}

		m_pImage->UnLock();

		BofFree(pBuf);
	}

	return bHidden;
}


bool CBofSprite::PtInSprite(CBofPoint cTestPoint) {
	int32 x, y;
	byte *pBuf;
	int nCels;
	bool bTouch;

	// Assume point is not in sprite
	bTouch = false;

	if (m_cRect.PtInRect(cTestPoint)) {
		if (m_nMaskColor == NOT_TRANSPARENT)
			return true;

		x = cTestPoint.x - m_cPosition.x;
		y = cTestPoint.y - m_cPosition.y;

		Assert(m_pImage != nullptr);

		m_pImage->Lock();

		pBuf = (byte *)m_pImage->GetPixelAddress(0, 0);
		Assert(pBuf != nullptr);

		nCels = m_nCelID;

		if (!m_pImage->IsTopDown()) {
			y = -y;
			nCels = -m_nCelID;
		}

		if (m_nCelCount > 1) {
			pBuf += ((int32)m_cSize.cx * nCels);
		}

		if (*(pBuf + (y * m_pImage->WidthBytes()) + x) != (byte)m_nMaskColor) {
			bTouch = true;
		}

		m_pImage->UnLock();
	}

	return bTouch;
}


bool CBofSprite::TestPossibleInterception(CBofPoint cPoint, CBofSprite *pSprite) {
	CBofRect cOldRect;
	bool bTouched;

	Assert(pSprite != nullptr);

	cOldRect = m_cRect;

	m_cRect.SetRect(cPoint.x, cPoint.y, cPoint.x + m_cSize.cx - 1, cPoint.y + m_cSize.cy - 1);

	bTouched = TestInterception(pSprite);

	m_cRect = cOldRect;

	return bTouched;
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
