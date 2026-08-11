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

#ifndef HODJNPODJ_HNPLIBS_SPRITE_H
#define HODJNPODJ_HNPLIBS_SPRITE_H

#include "bagel/hodjnpodj/globals.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"

namespace Bagel {
namespace HodjNPodj {

enum {
	COLOR_WHITE = RGB(255, 255, 255)
};

#define SPRITE_TOPMOST      0
#define SPRITE_FOREGROUND   64
#define SPRITE_MIDDLE       128
#define SPRITE_BACKGROUND   192
#define SPRITE_HINDMOST     255

class CSprite : public CObject {
	DECLARE_DYNCREATE(CSprite)

public:
	CSprite();  // use "new" operator to create sprites, then LoadSprite
	~CSprite();

	CSprite *DuplicateSprite(CDC *pDC);
	bool DuplicateSprite(CDC *pDC, CSprite *pSprite);

	bool LoadSprite(CDC *pDC, const char* pszPathName);
	bool LoadSprite(CBitmap *pBitmap, CPalette *pPalette = nullptr);
	bool LoadResourceSprite(CDC *pDC, const int resId);
	bool LoadResourceSprite(CDC *pDC, const char* pszName);

	bool LoadCels(CDC *pDC, const char* pszPathName, const int nCels);
	bool LoadCels(CBitmap *pBitmap, const int nCels, CPalette *pPalette = nullptr);
	bool LoadResourceCels(CDC *pDC, const int resId, const int nCels);
	bool LoadResourceCels(CDC *pDC, const char* pszName, const int nCels);

	bool SetPalette(CPalette *pPalette);
	bool SharePalette(CPalette *pPalette);
	bool GetShared() {
		return (m_bSharedPalette);
	}

	bool PaintSprite(CDC *pDC, const int x, const int y);
	bool PaintSprite(CDC *pDC, CPoint point) {
		return (PaintSprite(pDC, point.x, point.y));
	}

	bool SetupCels(const int nCels);
	void SetCel(const int nCelID);
	void UpdateCel() {
		SetCel(m_nCelID + 1);
	}

	bool CropImage(CDC *pDC, CRect *pRect);

	bool RefreshSprite(CDC *pDC) {
		return (PaintSprite(pDC, m_cPosition.x, m_cPosition.y));
	}
	bool RefreshBackground(CDC *pDC);

	bool EraseSprite(CDC *pDC);

	void ClearBackground();

	CSprite *Interception(CDC *pDC) {
		return (Interception(pDC, m_pSpriteChain));
	}
	CSprite *Interception(CDC *pDC, CSprite * pTestSprite);
	CSprite *Interception(CRect *newRect) {
		return (Interception(newRect, m_pSpriteChain));
	}
	CSprite *Interception(CRect *newRect, CSprite * pSprite);
	bool TestInterception(CDC *pDC, CSprite * pSprite, CPoint *pPoint = nullptr);

	bool GetVisible() {
		return (m_bVisible);
	}

	void SetIntercepts(bool bValue) {
		m_bIntercepts = bValue;
	}
	bool GetIntercepts() {
		return (m_bIntercepts);
	}

	void SetPosition(int x, int y);
	void SetPosition(CPoint point) {
		SetPosition(point.x, point.y);
	}
	CPoint GetPosition() const {
		return (m_cPosition);
	}

	void SetDelta(int x, int y) {
		m_cMovementDelta.x = x;
		m_cMovementDelta.y = y;
	}
	CPoint GetDelta() {
		return (m_cMovementDelta);
	}

	void SetHotspot(int x, int y) {
		m_cHotspot.x = x;
		m_cHotspot.y = y;
	}
	CPoint GetHotspot() {
		return (m_cHotspot);
	}

	CSize GetSize() {
		return (m_cSize);
	}

	CRect GetRect() {
		return (m_cRect);
	}
	CRect GetArea() {           // obsolete - use GetRect
		return (m_cRect);
	}

	void SetMasked(bool bValue) {
		m_bMasked = bValue;
	}
	bool GetMasked() {
		return (m_bMasked);
	}

	void SetMobile(bool bValue) {
		m_bMobile = bValue;
	}
	bool GetMobile() {
		return (m_bMobile);
	}

	void SetOptimizeSpeed(bool bValue) {
		m_bRetainContexts = false & bValue;
	}
	bool GetOptimizeSpeed() {
		return (m_bRetainContexts);
	}

	void SetTypeCode(int nValue) {
		m_nType = nValue;
	}
	int GetTypeCode() {
		return (m_nType);
	}

	void SetData(CObject *pData) {
		m_pData = pData;
	}
	CObject *GetData() {
		return (m_pData);
	}

	int GetId() {
		return (m_nId);
	}

	void SetZOrder(int nValue) {
		m_nZOrder = nValue;
		m_nZPosition = nValue;
	}
	int GetZOrder() {
		return (m_nZOrder);
	}
	int GetZPosition() {
		return (m_nZPosition);
	}

	int GetCelCount() {
		return (m_nCelCount);
	}
	int GetCelIndex() {
		return (m_nCelID);
	}

	void SetAnimated(bool bAnimated) {
		m_bAnimated = bAnimated;
	}
	bool GetAnimated() {
		return (m_bAnimated);
	}

	void SetRetainBackground(bool bValue);
	bool GetRetainBackground() {
		return (m_bRetainBackground);
	}

	bool IsLinked() {
		return (m_bLinked);
	}
	void LinkSprite();
	void UnlinkSprite();
	CSprite *GetNextSprite() {
		return (m_pNext);
	}
	CSprite *GetPrevSprite() {
		return (m_pPrev);
	}

	bool Touching(CPoint myPoint);

