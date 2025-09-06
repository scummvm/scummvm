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

#include "bagel/hodjnpodj/hnplibs/stdafx.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"

namespace Bagel {
namespace HodjNPodj {

#define SPRITE_DEBUG    0                        // set for debugging sprite reconstruction


int         CSprite::m_nIndex = 0;                  // used for generating unique Ids
CSprite     *CSprite::m_pSpriteChain = nullptr;        // pointer to chain of linked sprites
bool        CSprite::m_bTouchedSprite = false;      // set if sprite overlapped during painting
CSprite     *CSprite::m_pTouchedSprite = nullptr;      // pointer to sprite overlapped during painting
bool        CSprite::m_bHaveBackdrop = false;       // whether we have a bitmap for the background
CDC         *CSprite::m_pBackdropDC = nullptr;         // backdrop context for background painting
CBitmap     *CSprite::m_pBackdrop = nullptr;           // backdrop for background painting
CBitmap     *CSprite::m_pBackdropOld = nullptr;        // previous bitmap mapped to backdrop context
CPalette    *CSprite::m_pBackdropPalette = nullptr;    // palette mapped to backdrop context
CPalette    *CSprite::m_pBackdropPalOld = nullptr;     // previous palette mapped to backdrop context

IMPLEMENT_DYNCREATE(CSprite, CObject)


/*************************************************************************
 *
 * CSprite()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Constructor for sprite class.  Initialize all fields
 *                  to logical nullptr.  Calls should then be made to other
 *                  sprite routines to load palettes and bitmaps.
 *
 ************************************************************************/

CSprite::CSprite() {
	m_pImageDC = nullptr;                                  // no device context allocated for sprite image
	m_pImage = nullptr;                                    // no initial bitmap image for the sprite
	m_pImageOld = nullptr;                                 // ... hence no unmapped bitmap for the context
	m_pMaskDC = nullptr;                                   // no device context allocated for sprite mask
	m_pMask = nullptr;                                     // no initial image mask
	m_pMaskOld = nullptr;                                  // ... hence no unmapped bitmap for the context
	m_pBackgroundDC = nullptr;                             // no device context allocated for background
	m_pBackground = nullptr;                               // no initial background mask
	m_pBackgroundOld = nullptr;                            // ... hence no unmapped bitmap for the context
	m_pPalette = nullptr;                                  // no initial palette assigned to this sprite
	m_pPalImageOld = nullptr;                              // ... hence no unmapped palette for the image
	m_pPalBackOld = nullptr;                               // ... and no unmapped palette for the background
	m_bSharedPalette = false;                           // ... and thus it is not currently shared
	m_cSize = CSize(0, 0);                              // there is no size to the sprite image
	m_cRect = CRect(0, 0, 0, 0);                        // rectangular bounds not yet defined
	m_cImageRect = m_cRect;                             // image rectangle starts same as display bounds
	m_cPosition = CPoint(0, 0);                         // default position to upper left corner of display
	m_cMovementDelta = CPoint(0, 0);                    // no initial movement rate
	m_bPositioned = false;                              // not yet positioned
	m_cHotspot = CPoint(0, 0);                          // default the hot spot to upper left corner
	m_bVisible = false;                                 // sprite has no initial image
	m_bMasked = false;                                  // simple unmasked bitmap
	m_bMobile = false;                                  // it is a stationary sprite
	m_bOverlaps = false;                                // does not yet overlap other sprites
	m_bIntercepts = true;                               // can be detected by interception
	m_bRetainBackground = true;                         // retain background image for updates
	m_bRetainContexts = false;                          // do not retain device contexts
	m_bDuplicated = false;                              // not sharing resources with other sprites
	m_nId = ++m_nIndex;                                 // set its unique identifirer
	m_nType = 0;                                        // no user defined information
	m_pData = nullptr;                                     // no user define pointer information
	m_nZOrder = SPRITE_TOPMOST;                         // default to top most in fore/back ground order
	m_nZPosition = SPRITE_TOPMOST;                      // default to top most in fore/back ground order
	m_nCelID = -1;                                      // cel identifier not pointing at a cel
	m_nCelCount = 0;                                    // no cels to be played
	m_bAnimated = false;                                // not initially animated
	m_bLinked = false;                                  // not initially linked into the sprite chain
	m_pNext = nullptr;                                     // it is not yet in the sprite chain and
	m_pPrev = nullptr;                                     // ... thus has no links to other sprites
}


/*************************************************************************
 *
 * ~CSprite()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destructor for sprite class.  It is important to tear
 *                  things down in the order we built them; map out bitmap,
 *                  map out the palette, then discard the device context -
 *                  failure to release key resources will cause application
 *                  or system crashes.
 *
 ************************************************************************/

CSprite::~CSprite() {
	ClearImage();                                       // clear the sprite image bitmap and context

	ClearMask();                                        // clear the sprite mask bitmap

	ClearBackground();                                  // clear the sprite background bitmap and context

	ClearPalette();                                     // clear the palette resource

	if (m_bLinked)                                      // unlink from chain if still hooked in
		UnlinkSprite();
}


/*************************************************************************
 *
 * LinkSprite()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Link this sprite into the chain by placing it at the
 *                  the head of the chain
 *
 ************************************************************************/

void CSprite::LinkSprite() {
	m_bLinked = true;                                   // set for linked into chain

	if ((m_pPrev != nullptr) ||                            // punt if already in chain
	        (m_pNext != nullptr))
		return;

	m_pNext = m_pSpriteChain;                           // link sprite into head of chain
	m_pPrev = nullptr;                                     // ... by pointing it at the current
	if (m_pSpriteChain != nullptr)                         // ... initial sprite, and pointing it
		(*m_pSpriteChain).m_pPrev = this;               // ... back at us
	m_pSpriteChain = this;
}


/*************************************************************************
 *
 * UnLinkSprite()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Remove this sprite from the sprite chain and point its
 *                  neighbors at each other to fill the gap
 *
 ************************************************************************/

void CSprite::UnlinkSprite() {
	m_bLinked = false;                                  // set for not linked into chain

	if ((m_pPrev == nullptr) &&                            // punt if not in chain
	        (m_pNext == nullptr) &&
	        (m_pSpriteChain != this))
		return;

	if (m_pPrev != nullptr)                                // disconnect us from the sprite chain
		(*m_pPrev).m_pNext = m_pNext;                   // ... by pointing the one before us, and
	else                                                // ... the one after us, at each other
		m_pSpriteChain = m_pNext;                       // special case the instance where the
	// ... sprite to be removed is the first
	if (m_pNext != nullptr)                                // ... in the list - in particular, update
		(*m_pNext).m_pPrev = m_pPrev;                   // ... the head of chain pointer

	m_pNext = m_pPrev = nullptr;
}


/*************************************************************************
 *
 * FlushSpriteChain()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Remove all sprites from the chain and delete them via
 *                  the standard destructor
 *
 ************************************************************************/

void CSprite::FlushSpriteChain() {
	CSprite *pSprite = nullptr;

	while ((pSprite = CSprite::GetSpriteChain()) != nullptr) { // cycle getting head of chain
		(*pSprite).UnlinkSprite();                          // ... unlinking it
		delete pSprite;
	}                                    // ... and then deleting it
}


/*************************************************************************
 *
 * SetBackdrop()
 *
 * Parameters:
 *  CDC *pDC            screen device context
 *  CPalette *pPalette  palette to use for painting
 *  CBitmap *pBitmap    pointer to screen-size bitmap for background painting
 *
 * Return Value:
 *  bool                success/failure condition
 *
 * Description:     establish the bitmap used for restoring the background
 *                  as sprite images are moved around the screen.
 *
 ************************************************************************/

bool CSprite::SetBackdrop(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap) {
	if (m_pBackdropDC != nullptr)                              // if the backdrop context is active
		ReleaseBackdropDC();                                // ... then unmap all of its resources

	if (m_pBackdrop != nullptr)                                // delete the old bitmap
		delete m_pBackdrop;

	m_pBackdrop = pBitmap;                                  // save the new bitmap
	m_pBackdropPalette = pPalette;                          // ... and palette
	m_bHaveBackdrop = true;                                 // indicate we have a backdrop
	return true;
}


/*************************************************************************
 *
 * ClearBackdrop()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     clear the bitmap and context used for restoring the
 *                  background as sprite images were moved around the screen.
 *
 ************************************************************************/

void CSprite::ClearBackdrop() {
	ReleaseBackdropDC();                                // unmap any resources

	if (m_pBackdrop != nullptr) {                          // delete the backdrop bitmap
		delete m_pBackdrop;
		m_pBackdrop = nullptr;
	}

	m_pBackdropPalette = nullptr;

	if (m_pBackdropDC != nullptr) {                        //  delete the device context
		delete m_pBackdropDC;
		m_pBackdropDC = nullptr;
	}

	m_bHaveBackdrop = false;
}


/*************************************************************************
 *
 * RefreshBackdrop()
 *
 * Parameters:
 *  CDC *pDC            screen device context
 *  CPalette *pPalette  palette to use for painting
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     repaint the backdrop.
 *
 ************************************************************************/

bool CSprite::RefreshBackdrop(CDC *pDC, CPalette *pPalette) {
	bool        bSuccess = false;
	CPalette    *pPalOld = nullptr;

	if (m_bHaveBackdrop == false)                   // punt if no backdrop art
		return false;

	if (GetBackdropDC(pDC) == nullptr)                 // setup backdrop context
		return false;

	if (pPalette != nullptr) {                         // map in our palette
		pPalOld = (*pDC).SelectPalette(pPalette, false);
		(*pDC).RealizePalette();
	}
	// zap it to the screen
	bSuccess = (*pDC).BitBlt(0, 0, GAME_WIDTH, GAME_HEIGHT, m_pBackdropDC, 0, 0, SRCCOPY);

	if (pPalOld != nullptr)                            // map out the palette
		(*pDC).SelectPalette(pPalOld, false);

	ReleaseBackdropDC();                            // release backdrop context resources

	return bSuccess;
}


/*************************************************************************
 *
 * DuplicateSprite()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context to be used in creating
 *                  the sprite
 *
 * Return Value:
 *  CSprite *       pointer to new sprite (Success) / nullptr (failure)
 *
 * Description:     Create a sprite based on sharing the resources of a
 *                  master sprite (passed implicitly when this is invoked).
 *
 *                  NOTE:  Do NOT delete a sprite that has been duplicated
 *                  until ALL of its duplicates are deleted; using a duplicate
 *                  when its master has been deleted will result in a crash.
 *
 ************************************************************************/

CSprite *CSprite::DuplicateSprite(CDC *pDC) {
	CSprite *pSprite = nullptr;

	pSprite = new CSprite();                                // create an object for the sprite

	if ((pSprite != nullptr) &&                                // try to duplicate it
	        DuplicateSprite(pDC, pSprite))
		return (pSprite);

	return nullptr;                                          // return failure
}


/*************************************************************************
 *
 * DuplicateSprite()
 *
 * Parameters:
 *  CDC *pDC            pointer to device context to be used in creating
 *                      the sprite
 *  CSprite *pSprite    pointer to sprite to be duplicated
 *
 * Return Value:
 *  bool                success/failure condition
 *
 * Description:     Update a sprite based on sharing the resources of a
 *                  master sprite (passed implicitly when this is invoked).
 *
 *                  NOTE:  Do NOT delete a sprite that has been duplicated
 *                  until ALL of its duplicates are deleted; using a duplicate
 *                  when its master has been deleted will result in a crash.
 *
 ************************************************************************/

bool CSprite::DuplicateSprite(CDC *pDC, CSprite *pSprite) {
	CPalette    *pPalOld = nullptr;

	if (m_pPalette) {
		pPalOld = (*pDC).SelectPalette(m_pPalette, false);
		(*pDC).RealizePalette();
	}

	if (pSprite != nullptr) {                                  // only duplicate visible sprites                                   // ... unable to create that object
		if (m_bVisible) {
			if (m_bRetainContexts) {
				if (!SetupImage(pDC))
					return false;
				SetupMask(pDC);
				(*pSprite).m_pImageDC = m_pImageDC;
				(*pSprite).m_pImageOld = m_pImageOld;
				(*pSprite).m_pMaskDC = m_pMaskDC;
				(*pSprite).m_pMaskOld = m_pMaskOld;
			} else {
				ReleaseImageContext();                          // release existing device contexts.
				ReleaseMaskContext();
			}
			(*pSprite).m_pImage = m_pImage;
		}
		(*pSprite).m_pPalette = m_pPalette;
		if (m_pPalette != nullptr) {
			(*pSprite).m_bSharedPalette = true;
			(*pSprite).m_pPalImageOld = m_pPalImageOld;
		}
		(*pSprite).m_cRect = m_cRect;
		(*pSprite).m_cImageRect = m_cImageRect;
		(*pSprite).m_cSize = m_cSize;
		(*pSprite).m_cPosition = m_cPosition;
		(*pSprite).m_cMovementDelta = m_cMovementDelta;
		(*pSprite).m_cHotspot = m_cHotspot;
		(*pSprite).m_nType = m_nType;
		(*pSprite).m_pData = m_pData;
		(*pSprite).m_nZOrder = m_nZOrder;
		(*pSprite).m_nZPosition = m_nZPosition;
		(*pSprite).m_nCelID = m_nCelID;
		(*pSprite).m_nCelCount = m_nCelCount;
		(*pSprite).m_bVisible = m_bVisible;
		(*pSprite).m_bMasked = m_bMasked;
		(*pSprite).m_bMobile = m_bMobile;
		(*pSprite).m_bIntercepts = m_bIntercepts;
		(*pSprite).m_bRetainBackground = m_bRetainBackground;
		(*pSprite).m_bRetainContexts = m_bRetainContexts;
		(*pSprite).m_bAnimated = m_bAnimated;
		(*pSprite).m_bDuplicated = true;                    // mark it as a sprite with shared resources
		if (m_bVisible) {
			if (!m_bMasked || CreateMask(pDC)) {                // create an image mask if needed
				(*pSprite).m_pMask = m_pMask;
				if (m_pPalette)
					(*pDC).SelectPalette(pPalOld, false);
				return true;
			}
		} else
			return true;
	}

	if (m_pPalette)
		(*pDC).SelectPalette(pPalOld, false);

	return false;
}


/*************************************************************************
 *
 * LoadSprite()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used in creating
 *                          a compatible context for the sprite's bitmap
 *
 *  char *pszPathName      pointer to text string with the fully qualified file
 *                          specification for the DIB bitmap file
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DIB file and convert it to a DDB bitmap for use as
 *                  the sprite's image.  A device context for the image is retained
 *                  if speed optimization has been specified.  The DIB palette is
 *                  retained for use by the sprite if it does not yet have an
 *                  established palette.
 *
 ************************************************************************/

bool CSprite::LoadSprite(CDC *pDC, const char *pszPathName) {
	CDibDoc *myDib = nullptr;                          // pointer to our loaded DIB file
	bool bHavePalette = false;                   // whether or not we have a palette already
	CPalette *pPalOld = nullptr;                        // palette previously mapped to base context
	HPALETTE hPalette = nullptr;

	ClearImage();                                       // clear out any/all existing bitmaps, palettes,
	ClearMask();                                        // ... and device contexts

	myDib = new CDibDoc();                              // create an object for our DIB
	m_pImage = new CBitmap();                           // create an object for the sprite's image

	bHavePalette = (m_pPalette ? true : false);         // loading a DIB automatically includes
	// ... loading its color palette, which
	// ... must be deleted if we already
	// ... have one assigned to this sprite
	if ((myDib != nullptr) &&                              // verify we have the objects we just created
	        (m_pImage != nullptr) &&                           // .. and then attempt to open the requested
	        (*myDib).OpenDocument(pszPathName)) {           // .... bitmap file
		m_bVisible = true;
		if (!bHavePalette)                              // if we don't have a palette, then
			m_pPalette = (*myDib).DetachPalette();      // ... detach the DIB's palette for our use
		if (m_pPalette != nullptr) {
			pPalOld = (*pDC).SelectPalette(m_pPalette, false);
			(*pDC).RealizePalette();
			hPalette = (HPALETTE)(*m_pPalette).m_hObject;
		}
		if (CreateImageContext(pDC)) {
			// Create a context for the image
			HDIB hDib = (*myDib).GetHDIB();
			(*m_pImage).m_hObject = DIBtoBitmap(
				(*pDC).m_hDC,	// convert the DIB to a DDB
			    hPalette,
			    hDib);			// ... and store it in the sprite

			if ((*m_pImage).m_hObject != nullptr) {        // verify the conversion was successful
				if (pPalOld != nullptr)
					(*pDC).SelectPalette(pPalOld, false);
				if (!m_bRetainContexts)                  // release the context if not optimizing
					ReleaseImageContext();
				m_cSize = (*myDib).GetDocSize();        // ... size related variables
				m_cRect.right = m_cRect.left + m_cSize.cx;              // ... update rectangular dimensions
				m_cRect.bottom = m_cRect.top + m_cSize.cy;
				m_cImageRect.SetRect(0, 0, m_cSize.cx, m_cSize.cy);
				m_nCelID = -1;
				m_nCelCount = 0;
				delete myDib;                           // discard the DIB
				myDib = nullptr;
				return true;                          // return success
			}
		}
	}

	if (pPalOld != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	ClearImage();

	if (!bHavePalette &&                                // delete the palette resource and object
	        (m_pPalette != nullptr)) {                         // ... if it came from the DIB
		(*m_pPalette).DeleteObject();
		delete m_pPalette;
		m_pPalette = nullptr;
	}

	if (myDib != nullptr)                                  // lastly delete the DIB itself
		delete myDib;

	return false;                                     // return failure
}


/*************************************************************************
 *
 * LoadSprite()
 *
 * Parameters:
 *  CBitmap *pBitmap        pointer to bitmap to be used and owned
 *  CPalette *pPalette      pointer to palette to be shared
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     use a CBitmap object as the sprite's image.  A device context
 *                  for the image is retained if speed optimization has been specified.
 *                  The palette is retained for use by the sprite if it does not yet have an
 *                  established palette.
 *
 ************************************************************************/

bool CSprite::LoadSprite(CBitmap *pBitmap, CPalette *pPalette) {
	CSize   mySize;
	BITMAP  cBitmapData;

	if (pBitmap == nullptr)                                // punt if no bitmap
		return false;

	ClearImage();                                       // clear out any/all existing bitmaps, palettes,
	ClearMask();                                        // ... and device contexts

	m_pImage = pBitmap;                                 // save pointer to bitmap
	if (pPalette != nullptr)                               // ... and the palette, if specified
		SharePalette(pPalette);

	(*pBitmap).GetObject(sizeof(BITMAP), &cBitmapData); // get the size of the bitmap
	m_cSize.cx = cBitmapData.bmWidth;                   // ... and initialize size variables
	m_cSize.cy = cBitmapData.bmHeight;
	m_cRect.right = m_cRect.left + m_cSize.cx;
	m_cRect.bottom = m_cRect.top + m_cSize.cy;
	m_cImageRect.SetRect(0, 0, m_cSize.cx, m_cSize.cy);
	m_nCelID = -1;
	m_nCelCount = 0;
	m_bVisible = true;

	return true;                                     // return success
}


/*************************************************************************
 *
 * LoadResourceSprite()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used in creating
 *                          a compatible context for the sprite's bitmap
 *
 *  int resId               resource number for a bitmap identified in the game's
 *                          .RC resource file
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DDB bitmap resource and store it in the sprite object.
 *                  Resource bitmaps do not have an accessible palette, so you
 *                  may need to set one directly (SetPalette or SharedPalette);
 *                  if none is specified, then the current system palette is used.
 *
 ************************************************************************/

bool CSprite::LoadResourceSprite(CDC *pDC, const int resId) {
	bool    bSuccess = false;
	char    chResID[16];

	Common::sprintf_s(chResID, "#%d", resId);
	bSuccess = LoadResourceSprite(pDC, chResID);

	return bSuccess;                                     // return failure
}


/*************************************************************************
 *
 * LoadResourceSprite()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used in creating
 *                          a compatible context for the sprite's bitmap
 *
 *  char *pszPathName       resource name for a bitmap identified in the game's
 *                          .RC resource file
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DDB bitmap resource and store it in the sprite object.
 *                  Resource bitmaps do not have an accessible palette, so you
 *                  may need to set one directly (SetPalette or SharedPalette);
 *                  if none is specified, then the current system palette is used.
 *
 ************************************************************************/

bool CSprite::LoadResourceSprite(CDC *pDC, const char *pszName) {
	CDibDoc *myDib = nullptr;                          // pointer to our loaded DIB file
	bool bHavePalette = false;                   // whether or not we have a palette already
	CPalette *pPalOld = nullptr;                        // palette previously mapped to base context
	HPALETTE hPalette = nullptr;

	ClearImage();                                       // clear out any/all existing bitmaps, palettes,
	ClearMask();                                        // ... and device contexts

	myDib = new CDibDoc();                              // create an object for our DIB
	m_pImage = new CBitmap();                           // create an object for the sprite's image

	bHavePalette = (m_pPalette ? true : false);         // loading a DIB automatically includes
	// ... loading its color palette, which
	// ... must be deleted if we already
	// ... have one assigned to this sprite
	if ((myDib != nullptr) &&                              // verify we have the objects we just created
	        (m_pImage != nullptr) &&                           // .. and then attempt to open the requested
	        (*myDib).OpenResourceDocument(pszName)) {       // .... bitmap file
		m_bVisible = true;
		if (!bHavePalette)                              // if we don't have a palette, then
			m_pPalette = (*myDib).DetachPalette();      // ... detach the DIB's palette for our use
		if (m_pPalette != nullptr) {
			pPalOld = (*pDC).SelectPalette(m_pPalette, false);
			(*pDC).RealizePalette();
			hPalette = (HPALETTE)(*m_pPalette).m_hObject;
		}
		if (CreateImageContext(pDC)) {
			// Create a context for the image
			HDIB hDib = (*myDib).GetHDIB();
			(*m_pImage).m_hObject = DIBtoBitmap((*pDC).m_hDC,   // convert the DIB to a DDB
			    hPalette,
			    hDib);   // ... and store it in the sprite
			if ((*m_pImage).m_hObject != nullptr) {        // verify the conversion was sucessfull
				if (pPalOld != nullptr)
					(*pDC).SelectPalette(pPalOld, false);
				if (!m_bRetainContexts)                  // release the context if not optimizing
					ReleaseImageContext();
				m_cSize = (*myDib).GetDocSize();        // ... size related variables
				m_cRect.right = m_cRect.left + m_cSize.cx;              // ... update rectangular dimensions
				m_cRect.bottom = m_cRect.top + m_cSize.cy;
				m_cImageRect.SetRect(0, 0, m_cSize.cx, m_cSize.cy);
				m_nCelID = -1;
				m_nCelCount = 0;
				delete myDib;                           // discard the DIB
				myDib = nullptr;
				return true;                          // return success
			}
		}
	}

	if (pPalOld != nullptr)
		(*pDC).SelectPalette(pPalOld, false);

	ClearImage();

	if (!bHavePalette &&                                // delete the palette resource and object
	        (m_pPalette != nullptr)) {                         // ... if it came from the DIB
		(*m_pPalette).DeleteObject();
		delete m_pPalette;
		m_pPalette = nullptr;
	}

	if (myDib != nullptr)                                  // lastly delete the DIB itself
		delete myDib;

	return false;                                     // return failure
}


/*************************************************************************
 *
 * LoadCels()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used in creating
 *                          a compatible context for the sprite's cel bitmap
 *  char *pszPathName      pointer to text string with the fully qualified file
 *                          specification for the DIB bitmap file
 *  int nCels               number of cels in strip
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DIB file and convert it to a DDB bitmap for use as
 *                  the sprite's cels.
 *
 ************************************************************************/

bool CSprite::LoadCels(CDC *pDC, const char *pszPathName, const int nCels) {
	int nOldCelCount;

	nOldCelCount = m_nCelCount;                                 // retain previous cel count

	if (LoadSprite(pDC, pszPathName) &&                         // load the cel strip as if a normal
	        SetupCels(nCels))                                       // ... sprite image, then set specifics
		if (nOldCelCount == 0)                                  // if there wasn't a previous cel strip
			m_bAnimated = true;                                 // ... then set for initially animated
	return true;

	return false;
}


/*************************************************************************
 *
 * LoadCels()
 *
 * Parameters:
 *  CBitmap *pBitmap        pointer to bitmap to be used and owned
 *  int nCels               number of cels in strip
 *  CPalette *pPalette      pointer to palette to be shared
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DIB file and convert it to a DDB bitmap for use as
 *                  the sprite's cels.
 *
 ************************************************************************/

bool CSprite::LoadCels(CBitmap *pBitmap, const int nCels, CPalette *pPalette) {
	int nOldCelCount;

	nOldCelCount = m_nCelCount;                                 // retain previous cel count

	if (LoadSprite(pBitmap, pPalette) &&                        // load the cel strip as if a normal
	        SetupCels(nCels))                                       // ... sprite image, then set specifics
		if (nOldCelCount == 0)                                  // if there wasn't a previous cel strip
			m_bAnimated = true;                                 // ... then set for initially animated
	return true;

	return false;
}


/*************************************************************************
 *
 * LoadResourceCels()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used in creating
 *                          a compatible context for the sprite's cel bitmap
 *  int resId               resource number for a bitmap identified in the game's
 *  int nCels               number of cels in strip
 *                          .RC resource file
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DIB file and convert it to a DDB bitmap for use as
 *                  the sprite's cels.
 *
 ************************************************************************/

bool CSprite::LoadResourceCels(CDC *pDC, const int resId, const int nCels) {
	bool    bSuccess = false;
	char    chResID[16];

	Common::sprintf_s(chResID, "#%d", resId);
	bSuccess = LoadResourceCels(pDC, chResID, nCels);

	return bSuccess;                                     // return failure
}


/*************************************************************************
 *
 * LoadResourceCels()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context to be used in creating
 *                          a compatible context for the sprite's cel bitmap
 *  char *pszPathName       resource name for a bitmap identified in the game's
 *                          .RC resource file
 *  int nCels               number of cels in strip
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Read in a DIB file and convert it to a DDB bitmap for use as
 *                  the sprite's cels.
 *
 ************************************************************************/

bool CSprite::LoadResourceCels(CDC *pDC, const char *pszName, const int nCels) {
	int nOldCelCount;

	nOldCelCount = m_nCelCount;                                 // retain previous cel count

	if (LoadResourceSprite(pDC, pszName) &&                     // load the cel strip as if a normal
	        SetupCels(nCels))                                       // ... sprite image, then set specifics
		if (nOldCelCount == 0)                                  // if there wasn't a previous cel strip
			m_bAnimated = true;                                 // ... then set for initially animated
	return true;

	return false;
}


/*************************************************************************
 *
 * SetupCels()
 *
 * Parameters:      none
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     initialize cel animation strip variables and validate
 *                  strip length
 *
 ************************************************************************/

bool CSprite::SetupCels(const int nCels) {
	int     nStripWidth;                                        // temp place to hold cel strip width

	m_nCelID = -1;                                              // no current cel
	m_nCelCount = nCels;                                        // set cel count
	nStripWidth = m_cSize.cx;                                   // retain cell strip pixel length
	m_cSize.cx /= nCels;                                        // calculate width of a cel

	if (m_cSize.cx * nCels == nStripWidth) {                    // verify we have an even multiple
		m_cRect.right = m_cRect.left + m_cSize.cx;              // reset sprite rectangular bounds
		m_cRect.bottom = m_cRect.top + m_cSize.cy;              // ... based on cel dimensions
		m_cImageRect.SetRect(0, 0, m_cSize.cx, m_cSize.cy);     // set bounds for first cel in strip
		return true;
	}

	return false;
}


/*************************************************************************
 *
 * SetPalette()
 *
 * Parameters:
 *  CPalette *pPalette      color palette to be associated with the sprite
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Set to use the specified palette when dealing with this sprite.
 *                  If a palette has been previously associated with the sprite,
 *                  then map it out of the various contexts associated with the
 *                  sprite, then map in the new one.
 *
 ************************************************************************/

bool CSprite::SetPalette(CPalette *pPalette) {
	if (m_pImageDC != nullptr) {                               // if we already have a palette we must
		if (m_pImage != nullptr)                               // ... first map out any existing image
			(*m_pImageDC).SelectObject(m_pImageOld);  // ... then map out the old palette
		if (m_pPalette != nullptr)
			(*m_pImageDC).SelectPalette(m_pPalImageOld, false);
	}

	if (m_pBackgroundDC != nullptr) {                          // similarly we need to map out our background
		if (m_pBackground != nullptr)                          // ... bitmap as well
			(*m_pBackgroundDC).SelectObject(m_pBackgroundOld);
		if (m_pPalette != nullptr)                             // .. then map out the palette
			(*m_pBackgroundDC).SelectPalette(m_pPalBackOld, false);
	}

	ClearPalette();                                         // release existing palette
	m_pPalette = pPalette;                                  // set the new palette

	if (m_pImageDC == nullptr)                                 // done if no context to map it into
		return true;

	m_pPalImageOld = (*m_pImageDC).SelectPalette(m_pPalette, false); // map in the new palette and then
	(*m_pImageDC).RealizePalette();                           // ... tell the system to use it

	if (m_pImage != nullptr) {
		m_pImageOld = (*m_pImageDC).SelectObject(m_pImage);     // map in our image bitmap if it exists
		if (m_pImageOld == nullptr)
			return false;
	}

	if (m_pBackgroundDC == nullptr)                            // done if no background context
		return true;

	m_pPalBackOld = (*m_pBackgroundDC).SelectPalette(m_pPalette, false); // map in the new palette and then
	(*m_pBackgroundDC).RealizePalette();                          // ... tell the system to use it

	if (m_pBackground != nullptr) {
		m_pBackgroundOld = (*m_pBackgroundDC).SelectObject(m_pBackground);      // map in our background
		if (m_pBackgroundOld == nullptr)
			return false;
	}

	return true;
}


/*************************************************************************
 *
 * SharePalette()
 *
 * Parameters:
 *  CPalette *pPalette      color palette to be associated with the sprite
 *
 * Return Value:
 *  bool                    success/failure condition
 *
 * Description:     Use the specified palette when dealing with this sprite,
 *                  and mark it as a shared resource; hence don't delete it
 *                  when later changing the palette or destroying the sprite.
 *                  If a palette has been previously associated with the sprite,
 *                  then map it (and its bitmap) out of the device context,
 *                  delete it if it is not a shared resource, map the new
 *                  palette into the context, and then map in the bitmap.
 *
 ************************************************************************/

bool CSprite::SharePalette(CPalette *pPalette) {
	if (SetPalette(pPalette)) {                         // load a new palette into our sprite
		m_bSharedPalette = true;                        // ... and mark it as a shared resource
		return true;
	}                                  // ... whioh means it continues to exist
	// ... when this sprite is later destroyed
	m_pPalette = nullptr;                                  // not able to do that so ensure the
	return false;                                     // ... pointer is nulled
}


/*************************************************************************
 *
 * PaintSprite()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context for the destination
 *                  of the painting operation (e.g. display screen)
 *
 *  int x           coordinate which identifies where to begin painting
 *  int y           the sprite, and which corresponds to the upper lefthand
 *                  corner location of the sprite's bitmap
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Paint the sprite.  If it is a mobile sprite, then save
 *                  the bitmap area it will cover up so that when it moves
 *                  again so the saved bitmap area can be painted back to the
 *                  screen to correctly re-establish the background.  Background
 *                  restoration is either straightforward, or complex, depending
 *                  on whether or not the sprite overlaps other sprites.
 *
 ************************************************************************/

bool CSprite::PaintSprite(CDC *pDC, const int x, const int y) {
	bool        bSuccess = false;
	CRect       overlapRect, dstRect;
	CSprite     *pOverlap = nullptr;

	m_bTouchedSprite = false;                       // default to no sprite being overlapped by this
	m_pTouchedSprite = nullptr;                        // ... painting operation

	if (!m_bPositioned)                             // setup the initial location if not
		SetPosition(x, y);

	if (!m_bVisible) {                              // just place it if invisible
		SetPosition(x, y);
		return true;
	}

	if (pDC == nullptr)                                // punt if no device context for the sprite
		return false;

	if (m_bAnimated && m_nCelCount)                 // advance to the next cel in the strip
		UpdateCel();

	dstRect.SetRect(x, y, x + m_cSize.cx, y + m_cSize.cy); // calculate destination rectangle

	pOverlap = Interception(&dstRect);              // see if the sprite will intercept another
	if (pOverlap != nullptr) {                         // ... and if so, record that fact
		m_bTouchedSprite = true;                    // we've touched something
		m_pTouchedSprite = pOverlap;                // here's what we touched
	}

	if (!m_bLinked ||                               // how we do the painting depends on whether
	        (!m_bOverlaps &&
	         (pOverlap == nullptr))) {                       // ... we intercept another sprite
		if ((m_bHaveBackdrop || (m_pBackground != nullptr)) &&
		        overlapRect.IntersectRect(&m_cRect, &dstRect)) { // ... optimize painting of localized movement
			bSuccess = DoOptimizedPainting(pDC, &dstRect);  // ... by doing it all offscreen
			SetPosition(x, y);                      // establish the sprite's new position
		} else {
			if (m_bHaveBackdrop || (m_pBackground != nullptr)) {
				bSuccess = RefreshBackground(pDC);  // If it isn't close, just restore its background
				if (!bSuccess)
					return false;
			}
			SetPosition(x, y);                      // formally set the sprites new location
			if (!m_bHaveBackdrop && m_bRetainBackground) {
				bSuccess = SaveBackground(pDC);     // save the background art of its new location
				if (!bSuccess)
					return false;
			}
			bSuccess = UpdateSprite(pDC);           // paint the sprite in its new location
		}
		m_bOverlaps = false;                        // this sprite does not overlap another
		m_nZPosition = m_nZOrder;                   // ... so reset its z ordering
	} else {                                        // we have an overlap, so we need to do tear down
		bSuccess = DoOverlapPainting(pDC, &dstRect); // ... and rebuild sprite by sprite
		SetPosition(x, y);                          // now establish the sprite's new position
	}

	return bSuccess;
}


/*************************************************************************
 *
 * SetCel()
 *
 * Parameters:
 *  int nCelID      index of the desired cell
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     advance the cel image to the specified bitmap in the strip.
 *
 ************************************************************************/

void CSprite::SetCel(const int nCelID) {
	m_nCelID = nCelID;

	while (m_nCelID >= m_nCelCount) {
		m_nCelID -= m_nCelCount;
	}

	m_cImageRect.left = m_nCelID * m_cSize.cx;
	m_cImageRect.right = m_cImageRect.left + m_cSize.cx;
}


/*************************************************************************
 *
 * UpdateSprite()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context for the destination
 *                  of the painting operation (e.g. display screen)
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Repaint the sprite in its current location.  If it is a
 *                  mobile sprite without a saved background bitmap, then it
 *                  will be created and saved for later use.
 *
 *                  NOTE:  use PaintSprite() to paint a sprite to new location.
 *
 ************************************************************************/

bool CSprite::UpdateSprite(CDC *pDC) {
	CPalette *pPalOld = nullptr;
	bool        bSuccess = false;

	if (!m_bVisible)                                        // punt if not visible
		return true;

	if (pDC == nullptr)                                        // verify we have the necessary device contexts
		return false;

	if (!m_bHaveBackdrop && m_bRetainBackground &&          // if we don't have a bitmap backdrop, and if
	        (m_pBackgroundDC == nullptr)) {                        // ... we move and are painting for the first time
		if (!SaveBackground(pDC))                           // ... and then load it with the proper image
			return false;
	}

	if (m_pPalette != nullptr) {
		pPalOld = (*pDC).SelectPalette(m_pPalette, false);  // map in a palette if present
		(*pDC).RealizePalette();
	}                    // ... and tell the system we did that

	if (m_bMasked)                                          // if masked, then paint the sprite, masking out
		bSuccess = DoSpritePainting(pDC, m_cPosition);      // ... (ignoring) pure white pixels
	else  {
		bSuccess = SetupImage(pDC);                         // not masked - so just splat the entire bitmap
		if (bSuccess) {
			bSuccess = (*pDC).BitBlt(m_cPosition.x,
			                         m_cPosition.y,
			                         m_cSize.cx,
			                         m_cSize.cy,
			                         m_pImageDC,
			                         m_cImageRect.left,
			                         m_cImageRect.top,
			                         SRCCOPY);
			if (!m_bRetainContexts)
				ReleaseImageContext();
		}
	}

	if (m_pPalette != nullptr)                                 // map back the previous palette if needed
		(*pDC).SelectPalette(pPalOld, false);

	return bSuccess;
}


/*************************************************************************
 *
 * DoSpritePainting()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context for the destination
 *                  of the painting operation (e.g. display screen)
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Paint the source bitmap into the destination bitmap.
 *                  White pixels in the source bitmap are masked out (not painted)
 *                  thereby allowing irregular shaped images to be overlayed onto
 *                  a background.  Painting is done in an offscreen bitmap and
 *                  then transfered to the destination context; this reduces sprite
 *                  flicker, albeit at the cost of addition time.
 *
 ************************************************************************/

bool CSprite::DoSpritePainting(CDC * pDC, CPoint cPoint) {
	CDC         *pDC2 = nullptr;                           // context for manipulating background image
	CBitmap     *pBitmap2 = nullptr,                       // bitmap for pDC2
	             *pBitmap2Old = nullptr;                    // bitmap previously mapped into pDC2
	CPalette    *pPal2Old = nullptr;                       // palette previously mapped into pDC2
	bool        bSuccess = false;                       // success/failure

	if (!m_bVisible)                                    // punt if not visible
		return true;

	if (pDC == nullptr)                                    // punt if we don't have contexts
		return false;

	if (SetupImage(pDC) &&                              // put the image and mask bitmaps into
	        SetupMask(pDC)) {                               // ... device contexts

		pDC2 = new CDC();                               // create our temporary bitmap work areas
		pBitmap2 = new CBitmap();

		if ((pDC2 != nullptr) &&
		        (pBitmap2 != nullptr) &&
		        (*pDC2).CreateCompatibleDC(pDC)) {
			if (m_pPalette != nullptr) {                   // map in a palette if available
				pPal2Old = (*pDC2).SelectPalette(m_pPalette, false); // ... and for the background work area
				(*pDC2).RealizePalette();
			}                   // ... make it real too
			if ((*pBitmap2).CreateCompatibleBitmap(pDC, m_cSize.cx, m_cSize.cy)) {
				pBitmap2Old = (*pDC2).SelectObject(pBitmap2);
				if (pBitmap2Old != nullptr) {
					if (m_bHaveBackdrop) {              // get background image, either from
						if (GetBackdropDC(pDC) == nullptr) // .. the backdrop or the sprite itselt
							goto punt;
						(*pDC2).BitBlt(0, 0, m_cSize.cx, m_cSize.cy, m_pBackdropDC, cPoint.x, cPoint.y, SRCCOPY);
						ReleaseBackdropDC();
					} else
						(*pDC2).BitBlt(0, 0, m_cSize.cx, m_cSize.cy, pDC, cPoint.x, cPoint.y, SRCCOPY); // get the sprite's background image
					(*pDC2).BitBlt(0, 0, m_cSize.cx, m_cSize.cy, m_pMaskDC, m_cImageRect.left, m_cImageRect.top, SRCAND); // ... mask out where sprite will go
					(*pDC2).BitBlt(0, 0, m_cSize.cx, m_cSize.cy, m_pImageDC, m_cImageRect.left, m_cImageRect.top, SRCPAINT); // combine image with background
					bSuccess = (*pDC).BitBlt(cPoint.x, cPoint.y, m_cSize.cx, m_cSize.cy, pDC2, 0, 0, SRCCOPY); // paint result to the screen
				}
			}
		}

punt:
		if (pBitmap2Old != nullptr)                        // now release the temporary resources we used
			(*pDC2).SelectObject(pBitmap2Old);

		if (pBitmap2 != nullptr) {
			(*pBitmap2).DeleteObject();
			delete pBitmap2;
		}

		if (pPal2Old != nullptr)
			(*pDC2).SelectPalette(pPal2Old, false);

		if (pDC2 != nullptr) {
			(*pDC2).DeleteDC();
			delete pDC2;
		}
	}

	if (!m_bRetainContexts) {                           // release the sprite contexts if not optimizing
		ReleaseImageContext();
		ReleaseMaskContext();
	}

	return bSuccess;                                  // return success or failure
}


/*************************************************************************
 *
 * DoOptimizedPainting()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context for the destination
 *                  of the painting operation (e.g. display screen)
 *
 *  CRect *pDstRect rectangle defining the area where the sprite will be
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Determine the area affected by painting the sprite in a
 *                  new location (e.g. old area where the background bitmap
 *                  needs to be restored, as well as the area where the sprite
 *                  is to be newly painted), and then do all of that painting
 *                  in a compatible offscreen bitmap.  Once that is accomplished,
 *                  paint the offscreen bitmap onto the destination device context;
 *                  This elimates very noticeable sprite "flicker" that would
 *                  otherwise occur, albeit at the cost of additional time.
 *
 ************************************************************************/

bool CSprite::DoOptimizedPainting(CDC *pDC, CRect *pDstRect) {
	bool    bSuccess = false;                           // success/failure return status
	CPoint  cPoint;                                     // where to paint sprite in the work area
	CRect   unionRect;                                  // rectangle enclosing old and new sprite locations
	CDC     workDC;                                     // device context for offscreen work area
	CBitmap *pBitmap = nullptr,                            // bitmap for the work area
	         *pBitmapOldWork = nullptr;                     // bitmap previously mapped to work area's context
	CPalette *pPalOld = nullptr,                           // palette previously mapped to destination context
	          *pPalOldWork = nullptr;                        // palette prevously mapped to work area's context
	int     dx, dy;                                     // delta sizes of work area's bitmap

	if (pDC == nullptr)                                    // punt if no sprite device context
		return false;

	if (m_pPalette != nullptr) {                               // if there is a palette
		pPalOld = (*pDC).SelectPalette(m_pPalette, false);  // ... select it into the context
		(*pDC).RealizePalette();
	}                    // ... and tell the system about it

	unionRect = m_cRect;                                // the work area is defined by our sprite
	if (pDstRect != nullptr)
		unionRect.UnionRect(unionRect, pDstRect);       // calculate the smallest enclosing rectangle that
	dx = unionRect.right - unionRect.left;              // ... contains the bitmap area where the sprite was
	dy = unionRect.bottom - unionRect.top;              // ... and the bitmap area where it will be next

	pBitmap = new CBitmap();                            // create an offscreen bitmap where we do all the
	if (pBitmap == nullptr)                                // ... work; first create a bitmap for the enclosing
		return false;                                 // ... rectangle, and if that fails, then punt
	if (!(*pBitmap).CreateCompatibleBitmap(pDC, dx, dy)) {
		delete pBitmap;
		return false;
	}

	if (workDC.CreateCompatibleDC(pDC)) {               // setup the work area context
		if (m_pPalette != nullptr) {                       // if we have a palette, map it into the work context
			pPalOldWork = workDC.SelectPalette(m_pPalette, false);
			workDC.RealizePalette();
		}                // ... and tell the system to use it
		pBitmapOldWork = workDC.SelectObject(pBitmap);      // now map in the bitmap into that context
		if (pBitmapOldWork != nullptr) {                       // next copy the image enclosed by the rectangle
			if (m_bHaveBackdrop) {                          // restore the background where the sprite was
				if (GetBackdropDC(pDC) == nullptr)
					goto punt;
				bSuccess = workDC.BitBlt(0,
				                         0,
				                         dx,
				                         dy,
				                         m_pBackdropDC,
				                         unionRect.left,
				                         unionRect.top,
				                         SRCCOPY);
				ReleaseBackdropDC();
				if (!bSuccess)
					goto punt;
			} else {
				workDC.BitBlt(0, 0, dx, dy, pDC, unionRect.left, unionRect.top, SRCCOPY); // ... into the bitmap
				if (SetupBackground(pDC)) {
					workDC.BitBlt(m_cPosition.x - unionRect.left,    // restore the background where the sprite was
					                     m_cPosition.y - unionRect.top,
					                     m_cSize.cx,
					                     m_cSize.cy,
					                     m_pBackgroundDC,
					                     0,
					                     0,
					                     SRCCOPY);
					(*m_pBackgroundDC).BitBlt(0,      // save the background where the sprite will be
					                                0,
					                                m_cSize.cx,
					                                m_cSize.cy,
					                                &workDC,
					                                (*pDstRect).left - unionRect.left,
					                                (*pDstRect).top - unionRect.top,
					                                SRCCOPY);
					if (!m_bRetainContexts || m_bDuplicated)    // release background context if not optimizing
						ReleaseBackgroundContext();
				} else
					goto punt;
			}
			cPoint.x = (*pDstRect).left - unionRect.left;   // determine where to paint the new sprite image
			cPoint.y = (*pDstRect).top - unionRect.top;     // ... into the work area

			bSuccess = ((!m_bMasked || SetupMask(pDC)) &&   // setup the contexts we need
			            SetupImage(pDC));
			if (bSuccess) {
				if (m_bMasked) {                            // need to do a masked copy ...
					workDC.BitBlt(cPoint.x,          // ... mask out where sprite goes
					                     cPoint.y,
					                     m_cSize.cx,
					                     m_cSize.cy,
					                     m_pMaskDC,
					                     m_cImageRect.left,
					                     m_cImageRect.top,
					                     SRCAND);
					workDC.BitBlt(cPoint.x,          // ... paint in the sprite
					                     cPoint.y,
					                     m_cSize.cx,
					                     m_cSize.cy,
					                     m_pImageDC,
					                     m_cImageRect.left,
					                     m_cImageRect.top,
					                     SRCPAINT);
				} else                                      // just paint in the sprite
					workDC.BitBlt(cPoint.x,
					                     cPoint.y,
					                     m_cSize.cx,
					                     m_cSize.cy,
					                     m_pImageDC,
					                     m_cImageRect.left,
					                     m_cImageRect.top,
					                     SRCCOPY);
				if (!m_bRetainContexts) {                   // release contexts if not optimizing
					ReleaseImageContext();
					if (m_bMasked)
						ReleaseMaskContext();
				}
			}

			if (bSuccess) {
				(*pDC).BitBlt(unionRect.left,
				                    unionRect.top,
				                    dx,
				                    dy,
				                    &workDC,
				                    0,
				                    0,
				                    SRCCOPY);
				workDC.SelectObject(pBitmapOldWork);     // tear down the work area's bitmap, palette
				if (m_pPalette != nullptr) {                           // ... and device context, as well as the
					workDC.SelectPalette(pPalOldWork, false); // ... destination's palette
					(*pDC).SelectPalette(pPalOld, false);
				}
				workDC.DeleteDC();
				(*pBitmap).DeleteObject();
				delete pBitmap;
				return bSuccess;
			}
		}
	}

punt:

	if (pBitmapOldWork != nullptr)                             // we failed, so tear down the resources used
		workDC.SelectObject(pBitmapOldWork);         // ... map out the bitmap
	if (pPalOldWork != nullptr)                                // ... map out the palette
		workDC.SelectPalette(pPalOldWork, false);
	if (workDC.m_hDC != nullptr)                               // ... release the context
		workDC.DeleteDC();

	(*pBitmap).DeleteObject();                              // ... delete the bitmap
	delete pBitmap;

	if (pPalOld != nullptr)                                    // map out the palette from the output context
		(*pDC).SelectPalette(pPalOld, false);

	return false;
}


/*************************************************************************
 *
 * DoOverlapPainting()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context for the destination
 *                  of the painting operation (e.g. display screen)
 *  CRect *myRect   new area where the sprite will be painted
 *  CSprite *pSprite    pointer to intercepted sprite in chain
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Paint a sprite into its new location, after restoring
 *                  the background where it is currently located.  The sprite
 *                  is assumed to be part of a set of overlapping sprites, so
 *                  sprite-by-sprite reconstruction of the background is needed.
 *
 ************************************************************************/

bool CSprite::DoOverlapPainting(CDC *pDC, CRect *myRect) {
	CPalette    *pPalOld = nullptr;
	bool        bSuccess = false;

	if (pDC == nullptr)                                        // punt if no context
		return false;

	if (m_pPalette != nullptr) {                               // if there is a palette
		pPalOld = (*pDC).SelectPalette(m_pPalette, false);  // ... select it into the context
		(*pDC).RealizePalette();
	}                    // ... and tell the system about it

	bSuccess = ReconstructBackground(pDC, myRect);          // go do the actual painting

	if (m_pPalette != nullptr)                                 // map out the palette
		(*pDC).SelectPalette(pPalOld, false);

	return bSuccess;
}


/*************************************************************************
 *
 * RefreshBackground()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context for the destination
 *                  of the painting operation (e.g. display screen)
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Paint a sprite's saved background bitmap into its
 *                  appropriate area.
 *
 ************************************************************************/

bool CSprite::RefreshBackground(CDC *pDC) {
	CPalette    *pPalOld = nullptr;
	bool        bSuccess = false;

	if (!m_bVisible)                                        // punt if not visible
		return true;

	if (pDC == nullptr)                                        // punt if no sprite device context
		return false;

	if (!m_bHaveBackdrop &&
	        m_pBackground == nullptr)                              // done if no saved background
		return true;

	if (m_pPalette != nullptr) {                               // map a palette to the destination context
		pPalOld = (*pDC).SelectPalette(m_pPalette, false);  // ... if available
		(*pDC).RealizePalette();
	}

	if (m_bOverlaps)                                        // paint the background bitmap to the device
		bSuccess = ReconstructBackground(pDC, nullptr);        // ... from all the sprites that overlap it
	else if (m_bHaveBackdrop) {
		if (GetBackdropDC(pDC) == nullptr)
			bSuccess = false;
		else {
			bSuccess = (*pDC).BitBlt(m_cPosition.x, m_cPosition.y, m_cSize.cx, m_cSize.cy, m_pBackdropDC, m_cPosition.x, m_cPosition.y, SRCCOPY);
			ReleaseBackdropDC();
		}
	} else {
		bSuccess = SetupBackground(pDC);                    // paint the background bitmap to the device
		if (bSuccess) {
			bSuccess = (*pDC).BitBlt(m_cPosition.x, m_cPosition.y, m_cSize.cx, m_cSize.cy, m_pBackgroundDC, 0, 0, SRCCOPY);
			if (!m_bRetainContexts || m_bDuplicated)
				ReleaseBackgroundContext();
		}
	}

	if (m_pPalette != nullptr)                                 // map out the palette from the destination
		(*pDC).SelectPalette(pPalOld, false);

	m_bPositioned = false;                                  // no real position now
	m_nZPosition = m_nZOrder;                               // reset z ordering

	return bSuccess;
}


/*************************************************************************
 *
 * SaveBackground()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context where the sprite will be painted
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Fetch the area into which the sprite will be painted and
 *                  save it as a bitmap.  It can then be repainted there later
 *                  to restore the background.
 *
 ************************************************************************/

bool CSprite::SaveBackground(CDC *pDC) {
	CPalette    *pPalOld = nullptr;
	bool        bSuccess = false;

	if (m_bHaveBackdrop || !m_bRetainBackground)        // not valid to save background with backdrop
		return false;                                 // ... or if not retaining same

	if (!m_bVisible)                                    // punt if not visible
		return true;

	if (pDC == nullptr)                                    // punt if no output context
		return false;

	if (!CreateBackground(pDC))                         // create background context
		return false;

	if (m_pPalette != nullptr) {                           // map a palette to the destination context
		pPalOld = (*pDC).SelectPalette(m_pPalette, false);
		(*pDC).RealizePalette();
	}

	bSuccess = SetupBackground(pDC);                    // grab the background bitmap
	if (bSuccess) {
		bSuccess = (*m_pBackgroundDC).BitBlt(0, 0, m_cSize.cx, m_cSize.cy, pDC, m_cPosition.x, m_cPosition.y, SRCCOPY);
		if (!m_bRetainContexts || m_bDuplicated)        // release context if not optimizing
			ReleaseBackgroundContext();
	}

	if (m_pPalette != nullptr)                             // map out palette from destination context
		(*pDC).SelectPalette(pPalOld, false);

	return bSuccess;
}


/*************************************************************************
 *
 * ReconstructBackground()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context where the sprite will be painted
 *  CRect *myRect   nullptr or pointer to new area where sprite gets painted
 *  CSprite *pSprite    pointer to intercepted sprite in chain
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Reconstruct the background image by restoring the background
 *                  bitmaps from all overlapping sprites, working from topmost
 *                  down to including our sprite.  Then paint all those sprite's
 *                  images (excluding ourself) onto the restore background, thus
 *                  creating a bitmap that no longer contains our sprite's image.
 *                  Lastly, if we have a pointer to a rectangle, then paint our
 *                  sprite image into it, since that is its new location.
 *
 ************************************************************************/

bool CSprite::ReconstructBackground(CDC *pDC, CRect *myRect) {
	CDC         workDC;                                 // context for our work area
	CSprite     *pSprite = nullptr,                        // various sprite pointers
	             *pTestSprite,
	             *pZHead;
	bool        bDoingTopMost;                          // special case for single sprite that is topmost
	bool        bSuccess = false;                       // used to hold success/failure
	CRect       baseRect,                               // rectangle spanning the sprite update
	            unionRect,                              // rectangle spanning all sprites touched
	            overlapRect;                            // temporary rectangle information
	int         dx, dy;                                 // size of unionRect and work bitmap
	CPoint      cPoint;                                 // temporary point information
	CBitmap     *pBitmap = nullptr,                        // work area bitmap
	             *pBitmapOld = nullptr;                     // bitmap previously selected to work context
	CPalette    *pPalOld = nullptr,                        // palette previously selected to display context
	             *pPalOldWork = nullptr;                    // palette previously selected to work context
	#if SPRITE_DEBUG
	CRect       clipRect;                               // current clipping area
	CPoint      viewOrigin;                             // current viewport origin
	int         nClipResult;

	(*pDC).GetClipBox(&clipRect);
	nClipResult = (*pDC).SelectClipRgn(nullptr);
	(*pDC).GetClipBox(&clipRect);
	viewOrigin = (*pDC).GetViewportOrg();
	viewOrigin.x = - viewOrigin.x;
	viewOrigin.y = - viewOrigin.y;
	#endif

	unionRect = m_cRect;                                // the work area is defined by our sprite
	if (myRect != nullptr)                                 // include the area of our new destination
		unionRect.UnionRect(unionRect, myRect);         // ... if specified, and retain it for later
	baseRect = unionRect;

	pSprite = this;                                     // start processing from this sprite and make
	(*pSprite).m_pZNext = nullptr;                         // ... it be the first in the z order chain
	(*pSprite).m_pZPrev = nullptr;

	pTestSprite = m_pSpriteChain;                       // set all sprites to not having been tested
	while (pTestSprite != nullptr) {                       // ... for this reconstruction cycle
		if ((*pTestSprite).m_bVisible && (*pTestSprite).m_bPositioned)
			(*pTestSprite).m_bOverlapTest = false;
		else
			(*pTestSprite).m_bOverlapTest = true;
		pTestSprite = (*pTestSprite).m_pNext;
	}
	pTestSprite = m_pSpriteChain;                       // get first sprite to test against
	m_bOverlapTest = true;                              // set to not test against the first sprite
	m_bOverlaps = true;
	if (m_bHaveBackdrop || (m_pBackground != nullptr))
		m_bPaintOverlap = true;
	else
		m_bPaintOverlap = false;

	while (pTestSprite != nullptr) {                       // look for sprites we overlap or which are
		if (!(*pTestSprite).m_bOverlapTest &&           // ... overlapped by sprites that overlap us
		        overlapRect.IntersectRect(&unionRect, &(*pTestSprite).m_cRect)) {
			unionRect.UnionRect(unionRect, (*pTestSprite).m_cRect); // expand rectangle to encompass it
			(*pTestSprite).m_bOverlapTest = true;       // mark it as having been tested
			(*pTestSprite).m_bOverlaps = true;          // mark it as involved in an overlap situation
			if ((m_bHaveBackdrop ||                     // won't paint sprites without a background
			        ((*pTestSprite).m_pBackground != nullptr)) &&
			        overlapRect.IntersectRect(&baseRect, &(*pTestSprite).m_cRect))
				(*pTestSprite).m_bPaintOverlap = true;
			else
				(*pTestSprite).m_bPaintOverlap = false;
			while (true) {                              // insert the sprite in the sorted z chain
				if ((*pTestSprite).m_nZPosition < (*pSprite).m_nZPosition) {    // need to head leftward
					if (((*pSprite).m_pZPrev == nullptr) ||    // put it to the left of us (i.e. overlaps us)
					        ((*(*pSprite).m_pZPrev).m_nZPosition < (*pTestSprite).m_nZPosition)) {
						(*pTestSprite).m_pZPrev = (*pSprite).m_pZPrev;
						(*pTestSprite).m_pZNext = pSprite;
						(*pSprite).m_pZPrev = pTestSprite;
						if ((*pTestSprite).m_pZPrev != nullptr)
							(*(*pTestSprite).m_pZPrev).m_pZNext = pTestSprite;
						break;
					} else
						pSprite = (*pSprite).m_pZPrev;  // shift left in chain and set to test again
				} else                                  // need to head rightward
					if ((*pTestSprite).m_nZPosition >= (*pSprite).m_nZPosition) {
						if (((*pSprite).m_pZNext == nullptr) ||    // put it to the right of us (we overlap it)
						        ((*(*pSprite).m_pZNext).m_nZPosition >= (*pTestSprite).m_nZPosition)) {
							(*pTestSprite).m_pZNext = (*pSprite).m_pZNext;
							(*pTestSprite).m_pZPrev = pSprite;
							(*pSprite).m_pZNext = pTestSprite;
							if ((*pTestSprite).m_pZNext != nullptr)
								(*(*pTestSprite).m_pZNext).m_pZPrev = pTestSprite;
							break;
						} else
							pSprite = (*pSprite).m_pZNext;  // shift right in chain and set to test again
					}
			}
			pTestSprite = m_pSpriteChain;               // need to check all sprites again since the
		}                                           // ... the bounding rectangle has expanded and
		else                                            // ... may now intercept additional sprites
			pTestSprite = (*pTestSprite).m_pNext;       // set to test against the next sprite
	}

	while ((*pSprite).m_pZPrev != nullptr)                 // find the head of the z order chain so that
		pSprite = (*pSprite).m_pZPrev;                  // ... we can begin background reconstruction

	pZHead = pSprite;                                   // save head of z order chain

	if (((*pZHead).m_nId == m_nId) &&                   // if we are the head of the z order chain
	        (m_nZOrder == SPRITE_TOPMOST) &&                // ... and we are a top-most type of sprite
	        !m_bHaveBackdrop) {                             // ... and we're painting direct to the screen
		pTestSprite = (*pZHead).m_pZNext;               // ... then we only need to handle this sprite
		while (pTestSprite != nullptr) {                   // so set to not paint others in z order chain
			(*pTestSprite).m_bPaintOverlap = false;
			pTestSprite = (*pTestSprite).m_pZNext;
		}
		if (m_nCelCount <= 1)                           // if we're not a cel strip, then set a
			bDoingTopMost = true;                       // ... special flag indicating just our sprite
		else
			bDoingTopMost = false;                      // ... otherwise use default methodology
		unionRect = baseRect;                           // minimize the bounding rectangle
	} else
		bDoingTopMost = false;

	dx = unionRect.right - unionRect.left;              // get the width and height of the work area
	dy = unionRect.bottom - unionRect.top;              // ... spanned by all overlapping sprites

	pBitmap = new CBitmap();                            // create a bitmap to contain our work area
	if (pBitmap == nullptr)
		return false;
	if (!(*pBitmap).CreateCompatibleBitmap(pDC, dx, dy)) {
		delete pBitmap;
		return false;
	}

	if (workDC.CreateCompatibleDC(pDC)) {               // create a device context to contain our
		if (m_pPalette != nullptr) {                       // ... work area and map in the palette
			pPalOldWork = workDC.SelectPalette(m_pPalette, false);
			workDC.RealizePalette();
			pPalOld = (*pDC).SelectPalette(m_pPalette, false);
			(*pDC).RealizePalette();
		}
		pBitmapOld = workDC.SelectObject(pBitmap);      // map our bitmap into the work area
		if (pBitmapOld != nullptr) {                       // now need to create the original background
			if (m_bHaveBackdrop) {                      // ... which is easy if we are maintaining an
				if (GetBackdropDC(pDC) == nullptr)         // ... offscreen bitmap for the whole screen
					bSuccess = false;
				else {
					bSuccess = workDC.BitBlt(0,         // copy the background section into the work area
					                         0,
					                         dx,
					                         dy,
					                         m_pBackdropDC,
					                         unionRect.left,
					                         unionRect.top,
					                         SRCCOPY);
					ReleaseBackdropDC();
				}
				if (!bSuccess)
					goto punt;
				while (true) {
					pTestSprite = (*pSprite).m_pZNext;  // now update z order relative positions
					if (pTestSprite == nullptr)            // ... of every sprite in the z order chain
						break;                          // ... to ensure/maintain increasing ordering
					if ((*pSprite).m_nZOrder == (*pTestSprite).m_nZOrder)
						(*pTestSprite).m_nZPosition = (*pSprite).m_nZPosition + 1;
					pSprite = pTestSprite;
				}
			} else {                                    // no backdrop, so first grab the screen
				bSuccess = workDC.BitBlt(0, 0, dx, dy, pDC, unionRect.left, unionRect.top, SRCCOPY);
				#if SPRITE_DEBUG
				bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, dx, dy, &workDC, 0, 0, SRCCOPY);
				#endif
				while (true) {                          // now restore the saved background from each
					if ((*pSprite).m_bPaintOverlap && (*pSprite).m_bRetainBackground) {
						bSuccess = (*pSprite).SetupBackground(pDC); // ... sprite in the z order chain
						if (bSuccess) {
							if ((*pSprite).m_bMasked && // if just doing the single topmost sprite, then
							        bDoingTopMost &&        // ... replace its image area with its background
							        (*pSprite).SetupMask(pDC)) {    // .this is extra work but compensated by ignoring all others
								bSuccess = workDC.BitBlt((*pSprite).m_cPosition.x - unionRect.left,  // mask away where sprite image is located
								                         (*pSprite).m_cPosition.y - unionRect.top,
								                         (*pSprite).m_cSize.cx,
								                         (*pSprite).m_cSize.cy,
								                         (*pSprite).m_pMaskDC,
								                         (*pSprite).m_cImageRect.left,
								                         (*pSprite).m_cImageRect.top,
								                         SRCAND);
								#if SPRITE_DEBUG
								bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, dx, dy, &workDC, 0, 0, SRCCOPY);
								bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, (*pSprite).m_cSize.cx, (*pSprite).m_cSize.cy, (*pSprite).m_pMaskDC, 0, 0, SRCCOPY);
								#endif
								bSuccess = (*(*pSprite).m_pMaskDC).BitBlt(    // invert the image mask
								               (*pSprite).m_cImageRect.left,
								               (*pSprite).m_cImageRect.top,
								               (*pSprite).m_cSize.cx,
								               (*pSprite).m_cSize.cy,
								               (*pSprite).m_pMaskDC,
								               (*pSprite).m_cImageRect.left,
								               (*pSprite).m_cImageRect.top,
								               NOTSRCCOPY);
								#if SPRITE_DEBUG
								bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, (*pSprite).m_cSize.cx, (*pSprite).m_cSize.cy, (*pSprite).m_pMaskDC, 0, 0, SRCCOPY);
								#endif
								bSuccess = (*(*pSprite).m_pBackgroundDC).BitBlt(0, 0,   // mask away area of saved background that
								           (*pSprite).m_cSize.cx,         // ... is NOT spanned by the sprite image
								           (*pSprite).m_cSize.cy,
								           (*pSprite).m_pMaskDC,
								           (*pSprite).m_cImageRect.left,
								           (*pSprite).m_cImageRect.top,
								           SRCAND);
								#if SPRITE_DEBUG
								bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, (*pSprite).m_cSize.cx, (*pSprite).m_cSize.cy, (*pSprite).m_pBackgroundDC, 0, 0, SRCCOPY);
								#endif
								bSuccess = (*(*pSprite).m_pMaskDC).BitBlt(
								               (*pSprite).m_cImageRect.left,  // put the mask back the way it was
								               (*pSprite).m_cImageRect.top,
								               (*pSprite).m_cSize.cx,
								               (*pSprite).m_cSize.cy,
								               (*pSprite).m_pMaskDC,
								               (*pSprite).m_cImageRect.left,
								               (*pSprite).m_cImageRect.top,
								               NOTSRCCOPY);
								#if SPRITE_DEBUG
								bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, (*pSprite).m_cSize.cx, (*pSprite).m_cSize.cy, (*pSprite).m_pMaskDC, 0, 0, SRCCOPY);
								#endif
								bSuccess = workDC.BitBlt((*pSprite).m_cPosition.x - unionRect.left, // paint the remaining background into cleared area
								                         (*pSprite).m_cPosition.y - unionRect.top,  // i.e. we only updated the pixels where the image
								                         (*pSprite).m_cSize.cx,                     // ... was actually located, leaving all else untouched
								                         (*pSprite).m_cSize.cy,
								                         (*pSprite).m_pBackgroundDC,
								                         0,
								                         0,
								                         SRCPAINT);
							} else                              // just splat sprite's background to work area
								bSuccess = workDC.BitBlt((*pSprite).m_cPosition.x - unionRect.left,
								                         (*pSprite).m_cPosition.y - unionRect.top,
								                         (*pSprite).m_cSize.cx,
								                         (*pSprite).m_cSize.cy,
								                         (*pSprite).m_pBackgroundDC, 0, 0, SRCCOPY);
							if (!(*pSprite).m_bRetainContexts)  // release contexts if not optimizing
								(*pSprite).ReleaseMaskContext();
							#if SPRITE_DEBUG
							bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, dx, dy, &workDC, 0, 0, SRCCOPY);
							#endif
						}
						if (!bSuccess)
							goto punt;
					}

