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

#include "bagel/spacebar/boflib/gfx/bitmap.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/palette.h"
#include "bagel/spacebar/boflib/gui/window.h"

namespace Bagel {
namespace SpaceBar {

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
	void duplicateSprite(CBofSprite *pSprite);

	bool loadSprite(const char *pszPathName, int nCels = 1);
	bool loadSprite(CBofBitmap *pBitmap, int nCels = 1);

	bool paintSprite(CBofBitmap *pBmp, int x, int y);
	bool paintSprite(CBofBitmap *pBmp, CBofPoint point) {
		return paintSprite(pBmp, point.x, point.y);
	}

	bool paintSprite(CBofWindow *pWnd, int x, int y);
	bool paintSprite(CBofWindow *pWnd, CBofPoint point) {
		return paintSprite(pWnd, point.x, point.y);
	}

	bool paintCel(CBofWindow *pWnd, int nCelId, int x, int y);
	bool paintCel(CBofBitmap *pBmp, int nCelId, int x, int y);

	void batchPaint(int, int y);
	void batchErase();

	bool setupCels(int nCels);
	void setCel(int nCelID);

	void nextCel();
	void prevCel();

	bool refreshSprite(CBofBitmap *pBmp) {
		return paintSprite(pBmp, _cPosition.x, _cPosition.y);
	}

	bool refreshSprite(CBofWindow *pWnd) {
		return paintSprite(pWnd, _cPosition.x, _cPosition.y);
	}

	bool eraseSprite(CBofWindow *pWnd);

	// Notice how there is no eraseSprite for a CBofBitmap - that's because
	// sprites no longer retain their background, so there would be no way
	// to restore the background, and that's all eraseSprite does.

	CBofSprite *interception(CBofRect *newRect, CBofSprite *pTestSprite);
	CBofSprite *interception(CBofSprite *pTestSprite);

	CBofSprite *interception() {
		return interception(_pSpriteChain);
	}

	CBofSprite *interception(CBofRect *newRect) {
		return interception(newRect, _pSpriteChain);
	}

	bool testInterception(CBofSprite *pTestSprite, CBofPoint *pPoint = nullptr);

	void setPosition(int x, int y);

	CBofPoint getPosition() const {
		return _cPosition;
	}

	CBofSize getSize() const {
		return _cSize;
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
		_nMaskColor = nColor;
	}

	int getMaskColor() const {
		return _nMaskColor;
	}

	byte readPixel(int x, int y) const {
		return _pImage->readPixel(x, y);
	}

	void setZOrder(int nValue);

	int getCelCount() const {
		return _nCelCount;
	}
	int getCelIndex() const {
		return _nCelID;
	}

	void setAnimated(bool bAnimated) {
		_bAnimated = bAnimated;
	}
	bool getAnimated() const {
		return _bAnimated;
	}

	void linkSprite();
	void unlinkSprite();

	const char *getFileName() const {
		return _pImage->getFileName();
	}

	static void openLibrary(CBofPalette *pPal);
	static void closeLibrary();

	static void setSharedPalette(CBofPalette *pPalette);

	static CBofSprite *getSpriteChain() {
		return _pSpriteChain;
	}

	static bool updateDirtyRect(CBofWindow *pWnd, CBofSprite *pPrimarySprite = nullptr);
	static bool updateDirtyRect(CBofBitmap *pBmp, CBofSprite *pPrimarySprite = nullptr);
	static void addToDirtyRect(CBofRect *pRect);
	static void clearDirtyRect() {
		_cDirtyRect->setRectEmpty();
	}

	static CBofRect *getDirtyRect() {
		return _cDirtyRect;
	}

	static void flushSpriteChain();

	static void setupWorkArea(int dx, int dy);
	static void tearDownWorkArea();

	// Add a method for allowing callers of this object to block
	// next cell advancement

	void setBlockAdvance(bool b = true) {
		_bBlockAdvance = b;
	}
	bool getBlockAdvance() const {
		return _bBlockAdvance;
	}

private:
	void clearImage();

	bool spritesOverlap(CBofSprite *pSprite, CBofPoint *pPoint = nullptr);
	bool _bBlockAdvance; // Allow block next cell.
public:
	CBofBitmap *_pImage; // Bitmap for the sprite

protected:
	CBofPoint _cPosition; // Upper left corner of sprite on display
	CBofSize _cSize;      // dx/dy size of the sprite bitmap
	CBofRect _cRect;      // Bounding rectangle on display
	CBofRect _cImageRect; // Bounding rectangle within image bitmap

	int _nMaskColor; // Transparent color index for this sprite
	int _nZOrder;    // Foreground / background order
	int _nCelID;     // Index of current cel image
	int _nCelCount;  // Number of cels in the animation strip

	bool _bDuplicated : 1; // Shares bitmaps with some other sprite
	bool _bPositioned : 1; // Whether sprite has been positioned yet
	bool _bAnimated : 1;   // Whether cel advance occurs when painting
	bool _bLinked : 1;     // Whether sprite is linked into the chain
	bool _bReadOnly : 1;   // Whether image is read only or not

	static CBofRect *_cDirtyRect;
	static CBofSprite *_pSpriteChain;    // Pointer to linked chain of sprites
	static CBofSprite *_pTouchedSprite;  // Sprite touched during painting operation
	static CBofBitmap *_pWorkBmp;        // Offscreen work area
	static CBofPalette *_pSharedPalette; // Shared palette for ALL sprites
	static int _nWorkDX;
	static int _nWorkDY;
};

} // namespace SpaceBar
} // namespace Bagel

#endif
