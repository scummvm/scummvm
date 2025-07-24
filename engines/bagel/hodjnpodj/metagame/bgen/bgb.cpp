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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/hnplibs/bitmaps.h"
#include "bagel/hodjnpodj/metagame/bgen/bgb.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define CONTROL_PHYSICAL_MEMORY     TRUE


typedef DWORD (FAR PASCAL * FPGETFREEMEMINFO)(void);
FPGETFREEMEMINFO    lpfnGetFreeMemInfo;
BOOL    CheckLowMemory(DWORD);
DWORD   GetPhysicalMemory(void);

extern  CBfcMgr *lpMetaGameStruct;

// age
ULONG glOldest = 0xFFFFFFFF;
ULONG glAge = 0xFFFFFFFF;

CBgbMgr *gpBgbMgr;

/*****************************************************************************
*
*  CBgbMgr          - Constructor for Boffo Graphics Object Manager Class
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  pBgbMgr = new CBgbMgr;
*
*  RETURNS:  Constructors have no return type
*
*****************************************************************************/
CBgbMgr::CBgbMgr(void) {
	// Initialize all member variables to 0
	memset(&m_cStartData, 0, &m_cEndData - &m_cStartData);

	gpBgbMgr = this;
	m_bAnimationsPaused = TRUE;

	//ErrorLog("DEBUG.LOG", "Free Space: %ld", GetFreeSpace(0));
}


/*****************************************************************************
*
*  ~CBgbMgr         - Destructor for Boffo Graphics Object Manager Class
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  delete pBgbMgr;
*
*  RETURNS:  Destructors have no return type
*
*****************************************************************************/
CBgbMgr::~CBgbMgr(void) {
	ReleaseResources();

	// Release the Palette
	//
	assert(m_xpGamePalette != nullptr);
	if (m_xpGamePalette != nullptr) {
		m_xpGamePalette->DeleteObject();
		delete m_xpGamePalette;
		m_xpGamePalette = nullptr;
	}
}


/*****************************************************************************
*
*  InitBitmapObject  - set up file access in object
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  bSuccess = InitBitmapObject(lpcBgbObject, xpszFileName)
*  CBgbObject *lpcBgbObject;                pointer to bitmap object
*  XPSTR xpszFileName;                      file name to initialize with
*
*  RETURNS:  BOOL = TRUE on error.
*                   FALSE on success.
*
*****************************************************************************/
BOOL CBgbMgr::InitBitmapObject(CBgbObject *pBgbObject, XPSTR pszFileName) {
	CLList *pList;

	// can't access a null pointer
	assert(pBgbObject != nullptr);

	// why would this object already be initialized?
	assert(pBgbObject->m_bInit == FALSE);

	// if this object has not already been initialized
	//
	if (!pBgbObject->m_bInit) {

		// if not yet on chain to be freed
		//
		if (!pBgbObject->m_bChained) {
			pBgbObject->m_xpcNext = m_lpBgbChain;
			pBgbObject->m_xpcPrev = nullptr;

			if (m_lpBgbChain != nullptr)
				m_lpBgbChain->m_xpcPrev = pBgbObject;

			m_lpBgbChain = pBgbObject;
			pBgbObject->m_bChained = TRUE;
		}

		// don't overwrite the filename buffer
		assert(strlen(pszFileName) < MAX_FNAME_LENGTH);

		// init this objects bitmap filename (must be done before cache)
		Common::strcpy_s(pBgbObject->m_szFileName, pszFileName);

		// object must be Initialized before any Caching
		pBgbObject->m_bInit = TRUE;

		// don't yet need a ClearBackground()
		pBgbObject->m_bCleared = TRUE;

		switch (pBgbObject->m_iBgbType) {

		// object is a CSprite
		//
		case BGBT_SPRITE:

			// if this is a special animation
			//
			if (pBgbObject->m_bAnimated && pBgbObject->m_bSpecial) {

				// calculate how many paints are required for this
				// animation sequence.
				//
				assert(pBgbObject->m_nCels > 0);
				pBgbObject->m_nRepeat *= (UINT)pBgbObject->m_nCels;
				pBgbObject->m_nFreqCount = pBgbObject->m_nFreq - 1;

				if ((pList = new CLList((CObject *)pBgbObject)) != nullptr) {

					// add to special effects list
					if (m_pFXList == nullptr) {

						m_pFXList = pList;
						m_bAnimationsPaused = FALSE;

					} else {
						m_pFXList->Insert(pList);
					}
				}
				assert(pList != nullptr);

			} else {
				pBgbObject->m_bLocked = TRUE;
			}

			break;

		// object is a CBitmap
		//
		case BGBT_DIB:

			//
			// Nothing else to initialize if this object is a CBitmap
			//
			break;

		// Invalid ID
		//
		default:
			assert(0);
			break;
		}

		// Let 1st object init the game palette
		//
		if (m_xpGamePalette == nullptr) {
			CacheLoadObject(pBgbObject);
		}
	}

	return (FALSE);
}