					if ((*pSprite).m_pZNext == nullptr) {          // once we have restored all backgrounds
						if ((m_nZOrder == SPRITE_TOPMOST) &&    // ... then place us at the head of the
						        (m_nId != (*pZHead).m_nId)) {       // ... z order chain if we are to be topmost
							if ((*pSprite).m_nId == m_nId)      // maintain pointer to last in z order
								pSprite = m_pZPrev;             // ... backing up one if it is us
							pTestSprite = m_pZPrev;             // ... since we're moving to the head
							(*pTestSprite).m_pZNext = m_pZNext;
							if (m_pZNext != nullptr)
								(*m_pZNext).m_pZPrev = pTestSprite;
							m_pZNext = pZHead;
							m_pZPrev = nullptr;
							(*pZHead).m_pZPrev = this;
							pZHead = this;
							m_nZPosition = m_nZOrder;
						}
						break;
					}

					pSprite = (*pSprite).m_pZNext;
				}

				pTestSprite = pZHead;                           // update z order relative positions
				while ((*pTestSprite).m_pZNext != nullptr) {       // ... to ensure increasing ordering
					if ((*pTestSprite).m_nZOrder == (*(*pTestSprite).m_pZNext).m_nZOrder)
						(*(*pTestSprite).m_pZNext).m_nZPosition = (*pTestSprite).m_nZPosition + 1;
					pTestSprite = (*pTestSprite).m_pZNext;
				}
			}

			while (true) {                                      // now repaint the images of the sprites
				if (m_nId == (*pSprite).m_nId) {                // ... onto the work area
					if (myRect == nullptr)                         // no image, just erasing our sprite
						bSuccess = true;
					else {
						if (!m_bHaveBackdrop) {                 // but before we paint in the sprite's image
							if (m_bRetainBackground) {          // ... retain what that area looks like as
								if (m_pBackground == nullptr) {    // ... the new saved background
									if (!CreateBackground(pDC) ||
									        !SetupBackground(pDC))
										goto punt;
								}
								bSuccess = (*m_pBackgroundDC).BitBlt(0, 0,
								                                     m_cSize.cx,
								                                     m_cSize.cy,
								                                     &workDC,
								                                     (*myRect).left - unionRect.left,
								                                     (*myRect).top - unionRect.top,
								                                     SRCCOPY);
								if (!m_bRetainContexts || m_bDuplicated)
									ReleaseBackgroundContext();
								if (!bSuccess)
									goto punt;
							}
							m_bPaintOverlap = true;
						}
						cPoint.x = (*myRect).left - unionRect.left; // set the destination for where our
						cPoint.y = (*myRect).top - unionRect.top;   // ... sprite will be painted
						goto paint_sprite;
					}
				} else {                                        // save what the background looks like
					if (!m_bHaveBackdrop && (*pSprite).m_bRetainBackground &&
					        (*pSprite).m_bPaintOverlap) {
						bSuccess = (*(*pSprite).m_pBackgroundDC).BitBlt(0, 0,
						           (*pSprite).m_cSize.cx,
						           (*pSprite).m_cSize.cy,
						           &workDC,
						           (*pSprite).m_cPosition.x - unionRect.left,
						           (*pSprite).m_cPosition.y - unionRect.top,
						           SRCCOPY);
						if (!(*pSprite).m_bRetainContexts || (*pSprite).m_bDuplicated)
							(*pSprite).ReleaseBackgroundContext();
						if (!bSuccess)
							goto punt;
					}
					cPoint.x = (*pSprite).m_cPosition.x - unionRect.left;   // set the destination for sprite
					cPoint.y = (*pSprite).m_cPosition.y - unionRect.top;
paint_sprite:
					if ((*pSprite).m_bPaintOverlap) {
						bSuccess = (!(*pSprite).m_bMasked || (*pSprite).SetupMask(pDC)) &&  // setup image and mask contexts
						           (*pSprite).SetupImage(pDC);
						if (bSuccess) {
							if ((*pSprite).m_bMasked) {                             // need to do a masked transfer ...
								workDC.BitBlt(cPoint.x,                      // ... mask away where image will go
								                     cPoint.y,
								                     (*pSprite).m_cSize.cx,
								                     (*pSprite).m_cSize.cy,
								                     (*pSprite).m_pMaskDC,
								                     (*pSprite).m_cImageRect.left,
								                     (*pSprite).m_cImageRect.top,
								                     SRCAND);
								#if SPRITE_DEBUG
								bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, dx, dy, &workDC, 0, 0, SRCCOPY);
								#endif
								workDC.BitBlt(cPoint.x,                      // ... paint image into cleared area
								                     cPoint.y,
								                     (*pSprite).m_cSize.cx,
								                     (*pSprite).m_cSize.cy,
								                     (*pSprite).m_pImageDC,
								                     (*pSprite).m_cImageRect.left,
								                     (*pSprite).m_cImageRect.top,
								                     SRCPAINT);
								#if SPRITE_DEBUG
								bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, dx, dy, &workDC, 0, 0, SRCCOPY);
								#endif
							} else {
								workDC.BitBlt(cPoint.x,                      // just paint the image
								                     cPoint.y,
								                     (*pSprite).m_cSize.cx,
								                     (*pSprite).m_cSize.cy,
								                     (*pSprite).m_pImageDC,
								                     (*pSprite).m_cImageRect.left,
								                     (*pSprite).m_cImageRect.top,
								                     SRCCOPY);
								#if SPRITE_DEBUG
								bSuccess = (*pDC).BitBlt(viewOrigin.x, viewOrigin.y, dx, dy, &workDC, 0, 0, SRCCOPY);
								#endif
							}
							if (!(*pSprite).m_bRetainContexts) {                // release contexts if not optimizing
								(*pSprite).ReleaseImageContext();
								if ((*pSprite).m_bMasked)
									(*pSprite).ReleaseMaskContext();
							}
						}
					}
				}
				if (!bSuccess)
					goto punt;
				if ((*pSprite).m_pZPrev == nullptr)                // see if done with image painting
					break;
				pSprite = (*pSprite).m_pZPrev;
			}

			(*pDC).BitBlt(baseRect.left,                  // paint only the part of the work area
			                    baseRect.top,                   // ... spanned by our sprites source
			                    baseRect.right - baseRect.left, // ... and destination locations to
			                    baseRect.bottom - baseRect.top, // ... the screen; i.e. doing the whole
			                    &workDC,                        // ... work area would be painting stuff
			                    baseRect.left - unionRect.left, // ... that wasn't modified
			                    baseRect.top - unionRect.top,
			                    SRCCOPY);

			workDC.SelectObject(pBitmapOld);     // tear down the work area's bitmap, palette
			if (m_pPalette != nullptr) {                           // ... and device context, as well as the
				workDC.SelectPalette(pPalOldWork, false); // ... destination's palette
				(*pDC).SelectPalette(pPalOld, false);
			}
			workDC.DeleteDC();
			(*pBitmap).DeleteObject();
			delete pBitmap;
			return true;
		}
	}

