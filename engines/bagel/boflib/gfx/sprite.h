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
	bool DuplicateSprite(CBofSprite *pSprite);

	bool LoadSprite(const char *pszPathName, INT nCels = 1);
	bool LoadSprite(CBofBitmap *pBitmap, INT nCels = 1);

	bool PaintSprite(CBofBitmap *pBmp, const INT x, const INT y);
	bool PaintSprite(CBofBitmap *pBmp, CBofPoint point) {
		return PaintSprite(pBmp, point.x, point.y);
	}

	bool PaintSprite(CBofWindow *pWnd, const INT x, const INT y);
	bool PaintSprite(CBofWindow *pWnd, CBofPoint point) {
		return PaintSprite(pWnd, point.x, point.y);
	}

	bool PaintCel(CBofWindow *pWnd, INT nCelId, const INT x, const INT y);
	bool PaintCel(CBofBitmap *pBmp, INT nCelId, const INT x, const INT y);

	void BatchPaint(const INT, const INT y);
	void BatchErase();

	bool SetupCels(const INT nCels);
	void SetCel(const INT nCelID);

	void NextCel();
	void PrevCel();

	bool CropImage(CBofWindow *pWnd, CBofRect *pRect, bool bUpdateNow = TRUE);

	bool RefreshSprite(CBofBitmap *pBmp) {
		return PaintSprite(pBmp, m_cPosition.x, m_cPosition.y);
	}

	bool RefreshSprite(CBofWindow *pWnd) {
		return PaintSprite(pWnd, m_cPosition.x, m_cPosition.y);
	}

	bool EraseSprite(CBofWindow *pWnd);

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

	bool TestInterception(CBofSprite *pSprite, CBofPoint *pPoint = nullptr);

	void SetReadOnly(bool bReadOnly = TRUE);
	bool GetReadOnly() const {
		return m_bReadOnly;
	}

	void SetPosition(INT x, INT y);
	void SetPosition(CBofPoint point) {
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

	void SetMasked(bool bMasked);
	bool GetMasked() const {
		return m_nMaskColor != NOT_TRANSPARENT;
	}

	void SetMaskColor(INT nColor) {
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

	void SetZOrder(INT nValue);

	INT GetZOrder() const {
		return m_nZOrder;
	}

	INT GetCelCount() const {
		return m_nCelCount;
	}
	INT GetCelIndex() const {
		return m_nCelID;
	}

	void SetAnimated(bool bAnimated) {
		m_bAnimated = bAnimated;
	}
	bool GetAnimated() const {
		return m_bAnimated;
	}

	bool IsLinked() const {
		return m_bLinked;
	}
	void LinkSprite();
	void UnlinkSprite();

	CBofSprite *GetNextSprite() const {
		return (CBofSprite *)m_pNext;
	}
	CBofSprite *GetPrevSprite() const {
		return (CBofSprite *)m_pPrev;
	}

	const char *GetFileName() const {
		return m_pImage->GetFileName();
	}

	bool Touching(CBofPoint myPoint);
	bool IsSpriteInSprite(CBofSprite *pSprite);
	bool IsSpriteHidden();
	bool PtInSprite(CBofPoint cTestPoint);
	bool TestPossibleInterception(CBofPoint cPoint, CBofSprite *pSprite);

	static void OpenLibrary(CBofPalette *pPal);
	static void CloseLibrary();

	static void SetSharedPalette(CBofPalette *pPalette);

	static CBofSprite *Touched(CBofPoint myPoint) {
		return Touched(myPoint, m_pSpriteChain);
	}
	static CBofSprite *Touched(CBofPoint myPoint, CBofSprite *pSprite);
	static bool InterceptOccurred() {
		return m_pTouchedSprite != nullptr;
	}
	static CBofSprite *GetInterception() {
		return m_pTouchedSprite;
	}

	static CBofSprite *GetSpriteChain() {
		return m_pSpriteChain;
	}

	static bool UpdateDirtyRect(CBofWindow *pWnd, CBofSprite *pSprite = nullptr);
	static bool UpdateDirtyRect(CBofBitmap *pBmp, CBofSprite *pSprite = nullptr);
	static void AddToDirtyRect(CBofRect *pRect);
	static void ClearDirtyRect() {
		m_cDirtyRect.SetRectEmpty();
	}

	static CBofRect *GetDirtyRect() {
		return &m_cDirtyRect;
	}

	static bool EraseSprites(CBofWindow *pWnd);
	static void FlushSpriteChain();

	static bool RefreshBackdrop(CBofWindow *pWnd, CBofRect *pRect = nullptr); // obsolete

	static bool SetupWorkArea(INT dx, INT dy);
	static void TearDownWorkArea();

	// add a method for allowing callers of this object to block
	// next cell advancement

	void SetBlockAdvance(bool b = TRUE) {
		m_bBlockAdvance = b;
	}
	bool GetBlockAdvance() const {
		return m_bBlockAdvance;
	}

private:
	void ClearImage();

	bool SpritesOverlap(CBofSprite *pSprite, CBofPoint *pPoint = nullptr);
	bool m_bBlockAdvance; // allow block next cell.
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

	bool m_bDuplicated : 1; // shares bitmaps with some other sprite
	bool m_bPositioned : 1; // whether sprite has been positioned yet
	bool m_bAnimated : 1;   // whether cel advance occurs when painting
	bool m_bLinked : 1;     // whether sprite is linked into the chain
	bool m_bReadOnly : 1;   // whether image is read only or not

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