/*****************************************************************************
*
*  CacheLoadObject  - Loads specified object into memory if not already loaded
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:    CacheLoadObject(pBgbObject);
*  CBgbObject *pBgbObject;              pointer to Bitmap Object to be loaded
*
*
*****************************************************************************/
VOID CBgbMgr::CacheLoadObject(CBgbObject *pBgbObject) {
	CSprite *pSprite;
	BOOL    bSuccess;

	// can't access a null pointer
	assert(pBgbObject != nullptr);

	// object must have already been Initialized
	assert(pBgbObject->m_bInit);

	// assign age to this object every time it's accessed
	pBgbObject->m_lAge = glAge--;

	// if it's not already loaded
	//
	if (pBgbObject->m_bLoaded) {

		if (pBgbObject->m_bLocked)
			CacheOptimize(0);
		else {
			pBgbObject->m_bLocked = TRUE;
			CacheOptimize(0);
			pBgbObject->m_bLocked = FALSE;
		}
	} else {

		// maintain default free
		CacheOptimize(0);

		//ErrorLog("DEBUG.LOG", "Loading %s with %ld bytes free", pBgbObject->m_szFileName, GetFreeSpace(0));

		// make sure this file-bitmap exists
		assert(FileExists(pBgbObject->m_szFileName));

		// object must start empty
		assert(pBgbObject->m_pObject == nullptr);

		// get a DC if we don't already have one
		//
		if (m_xpDc == nullptr) {
			ReInitDc();
		}
		assert(m_xpDc != nullptr);

		// 1st bitmap loaded installs m_pGamePalette
		//
		if (m_xpGamePalette == nullptr) {
			CBitmap *pBmp;

			pBmp = FetchBitmap(m_xpDc, &m_xpGamePalette, pBgbObject->m_szFileName);
			delete pBmp;
		}

		// which object type
		//
		switch (pBgbObject->m_iBgbType) {

		// Object is a Sprite
		//
		case BGBT_SPRITE:

			// palette must be valid
			assert(m_xpGamePalette != nullptr);

			// alloc sprite and test
			if ((pSprite = new CSprite) != nullptr) {

				// set its palette to be the same as the game's
				//
				pSprite->SharePalette(m_xpGamePalette);

				if (pBgbObject->m_bAnimated) {
					bSuccess = pSprite->LoadCels(m_xpDc, pBgbObject->m_szFileName, pBgbObject->m_nCels);
					if (!pBgbObject->m_bSpecial)
						pSprite->SetAnimated(FALSE);

				} else {
					bSuccess = pSprite->LoadSprite(m_xpDc, pBgbObject->m_szFileName);
				}

				assert(bSuccess);
				if (!bSuccess) {
					delete pSprite;
					pBgbObject->m_pObject = nullptr;
					break;
				}

				// it is a masked image (white/transparent background)
				// keep its bitmap inside a device context
				pSprite->SetMasked(pBgbObject->m_bMasked);

				pSprite->SetZOrder(pBgbObject->m_nZOrder);

				// it will move around the screen
				pSprite->SetMobile(TRUE);

				// link into sprite chain
				pSprite->LinkSprite();
			}

			pBgbObject->m_pObject = (CObject *)pSprite;

			break;


		// Object is a bitmap
		//
		case BGBT_DIB:

			pBgbObject->m_pObject = (CObject *)FetchBitmap(m_xpDc, (m_xpGamePalette == nullptr ? &m_xpGamePalette : nullptr), pBgbObject->m_szFileName);
			assert(pBgbObject->m_pObject != nullptr);
			break;


		// Invalid ID
		//
		default:
			assert(0);
			break;
		}

		// make sure we were able to load this object
		assert(pBgbObject->m_pObject != nullptr);

		if (pBgbObject->m_pObject == nullptr)
			pBgbObject->m_bLoaded = FALSE;
		else
			pBgbObject->m_bLoaded = TRUE;
	}
}