punt:

	if (pBitmapOld != nullptr)                             // we failed, so tear down the resources used
		workDC.SelectObject(pBitmapOld);         // ... map out the bitmap
	if (pPalOldWork != nullptr)                            // ... map out the palette
		workDC.SelectPalette(pPalOldWork, false);
	if (workDC.m_hDC != nullptr)                           // ... release the context
		workDC.DeleteDC();

	(*pBitmap).DeleteObject();                          // ... delete the bitmap
	delete pBitmap;

	if (pPalOld != nullptr)                                // map out the palette from the output context
		(*pDC).SelectPalette(pPalOld, false);

	return false;
}


/*************************************************************************
 *
 * EraseSprites()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context where the sprites are painted
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     For each sprite in the sprite chain, clear the sprite's
 *                  image by repainting its stored background and then clear
 *                  the background bitmap.
 *
 ************************************************************************/

bool CSprite::EraseSprites(CDC *pDC) {
	CSprite *pSprite = nullptr;

	pSprite = m_pSpriteChain;

	while (pSprite != nullptr) {
		if (!(*pSprite).EraseSprite(pDC))
			return false;
		pSprite = (*pSprite).m_pNext;
	}

	return true;
}


/*************************************************************************
 *
 * EraseSprite()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context where the sprite is painted
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Clear the sprite's image by repainting its stored background
 *                  and then clear the background bitmap.
 *
 ************************************************************************/

