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

#ifndef BAGEL_BOFLIB_GFX_SPRITE_H
#define BAGEL_BOFLIB_GFX_SPRITE_H

#include "bagel/boflib/gfx/bitmap.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/gfx/palette.h"
#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/debug.h"

namespace Bagel {

#define SPRITE_TOPMOST 0
#define SPRITE_FOREGROUND 64
#define SPRITE_MIDDLE 128
#define SPRITE_BACKGROUND 192
#define SPRITE_HINDMOST 255

class CBofSprite : public CBofError, public CBofObject, public CLList {
public:
	// Constructors
	CBofSprite(); // use "new" operator to create sprites, then LoadSprite

	// Destructors
	virtual ~CBofSprite();

	//////////////////////////////////////////

	// Implementation
	CBofSprite *DuplicateSprite();
	BOOL DuplicateSprite(CBofSprite *pSprite);

	BOOL LoadSprite(const CHAR *pszPathName, INT nCels = 1);
	BOOL LoadSprite(CBofBitmap *pBitmap, INT nCels = 1);

	BOOL PaintSprite(CBofBitmap *pBmp, const INT x, const INT y);
	BOOL PaintSprite(CBofBitmap *pBmp, CBofPoint point) {
		return PaintSprite(pBmp, point.x, point.y);
	}

	BOOL PaintSprite(CBofWindow *pWnd, const INT x, const INT y);
	BOOL PaintSprite(CBofWindow *pWnd, CBofPoint point) {
		return PaintSprite(pWnd, point.x, point.y);
	}

	BOOL PaintCel(CBofWindow *pWnd, INT nCelId, const INT x, const INT y);
	BOOL PaintCel(CBofBitmap *pBmp, INT nCelId, const INT x, const INT y);

	VOID BatchPaint(const INT, const INT y);
	VOID BatchErase();

	BOOL SetupCels(const INT nCels);
	VOID SetCel(const INT nCelID);

	VOID NextCel();
	VOID PrevCel();

	BOOL CropImage(CBofWindow *pWnd, CBofRect *pRect, BOOL bUpdateNow = TRUE);

	BOOL RefreshSprite(CBofBitmap *pBmp) {
		return PaintSprite(pBmp, m_cPosition.x, m_cPosition.y);
	}

	BOOL RefreshSprite(CBofWindow *pWnd) {
		return PaintSprite(pWnd, m_cPosition.x, m_cPosition.y);
	}

	BOOL EraseSprite(CBofWindow *pWnd);

	// notice how there is no EraseSprite for a CBofBitmap - that's because
	// sprites no longer retain their background, so there would be no way
	// to restore the background, and that's all EraseSprite does.

	CBofSprite *Interception(CBofRect *newRect, CBofSprite *pSprite);
	CBofSprite *Interception(CBofSprite *pTestSprite);

	CBofSprite *Interception() {
		return Interception(m_pSpriteChain);
	}

	CBofSprite *Interception(CBofRect *newRect) {
		return Interception(newRect, m_pSpriteChain);
	}

	BOOL TestInterception(CBofSprite *pSprite, CBofPoint *pPoint = nullptr);

	VOID SetReadOnly(BOOL bReadOnly = TRUE);
	BOOL GetReadOnly() const {
		return m_bReadOnly;
	}

	VOID SetPosition(INT x, INT y);
	VOID SetPosition(CBofPoint point) {
		SetPosition(point.x, point.y);
	}
	CBofPoint GetPosition() const {
		return m_cPosition;
	}

	CBofSize GetSize() const {
		return m_cSize;
	}

	CBofRect GetRect() const {
		return m_cRect;
	}

	INT Height() const {
		return m_cRect.Height();
	}

	INT Width() const {
		return m_cRect.Width();
	}

	VOID SetMasked(BOOL bMasked);
	BOOL GetMasked() const {
		return m_nMaskColor != NOT_TRANSPARENT;
	}

	VOID SetMaskColor(INT nColor) {
		m_nMaskColor = nColor;
	}

	INT GetMaskColor() const {
		return m_nMaskColor;
	}

	byte ReadPixel(CBofPoint *pPoint) const {
		return m_pImage->ReadPixel(pPoint);
	}
	byte ReadPixel(INT x, INT y) const {
		return m_pImage->ReadPixel(x, y);
	}

	VOID SetZOrder(INT nValue);

	INT GetZOrder() const {
		return m_nZOrder;
	}

