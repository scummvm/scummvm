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

namespace Bagel {

#define SPRITE_TOPMOST 0
#define SPRITE_FOREGROUND 64
#define SPRITE_MIDDLE 128
#define SPRITE_BACKGROUND 192
#define SPRITE_HINDMOST 255

class CBofSprite : public CBofError, public CBofObject, public CLList {
public:
	static void initialize();
	static void shutdown();

	// Constructors
	CBofSprite();

	// Destructors
	virtual ~CBofSprite();

	//////////////////////////////////////////

	// Implementation
	CBofSprite *duplicateSprite();
	bool duplicateSprite(CBofSprite *pSprite);

	bool loadSprite(const char *pszPathName, int nCels = 1);
	bool loadSprite(CBofBitmap *pBitmap, int nCels = 1);

	bool paintSprite(CBofBitmap *pBmp, const int x, const int y);
	bool paintSprite(CBofBitmap *pBmp, CBofPoint point) {
		return paintSprite(pBmp, point.x, point.y);
	}

	bool paintSprite(CBofWindow *pWnd, const int x, const int y);
	bool paintSprite(CBofWindow *pWnd, CBofPoint point) {
		return paintSprite(pWnd, point.x, point.y);
	}

	bool paintCel(CBofWindow *pWnd, int nCelId, const int x, const int y);
	bool paintCel(CBofBitmap *pBmp, int nCelId, const int x, const int y);

	void batchPaint(const int, const int y);
	void batchErase();

	bool setupCels(const int nCels);
	void setCel(const int nCelID);

	void nextCel();
	void prevCel();

	bool cropImage(CBofWindow *pWnd, CBofRect *pRect, bool bUpdateNow = true);

	bool refreshSprite(CBofBitmap *pBmp) {
		return paintSprite(pBmp, m_cPosition.x, m_cPosition.y);
	}

	bool refreshSprite(CBofWindow *pWnd) {
		return paintSprite(pWnd, m_cPosition.x, m_cPosition.y);
	}

	bool eraseSprite(CBofWindow *pWnd);

	// Notice how there is no eraseSprite for a CBofBitmap - that's because
	// sprites no longer retain their background, so there would be no way
	// to restore the background, and that's all eraseSprite does.

	CBofSprite *interception(CBofRect *newRect, CBofSprite *pSprite);
	CBofSprite *interception(CBofSprite *pTestSprite);

	CBofSprite *interception() {
		return interception(m_pSpriteChain);
	}

	CBofSprite *interception(CBofRect *newRect) {
		return interception(newRect, m_pSpriteChain);
	}

	bool testInterception(CBofSprite *pSprite, CBofPoint *pPoint = nullptr);

	void setPosition(int x, int y);

	CBofPoint getPosition() const {
		return m_cPosition;
	}

	CBofSize getSize() const {
		return m_cSize;
	}

	CBofRect getRect() const {
		return _cRect;
	}

	int height() const {
		return _cRect.height();
	}

	int width() const {
		return _cRect.width();
	}

	void setMaskColor(int nColor) {
		m_nMaskColor = nColor;
	}

	int getMaskColor() const {
		return m_nMaskColor;
	}

	byte readPixel(int x, int y) const {
		return m_pImage->readPixel(x, y);
	}

	void setZOrder(int nValue);

	int getCelCount() const {
		return m_nCelCount;
	}
	int getCelIndex() const {
		return m_nCelID;
	}

	void setAnimated(bool bAnimated) {
		m_bAnimated = bAnimated;
	}
	bool getAnimated() const {
		return m_bAnimated;
	}

	void linkSprite();
	void unlinkSprite();

	const char *getFileName() const {
		return m_pImage->getFileName();
	}

	static void openLibrary(CBofPalette *pPal);
	static void closeLibrary();

	static void setSharedPalette(CBofPalette *pPalette);

	static CBofSprite *getSpriteChain() {
		return m_pSpriteChain;
	}

	static bool updateDirtyRect(CBofWindow *pWnd, CBofSprite *pSprite = nullptr);
	static bool updateDirtyRect(CBofBitmap *pBmp, CBofSprite *pSprite = nullptr);
	static void addToDirtyRect(CBofRect *pRect);
	static void clearDirtyRect() {
		m_cDirtyRect->SetRectEmpty();
	}

	static CBofRect *getDirtyRect() {
		return m_cDirtyRect;
	}

	static void flushSpriteChain();

	static bool setupWorkArea(int dx, int dy);
	static void tearDownWorkArea();

	// Add a method for allowing callers of this object to block
	// next cell advancement

	void setBlockAdvance(bool b = true) {
		m_bBlockAdvance = b;
	}
	bool getBlockAdvance() const {
		return m_bBlockAdvance;
	}

private:
	void clearImage();

	bool spritesOverlap(CBofSprite *pSprite, CBofPoint *pPoint = nullptr);
	bool m_bBlockAdvance; // Allow block next cell.
public:
	CBofBitmap *m_pImage; // Bitmap for the sprite

protected:
	CBofPoint m_cPosition; // Upper left corner of sprite on display
	CBofSize m_cSize;      // dx/dy size of the sprite bitmap
	CBofRect _cRect;      // Bounding rectangle on display
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

	static CBofRect *m_cDirtyRect;
	static CBofSprite *m_pSpriteChain;    // Pointer to linked chain of sprites
	static CBofSprite *m_pTouchedSprite;  // Sprite touched during painting operation
	static CBofBitmap *m_pWorkBmp;        // Offscreen work area
	static CBofPalette *m_pSharedPalette; // Shared palette for ALL sprites
	static int m_nWorkDX;
	static int m_nWorkDY;
};

} // namespace Bagel

#endif