/*****************************************************************************
*
*  CacheReleaseObject - Releases specified object from memory
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:    CacheReleaseObject(pBgbObject);
*
*  CBgbObject *pBgbObject;              pointer to Bitmap Object to be released
*
*
*****************************************************************************/
VOID CBgbMgr::CacheReleaseObject(CBgbObject *pBgbObject) {
	// can't access a null pointer
	assert(pBgbObject != nullptr);

	// object must have already been Initialized
	assert(pBgbObject->m_bInit);

	if (pBgbObject->m_bLoaded && !pBgbObject->m_bLocked) {

		// make sure the object is valid
		assert(pBgbObject->m_pObject != nullptr);

		//ErrorLog("DEBUG.LOG", "Releasing %s", pBgbObject->m_szFileName);

		switch (pBgbObject->m_iBgbType) {

		// Object is a sprite
		//
		case BGBT_SPRITE:

			if (pBgbObject->m_pObject != nullptr) {

				// remove from sprite chain
				((CSprite *)pBgbObject->m_pObject)->UnlinkSprite();

				// free it
				delete (CSprite *)pBgbObject->m_pObject;

				// no more pointer
				pBgbObject->m_pObject = nullptr;
			}

			break;

		// Object is a bitmap
		//
		case BGBT_DIB:

			if (pBgbObject->m_pObject != nullptr) {
				delete (CBitmap *)pBgbObject->m_pObject;
				pBgbObject->m_pObject = nullptr;
			}

			break;

		// Invalid ID
		//
		default:
			assert(0);
			break;
		}

		// this object is now loaded
		pBgbObject->m_bLoaded = FALSE;
	}
}


/*****************************************************************************
*
*  CacheOptimize    - Optimizes the cache so as to keep mostly used items only
*
*  DESCRIPTION:     Attempts to keep a specified ammount of memory free
*                   by releasing some of the objects in the cache.
*
*  SAMPLE USAGE:
*      CacheOptimize(lReqSpace);
*  ULONG lReqSpace;                     Ammount of requested free space
*
*****************************************************************************/
VOID CBgbMgr::CacheOptimize(ULONG lReqSpace) {
	CBgbObject  *pBgbObject;
	ULONG       lAvgAge;
	INT         nObjects;
	ULONG       lSpace;

	// There are no objects to flush
	//
	assert(m_lpBgbChain != nullptr);

	//ErrorLog("DEBUG.LOG", "CacheOptimize");

	if (lReqSpace < lpMetaGameStruct->m_dwFreeSpaceMargin)
		lSpace = lpMetaGameStruct->m_dwFreeSpaceMargin;
	else
		lSpace = lReqSpace;

	while (CheckLowMemory(lSpace)) {

		GlobalCompact(lSpace);
		LocalCompact(4000);

		nObjects = 0;

		lAvgAge = ((glOldest - glAge) / 2) + glAge;
		glOldest = lAvgAge;

		pBgbObject = m_lpBgbChain;
		while (pBgbObject != nullptr) {

			//
			// remove objects that are too old
			//
			if (pBgbObject->m_bLoaded && !pBgbObject->m_bLocked) {

				nObjects++;
				if (pBgbObject->m_lAge >= lAvgAge) {
					CacheReleaseObject(pBgbObject);
				}
			}

			pBgbObject = pBgbObject->m_xpcNext;
		}

		// if there are no objects loaded then we really are out of memory
		//
		if (nObjects == 0)
			break;
	}
	//ErrorLog("DEBUG.LOG", "CacheOptimize: exit");
}


/*****************************************************************************
*
*  CacheFlush       - Flushes all objects in cache store
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:    CacheFlush();
*
*****************************************************************************/
VOID CBgbMgr::CacheFlush(VOID) {
	CBgbObject *pBgbObject;

	//ErrorLog("DEBUG.LOG", "CBgbMgr::CacheFlush");

	// walk thru object chain
	//
	pBgbObject = m_lpBgbChain;
	while (pBgbObject != nullptr) {

		// flush each object from the cache
		//
		CacheReleaseObject(pBgbObject);
		pBgbObject = pBgbObject->m_xpcNext;
	}
}