	INT GetCelCount() const {
		return m_nCelCount;
	}
	INT GetCelIndex() const {
		return m_nCelID;
	}

	VOID SetAnimated(BOOL bAnimated) {
		m_bAnimated = bAnimated;
	}
	BOOL GetAnimated() const {
		return m_bAnimated;
	}

	BOOL IsLinked() const {
		return m_bLinked;
	}
	VOID LinkSprite();
	VOID UnlinkSprite();

	CBofSprite *GetNextSprite() const {
		return (CBofSprite *)m_pNext;
	}
	CBofSprite *GetPrevSprite() const {
		return (CBofSprite *)m_pPrev;
	}

	const CHAR *GetFileName() const {
		return m_pImage->GetFileName();
	}

	BOOL Touching(CBofPoint myPoint);
	BOOL IsSpriteInSprite(CBofSprite *pSprite);
	BOOL IsSpriteHidden();
	BOOL PtInSprite(CBofPoint cTestPoint);
	BOOL TestPossibleInterception(CBofPoint cPoint, CBofSprite *pSprite);

	static VOID OpenLibrary(CBofPalette *pPal);
	static VOID CloseLibrary();

	static VOID SetSharedPalette(CBofPalette *pPalette);

	static CBofSprite *Touched(CBofPoint myPoint) {
		return Touched(myPoint, m_pSpriteChain);
	}
	static CBofSprite *Touched(CBofPoint myPoint, CBofSprite *pSprite);
	static BOOL InterceptOccurred() {
		return m_pTouchedSprite != nullptr;
	}
	static CBofSprite *GetInterception() {
		return m_pTouchedSprite;
	}

	static CBofSprite *GetSpriteChain() {
		return m_pSpriteChain;
	}

	static BOOL UpdateDirtyRect(CBofWindow *pWnd, CBofSprite *pSprite = nullptr);
	static BOOL UpdateDirtyRect(CBofBitmap *pBmp, CBofSprite *pSprite = nullptr);
	static VOID AddToDirtyRect(CBofRect *pRect);
	static VOID ClearDirtyRect() {
		m_cDirtyRect.SetRectEmpty();
	}

	static CBofRect *GetDirtyRect() {
		return &m_cDirtyRect;
	}

	static BOOL EraseSprites(CBofWindow *pWnd);
	static VOID FlushSpriteChain();

	static BOOL RefreshBackdrop(CBofWindow *pWnd, CBofRect *pRect = nullptr); // obsolete

	static BOOL SetupWorkArea(INT dx, INT dy);
	static VOID TearDownWorkArea();

	// add a method for allowing callers of this object to block
	// next cell advancement

	VOID SetBlockAdvance(BOOL b = TRUE) {
		m_bBlockAdvance = b;
	}
	BOOL GetBlockAdvance() const {
		return m_bBlockAdvance;
	}

private:
	VOID ClearImage();

	BOOL SpritesOverlap(CBofSprite *pSprite, CBofPoint *pPoint = nullptr);
	BOOL m_bBlockAdvance; // allow block next cell.
public:
	CBofBitmap *m_pImage; // bitmap for the sprite

protected:
	CBofPoint m_cPosition; // upper left corner of sprite on display
	CBofSize m_cSize;      // dx/dy size of the sprite bitmap
	CBofRect m_cRect;      // bounding rectangle on display
	CBofRect m_cImageRect; // bounding rectangle within image bitmap

	INT m_nMaskColor; // Transparent color index for this sprite
	INT m_nZOrder;    // foreground / background order
	INT m_nCelID;     // index of current cel image
	INT m_nCelCount;  // number of cels in the animation strip

	BOOL m_bDuplicated : 1; // shares bitmaps with some other sprite
	BOOL m_bPositioned : 1; // whether sprite has been positioned yet
	BOOL m_bAnimated : 1;   // whether cel advance occurs when painting
	BOOL m_bLinked : 1;     // whether sprite is linked into the chain
	BOOL m_bReadOnly : 1;   // whether image is read only or not

	static CBofRect m_cDirtyRect;
	static CBofSprite *m_pSpriteChain;    // pointer to linked chain of sprites
	static CBofSprite *m_pTouchedSprite;  // sprite touched during painting operation
	static CBofBitmap *m_pWorkBmp;        // offscreen work area
	static CBofPalette *m_pSharedPalette; // shared palette for ALL sprites
	static INT m_nWorkDX;
	static INT m_nWorkDY;
};

} // namespace Bagel

#endif