bool CSprite::EraseSprite(CDC *pDC) {
	if (!m_bVisible)                                        // punt if not visible
		return true;

	if (RefreshBackground(pDC)) {                           // repaint the background
		ClearBackground();                                  // clear the background art
		m_bPositioned = false;                              // no longer has a real position
		return true;
	}

	return false;
}


/*************************************************************************
 *
 * TestInterception()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context where the sprite gets painted
 *  CSprite * pTestSprite   pointer to a lone test sprite for testing
 *  CPoint *pPoint         optional address of place to store estimated pixel overlap
 *
 * Return Value:
 *  CSprite *               pointer to sprite touched, or nullptr
 *
 * Description:     Determine whether a given sprite touches another given sprite.
 *                  The defining rectangular areas of the sprites are tested first,
 *                  and bit masking techniques are used if both sprites are masked.
 *
 ************************************************************************/

bool CSprite::TestInterception(CDC *pDC, CSprite * pTestSprite, CPoint *pPoint) {
	CRect   myRect,                                     // rectangle occupied by current sprite
	        testRect,                                   // sprite retangle to be tested against
	        overlapRect;                                // area of overlap between rectangles

	if (!m_bIntercepts || !(*pTestSprite).m_bIntercepts)    // punt if no interception allowed
		return false;

	myRect = m_cRect;                                   // acquire the rectangle for base sprite

	if (m_nId != (*pTestSprite).m_nId) {                // be sure to not test against ourself
		testRect = (*pTestSprite).m_cRect;              // get bounding rectangle to test against
		if (overlapRect.IntersectRect(&myRect, &testRect)) { // use simple rectangle screening first
			if (!m_bMasked ||                           // ... and if that succeeds, see if we
			        !(*pTestSprite).m_bMasked ||            // ... and if that succeeds, see if we
			        SpritesOverlap(pDC, pTestSprite, pPoint))   // ... have image masks that overlap
				return true;                          // got a simple or complex overlap
		}
	}

	return false;
}