/*****************************************************************************
*
*  SetPosition      - set position of bitmap (or sprite?)
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  success = SetPosition(lpcBgbObject, crPosition);
*  CBgbObject *lpcBgbObject;                bitmap object
*  CRPoint crPosition;                      new position
*
*  RETURNS:  BOOL = TRUE on error, FALSE on success
*
*****************************************************************************/
BOOL CBgbMgr::SetPosition(CBgbObject *pBgbObject, CRPoint crPosition) {
	// can't access a null pointer
	assert(pBgbObject != nullptr);

	pBgbObject->m_crPosition = crPosition;

	return (FALSE);
}


/*****************************************************************************
*
*  PaintBitmapObject - paint bitmap
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  errCode = PaintBitmapObject(pBgbObject, bPaint = FALSE, *pClipRect = nullptr);
*  CBgbObject *pBgbObject;                bitmap objet to paint
*  BOOL bPaint;                             TRUE if from WM_PAINT
*  CRect *pClipRect;                        clipping rectangle for bitmap
*
*  RETURNS:  BOOL = TRUE on error.
*                   FALSE on success.
*
*****************************************************************************/
BOOL CBgbMgr::PaintBitmapObject(CBgbObject *pBgbObject, BOOL bPaint, CRect *pClipRect) {
	CRect cBmpRect;         // bitmap rectangle
	CRRect crDestRect;      // relocatable destination rectangle
	BOOL bVisible, bEdge;
	BOOL bOkToPaint;

	// can't access a null pointer
	assert(pBgbObject != nullptr);

	// load this BMP if not already loaded
	if (pBgbObject->m_bInit) {

		if (m_xpDc == nullptr) {
			ReInitDc();
		}

		m_xpBsuSet->PrepareDc(m_xpDc, pBgbObject->IfRelocatable());

		cBmpRect = CRect(CPoint(0, 0), pBgbObject->m_cSize);
		crDestRect = pBgbObject->GetRect();

		m_xpBsuSet->TestRect(crDestRect, bVisible, bEdge);
		pBgbObject->m_bVisible = bVisible;

		if (pBgbObject->m_iBgbType == BGBT_SPRITE) {

			// if sprite is not visible or we are forcing a repaint
			//
			if ((!pBgbObject->m_bVisible && !pBgbObject->m_bCleared) || bPaint) {

				// make sure this object is in memory
				CacheLoadObject(pBgbObject);

				if (!pBgbObject->m_bLoaded)
					return (TRUE);

				// then clear it's background
				((CSprite *)pBgbObject->m_pObject)->ClearBackground();

				// don't need another ClearBackground()
				pBgbObject->m_bCleared = TRUE;
			}

			// if sprite is visible on screen
			//
			if (pBgbObject->m_bVisible) {

				bOkToPaint = TRUE;
				if (pBgbObject->m_bAnimated && pBgbObject->m_bSpecial) {

					// if it is not time to paint this animation
					//
					pBgbObject->m_nFreqCount++;
					if (pBgbObject->m_nFreqCount < pBgbObject->m_nFreq) {

						bOkToPaint = FALSE;

					} else if (pBgbObject->m_nFreqCount >= pBgbObject->m_nFreq + pBgbObject->m_nRepeat) {

						// stop painting till next time
						//
						bOkToPaint = FALSE;
						pBgbObject->m_nFreqCount = 0;
					}
				}

				if (bOkToPaint) {

					// make sure this object is in memory
					CacheLoadObject(pBgbObject);

					if (!pBgbObject->m_bLoaded)
						return (TRUE);

					// paint it
					((CSprite *)pBgbObject->m_pObject)->PaintSprite(m_xpDc, pBgbObject->m_crPosition.x, pBgbObject->m_crPosition.y);

					// might need a ClearBackground()
					pBgbObject->m_bCleared = FALSE;
				}
			}

		} else if (pBgbObject->m_bVisible) {

			// make sure this object is in memory
			CacheLoadObject(pBgbObject);

			if (!pBgbObject->m_bLoaded)
				return (TRUE);

			if (pBgbObject->m_bMasked) {

				CRect rect(pBgbObject->m_crPosition.x, pBgbObject->m_crPosition.y, pBgbObject->m_crPosition.x + pBgbObject->m_cSize.cx - 1, pBgbObject->m_crPosition.y + pBgbObject->m_cSize.cy - 1);

				BltMaskedBitmap(m_xpDc, m_xpGamePalette, (CBitmap *)pBgbObject->m_pObject, &rect, pBgbObject->m_crPosition.x, pBgbObject->m_crPosition.y);

				// there's a DIB
				//
			} else {

				if (1 && pClipRect != nullptr) {
					// transfer the bitmap to the screen
					int x, y;

					crDestRect = *pClipRect;
					m_xpDc->DPtoLP(&crDestRect);
					cBmpRect = pBgbObject->GetRect();
					x = cBmpRect.left;
					y = cBmpRect.top;
					cBmpRect.left = crDestRect.left - x;
					cBmpRect.top = crDestRect.top - y;
					cBmpRect.right = crDestRect.right - x;
					cBmpRect.bottom = crDestRect.bottom - y;
				}

				BltBitmap(m_xpDc, m_xpGamePalette, (CBitmap *)pBgbObject->m_pObject, &cBmpRect, &crDestRect, SRCCOPY);
			}
		}
	}

	return (FALSE);
}


