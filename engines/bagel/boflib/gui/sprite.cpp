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

#include "bagel/boflib/gui/sprite.h"
#include "bagel/boflib/debug.h"

namespace Bagel {

#define SPRITE_DEBUG    0                        // set for debugging sprite reconstruction


CSprite::CSprite() {
	m_pBackground = NULL;                               // no initial background mask

	m_bOverlaps = FALSE;                                // does not yet overlap other sprites
	m_bRetainBackground = TRUE;                         // retain background image for updates
	m_nType = 0;                                        // no user defined information
	m_nZPosition = SPRITE_TOPMOST;                      // default to top most in fore/back ground order
	m_pZNext = NULL;                                    // it has not yet been Z-Order sorted
	m_pZPrev = NULL;
}


CSprite::~CSprite() {
	Assert(IsValidObject(this));

	ClearBackground();                                  // clear the sprite background bitmap and context
}


CSprite *CSprite::DuplicateSprite(VOID) {
	Assert(IsValidObject(this));

	CSprite *pSprite;

	// create an object for the sprite
	if ((pSprite = new CSprite) != NULL) {
		DuplicateSprite(pSprite);
	}

	return (pSprite);
}


BOOL CSprite::DuplicateSprite(CSprite *pSprite) {
	Assert(IsValidObject(this));

	// we require a valid sprite to copy
	Assert(pSprite != NULL);

	CBofSprite::DuplicateSprite(pSprite);

	pSprite->m_nType = m_nType;
	pSprite->m_nZPosition = m_nZPosition;
	pSprite->m_bRetainBackground = m_bRetainBackground;

	// mark it as a sprite with shared resources
	pSprite->m_bDuplicated = TRUE;

	return (TRUE);
}


BOOL CSprite::PaintSprite(CBofWindow *pWnd, const INT x, const INT y) {
	BOOL        bSuccess = FALSE;

	Assert(IsValidObject(this));

	if (pWnd->GetBackdrop() != NULL) {

		bSuccess = CBofSprite::PaintSprite(pWnd, x, y);

	} else {

		CBofRect    overlapRect, dstRect;
		CSprite    *pOverlap = NULL;
		BOOL        bPosChanged, bFirstTime;

		// can't paint to a non-existant device
		Assert(pWnd != NULL);

		m_pTouchedSprite = NULL;                        // ... painting operation

		// setup the initial location if not already specified
		//
		bFirstTime = FALSE;
		bPosChanged = FALSE;
		if (!m_bPositioned)  {
			SetPosition(x, y);
			bFirstTime = TRUE;
			bPosChanged = TRUE;

		} else {

			// if not painting to same loation
			//
			if ((x != m_cPosition.x) || (y != m_cPosition.y)) {
				bPosChanged = TRUE;
			}
		}

		if (m_bAnimated && (m_nCelCount > 1))                 // advance to the next cel in the strip
			NextCel();

		dstRect.SetRect(x, y, x + m_cSize.cx - 1, y + m_cSize.cy - 1); // calculate destination rectangle

		pOverlap = (CSprite *)Interception(&m_cRect);              // see if the sprite will intercept another at new location
		if (pOverlap == NULL) {
			pOverlap = (CSprite *)Interception(&dstRect);          // or if touching sprite at current location
		}
		if (pOverlap != NULL) {                         // ... and if so, record that fact
			m_pTouchedSprite = pOverlap;                // here's what we touched
		}

		// how we do the painting depends on whether we intercept another sprite
		//
		if (!m_bLinked || (pOverlap == NULL)) {

			if (!bFirstTime && ((bPosChanged && overlapRect.IntersectRect(&m_cRect, &dstRect)) || (!bPosChanged && (m_nMaskColor != NOT_TRANSPARENT) && m_bAnimated))) {

				// optimize painting of localized movement by doing it all offscreen
				bSuccess = DoOptimizedPainting(pWnd, &dstRect);

				// establish the sprite's new position
				SetPosition(x, y);

			} else {

				if (!bFirstTime && (bPosChanged || ((m_nMaskColor != NOT_TRANSPARENT) && m_bAnimated))) {

					// If it isn't close, just restore its background
					RefreshBackground(pWnd);
				}

				// set the sprites new location
				SetPosition(x, y);

				// paint the sprite in its new location
				bSuccess = UpdateSprite(pWnd, bPosChanged);
			}
			m_bOverlaps = FALSE;                        // this sprite does not overlap another
			m_nZPosition = m_nZOrder;                   // ... so reset its z ordering

			// otherwise, we have an overlap situation
			//
		} else {

			// ... and rebuild sprite by sprite
			bSuccess = DoOverlapPainting(pWnd, &dstRect);
			SetPosition(x, y);
		}
	}

	return (bSuccess);
}


BOOL CSprite::UpdateSprite(CBofWindow *pWnd, BOOL bSaveBackground) {
	Assert(IsValidObject(this));
	Assert(pWnd != NULL);

	CBofBitmap *pWork;
	INT dx, dy;
	BOOL bTempWorkArea, bSuccess;

	bSuccess = TRUE;

	// if we don't have a backdrop bitmap, and if we move and are painting
	// for the first time then save the background behind where the sprite will be painted
	//
	if (m_bRetainBackground && (m_pBackground == NULL || bSaveBackground)) {
		bSuccess = SaveBackground(pWnd);
	}

	if (m_nMaskColor != NOT_TRANSPARENT) {

		pWork = m_pWorkBmp;
		dx = m_cImageRect.Width();
		dy = m_cImageRect.Height();

		bTempWorkArea = FALSE;
		if ((m_pWorkBmp == NULL) || (dx > m_nWorkDX) || (dy > m_nWorkDY)) {

			bTempWorkArea = TRUE;
			pWork = new CBofBitmap(dx, dy, m_pSharedPalette);
		}

		// by using our offscreen backdrop
		//
		CBofRect cRect(m_cPosition, m_cSize);
		CBofRect cTempRect(0, 0, dx - 1, dy - 1);
		pWork->CaptureScreen(pWnd, &cRect, &cTempRect);

		m_pImage->Paint(pWork, 0, 0, &m_cImageRect, m_nMaskColor);

		pWork->Paint(pWnd, m_cPosition.x, m_cPosition.y, &cTempRect);

		// delete the temp work area
		//
		if (bTempWorkArea) {
			delete pWork;
		}

	} else {

		// if masked, then paint the sprite, masking out it's transparent color
		// NOTE: m_nMaskColor determines whether or not the image is masked.
		//
		m_pImage->Paint(pWnd, m_cPosition.x, m_cPosition.y, &m_cImageRect, m_nMaskColor);
	}

	return (bSuccess);
}


BOOL CSprite::DoOptimizedPainting(CBofWindow *pWnd, CBofRect *pDstRect) {
	Assert(IsValidObject(this));

	BOOL    bSuccess = FALSE;                           // success/failure return status
	CBofPoint  cPoint;                                     // where to paint sprite in the work area
	CBofRect   unionRect;                                  // rectangle enclosing old and new sprite locations
	CBofRect   cTempRect;
	CBofBitmap *pWork;
	INT     dx, dy;                                     // delta sizes of work area's bitmap
	BOOL    bTempWorkArea;

	Assert(pWnd != NULL);

	// assume fixed work area
	bTempWorkArea = FALSE;

	// the work area is defined by our sprite
	//
	unionRect = m_cRect;
	if (pDstRect != NULL)
		unionRect.UnionRect(&unionRect, pDstRect);

	// calculate the smallest enclosing rectangle that contains the bitmap
	// area where the sprite was and the bitmap area where it will be next.
	//
	dx = unionRect.Width();
	dy = unionRect.Height();

	pWork = m_pWorkBmp;
	if ((m_pWorkBmp == NULL) || (dx > m_nWorkDX) || (dy > m_nWorkDY)) {

		bTempWorkArea = TRUE;
		pWork = new CBofBitmap(dx, dy, m_pSharedPalette);
	}

	cPoint.x = pDstRect->left - unionRect.left;   // determine where to paint the new sprite image
	cPoint.y = pDstRect->top - unionRect.top;     // ... into the work area

	// restore the background where the sprite was
	//

	// by using our offscreen backdrop
	//
	cTempRect.SetRect(0, 0, dx - 1, dy - 1);

	pWork->CaptureScreen(pWnd, &unionRect, &cTempRect);

	m_pBackground->Paint(pWork, m_cPosition.x - unionRect.left, m_cPosition.y - unionRect.top);

	cTempRect.SetRect(cPoint.x, cPoint.y, cPoint.x + m_cSize.cx - 1, cPoint.y + m_cSize.cy - 1);
	pWork->Paint(m_pBackground, 0, 0, &cTempRect);


	// just paint in the sprite
	//
	cTempRect.SetRect(cPoint.x, cPoint.y, cPoint.x + m_cSize.cx - 1, cPoint.y + m_cSize.cy - 1);
	m_pImage->Paint(pWork, &cTempRect, &m_cImageRect, m_nMaskColor);

	// paint the final outcome to the screen
	//
	cTempRect.SetRect(0, 0, dx - 1, dy - 1);
	pWork->Paint(pWnd, &unionRect, &cTempRect);

	// ... delete the bitmap
	if (bTempWorkArea) {
		delete pWork;
	}

	return (bSuccess);
}


BOOL CSprite::DoOverlapPainting(CBofWindow *pWnd, CBofRect *pMyRect) {
	Assert(IsValidObject(this));
	Assert(pWnd != NULL);

	BOOL        bSuccess = FALSE;

	// go do the actual painting
	bSuccess = ReconstructBackground(pWnd, pMyRect);

	return (bSuccess);
}


BOOL CSprite::RefreshBackground(CBofWindow *pWnd) {
	Assert(IsValidObject(this));
	Assert(pWnd != NULL);

	BOOL        bSuccess;

	bSuccess = TRUE;

	if (m_bPositioned && (m_pBackground != NULL)) {

		// paint the background bitmap from all the sprites that overlap it
		//
		if (Interception(&m_cRect) != NULL) {

			bSuccess = ReconstructBackground(pWnd, NULL);

		} else {

			// paint the background bitmap to the device
			//
			m_errCode = m_pBackground->Paint(pWnd, m_cPosition.x, m_cPosition.y);
		}

		m_bPositioned = FALSE;                                  // no real position now
		m_nZPosition = m_nZOrder;                               // reset z ordering
		//m_bOverlaps = FALSE;
	}

	return (bSuccess);
}


BOOL CSprite::SaveBackground(CBofWindow *pWnd) {
	Assert(IsValidObject(this));
	Assert(pWnd != NULL);

	BOOL        bSuccess;

	bSuccess = TRUE;

	if (!pWnd->HasBackdrop() && m_bRetainBackground) {

		// create background context
		//
		if ((bSuccess = CreateBackground()) != FALSE) {

			// grab the background bitmap
			CBofRect cRect(m_cPosition, m_cSize);
			m_errCode = m_pBackground->CaptureScreen(pWnd, &cRect);

			bSuccess = (m_errCode == ERR_NONE);
		}
	}

	return (bSuccess);
}


BOOL CSprite::ReconstructBackground(CBofWindow *pWnd, CBofRect *pMyRect) {
	CBofRect        baseRect,                               // rectangle spanning the sprite update
	                unionRect,                              // rectangle spanning all sprites touched
	                overlapRect;                            // temporary rectangle information
	CBofPoint       cPoint;                                 // temporary point information
	CBofBitmap  *pWork;
	CSprite  *pSprite = NULL,                         // various sprite pointers
	          *pTestSprite,
	          *pZHead;
	BOOL         bSuccess;                               // used to hold success/failure
	INT          dx, dy;                                 // size of unionRect and work bitmap
	BOOL         bTempWork;
	CBofRect        cTempRect;

	// assume success
	bSuccess = TRUE;

	unionRect = m_cRect;                                // the work area is defined by our sprite
	if (pMyRect != NULL)                                // include the area of our new destination
		unionRect.UnionRect(&unionRect, pMyRect);        // ... if specified, and retain it for later
	baseRect = unionRect;

	pSprite = this;                                     // start processing from this sprite and make
	pSprite->m_pZNext = NULL;                           // ... it be the first in the z order chain
	pSprite->m_pZPrev = NULL;

	pTestSprite = (CSprite *)m_pSpriteChain;                       // set all sprites to not having been tested
	while (pTestSprite != NULL) {                       // ... for this reconstruction cycle

		pTestSprite->m_bOverlapTest = FALSE;
		if (!pTestSprite->m_bPositioned)
			pTestSprite->m_bOverlapTest = TRUE;

		pTestSprite = (CSprite *)pTestSprite->m_pNext;
	}
	pTestSprite = (CSprite *)m_pSpriteChain;                       // get first sprite to test against
	m_bOverlapTest = TRUE;                              // set to not test against the first sprite
	m_bOverlaps = TRUE;

	m_bPaintOverlap = TRUE;

	if (m_pBackground == NULL)
		m_bPaintOverlap = FALSE;

	// look for sprites we overlap or which are
	// ... overlapped by sprites that overlap us
	//
	while (pTestSprite != NULL) {

		if (!pTestSprite->m_bOverlapTest && overlapRect.IntersectRect(&unionRect, &pTestSprite->m_cRect)) {

			// expand rectangle to encompass it
			unionRect.UnionRect(&unionRect, &pTestSprite->m_cRect);

			// mark it as having been tested
			pTestSprite->m_bOverlapTest = TRUE;

			// mark it as involved in an overlap situation
			pTestSprite->m_bOverlaps = TRUE;

			// won't paint sprites without a background
			//
			pTestSprite->m_bPaintOverlap = FALSE;
			if ((pTestSprite->m_pBackground != NULL) && overlapRect.IntersectRect(&baseRect, &pTestSprite->m_cRect))
				pTestSprite->m_bPaintOverlap = TRUE;

			// insert the sprite in the sorted z chain
			//
			for (;;) {

				// need to head leftward
				//
				if (pTestSprite->m_nZPosition < pSprite->m_nZPosition) {

					// put it to the left of us (i.e. overlaps us)
					//
					if ((pSprite->m_pZPrev == NULL) || (pSprite->m_pZPrev->m_nZPosition < pTestSprite->m_nZPosition)) {
						pTestSprite->m_pZPrev = pSprite->m_pZPrev;
						pTestSprite->m_pZNext = pSprite;
						pSprite->m_pZPrev = pTestSprite;
						if (pTestSprite->m_pZPrev != NULL)
							pTestSprite->m_pZPrev->m_pZNext = pTestSprite;
						break;
					} else {
						pSprite = pSprite->m_pZPrev;  // shift left in chain and set to test again
					}

					// need to head rightward
					//
				} else {

					// put it to the right of us (we overlap it)
					//
					if ((pSprite->m_pZNext == NULL) || (pSprite->m_pZNext->m_nZPosition >= pTestSprite->m_nZPosition)) {

						pTestSprite->m_pZNext = pSprite->m_pZNext;
						pTestSprite->m_pZPrev = pSprite;
						pSprite->m_pZNext = pTestSprite;
						if (pTestSprite->m_pZNext != NULL)
							pTestSprite->m_pZNext->m_pZPrev = pTestSprite;
						break;

					} else {
						// shift right in chain and set to test again
						pSprite = pSprite->m_pZNext;
					}
				}
			}
			// need to check all sprites again since the bounding rectangle
			// has expanded and may now intercept additional sprites
			pTestSprite = (CSprite *)m_pSpriteChain;

		} else {

			// set to test against the next sprite
			pTestSprite = (CSprite *)pTestSprite->m_pNext;
		}
	}

	while (pSprite->m_pZPrev != NULL)                 // find the head of the z order chain so that
		pSprite = pSprite->m_pZPrev;                  // ... we can begin background reconstruction

	pZHead = pSprite;                                   // save head of z order chain

	// if we are the head of the z order chain and we are a top-most type
	// of sprite and we're painting direct to the screen
	//
	if ((pZHead == this) && (m_nZOrder == SPRITE_TOPMOST)) {

		// then we only need to handle this sprite
		// so set to not paint others in z order chain
		//
		pTestSprite = pZHead->m_pZNext;
		while (pTestSprite != NULL) {
			pTestSprite->m_bPaintOverlap = FALSE;
			pTestSprite = pTestSprite->m_pZNext;
		}
		unionRect = baseRect;                           // minimize the bounding rectangle
	}

	// get the width and height of the work area spanned by all
	// overlapping sprites
	//
	dx = unionRect.Width();
	dy = unionRect.Height();

	bTempWork = FALSE;
	pWork = m_pWorkBmp;
	if (m_pWorkBmp == NULL || (dx > m_nWorkDX) || (dy > m_nWorkDY)) {
		bTempWork = TRUE;
		if ((pWork = new CBofBitmap(dx, dy, m_pSharedPalette)) == NULL) {
			return (FALSE);
		}
	}

	pWork->CaptureScreen(pWnd, &unionRect);

	// now restore the saved background from each
	for (;;) {

		if (pSprite->m_bPaintOverlap && pSprite->m_bRetainBackground) {
			pSprite->m_pBackground->Paint(pWork, pSprite->m_cPosition.x - unionRect.left, pSprite->m_cPosition.y - unionRect.top);
		}

		// once we have restored all backgrounds
		//
		if (pSprite->m_pZNext == NULL) {

			// then place us at the head of the z order chain if we
			// are to be topmost
			//
			if ((m_nZOrder == SPRITE_TOPMOST) && (this != pZHead)) {

				if (pSprite == this)      // maintain pointer to last in z order
					pSprite = m_pZPrev;             // ... backing up one if it is us
				pTestSprite = m_pZPrev;             // ... since we're moving to the head
				pTestSprite->m_pZNext = m_pZNext;
				if (m_pZNext != NULL)
					m_pZNext->m_pZPrev = pTestSprite;
				m_pZNext = pZHead;
				m_pZPrev = NULL;
				pZHead->m_pZPrev = this;
				pZHead = this;
				m_nZPosition = m_nZOrder;
			}
			break;
		}

		pSprite = pSprite->m_pZNext;
	}


	pTestSprite = pZHead;                               // update z order relative positions
	while (pTestSprite->m_pZNext != NULL) {            // ... to ensure increasing ordering
		if (pTestSprite->m_nZOrder == pTestSprite->m_pZNext->m_nZOrder)
			pTestSprite->m_pZNext->m_nZPosition = pTestSprite->m_nZPosition + 1;
		pTestSprite = pTestSprite->m_pZNext;
	}

	// now repaint the images of the sprites onto the work area
	//
	for (;;) {

		if (this == pSprite) {

			if (pMyRect != NULL) {

				// but before we paint in the sprite's image, retain what that
				// area looks like as the new saved background
				//
				if (m_bRetainBackground) {

					if (m_pBackground == NULL) {

						bSuccess = CreateBackground();
						if (!bSuccess)
							goto punt;
					}
					CBofRect r((CBofPoint)(pMyRect->TopLeft() - unionRect.TopLeft()), m_cSize);
					pWork->Paint(m_pBackground, 0, 0, &r);
				}
				m_bPaintOverlap = TRUE;

				cPoint.x = pMyRect->left - unionRect.left; // set the destination for where our
				cPoint.y = pMyRect->top - unionRect.top;   // ... sprite will be painted
				goto paint_sprite;
			}

			// otherwise, save what the background looks like
			//
		} else {

			if (pSprite->m_bRetainBackground && pSprite->m_bPaintOverlap) {

				CBofRect r((CBofPoint)(pSprite->m_cPosition - unionRect.TopLeft()), pSprite->m_cSize);
				pWork->Paint(pSprite->m_pBackground, 0, 0, &r);
			}
			cPoint.x = pSprite->m_cPosition.x - unionRect.left;   // set the destination for sprite
			cPoint.y = pSprite->m_cPosition.y - unionRect.top;

paint_sprite:
			if (pSprite->m_bPaintOverlap) {
				CBofRect r(pSprite->m_cImageRect.TopLeft(), pSprite->m_cSize);
				pSprite->m_pImage->Paint(pWork, cPoint.x, cPoint.y, &r, m_nMaskColor);

#if SPRITE_DEBUG
				pWork->Paint(pWnd, 0, 0);
#endif
			}
		}

		if (pSprite->m_pZPrev == NULL)                // see if done with image painting
			break;
		pSprite = pSprite->m_pZPrev;
	}

	cTempRect.left = baseRect.left - unionRect.left;
	cTempRect.top = baseRect.top - unionRect.top;
	cTempRect.right = cTempRect.left + baseRect.Width();
	cTempRect.bottom = cTempRect.top + baseRect.Height();

	//cTempRect.SetRect(0, 0, unionRect.Width() - 1, unionRect.Height() - 1);

	pWork->Paint(pWnd, &baseRect, &cTempRect);

	// do we still touch another sprite?
	//m_bOverlaps = (Interception() != NULL);

punt:

	if (bTempWork && (pWork != NULL)) {
		delete pWork;
		pWork = NULL;
	}

	return (bSuccess);
}


BOOL CSprite::EraseSprites(CBofWindow *pWnd) {
	Assert(pWnd != NULL);
	BOOL bSuccess;

	// assume success
	bSuccess = TRUE;

	if (pWnd->GetBackdrop() != NULL) {

		bSuccess = CBofSprite::EraseSprites(pWnd);

	} else {

		CSprite *pSprite;


		pSprite = (CSprite *)m_pSpriteChain;

		while (pSprite != NULL) {
			if ((bSuccess = pSprite->EraseSprite(pWnd)) == FALSE)
				break;
			pSprite = (CSprite *)pSprite->m_pNext;
		}
	}

	return (bSuccess);
}


BOOL CSprite::EraseSprite(CBofWindow *pWnd) {
	Assert(IsValidObject(this));
	Assert(pWnd != NULL);

	BOOL bSuccess;

	bSuccess = TRUE;

	if (pWnd->GetBackdrop() != NULL) {

		bSuccess = CBofSprite::EraseSprite(pWnd);

	} else {

		if (m_bPositioned) {

			bSuccess = RefreshBackground(pWnd);
			ClearBackground();
		}
	}

	return (bSuccess);
}


BOOL CSprite::CropImage(CBofWindow *pWnd, CBofRect *pRect, BOOL bUpdateNow) {
	Assert(IsValidObject(this));
	Assert(pWnd != NULL);
	Assert(pRect != NULL);
	Assert(m_pImage != NULL);

	if (pWnd->GetBackdrop() != NULL) {
		return (CBofSprite::CropImage(pWnd, pRect, bUpdateNow));

	} else {

		if (m_nMaskColor != NOT_TRANSPARENT) {

			CBofRect       myRect, cDestRect;

			myRect = *pRect;                                // offset crop area by image rect
			myRect.left += m_cImageRect.left;
			myRect.right += m_cImageRect.left;

			cDestRect = myRect + m_cPosition;

			m_pImage->FillRect(&myRect, (UBYTE)m_nMaskColor);

			if (bUpdateNow) {
				Assert(m_pBackground != NULL);
				m_pBackground->Paint(pWnd, &cDestRect, pRect);
			}
		}
	}

	return (!ErrorOccurred());
}


BOOL CSprite::CreateBackground(VOID) {
	Assert(IsValidObject(this));

	if (m_pBackground == NULL) {

		// create an object to hold things

		if ((m_pBackground = new CBofBitmap(m_cSize.cx, m_cSize.cy, m_pSharedPalette)) != NULL) {

		} else {
			ReportError(ERR_MEMORY, "Could not create sprite background (%d x %d)", m_cSize.cx, m_cSize.cy);
		}
	}

	return (TRUE);
}


VOID CSprite::ClearBackgrounds(VOID) {
	CSprite *pSprite;

	pSprite = (CSprite *)m_pSpriteChain;

	while (pSprite != NULL) {
		pSprite->ClearBackground();
		pSprite = (CSprite *)pSprite->m_pNext;
	}
}


VOID CSprite::ClearBackground(VOID) {
	Assert(IsValidObject(this));

	m_nZPosition = m_nZOrder;                       // reset z ordering
	m_bOverlaps = FALSE;                            // no longer overlaps other sprites
	m_bPositioned = FALSE;                          // no longer has a real position

	if (m_pBackground != NULL) {                // destroy the backgrond bitmap object
		delete m_pBackground;
		m_pBackground = NULL;
	}
}


VOID CSprite::SetRetainBackground(BOOL bValue) {
	Assert(IsValidObject(this));

	if (!bValue)                                        // if not retaining backgrounds now
		ClearBackground();                              // ... then clear what we have

	m_bRetainBackground = bValue;                       // set new state
}

} // namespace Bagel