/*************************************************************************
 *
 * Interception()
 *
 * Parameters:
 *  CRect *newRect  rectangle defining the new location of a sprite
 *  CSprite *       pointer to a lone sprite or a sprite chain
 *
 * Return Value:
 *  CSprite *       pointer to sprite touched, or nullptr
 *
 * Description:     Determine whether a sprite will touch another sprite if
 *                  moved to its new location.  Simply determine whether
 *                  the defining rectangular areas of the sprites intersect
 *                  for basic sprites.
 *
 ************************************************************************/

CSprite *CSprite::Interception(CRect *newRect, CSprite * pTestSprite) {
	CSprite *pSprite;                                   // pointer to current sprite
	CRect   myRect,                                     // rectangle occupied by current sprite
	        testRect,                                   // sprite retangle to be tested against
	        overlapRect;                                // area of overlap between rectangles

	if (!m_bIntercepts)                                 // punt if interceptions not allowed
		return nullptr;

	pSprite = pTestSprite;                              // get first sprite to be tested
	myRect = *newRect;                                  // acquire the rectangle for base sprite

	while (pSprite != nullptr) {                           // thumb through the sprite chain
		if ((m_nId != (*pSprite).m_nId) &&              // be sure to not test against ourself
		        (*pSprite).m_bIntercepts) {                 // ... and only test against overlapping sprites
			testRect = (*pSprite).m_cRect;              // sprites touch if their rectangles intersect
			if (overlapRect.IntersectRect(&myRect, &testRect))  // does our sprite overlap another?
				return (pSprite);                       // ... if so return a pointer to it
		}
		pSprite = (*pSprite).m_pNext;
	}                  // fetch next sprite in chain for testing

	return nullptr;
}