ULONG Sqrt(ULONG x) {
	ULONG num, lHigh, lLow, lSqr;

	lHigh = x;
	lLow = 0;
	while (TRUE) {
		num = (lHigh + lLow) / 2;
		lSqr = num * num;

		// if n squared is smaller then n, then we have overflowed
		assert(lSqr >= num);

		if (lSqr > x) {

			lHigh = num;

		} else if (lSqr < x) {

			lLow = num;

		} else {
			break;
		}
		if (abs((LONG)(lHigh - lLow)) <= 2)
			break;
	}

	return (num);
}


/*****************************************************************************
*
*  AnimateSprite    - moves sprite along straight line
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  bSuccess = AnimateSprite(xpcBgbSprite, crOldPosition, cNewPosition);
*  CBgbObject *xpcBgbSprite;                object to animate
*  CPoint      cOldPosition;                object's old location
*  CPoint      cNewPosition;                object's new location
*
*  RETURNS:  BOOL = TRUE on failure.
*                   FALSE on success.
*
*****************************************************************************/
BOOL CBgbMgr::AnimateSprite(CBgbObject *pBgbSprite, CPoint cOldPosition, CPoint cNewPosition) {
	CPoint cCurrentPosition = cOldPosition;
	int iK ;                // loop variable
	int iSegments ;         // # of segments
	int x, y;

	// make sure this object is available in memory
	CacheLoadObject(pBgbSprite);

	if (!pBgbSprite->m_bLoaded)
		return (TRUE);

	// turn on animation for this sprite
	((CSprite *)pBgbSprite->m_pObject)->SetAnimated(TRUE);

	x = cNewPosition.x - cOldPosition.x;
	y = cNewPosition.y - cOldPosition.y;

	iSegments = (int)(Sqrt((ULONG)x * x + (ULONG)y * y) / 10 + 1);

	for (iK = 1; iK <= iSegments; ++iK) {

		CSound::handleMessages();

		if (m_xpDc == nullptr) {
			ReInitDc();
		}

		m_xpBsuSet->PrepareDc(m_xpDc, pBgbSprite->IfRelocatable());

		assert(m_xpBsuSet != nullptr);

		// set new location
		pBgbSprite->m_crPosition = CRPoint(cOldPosition.x + (x * iK / iSegments), cOldPosition.y + (y * iK / iSegments), TRUE);

		if (!lpMetaGameStruct->m_bScrolling) {
			((CSprite *)pBgbSprite->m_pObject)->PaintSprite(m_xpDc, pBgbSprite->m_crPosition.x, pBgbSprite->m_crPosition.y);
		}

		// save new current position (logical coordinates)
		cCurrentPosition = pBgbSprite->m_crPosition;

		cCurrentPosition.Offset(((CSprite *)pBgbSprite->m_pObject)->GetHotspot());

		m_xpBsuSet->EdgeToCenter(pBgbSprite->m_crPosition, lpMetaGameStruct->m_bScrolling);

		// delay 30 ms
		if (!lpMetaGameStruct->m_bScrolling)
			Sleep(30);
	}

	// turn off animation for this sprite
	((CSprite *)pBgbSprite->m_pObject)->SetAnimated(FALSE);

	return (FALSE);
}


