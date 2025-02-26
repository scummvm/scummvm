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

#ifndef BAGEL_HODJNPODJ_BOFLIB_SPRITE_H
#define BAGEL_HODJNPODJ_BOFLIB_SPRITE_H

#include "bagel/mfc/afx.h"

namespace Bagel {
namespace HodjNPodj {

enum {
	COLOR_WHITE = 255		// RGB(255,255,255)
};

#define SPRITE_TOPMOST		0
#define SPRITE_FOREGROUND	64
#define SPRITE_MIDDLE		128
#define SPRITE_BACKGROUND	192
#define SPRITE_HINDMOST		255

class CSprite : public CObject {
	DECLARE_DYNCREATE(CSprite)

	// Constructors
public:
	CSprite();	// use "new" operator to create sprites, then LoadSprite

	// Destructors
public:
	~CSprite();

	// Implementation
public:
	CSprite *DuplicateSprite(CDC *pDC);
	BOOL DuplicateSprite(CDC *pDC, CSprite *pSprite);

	BOOL LoadSprite(CDC *pDC, const char *pszPathName);
	BOOL LoadSprite(CBitmap *pBitmap, CPalette *pPalette = NULL);
	BOOL LoadResourceSprite(CDC *pDC, const int resId);
	BOOL LoadResourceSprite(CDC *pDC, const char *pszName);

	BOOL LoadCels(CDC *pDC, const char *pszPathName, const int nCels);
	BOOL LoadCels(CBitmap *pBitmap, const int nCels, CPalette *pPalette = NULL);
	BOOL LoadResourceCels(CDC *pDC, const int resId, const int nCels);
	BOOL LoadResourceCels(CDC *pDC, const char *pszName, const int nCels);

	BOOL SetPalette(CPalette *pPalette);
	BOOL SharePalette(CPalette *pPalette);
	BOOL GetShared() {
		return(m_bSharedPalette);
	}

	BOOL PaintSprite(CDC *pDC, const int x, const int y);
	BOOL PaintSprite(CDC *pDC, CPoint point) {
		return(PaintSprite(pDC, point.x, point.y));
	}

	BOOL SetupCels(const int nCels);
	void SetCel(const int nCelID);
	void UpdateCel(void) {
		SetCel(m_nCelID + 1);
	}

	BOOL CropImage(CDC *pDC, CRect *pRect);

	BOOL RefreshSprite(CDC *pDC) {
		return(PaintSprite(pDC, m_cPosition.x, m_cPosition.y));
	}
	BOOL RefreshBackground(CDC *pDC);

	BOOL EraseSprite(CDC *pDC);

	void ClearBackground(void);

	CSprite *Interception(CDC *pDC) {
		return(Interception(pDC, m_pSpriteChain));
	}
	CSprite *Interception(CDC *pDC, CSprite *pTestSprite);
	CSprite *Interception(CRect *newRect) {
		return(Interception(newRect, m_pSpriteChain));
	}
	CSprite *Interception(CRect *newRect, CSprite *pSprite);
	BOOL TestInterception(CDC *pDC, CSprite *pSprite, CPoint *pPoint = NULL);

	BOOL GetVisible(void) {
		return(m_bVisible);
	}

	void SetIntercepts(BOOL bValue) {
		m_bIntercepts = bValue;
	}
	BOOL GetIntercepts(void) {
		return(m_bIntercepts);
	}

	void SetPosition(int x, int y);
	void SetPosition(CPoint point) {
		SetPosition(point.x, point.y);
	}
	CPoint GetPosition(void) {
		return(m_cPosition);
	}

	void SetDelta(int x, int y) {
		m_cMovementDelta.x = x;
		m_cMovementDelta.y = y;
	}
	CPoint GetDelta(void) {
		return(m_cMovementDelta);
	}

	void SetHotspot(int x, int y) {
		m_cHotspot.x = x;
		m_cHotspot.y = y;
	}
	CPoint GetHotspot(void) {
		return(m_cHotspot);
	}

	CSize GetSize(void) {
		return(m_cSize);
	}