/*************************************************************************
 *
 * Interception()
 *
 * Parameters:
 *  CDC *pDC                pointer to device context where the sprite gets painted
 *  CSprite * pTestSprite   pointer to a lone test sprite or a sprite chain
 *
 * Return Value:
 *  CSprite *               pointer to sprite touched, or nullptr
 *
 * Description:     Determine whether a sprite will touch another sprite if
 *                  moved to its new location.  Simply determine whether
 *                  the defining rectangular areas of the sprites intersect
 *                  for basic sprites; use bit masking techniques for the
 *                  complex sprite images.
 *
 ************************************************************************/

CSprite *CSprite::Interception(CDC *pDC, CSprite * pTestSprite) {
	CSprite *pSprite = nullptr; // pointer to current sprite

	pSprite = pTestSprite;                              // get first sprite to be tested

	while (pSprite != nullptr) {                           // thumb through the entire sprite collection
		if (TestInterception(pDC, pSprite, nullptr))       // ... testing against each sprite in turn
			return (pSprite);                           // found an interception
		pSprite = (*pSprite).m_pNext;
	}                  // fetch next sprite in chain for testing

	return nullptr;
}


/*************************************************************************
 *
 * SpritesOverlap()
 *
 * Parameters:
 *  CDC *pDC            pointer to device context where the sprite gets painted
 *  CSprite * pSprite   pointer to a lone test sprite
 *  CPoint *pPoint     optional address of place to store estimated pixel overlap
 *
 * Return Value:
 *  bool                success/failure condition
 *
 * Description:     Determine whether a sprite touches another sprite by
 *                  checking for overlap in their monochrome bitmap masks
 *
 ************************************************************************/

