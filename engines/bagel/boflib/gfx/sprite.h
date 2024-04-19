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
	CBofSprite();

	// Destructors
	virtual ~CBofSprite();

	//////////////////////////////////////////

	// Implementation
	CBofSprite *DuplicateSprite();
	bool DuplicateSprite(CBofSprite *pSprite);

	bool LoadSprite(const char *pszPathName, int nCels = 1);
	bool LoadSprite(CBofBitmap *pBitmap, int nCels = 1);

	bool PaintSprite(CBofBitmap *pBmp, const int x, const int y);
	bool PaintSprite(CBofBitmap *pBmp, CBofPoint point) {
		return PaintSprite(pBmp, point.x, point.y);
	}

	bool PaintSprite(CBofWindow *pWnd, const int x, const int y);
	bool PaintSprite(CBofWindow *pWnd, CBofPoint point) {
		return PaintSprite(pWnd, point.x, point.y);
	}

	bool PaintCel(CBofWindow *pWnd, int nCelId, const int x, const int y);
	bool PaintCel(CBofBitmap *pBmp, int nCelId, const int x, const int y);

	void BatchPaint(const int, const int y);
	void BatchErase();

	bool SetupCels(const int nCels);
	void SetCel(const int nCelID);

	void NextCel();
	void PrevCel();

	bool CropImage(CBofWindow *pWnd, CBofRect *pRect, bool bUpdateNow = true);

	bool RefreshSprite(CBofBitmap *pBmp) {
		return PaintSprite(pBmp, m_cPosition.x, m_cPosition.y);
	}

	bool RefreshSprite(CBofWindow *pWnd) {
		return PaintSprite(pWnd, m_cPosition.x, m_cPosition.y);
	}

	bool EraseSprite(CBofWindow *pWnd);

	// Notice how there is no EraseSprite for a CBofBitmap - that's because
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

	void SetReadOnly(bool bReadOnly = true);
	bool GetReadOnly() const {
		return m_bReadOnly;
	}

	void SetPosition(int x, int y);
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

	int Height() const {
		return m_cRect.Height();
	}

	int Width() const {
		return m_cRect.Width();
	}

	void SetMasked(bool bMasked);
	bool GetMasked() const {
		return m_nMaskColor != NOT_TRANSPARENT;
	}

	void SetMaskColor(int nColor) {
		m_nMaskColor = nColor;
	}

	int GetMaskColor() const {
		return m_nMaskColor;
	}

	byte ReadPixel(CBofPoint *pPoint) const {
		return m_pImage->ReadPixel(pPoint);
	}
	byte ReadPixel(int x, int y) const {
		return m_pImage->ReadPixel(x, y);
	}

	void SetZOrder(int nValue);

	int GetZOrder() const {
		return m_nZOrder;
	}

	int GetCelCount() const {
		return m_nCelCount;
	}
	int GetCelIndex() const {
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

	static bool SetupWorkArea(int dx, int dy);
	static void TearDownWorkArea();

	// Add a method for allowing callers of this object to block
	// next cell advancement

	void SetBlockAdvance(bool b = true) {
		m_bBlockAdvance = b;
	}
	bool GetBlockAdvance() const {
		return m_bBlockAdvance;
	}

private:
	void ClearImage();

	bool SpritesOverlap(CBofSprite *pSprite, CBofPoint *pPoint = nullptr);
	bool m_bBlockAdvance; // Allow block next cell.
public:
	CBofBitmap *m_pImage; // Bitmap for the sprite

protected:
	CBofPoint m_cPosition; // Upper left corner of sprite on display
	CBofSize m_cSize;      // dx/dy size of the sprite bitmap
	CBofRect m_cRect;      // Bounding rectangle on display
	CBofRect m_cImageRect; // Bounding rectangle within image bitmap

	int m_nMaskColor; // Transparent color index for this sprite
	int m_nZOrder;    // Foreground / background order
	int m_nCelID;     // Index of current cel image
	int m_nCelCount;  // Number of cels in the animation strip

	bool m_bDuplicated : 1; // Shares bitmaps with some other sprite
	bool m_bPositioned : 1; // Whether sprite has been positioned yet
	bool m_bAnimated : 1;   // Whether cel advance occurs when painting
	bool m_bLinked : 1;     // Whether sprite is linked into the chain
	bool m_bReadOnly : 1;   // Whether image is read only or not

	static CBofRect m_cDirtyRect;
	static CBofSprite *m_pSpriteChain;    // Pointer to linked chain of sprites
	static CBofSprite *m_pTouchedSprite;  // Sprite touched during painting operation
	static CBofBitmap *m_pWorkBmp;        // Offscreen work area
	static CBofPalette *m_pSharedPalette; // Shared palette for ALL sprites
	static int m_nWorkDX;
	static int m_nWorkDY;
};

} // namespace Bagel

#endif