	CRect GetRect(void) {
		return(m_cRect);
	}
	CRect GetArea(void)				// obsolete - use GetRect
	{
		return(m_cRect);
	}

	void SetMasked(BOOL bValue) {
		m_bMasked = bValue;
	}
	BOOL GetMasked(void) {
		return(m_bMasked);
	}

	void SetMobile(BOOL bValue) {
		m_bMobile = bValue;
	}
	BOOL GetMobile(void) {
		return(m_bMobile);
	}

	void SetOptimizeSpeed(BOOL bValue) {
		m_bRetainContexts = FALSE & bValue;
	}
	BOOL GetOptimizeSpeed(void) {
		return(m_bRetainContexts);
	}

	void SetTypeCode(int nValue) {
		m_nType = nValue;
	}
	int GetTypeCode(void) {
		return(m_nType);
	}

	void SetData(CObject *pData) {
		m_pData = pData;
	}
	CObject *GetData(void) {
		return(m_pData);
	}

	int GetId(void) {
		return(m_nId);
	}

	void SetZOrder(int nValue) {
		m_nZOrder = nValue;
		m_nZPosition = nValue;
	}
	int GetZOrder(void) {
		return(m_nZOrder);
	}
	int GetZPosition(void) {
		return(m_nZPosition);
	}

	int GetCelCount(void) {
		return(m_nCelCount);
	}
	int GetCelIndex(void) {
		return(m_nCelID);
	}

	void SetAnimated(BOOL bAnimated) {
		m_bAnimated = bAnimated;
	}
	BOOL GetAnimated(void) {
		return(m_bAnimated);
	}

	void SetRetainBackground(BOOL bValue);
	BOOL GetRetainBackground(void) {
		return(m_bRetainBackground);
	}

	BOOL IsLinked(void) {
		return(m_bLinked);
	}
	void LinkSprite(void);
	void UnlinkSprite(void);
	CSprite *GetNextSprite(void) {
		return(m_pNext);
	}
	CSprite *GetPrevSprite(void) {
		return(m_pPrev);
	}

	BOOL Touching(CPoint myPoint);

	static	CSprite *Touched(CPoint myPoint) {
		return(Touched(myPoint, m_pSpriteChain));
	}
	static	CSprite *Touched(CPoint myPoint, CSprite *pSprite);
	static	BOOL InterceptOccurred(void) {
		return(m_bTouchedSprite);
	}
	static	CSprite *GetInterception(void) {
		return(m_pTouchedSprite);
	}

	static	CSprite *GetSpriteChain(void) {
		return(m_pSpriteChain);
	}
	static	BOOL EraseSprites(CDC *pDC);
	static	void ClearBackgrounds(void);
	static	void FlushSpriteChain(void);

	static	BOOL HaveBackdrop(void) {
		return(m_bHaveBackdrop);
	}
	static	CDC *GetBackdropDC(CDC *pDC);
	static	void ReleaseBackdropDC(void);
	static	BOOL SetBackdrop(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap);
	static  void ClearBackdrop(void);
	static	BOOL RefreshBackdrop(CDC *pDC, CPalette *pPalette);

private:
	BOOL UpdateSprite(CDC *pDC);

	BOOL SaveBackground(CDC *pDC);

	BOOL CreateBackground(CDC *pDC);
	BOOL CreateMask(CDC *pDC);

	BOOL SetupImage(CDC *pDC);
	BOOL SetupBackground(CDC *pDC);
	BOOL SetupMask(CDC *pDC);

	BOOL CreateImageContext(CDC *pDC);
	BOOL CreateBackgroundContext(CDC *pDC);
	BOOL CreateMaskContext(CDC *pDC);

	void ReleaseImageContext(void);
	void ReleaseBackgroundContext(void);
	void ReleaseMaskContext(void);

	void ClearImage(void);
	void ClearMask(void);
	void ClearPalette(void);

	BOOL SpritesOverlap(CDC *pDC, CSprite *pSprite, CPoint *pPoint = NULL);