bool CSprite::SpritesOverlap(CDC * pDC, CSprite * pSprite, CPoint *pPoint) {
	bool    bSuccess = false;                           // success/failure return status
	CRect   unionRect;                                  // rectangle enclosing old and new sprite locations
	CDC     *cDC1 = nullptr,                               // device context for sprite 1
	         *cDC2 = nullptr;                               // device context for sprite 2
	CBitmap *pBitmap1 = nullptr,                           // bitmap for context 1
	         *pBitmap2 = nullptr,                           // bitmap for context 2
	          *pBitmap1Old = nullptr,                        // bitmap previously mapped to context 1
	           *pBitmap2Old = nullptr;                        // bitmap previously mapped to context 2
	byte    *chPixels = nullptr;                           // buffer for holding pixel values
	int     dx, dy,                                     // dimensions of context bitmaps
	        i, j;
	size_t  stN;
	uint32   dwN; //, dwByte;                               // delta sizes of work area's bitmap
	BITMAP  cBitmapData;
	int     bx, by, bdx, bdy;

	if (!m_bVisible || !(*pSprite).m_bVisible)          // punt if not visible
		return false;

	if (pDC == nullptr)                                    // punt if no output context
		return false;

	unionRect.UnionRect(m_cRect, (*pSprite).m_cRect);   // calculate the smallest enclosing rectangle that
	dx = unionRect.right - unionRect.left;              // ... contains the bitmap area where the sprite was
	dy = unionRect.bottom - unionRect.top;              // ... and the bitmap area where it will be next

	dwN = stN = ((dx + 15) >> 3) * dy;                    // calculate the amount of memory that a bitmap mask
	chPixels = (byte *) calloc((size_t) 1, stN);        // ... will occupy and allocation that amount of space
	if (!chPixels)
		return false;

	cDC1 = new CDC();                                   // get objects for the offscreen bitmaps
	cDC2 = new CDC();                                   // ... i.e. for the contexts and bitmaps themselves
	pBitmap1 = new CBitmap();
	pBitmap2 = new CBitmap();

	if ((cDC1 != nullptr) &&                               // verify we got the objects we asked for
	        (cDC2 != nullptr) &&
	        (*cDC1).CreateCompatibleDC(pDC) &&              // create the actual device contexts
	        (*cDC2).CreateCompatibleDC(pDC) &&
	        (*pBitmap1).CreateBitmap(dx, dy, 1, 1, chPixels) && // create the actual bitmaps
	        (*pBitmap2).CreateBitmap(dx, dy, 1, 1, chPixels)) {
		pBitmap1Old = (*cDC1).SelectObject(pBitmap1);   // map the bitmaps into the contexts
		pBitmap2Old = (*cDC2).SelectObject(pBitmap2);
		if ((pBitmap1Old != nullptr) &&
		        (pBitmap2Old != nullptr)) {
			bx = m_cPosition.x - unionRect.left;        // get positioning information for sprite 1
			by = m_cPosition.y - unionRect.top;
			bdx = m_cSize.cx;
			bdy = m_cSize.cy;
			bSuccess = SetupMask(pDC);
			if (bSuccess) {
				(*cDC1).BitBlt(bx, by, bdx, bdy, m_pMaskDC, m_cImageRect.left, m_cImageRect.top, NOTSRCCOPY); // get sprite 1's mask
				if (!m_bRetainContexts)
					ReleaseMaskContext();
				bx = (*pSprite).m_cPosition.x - unionRect.left;     // get positioning information for sprite 2
				by = (*pSprite).m_cPosition.y - unionRect.top;
				bdx = (*pSprite).m_cSize.cx;
				bdy = (*pSprite).m_cSize.cy;
				bSuccess = (*pSprite).SetupMask(pDC);
				if (bSuccess) {
					(*cDC2).BitBlt(bx, by, bdx, bdy, (*pSprite).m_pMaskDC, (*pSprite).m_cImageRect.left, (*pSprite).m_cImageRect.top, NOTSRCCOPY); // get sprite 2's mask
					if (!(*pSprite).m_bRetainContexts)
						(*pSprite).ReleaseMaskContext();
					(*cDC1).BitBlt(0, 0, dx, dy, cDC2, 0, 0, SRCAND); // logically AND the masks together
					(*cDC1).SelectObject(pBitmap1Old);            // ... leaving bits set where they overlap
					pBitmap1Old = nullptr;
					(*pBitmap1).GetBitmapBits(dwN, chPixels); // fetch the image we created
					(*pBitmap1).GetObject(sizeof(BITMAP), &cBitmapData); // .. get the scanline length
					bSuccess = false;
					for (i = 0; i < cBitmapData.bmHeight; i++) {        // ... and look for a byte that is nonzero
						for (j = 0; j < cBitmapData.bmWidthBytes; j++) {// ... in which case we have an image overlap
							if (chPixels[(i * cBitmapData.bmWidthBytes) + j] != 0) {
								bSuccess = true;
								if (pPoint != nullptr) {                   // estimate point of intersection
									(*pPoint).x = (j * 8) - (m_cPosition.x - unionRect.left);
									(*pPoint).y = i - (m_cPosition.y - unionRect.top);
									if ((*pPoint).x < 0)
										(*pPoint).x = 0;
									if ((*pPoint).y < 0)
										(*pPoint).y = 0;
								}
								break;
							}
						}
					}
				}
			}
		}
	}

	if (chPixels != nullptr)
		free(chPixels);                                 // free up the work area's bitmap

	if (pBitmap1Old != nullptr)                            // map out the bitmaps we used
		(*cDC1).SelectObject(pBitmap1Old);
	if (pBitmap2Old != nullptr)
		(*cDC2).SelectObject(pBitmap2Old);

	if (pBitmap1 != nullptr) {                             // delete the bitmap resources and objects
		(*pBitmap1).DeleteObject();
		delete pBitmap1;
	}
	if (pBitmap2 != nullptr) {
		(*pBitmap2).DeleteObject();
		delete pBitmap2;
	}

	if (cDC1 != nullptr) {                                 // release the contexts and delete the objects
		(*cDC1).DeleteDC();
		delete cDC1;
	}
	if (cDC2 != nullptr) {
		(*cDC2).DeleteDC();
		delete cDC2;
	}

	return bSuccess;                                  // return success/failure
}


/*************************************************************************
 *
 * Touched()
 *
 * Parameters:
 *  CPoint testPoint    X/Y point against which to test
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Determine whether a sprite is touched by the given point
 *
 ************************************************************************/

bool CSprite::Touching(CPoint myPoint) {
	if (m_bIntercepts &&                                // ignoring sprites that don't intercept
	        m_cRect.PtInRect(myPoint))                      // see if the point is in the sprite's rectangle
		return true;                                  // ... and if so, return success

	return false;
}


/*************************************************************************
 *
 * Touched()
 *
 * Parameters:
 *  CPoint testPoint    X/Y point against which to test succeeding sprites
 *  CSprite *           pointer to sprite with which to begin testing
 *
 * Return Value:
 *  CSprite *       pointer to sprite touched, or nullptr
 *
 * Description:     Determine whether a sprite is touched by the given point,
 *                  if not test against succeeding members in its chain.
 *
 ************************************************************************/

CSprite *CSprite::Touched(CPoint myPoint, CSprite *pSprite) {
	CRect   testRect;                                   // sprite area to be tested

	while (pSprite != nullptr) {                           // thumb through the entire sprite collection
		if ((*pSprite).m_bIntercepts) {                 // ... ignoring sprites that don't intercept
			testRect = (*pSprite).m_cRect;
			if (testRect.PtInRect(myPoint))             // See if the point is in the sprite's rectangle
				return (pSprite);                       // ... and if so, return a pointer to it
		}
		pSprite = (*pSprite).m_pNext;
	}                  // fetch next sprite for testing

	return nullptr;
}


/*************************************************************************
 *
 * SetPosition()
 *
 * Parameters:
 *  int x           coordinate which identifies where to begin painting
 *  int y           the sprite, and which corresponds to the upper lefthand
 *                  corner location of the sprite's bitmap
 *
 * Return Value:    none
 *
 * Description:     Establish the new location, and encompassing rectangular
 *                  area, for the sprite.
 *
 ************************************************************************/

void CSprite::SetPosition(int x, int y) {
	m_bPositioned = true;                                   // now have a real location
	m_cPosition.x = x;                                      // establish the new location of the sprite
	m_cPosition.y = y;                                      // ... and setup the bitmap's bounding rectangle
	m_cRect.SetRect(m_cPosition.x, m_cPosition.y, m_cPosition.x + m_cSize.cx, m_cPosition.y + m_cSize.cy);
}


/*************************************************************************
 *
 * CropImage()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context used by the sprite
 *  CRect *pRect    pointer of rectangular area to be cropped;
 *                  position is relative to upper left hand corner
 *                  of sprite image which is the origin (0,0)
 *
 *  bool            success/failure condition
 *
 * Description:     Crop a rectangular area from a sprite's image;
 *                  i.e. turn it black so that the background art
 *                  shows through, and make the corresponding area
 *                  in the mask be white.
 *
 ************************************************************************/

bool CSprite::CropImage(CDC *pDC, CRect *pRect) {
	CRect       myRect;
	CBrush      myBrush;
	bool        bSuccess = false;

	myRect = *pRect;                                // offset crop area by image rect
	myRect.left += m_cImageRect.left;
	myRect.right += m_cImageRect.left;

	if (m_pImage != nullptr) {
		if (SetupImage(pDC)) {                      // blacken image section to crop it
			myBrush.CreateStockObject(BLACK_BRUSH);
			(*m_pImageDC).FillRect(&myRect, &myBrush);
			if (!m_bMasked || (m_pMask == nullptr))
				bSuccess = true;
			else {
				bSuccess = SetupMask(pDC);          // whiten mask section to crop it
				myBrush.CreateStockObject(WHITE_BRUSH);
				(*m_pMaskDC).FillRect(&myRect, &myBrush);
			}
			if (bSuccess && m_bHaveBackdrop) {
				if (GetBackdropDC(pDC) == nullptr)
					bSuccess = false;
				else {
					bSuccess = (*pDC).BitBlt(
					               m_cRect.left + (*pRect).left,
					               m_cRect.top + (*pRect).top,
					               (*pRect).right - (*pRect).left,
					               (*pRect).bottom - (*pRect).top,
					               m_pBackdropDC,
					               m_cRect.left + (*pRect).left,
					               m_cRect.top + (*pRect).top,
					               SRCCOPY);
					ReleaseBackdropDC();
				}
			} else if (bSuccess && (m_pBackground != nullptr)) {
				bSuccess = SetupBackground(pDC);    // update background with cropped area
				if (bSuccess) {
					bSuccess = (*pDC).BitBlt(
					               m_cRect.left + (*pRect).left,
					               m_cRect.top + (*pRect).top,
					               (*pRect).right - (*pRect).left,
					               (*pRect).bottom - (*pRect).top,
					               m_pBackgroundDC,
					               (*pRect).left,
					               (*pRect).top,
					               SRCCOPY);
					ReleaseBackgroundContext();
				}
			}
		}
	}

	if (!m_bRetainContexts) {
		ReleaseImageContext();
		ReleaseMaskContext();
	}

	return bSuccess;
}


/*************************************************************************
 *
 * CreateMask()
 *
 * Parameters:
 *  CDC *myDC       pointer to device context used by the sprite
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Create a monochrome bitmap mask for the sprite, with
 *                  white (1s) for the image, and black (0s) for the background.
 *
 ************************************************************************/

bool CSprite::CreateMask(CDC *pDC) {
	bool    bHaveImageContext = false,                  // keep track of whether contexts exist
	        bHaveMaskContext = false;
	CSize   mySize;

	if (!m_bVisible)                                    // punt if not visible
		return false;

	if (m_pMask != nullptr)                                // done if already have a mask
		return true;

	if (!m_bMasked ||                                   // fail if not masked or, no image from
	        (m_pImage == nullptr) ||                           // ... which to obtain a mask, or no
	        (pDC == nullptr))                                  // ... output context
		return false;

	if (m_pImageDC != nullptr)                             // see if contexts already exist
		bHaveImageContext = true;
	if (m_pMaskDC != nullptr)
		bHaveMaskContext = true;

	mySize.cx = m_nCelCount ? m_cSize.cx * m_nCelCount : m_cSize.cx;
	mySize.cy = m_cSize.cy;

	m_pMask = new CBitmap();                            // create object to hold the mask
	if ((m_pMask != nullptr) &&                            // verify that worked
	        SetupImage(pDC) &&                              // setup the image bitmap and context
	        CreateMaskContext(pDC)) {                       // create a context for the mask
		if ((*m_pMask).CreateBitmap(mySize.cx, mySize.cy, 1, 1, nullptr)) { // create mask bitmap
			m_pMaskOld = (*m_pMaskDC).SelectObject(m_pMask);    // map bitmap into context
			if (m_pMaskOld) {
				(*m_pMaskDC).BitBlt(0,            // copy in the image, doing an inversion
				                          0,              // ... so that we can mask out the sprite's
				                          mySize.cx,      // ... transparent area
				                          mySize.cy,
				                          m_pImageDC,
				                          0,
				                          0,
				                          NOTSRCCOPY);
				(*m_pImageDC).BitBlt(0,           // remove transparent area from sprite image
				                           0,
				                           mySize.cx,
				                           mySize.cy,
				                           m_pMaskDC,
				                           0,
				                           0,
				                           SRCAND);
				(*m_pMaskDC).BitBlt(0,            // invert mask again so it can be used later
				                          0,              // ... to mask away the sprite image area
				                          mySize.cx,      // ... from the background for each new
				                          mySize.cy,      // ... destination
				                          m_pMaskDC,
				                          0,
				                          0,
				                          DSTINVERT);
				if (!bHaveImageContext)                 // release contexts if not optimizing
					ReleaseImageContext();
				if (!bHaveMaskContext)
					ReleaseMaskContext();
				return true;
			}
		}
	}

	if (!bHaveImageContext)                             // release contexts if they didn't
		ReleaseImageContext();                          // ... already exist
	if (!bHaveMaskContext)
		ReleaseMaskContext();

	if (m_pMask != nullptr) {                              // delete mask bitmap since we failed
		delete m_pMask;
		m_pMask = nullptr;
	}

	return false;
}


/*************************************************************************
 *
 * CreateBackground()
 *
 * Parameters:
 *  CDC *myDC       pointer to device context used by the sprite
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Create an offscreen bitmap to hold the background image
 *                  that the sprite covers up.
 *
 ************************************************************************/

bool CSprite::CreateBackground(CDC *pDC) {
	if (!m_bVisible || m_bHaveBackdrop)                 // punt if not visible or have backdrop
		return false;

	if (m_pBackground != nullptr)                          // done if already have a background
		return true;

	if (pDC != nullptr) {                                  // create an object to hold things
		m_pBackground = new CBitmap();
		if (m_pBackground != nullptr) {                    // create the background bitmap
			if ((*m_pBackground).CreateCompatibleBitmap(pDC, m_cSize.cx, m_cSize.cy))
				return true;
			delete m_pBackground;                       // tear things down if we failed
			m_pBackground = nullptr;
		}
	}

	return false;
}


/*************************************************************************
 *
 * SetupImage()
 *
 * Parameters:
 *  CDC *myDC       pointer to device context used by the sprite
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Establish a device context to hold the sprite's image
 *                  and then map in its palette and bitmap.
 *
 ************************************************************************/

bool CSprite::SetupImage(CDC * pDC) {
	bool    bHaveContext = false;                       // whether we already have a context

	if (!m_bVisible)                                    // punt if not visible
		return false;

	if ((m_pImage != nullptr) &&                           // punt if no image bitmap
	        (pDC != nullptr)) {                                // ... or no output context

		if (m_pImageDC != nullptr)                         // see if already have a context
			bHaveContext = true;

		if (CreateImageContext(pDC)) {                  // create a context for the image
			if (m_pImageOld == nullptr)                    // ... then map in the bitmap
				m_pImageOld = (*m_pImageDC).SelectObject(m_pImage);
			if (m_pImageOld != nullptr)
				return true;
		}

		if (!bHaveContext)                              // release context if we didn't
			ReleaseImageContext();                      // ... have it already
	}

	return false;
}