	static  CSprite *Touched(CPoint myPoint) {
		return (Touched(myPoint, m_pSpriteChain));
	}
	static  CSprite *Touched(CPoint myPoint, CSprite *pSprite);
	static  bool InterceptOccurred() {
		return (m_bTouchedSprite);
	}
	static  CSprite *GetInterception() {
		return (m_pTouchedSprite);
	}

	static  CSprite *GetSpriteChain() {
		return (m_pSpriteChain);
	}
	static  bool EraseSprites(CDC *pDC);
	static  void ClearBackgrounds();
	static  void FlushSpriteChain();

	static  bool HaveBackdrop() {
		return (m_bHaveBackdrop);
	}
	static  CDC *GetBackdropDC(CDC *pDC);
	static  void ReleaseBackdropDC();
	static  bool SetBackdrop(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap);
	static  void ClearBackdrop();
	static  bool RefreshBackdrop(CDC *pDC, CPalette *pPalette);

private:
	bool UpdateSprite(CDC *pDC);

	bool SaveBackground(CDC *pDC);

	bool CreateBackground(CDC *pDC);
	bool CreateMask(CDC *pDC);

	bool SetupImage(CDC *pDC);
	bool SetupBackground(CDC *pDC);
	bool SetupMask(CDC *pDC);

	bool CreateImageContext(CDC *pDC);
	bool CreateBackgroundContext(CDC *pDC);
	bool CreateMaskContext(CDC *pDC);

	void ReleaseImageContext();
	void ReleaseBackgroundContext();
	void ReleaseMaskContext();

	void ClearImage();
	void ClearMask();
	void ClearPalette();

	bool SpritesOverlap(CDC *pDC, CSprite *pSprite, CPoint *pPoint = nullptr);

	bool DoSpritePainting(CDC *pDC, CPoint cPoint);
	bool DoOptimizedPainting(CDC *pDC, CRect *pDst);
	bool DoOverlapPainting(CDC *pDC, CRect *myRect);
	bool ReconstructBackground(CDC *pDC, CRect *myRect);

private:
	CDC         *m_pImageDC;        // offscreen bitmap device context for image
	CBitmap     *m_pImage;          // bitmap for the sprite
	CBitmap     *m_pImageOld;       // previous bitmap mapped in the DC
	CDC         *m_pMaskDC;         // offscreen bitmap device context for mask
	CBitmap     *m_pMask;           // bitmap for the sprite's mask
	CBitmap     *m_pMaskOld;        // previous bitmap mapped in the DC
	CDC         *m_pBackgroundDC;   // offscreen bitmap device context for background
	CBitmap     *m_pBackground;     // bitmap for the sprite's background
	CBitmap     *m_pBackgroundOld;  // previous bitmap mapped in the DC
	CPalette    *m_pPalette;        // color palette for the sprite
	CPalette    *m_pPalImageOld;    // previous palette mapped to image DC
	CPalette    *m_pPalBackOld;     // previous palette mapped to background DC
	CPoint      m_cPosition;        // upper left corner of sprite on display
	CPoint      m_cMovementDelta;   // dx/dy increments for sprite movement
	CPoint      m_cHotspot;         // logical hotspot; e.g. for cursors
	CSize       m_cSize;            // dx/dy size of the sprite bitmap
	CRect       m_cRect;            // bounding rectangle on display
	CRect       m_cImageRect;       // bounding rectangle within image bitmap
	bool        m_bVisible;         // sprite has an image to be painted
	bool        m_bIntercepts;      // sprite can be detected by interception
	bool        m_bSharedPalette;   // palette is shared and not deleteable
	bool        m_bMasked;          // sprite is to be masked (ignore white)
	bool        m_bMobile;          // sprite is mobile or stationary
	bool        m_bOverlaps;        // sprite covers other sprites
	bool        m_bPaintOverlap;    // whether to paint sprite for overlaps
	bool        m_bRetainBackground;// retain background for screen updates
	bool        m_bRetainContexts;  // retain device contexts across calls
	bool        m_bDuplicated;      // shares bitmaps with some other sprite
	bool        m_bOverlapTest;     // used for positional testing
	bool        m_bPositioned;      // whether sprite has been positioned yet
	bool        m_bAnimated;        // whether cel advance occurs when painting
	bool        m_bLinked;          // whether sprite is linked into the chain
	int         m_nId;              // unique sprite identifier
	int         m_nType;            // user defined information
	CObject     *m_pData;           // user defined data pointer
	int         m_nZOrder;          // foreground / background order
	int         m_nZPosition;       // foreground / background placement
	int         m_nCelID;           // index of current cel image
	int         m_nCelCount;        // number of cels in the animation strip
	CSprite     *m_pNext;           // pointer to next sprite in master chain
	CSprite     *m_pPrev;           // pointer to previous sprite in master chain
	CSprite     *m_pZNext;          // pointer to next sprite in z chain
	CSprite     *m_pZPrev;          // pointer to previous sprite in z chain

	static int      m_nIndex;           // generator of sprite identifiers
	static CSprite  *m_pSpriteChain;    // pointer to linked chain of sprites
	static bool     m_bTouchedSprite;   // set when sprite painting intercepts another
	static CSprite  *m_pTouchedSprite;  // sprite touched during painting operation
	static bool     m_bHaveBackdrop;    // whether we have a backdrop bitmap
	static CDC      *m_pBackdropDC;     // context used for backdrop artwork
	static CBitmap  *m_pBackdrop;       // pointer to backdrop bitmap for background
	static CBitmap  *m_pBackdropOld;    // bitmap previously mapped to backdrop context
	static CPalette *m_pBackdropPalette;// palette mapped to backdrop context
	static CPalette *m_pBackdropPalOld; // palette previously mapped to backdrop context
};

} // namespace HodjNPodj
} // namespace Bagel

#endif