	BOOL DoSpritePainting(CDC *pDC, CPoint cPoint);
	BOOL DoOptimizedPainting(CDC *pDC, CRect *pDst);
	BOOL DoOverlapPainting(CDC *pDC, CRect *myRect);
	BOOL ReconstructBackground(CDC *pDC, CRect *myRect);

private:
	CDC *m_pImageDC;		// offscreen bitmap device context for image
	CBitmap *m_pImage;			// bitmap for the sprite
	CBitmap *m_pImageOld;		// previous bitmap mapped in the DC
	CDC *m_pMaskDC;			// offscreen bitmap device context for mask
	CBitmap *m_pMask;			// bitmap for the sprite's mask
	CBitmap *m_pMaskOld;		// previous bitmap mapped in the DC
	CDC *m_pBackgroundDC;	// offscreen bitmap device context for background
	CBitmap *m_pBackground;		// bitmap for the sprite's background
	CBitmap *m_pBackgroundOld;	// previous bitmap mapped in the DC
	CPalette *m_pPalette;		// color palette for the sprite
	CPalette *m_pPalImageOld;	// previous palette mapped to image DC 
	CPalette *m_pPalBackOld;		// previous palette mapped to background DC 
	CPoint		m_cPosition;		// upper left corner of sprite on display
	CPoint		m_cMovementDelta;	// dx/dy increments for sprite movement
	CPoint		m_cHotspot;			// logical hotspot; e.g. for cursors
	CSize		m_cSize;			// dx/dy size of the sprite bitmap
	CRect		m_cRect;			// bounding rectangle on display
	CRect		m_cImageRect;		// bounding rectangle within image bitmap
	BOOL		m_bVisible;			// sprite has an image to be painted
	BOOL		m_bIntercepts;		// sprite can be detected by interception
	BOOL		m_bSharedPalette;	// palette is shared and not deleteable
	BOOL		m_bMasked;			// sprite is to be masked (ignore white)
	BOOL		m_bMobile;			// sprite is mobile or stationary
	BOOL		m_bOverlaps;		// sprite covers other sprites
	BOOL		m_bPaintOverlap;	// whether to paint sprite for overlaps
	BOOL		m_bRetainBackground;// retain background for screen updates
	BOOL		m_bRetainContexts;	// retain device contexts across calls
	BOOL		m_bDuplicated;		// shares bitmaps with some other sprite
	BOOL		m_bOverlapTest;		// used for positional testing
	BOOL		m_bPositioned;		// whether sprite has been positioned yet
	BOOL		m_bAnimated;		// whether cel advance occurs when painting
	BOOL		m_bLinked;			// whether sprite is linked into the chain
	int			m_nId;				// unique sprite identifier
	int			m_nType;			// user defined information
	CObject *m_pData;			// user defined data pointer
	int			m_nZOrder;			// foreground / background order
	int			m_nZPosition;		// foreground / background placement
	int			m_nCelID;			// index of current cel image
	int			m_nCelCount;		// number of cels in the animation strip
	CSprite *m_pNext;			// pointer to next sprite in master chain
	CSprite *m_pPrev;			// pointer to previous sprite in master chain
	CSprite *m_pZNext;			// pointer to next sprite in z chain
	CSprite *m_pZPrev;			// pointer to previous sprite in z chain

	static int		m_nIndex;			// generator of sprite identifiers
	static CSprite *m_pSpriteChain;	// pointer to linked chain of sprites
	static BOOL		m_bTouchedSprite;	// set when sprite painting intercepts another
	static CSprite *m_pTouchedSprite;	// sprite touched during painting operation
	static BOOL		m_bHaveBackdrop;	// whether we have a backdrop bitmap
	static CDC *m_pBackdropDC;		// context used for backdrop artwork
	static CBitmap *m_pBackdrop;		// pointer to backdrop bitmap for background
	static CBitmap *m_pBackdropOld;	// bitmap previously mapped to backdrop context
	static CPalette *m_pBackdropPalette;// palette mapped to backdrop context
	static CPalette *m_pBackdropPalOld;	// palette previously mapped to backdrop context
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