//* CBgbMgr::InitDc -- initialize device context
BOOL CBgbMgr::InitDc(CView *xpView, CBsuSet *xpBsuSet, CDC *xpDc)
// xpView -- pointer to window
// xpBsuSet -- pointer to scroll bar set, if any
// xpDc -- pointer to device context, if already allocated
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBgbMgr::InitDc);
	int iError = 0 ;        // error code

	if (m_iLockCount <= 0) {
		ReleaseDc();   // release Dc, if one already allocated

		m_xpcView = xpView;
		m_xpBsuSet = xpBsuSet;
		m_bReleaseDc = !xpDc;

		if (((m_xpDc = xpDc) == nullptr) && ((m_xpDc = m_xpcView->GetDC()) == nullptr)) {
			iError = 101;   // device context allocation failure
			goto cleanup;
		}

		CBdbgMgr::GetPointer()->TraceConstructor("CDC", m_xpDc);

		if (m_xpGamePalette != nullptr) {

			CBdbgMgr::GetPointer()->TestTraceObject("Palette", m_xpGamePalette);
			m_xpOldPalette = m_xpDc->SelectPalette(m_xpGamePalette, FALSE);
			m_xpDc->RealizePalette();
		}
	}

cleanup:

	JXELEAVE(CBgbMgr::InitDc) ;
	RETURN(iError != 0) ;
}

//* CBgbMgr::ReInitDc -- do InitDc with existing view and BsuSet
BOOL CBgbMgr::ReInitDc(void)
// returns: TRUE if error, FALSE otherwise
{
	InitDc(m_xpcView, m_xpBsuSet);

	return (FALSE);
}

//* CBgbMgr::AdjustLockCount --
BOOL CBgbMgr::AdjustLockCount(int iIncr)
// iIncr -- if <0 or >0, add to lock count; if =0, zero lock count
//// int PASCAL CBgbMgr::AdjustLockCount(void)
// returns: TRUE if error, FALSE otherwise
{
	if (iIncr != 0)
		m_iLockCount += iIncr;
	else
		m_iLockCount = 0;

	return (FALSE);
}

//* CBgbMgr::SetBrush -- set brush attributes in current device context
BOOL CBgbMgr::SetBrush(COLORREF cBrushColor, int iBrushStyle)
// cBrushColor -- brush color
// iBrushStyle -- BS_SOLID or BS_NULL (=BS_HOLLOW)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBgbMgr::SetBrush) ;
	int iError = 0 ;        // error code

	if (!m_bCreateBrush || (cBrushColor != m_cBrushColor) || (iBrushStyle != m_iBrushStyle)) {

		if (m_bCreateBrush) {
			m_xpDc->SelectObject(m_xpOldBrush);
			m_xpOldBrush = nullptr;
			m_cBrush.DeleteObject();
			m_bCreateBrush = FALSE;
		}

		m_cBrushColor = cBrushColor;
		m_iBrushStyle = iBrushStyle;

		if (m_iBrushStyle == BS_NULL) {
			m_cBrush.CreateStockObject(NULL_BRUSH);
			m_bCreateBrush = TRUE;

			// BS_SOLID or illegal value
			//
		} else {
			m_cBrush.CreateSolidBrush(m_cBrushColor);
			m_bCreateBrush = TRUE;
		}

		m_xpOldBrush = m_xpDc->SelectObject(&m_cBrush);
	}

// cleanup:

	JXELEAVE(CBgbMgr::SetBrush) ;
	RETURN(iError != 0) ;
}

//* CBgbMgr::SetPen -- set pen attributes in current device context
BOOL CBgbMgr::SetPen(COLORREF cPenColor, int iPenWidth)
// cPenColor -- pen color
// iPenWidth -- pen width
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBgbMgr::SetPen);
	int iError = 0 ;        // error code

	if (!m_bCreatePen || cPenColor != m_cPenColor || iPenWidth != m_iPenWidth) {

		if (m_bCreatePen) {
			m_xpDc->SelectObject(m_xpOldPen);
			m_xpOldPen = nullptr;
			m_cPen.DeleteObject();
			m_bCreatePen = FALSE;
		}

		m_cPenColor = cPenColor;
		m_iPenWidth = iPenWidth;

		m_cPen.CreatePen(PS_SOLID, m_iPenWidth, m_cPenColor);
		m_bCreatePen = TRUE;

		m_xpOldPen = m_xpDc->SelectObject(&m_cPen);
	}