/*************************************************************************
 *
 * SetupBackground()
 *
 * Parameters:
 *  CDC *myDC       pointer to device context used by the sprite
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Establish a device context to hold the sprite's Background
 *                  and then map in its palette and bitmap.
 *
 ************************************************************************/

bool CSprite::SetupBackground(CDC * pDC) {
	bool    bHaveContext = false;                       // whether there is already a context

	if (!m_bVisible || m_bHaveBackdrop)                 // punt if not visible or have backdrop
		return false;

	if ((m_pBackground != nullptr) &&                      // verify there is a background bitmap
	        (pDC != nullptr)) {                                // ... and an output context

		if (m_pBackgroundDC != nullptr)                    // see if a context already exists
			bHaveContext = true;

		if (CreateBackgroundContext(pDC)) {             // create a context for
			if (m_pBackgroundOld == nullptr)               // ... and map in the bitmap
				m_pBackgroundOld = (*m_pBackgroundDC).SelectObject(m_pBackground);
			if (m_pBackgroundOld != nullptr)
				return true;
		}

		if (!bHaveContext)                              // release context if we didn't
			ReleaseBackgroundContext();                 // ... have it already
	}

	return false;
}


/*************************************************************************
 *
 * SetupMask()
 *
 * Parameters:
 *  CDC *myDC       pointer to device context used by the sprite
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Establish a device context to hold the sprite's Mask
 *                  and then map in its bitmap.
 *
 ************************************************************************/

bool CSprite::SetupMask(CDC * pDC) {
	bool    bHaveContext = false;                       // whether we have a context

	if (!m_bVisible)                                    // punt if not visible
		return false;

	if (m_bMasked &&                                    // verify we have a bitmap
	        (pDC != nullptr)) {                                // ... and an output context

		if (m_pMaskDC != nullptr)                          // see if we already have a context
			bHaveContext = true;

		if (CreateMaskContext(pDC) &&                   // create a context for the mask
		        CreateMask(pDC)) {                          // create the mask bitmap and content
			if (m_pMaskOld == nullptr)                     // map bitmap into context
				m_pMaskOld = (*m_pMaskDC).SelectObject(m_pMask);
			if (m_pMaskOld != nullptr)
				return true;
		}

		if (!bHaveContext)                              // release context if we didn't
			ReleaseMaskContext();                       // ... already have it
	}

	return false;
}


/*************************************************************************
 *
 * CreateImageContext()
 *
 * Parameters:
 *  CDC *myDC       pointer to device context used by the sprite
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Create a device context to hold the sprite's image.
 *
 ************************************************************************/

bool CSprite::CreateImageContext(CDC * pDC) {
	if (!m_bVisible)                                    // punt if not visible
		return false;

	if (pDC != nullptr) {                                  // verify output context
		if (m_pImageDC == nullptr) {                       // if we don't already have
			m_pImageDC = new CDC();                     // ... a context for the image
			if (!(*m_pImageDC).CreateCompatibleDC(pDC)) // ... we create one now
				goto Error;
		}
		if ((m_pPalette != nullptr) && (m_pPalImageOld == nullptr)) { // map in palette if present
			m_pPalImageOld = (*m_pImageDC).SelectPalette(m_pPalette, false);
			(*m_pImageDC).RealizePalette();
		}
		return true;
	}

Error:

	ReleaseImageContext();                              // failed, so release the context

	return false;
}


/*************************************************************************
 *
 * CreateBackgroundContext()
 *
 * Parameters:
 *  CDC *myDC       pointer to device context used by the sprite
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Create a device context to hold the sprite's background.
 *
 ************************************************************************/

bool CSprite::CreateBackgroundContext(CDC * pDC) {
	if (!m_bVisible || m_bHaveBackdrop)                 // punt if not visible or have backdrop
		return false;

	if (pDC != nullptr) {                                  // verify the output context
		if (m_pBackgroundDC == nullptr) {                  // if we don't have a context
			m_pBackgroundDC = new CDC();                // ... then we create one now
			if (!(*m_pBackgroundDC).CreateCompatibleDC(pDC))
				goto Error;
		}
		if ((m_pPalette != nullptr) && (m_pPalBackOld == nullptr)) {  // map in palette if present
			m_pPalBackOld = (*m_pBackgroundDC).SelectPalette(m_pPalette, false);
			(*m_pBackgroundDC).RealizePalette();
		}
		return true;
	}

Error:

	ReleaseBackgroundContext();                         // failed, so release the context

	return false;
}


/*************************************************************************
 *
 * CreateMaskContext()
 *
 * Parameters:
 *  CDC *myDC       pointer to device context used by the sprite
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     Create a device context to hold the sprite's mask.
 *
 ************************************************************************/

bool CSprite::CreateMaskContext(CDC * pDC) {
	if (!m_bVisible)                                    // punt if not visible
		return false;

	if (m_bMasked &&                                    // verify it is a masked sprite
	        (pDC != nullptr)) {                                // ... and we have an output context
		if (m_pMaskDC != nullptr)                          // done if already have a context
			return true;
		m_pMaskDC = new CDC();                          // create an object for the context
		if ((m_pMaskDC != nullptr) &&                      // ... then create the context itself
		        (*m_pMaskDC).CreateCompatibleDC(pDC))
			return true;
		ReleaseMaskContext();
	}                          // release the context if we failed

	return false;
}


/*************************************************************************
 *
 * ReleaseImageContext()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Release the sprite's image context.
 *
 ************************************************************************/

void CSprite::ReleaseImageContext() {
	if (!m_bVisible)                                    // punt if not visible
		return;

	if (m_pImageDC != nullptr) {                           // if there is an image device context
		if (m_pImageOld != nullptr)                        // ... map out existing bitmap
			(*m_pImageDC).SelectObject(m_pImageOld);
		if (m_pPalImageOld != nullptr)                     // ... map out the palette
			(*m_pImageDC).SelectPalette(m_pPalImageOld, false);
		(*m_pImageDC).DeleteDC();                       // ... release the context
		delete m_pImageDC;                              // ... then delete the device context

		m_pImageOld = nullptr;
		m_pPalImageOld = nullptr;
		m_pImageDC = nullptr;
	}
}


/*************************************************************************
 *
 * ReleaseBackgroundContext()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Release the sprite's background context.
 *
 ************************************************************************/

void CSprite::ReleaseBackgroundContext() {
	if (!m_bVisible || m_bHaveBackdrop)                 // punt if not visible or have backdrop
		return;

	if (m_pBackgroundDC != nullptr) {                      // if there is a background device context
		if (m_pBackgroundOld != nullptr) {                 // ... map out existing bitmap
			(*m_pBackgroundDC).SelectObject(m_pBackgroundOld);
			m_pBackgroundOld = nullptr;
		}
		if (m_pPalBackOld != nullptr) {                    // ... map out the palette
			(*m_pBackgroundDC).SelectPalette(m_pPalBackOld, false);
			m_pPalBackOld = nullptr;
		}
		(*m_pBackgroundDC).DeleteDC();                  // ... release the context
		delete m_pBackgroundDC;
		m_pBackgroundDC = nullptr;
	}                        // ... then delete the device context
}


/*************************************************************************
 *
 * ReleaseMaskContext()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Release the sprite's mask context.
 *
 ************************************************************************/

void CSprite::ReleaseMaskContext() {
	if (!m_bVisible)                                    // punt if not visible
		return;

	if (m_bMasked &&
	        (m_pMaskDC != nullptr)) {                          // if there is a mask device context
		if (m_pMaskOld != nullptr)                         // ... map out existing bitmap
			(*m_pMaskDC).SelectObject(m_pMaskOld);
		(*m_pMaskDC).DeleteDC();                        // ... release the context
		delete m_pMaskDC;                               // ... then delete the device context

		m_pMaskOld = nullptr;
		m_pMaskDC = nullptr;
	}
}


/*************************************************************************
 *
 * ClearImage()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destroy the sprite's image bitmap and context
 *
 ************************************************************************/

void CSprite::ClearImage() {
	if (!m_bVisible)                                    // punt if not visible
		return;

	if (!m_bDuplicated) {
		ReleaseImageContext();                          // release the imnage context

		if (m_pImage != nullptr) {                         // destroy the image bitmap object
			(*m_pImage).DeleteObject();                 // ... if present
			delete m_pImage;
		}
	}

	m_pImage = nullptr;
}


/*************************************************************************
 *
 * ClearBackgrounds()
 *
 * Parameters:
 *  CDC *pDC        pointer to device context where the sprites are painted
 *
 * Return Value:
 *  bool            success/failure condition
 *
 * Description:     For each sprite in the sprite chain, clear the sprite's
 *                  background bitmap.
 *
 ************************************************************************/

void CSprite::ClearBackgrounds() {
	CSprite *pSprite = nullptr;

	pSprite = m_pSpriteChain;

	while (pSprite != nullptr) {
		(*pSprite).ClearBackground();
		pSprite = (*pSprite).m_pNext;
	}
}


/*************************************************************************
 *
 * ClearBackground()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destroy the sprite's background bitmap and context
 *
 ************************************************************************/

void CSprite::ClearBackground() {
	m_nZPosition = m_nZOrder;                           // reset z ordering
	m_bOverlaps = false;                                // no longer overlaps other sprites
	m_bPositioned = false;                              // no longer has a real position

	if (!m_bVisible || m_bHaveBackdrop)                 // punt if not visible or have backdrop
		return;

	ReleaseBackgroundContext();                         // release the background context

	if (m_pBackground != nullptr) {                        // destroy the backgrond bitmap object
		(*m_pBackground).DeleteObject();                // ... if present
		delete m_pBackground;
		m_pBackground = nullptr;
	}
}


/*************************************************************************
 *
 * SetRetainBackground()
 *
 * Parameters:      true / false
 *
 * Return Value:    none
 *
 * Description:     set for retaining or ignoring background images for updates,
 *                  and clear away existing background image if required.
 *
 ************************************************************************/

void CSprite::SetRetainBackground(bool bValue) {
	if (!bValue)                                        // if not retaining backgrounds now
		ClearBackground();                              // ... then clear what we have

	m_bRetainBackground = bValue;                       // set new state
}


/*************************************************************************
 *
 * ClearMask()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destroy the sprite's mask bitmap and context
 *
 ************************************************************************/

void CSprite::ClearMask() {
	if (!m_bVisible)                                    // punt if not visible
		return;

	if (m_bMasked && !m_bDuplicated) {
		ReleaseMaskContext();                           // release mask context

		if (m_pMask != nullptr) {                          // destroy the Mask bitmap resource and object
			(*m_pMask).DeleteObject();                  // ... if present
			delete m_pMask;
		}
	}

	m_pMask = nullptr;
}


/*************************************************************************
 *
 * ClearPalette()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     Destroy the sprite's palette resource if it is not
 *                  shared by other sprites.
 *
 ************************************************************************/

void CSprite::ClearPalette() {
	if ((m_pPalette != nullptr) && !m_bSharedPalette) {            // explicitly delete the palette resource
		(*m_pPalette).DeleteObject();                           // ... if it is not shared by other sprites
		delete m_pPalette;
	}

	m_bSharedPalette = false;
	m_pPalette = nullptr;
}


/*************************************************************************
 *
 * GetBackdropDC()
 *
 * Parameters:      none
 *
 * Return Value:
 *  CDC *           pointer to device context or nullptr
 *
 * Description:     setup the device context for the backdrop.
 *
 ************************************************************************/

CDC *CSprite::GetBackdropDC(CDC *pDC) {
	if (m_pBackdropDC != nullptr)                              // punt if no context
		return (m_pBackdropDC);

	m_pBackdropDC = new CDC();
	if (!(*m_pBackdropDC).CreateCompatibleDC(pDC)) {
		delete m_pBackdropDC;
		m_pBackdropDC = nullptr;
		return nullptr;
	}

	if (m_pBackdropPalette != nullptr) {                       // map in the palette if present
		m_pBackdropPalOld = (*m_pBackdropDC).SelectPalette(m_pBackdropPalette, false);
		if (m_pBackdropPalOld == nullptr) {                     // punt if not successful
			delete m_pBackdropDC;
			m_pBackdropDC = nullptr;
			return nullptr;
		}
		(*m_pBackdropDC).RealizePalette();            // make the system use the palette
	}

	m_pBackdropOld = (*m_pBackdropDC).SelectObject(m_pBackdrop);    // map in our bitmap
	if (m_pBackdropOld == nullptr) {
		if (m_pBackdropPalOld != nullptr) {                    // map out palette on failure
			(*m_pBackdropDC).SelectPalette(m_pBackdropPalOld, false);
			m_pBackdropPalOld = nullptr;
			delete m_pBackdropDC;
			m_pBackdropDC = nullptr;
		}
		return nullptr;
	}

	return (m_pBackdropDC);                                 // send back the context
}


/*************************************************************************
 *
 * ReleaseBackdropDC()
 *
 * Parameters:      none
 *
 * Return Value:    none
 *
 * Description:     release the backdrop device context resources.
 *
 ************************************************************************/

void CSprite::ReleaseBackdropDC() {
	if (m_pBackdropOld != nullptr) {                       // map back the previous bitmap
		(*m_pBackdropDC).SelectObject(m_pBackdropOld);
		m_pBackdropOld = nullptr;
	}

	if (m_pBackdropPalOld != nullptr) {                    // map back the previous palette
		(*m_pBackdropDC).SelectPalette(m_pBackdropPalOld, false);
		m_pBackdropPalOld = nullptr;
	}

	if (m_pBackdropDC != nullptr) {
		delete m_pBackdropDC;
		m_pBackdropDC = nullptr;
	}
}

} // namespace HodjNPodj
} // namespace Bagel