// cleanup:

	JXELEAVE(CBgbMgr::SetPen);
	RETURN(iError != 0);
}

//* CBgbMgr::ReleaseDc -- release Dc, if one has been gotten
BOOL CBgbMgr::ReleaseDc(void)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBgbMgr::ReleaseDc) ;
	int iError = 0 ;        // error code

	if (m_iLockCount <= 0 && m_xpDc) {

		if (m_xpOldPalette) {
			m_xpDc->SelectPalette(m_xpOldPalette, FALSE) ;
			m_xpOldPalette = nullptr ;
		}

		if (m_bCreatePen) {
			m_xpDc->SelectObject(m_xpOldPen);
			m_xpOldPen = nullptr;
			m_cPen.DeleteObject();
			m_bCreatePen = FALSE;
		}

		if (m_bCreateBrush) {
			m_xpDc->SelectObject(m_xpOldBrush);
			m_xpOldBrush = nullptr;
			m_cBrush.DeleteObject();
			m_bCreateBrush = FALSE;
		}

		CBdbgMgr::GetPointer()->TraceDestructor("CDC", m_xpDc);

		// release the window's context
		if (m_xpcView && m_bReleaseDc)
			m_xpcView->ReleaseDC(m_xpDc);

		m_xpDc = nullptr;
	}

// cleanup:

	JXELEAVE(CBgbMgr::ReleaseDc);
	RETURN(iError != 0);
}

/*****************************************************************************
*
*  ClearBitmapObject - Destroys a CBgbObject
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  success = ClearBitmapObject(pBgbObject);
*
*  CBgbObject *pBgbObject;                Bitmap Object to be deleted
*
*  RETURNS:  BOOL = TRUE if this function failed.
*                   FALSE on success.
*
*****************************************************************************/
BOOL CBgbMgr::ClearBitmapObject(CBgbObject *pBgbObject) {
	pBgbObject->m_bLocked = FALSE;
	CacheReleaseObject(pBgbObject);

	// object no longer initialized
	pBgbObject->m_bInit = FALSE;

	return (FALSE);
}

//* CBgbMgr::ReleaseResources -- release all resources at termination
BOOL CBgbMgr::ReleaseResources(void)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CBgbMgr::ReleaseResources) ;
	int iError = 0 ;                // error code
	CBgbObject *pBgbObject ;      // objects to be freed

	// stop all animations
	m_bAnimationsPaused = TRUE;

	// flush the special effects list
	m_pFXList->FlushList();
	delete m_pFXList;
	m_pFXList = nullptr;

	while ((pBgbObject = m_lpBgbChain) != nullptr) {

		m_lpBgbChain = pBgbObject->m_xpcNext;
		pBgbObject->m_xpcNext = nullptr;     // out of habit
		ClearBitmapObject(pBgbObject);    // free resources

		if (!pBgbObject->m_bNoDelete)     // if on the heap
			delete pBgbObject;
	}

// cleanup:

	JXELEAVE(CBgbMgr::ReleaseResources);
	RETURN(iError != 0) ;
}


/*****************************************************************************
*
*  DoAnimations     - Repaints all animation objects
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  DoAnimations();
*
*****************************************************************************/
void CBgbMgr::DoAnimations(void) {
	CBgbObject *pBgbObject;
	CLList *pList;

	#ifdef NODEEDIT
	return;
	#endif

	// if animations are ON and not paused
	//
	if (lpMetaGameStruct->m_bAnimations && !m_bAnimationsPaused) {

		//
		// Handle special effects (Animation)
		//
		pList = m_pFXList;
		while (pList != nullptr) {

			pBgbObject = (CBgbObject *)pList->getData();

			assert(pBgbObject != nullptr);

			// this must be a sprite
			assert(pBgbObject->m_iBgbType == BGBT_SPRITE);

			//
			// Paint this object only if it is visible on the screen
			//
			PaintBitmapObject(pBgbObject);

			pList = pList->getNext();
		}
	}
}

BOOL CheckLowMemory(DWORD dwSpace) {
	BOOL    bMemoryProblem;
	DWORD   dwFreeSpace;
	bMemoryProblem = FALSE;

	dwFreeSpace = GetFreeSpace(0);
	if (dwFreeSpace < dwSpace)
		bMemoryProblem = TRUE;

	return bMemoryProblem;
}

DWORD GetPhysicalMemory() {
	return GetFreeSpace(0);
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
