
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

#ifndef BAGEL_BOFLIB_GFX_BMP_H
#define BAGEL_BOFLIB_GFX_BMP_H

#include "graphics/managed_surface.h"
#include "bagel/boflib/palette.h"
#include "bagel/boflib/cache.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/size.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {
namespace SpaceBar {

// Color constants
//
enum {
	NOT_TRANSPARENT = -1,
	COLOR_WHITE     = 255,
	COLOR_BLACK     = 0
};

#define CBMP_FADE_SPEED 10
#define CBMP_FADE_SIZE 4

#define CBMP_CURT_SPEED 8 // Must be a power of 2

#define CBMP_LINE_SPEED 32 // Should be a power of 4

// forward declare CBofWindow
class CBofWindow;

class CBofBitmap : public CBofError, public CBofObject, public CCache {
protected:
	/**
	 * Does the actual allocation for this bitmap
	 * @return  true is this bitmap was successfully loaded into the cache
	 */
	bool alloc() override;

	/**
	 * Frees the data used by this bitmap (removes from cache)
	 */
	void free() override;

	//
	// data members
	//
	static bool _bUseBackdrop;

	char _szFileName[MAX_FNAME];

	Graphics::ManagedSurface _bitmap;

	byte *_pBits = nullptr;

	CBofPalette *_pPalette = nullptr;

	int _nScanDX = 0;
	int _nDX = 0;
	int _nDY = 0;
	bool _bTopDown = false;

	bool _bOwnPalette = false;
	bool _bReadOnly = false;
	bool _bInitialized = false;

public:
	/**
	 * Default constructor
	 */
	CBofBitmap();

	/**
	 * Constructs a CBofBitmap
	 * @param dx            Width of new bitmap
	 * @param dy            Height of new bitmap
	 * @param pPalette      Palette to use for this bitmap
	 * @param bOwnPalette   true if destructor should delete palette
	 * @param pPrivateBuff
	 */
	CBofBitmap(int dx, int dy, CBofPalette *pPalette, bool bOwnPalette = false, byte *pPrivateBuff = nullptr);

	/**
	 * Constructs a CBofBitmap
	 * @param pszFileName   Path and Filename for Bitmap on disk
	 * @param pPalette      Palette to use for this bitmap
	 * @param bOwnPalette   true if destructor should delete palette
	 */
	CBofBitmap(const char *pszFileName, CBofPalette *pPalette = nullptr, bool bOwnPalette = false);

	/**
	 * Destructor
	 */
	virtual ~CBofBitmap();

	/**
	 * Allocates the structures needed for a CBofBitmap
	 * @param pPalette      Palette to be assigned into this bitmap
	 */
	ErrorCode buildBitmap(CBofPalette *pPalette);

	/**
	 * Loads the specified bitmap from disk
	 * @param pszFileName   Filename
	 * @param pPalette      Palette
	 * @return              Error return code
	 */
	ErrorCode loadBitmap(const char *pszFileName, CBofPalette *pPalette);

	/**
	 * Frees the data used by this bitmap
	 */
	void releaseBitmap();

	//
	// Palette routines
	//

	/**
	 * Assigns specified palette to this bitmap
	 * @param pPalette      Pointer to CBofPalette to be assigned
	 * @param bOwnPalette   true if bitmap is to own this palette
	 */
	void setPalette(CBofPalette *pPalette, bool bOwnPalette = false);

	CBofPalette *getPalette() {
		return _pPalette;
	}

	void setIsOwnPalette(bool own) {
		_bOwnPalette = own;
	}

	//
	// Misc routines
	//

	/**
	 * Returns the bit address of the (x, y) location in this bmp
	 * @param x     Column in _pBits
	 * @param y     Row in _pBits
	 * @return      Address of (x,y) in bitmap surface
	 */
	byte *getPixelAddress(int x, int y);
	byte *getPixelAddress(CBofPoint *pPoint) {
		return getPixelAddress(pPoint->x, pPoint->y);
	}

	CBofSize getSize() {
		return CBofSize(_nDX, _nDY);
	}

	CBofRect getRect() {
		return CBofRect(0, 0, _nDX - 1, _nDY - 1);
	}

	void setReadOnly(bool bReadOnly) {
		_bReadOnly = bReadOnly;
	}

	bool getReadOnly() {
		return _bReadOnly;
	}

	bool isTopDown() {
		return _bTopDown;
	}

	int width() {
		return _nDX;
	}

	int widthBytes() {
		return _nScanDX;
	}

	int height() {
		return _nDY;
	}

	operator Graphics::ManagedSurface &() {
		return _bitmap;
	}

	Graphics::ManagedSurface getSurface();

	/**
	 * Returns current bitmap's filename (if any)
	 * @return      Pointer to bitmap's filename
	 */
	const char *getFileName();

	//
	// Drawing routines
	//
	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pWnd          Destination device for painting
	 * @param x             Destination column
	 * @param y             Destination row
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    Transparency color
	 * @return              error return code
	 */
	ErrorCode paint(CBofWindow *pWnd, int x, int y, CBofRect *pSrcRect = nullptr, int nMaskColor = NOT_TRANSPARENT);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pWnd          Destination Device to paint to
	 * @param pDstRect      Destination rectangle (for stretching)
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    transparency color
	 * @return              Error return code
	 */
	ErrorCode paint(CBofWindow *pWnd, CBofRect *pDstRect = nullptr, CBofRect *pSrcRect = nullptr, int nMaskColor = NOT_TRANSPARENT);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pWnd          Destination Device to paint to
	 * @param pDstRect      Destination rectangle (for stretching)
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    Transparency color
	 * @return              Error return code
	 */
	ErrorCode paintMaskBackdrop(CBofWindow *pWnd, CBofRect *pDstRect = nullptr, CBofRect *pSrcRect = nullptr, int nMaskColor = NOT_TRANSPARENT);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pBmp          Destination bitmap to paint to
	 * @param x             Destination column
	 * @param y             Destination row
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    Transparency color
	 * @return              Error return code
	 */
	ErrorCode paint(CBofBitmap *pBmp, int x, int y, CBofRect *pSrcRect = nullptr, int nMaskColor = NOT_TRANSPARENT);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pBmp          Destination bitmap to paint to
	 * @param pDstRect      Destination rectangle (for stretching)
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    Transparency color
	 * @return              Error return code.
	 */
	ErrorCode paint(CBofBitmap *pBmp, CBofRect *pDstRect = nullptr, CBofRect *pSrcRect = nullptr, int nMaskColor = NOT_TRANSPARENT);

	//
	// Special Paint routines Optimized for specific tasks
	//

	// Stretches 4 pixel wide
	/**
	 * Stretches 4 pixel wide strips from source to destination
	 * @brief The Destination rectangle MUST be divisible by 4.
	 * Both bitmaps must be Bottom-Up.  The Source must be smaller than the Destination.
	 *
	 * @param pBmp          Destination bitmap to paint to
	 * @param pDstRect      Destination rectangle (for stretching)
	 * @param pSrcRect      Source rectangle from bitmap
	 * @return              Error return code
	 */
	ErrorCode paintStretch4(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect);

	/**
	 * Stretches a multiple of 4 pixel wide strips from source to destination
	 * @param pBmp          Destination bitmap to paint to
	 * @param pDstRect      Destination rectangle (for stretching)
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nOptSize
	 * @return              Error return code
	 */
	ErrorCode paintStretchOpt(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect, int nOptSize);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pBmp      Destination bitmap to paint to
	 * @return          Error return code
	 */
	ErrorCode paint1To1(CBofBitmap *pBmp);

	/** Copy specified section of screen (or window) to bitmap.
	 * @param pWnd      Window to capture
	 * @param pSrcRect  Source rectangle in window
	 * @param pDstRect  Destination area to copy image to
	 * @return          Error return code
	 */
	ErrorCode captureScreen(CBofWindow *pWnd, CBofRect *pSrcRect, CBofRect *pDstRect = nullptr);

	/**
	 * Performs a "Fade" onto the specified window
	 * @param pWnd          Pointer to window to fade into
	 * @param x             Fade upper left X
	 * @param y             Fade upper left Y
	 * @param nMaskColor    Transparency color (if any)
	 * @param nBlockSize    Size of Fade Blocks
	 * @param nSpeed        Speed for fade (not implemented yet)
	 * @return              Error return code
	 */
	ErrorCode fadeIn(CBofWindow *pWnd, int x = 0, int y = 0, int nMaskColor = NOT_TRANSPARENT, int nBlockSize = CBMP_FADE_SIZE, int nSpeed = CBMP_FADE_SPEED);

	ErrorCode curtain(CBofWindow *pWnd, int nSpeed = CBMP_CURT_SPEED, int nMaskColor = NOT_TRANSPARENT);
	ErrorCode fadeLines(CBofWindow *pWnd, int nSpeed = CBMP_LINE_SPEED, int nMaskColor = NOT_TRANSPARENT);

	/**
	 * Returns the color at the (x, y) location in this bmp
	 * @param x         X position
	 * @param y         Y position
	 * @return          Color Index of specified (x,y) location in _pBits
	 */
	byte readPixel(int x, int y);

	/**
	 * Assigns the specified color to the (x, y) location
	 * @param x         X position
	 * @param y         Y position
	 * @param iColor    Pixel value
	 */
	void writePixel(int x, int y, byte iColor);

	/**
	 * Writes a circle into this bitmap
	 * @param x         X center position
	 * @param y         Y center position
	 * @param nRadius   Radius of circle
	 * @param iColor    Pixel value
	 */
	void circle(int x, int y, uint16 nRadius, byte iColor);

	/**
	 * Writes a line into this bitmap
	 * @param nSrcX     Endpoint 1 x
	 * @param nSrcY     Endpoint 1 y
	 * @param nDstX     Endpoint 2 x
	 * @param nDstY     Endpoint 2 y
	 * @param iColor    Pixel value
	 */
	void line(int nSrcX, int nSrcY, int nDstX, int nDstY, byte iColor);

	/**
	 * Writes a line into this bitmap
	 * @param pSrc      Endpoint 1
	 * @param pDest     Endpoint 2
	 * @param iColor    Pixel value
	 */
	void line(CBofPoint *pSrc, CBofPoint *pDest, byte iColor);

	/**
	 * Writes a Rectangle into this bitmap
	 * @param cRect     Pointer to rectangle Coordinates
	 * @param iColor    Color of rectangle
	 */
	void drawRect(CBofRect *cRect, byte iColor);

	/**
	 * Writes a filled in Rectangle to this bitmap
	 * @param cRect     Pointer to rectangle Coordinates
	 * @param iColor    Color of rectangle
	 */
	void fillRect(CBofRect *cRect, byte iColor);

	/**
	 * Scrolls current bitmap horizontally
	 * @param nPixels       Number of pixels to scroll by
	 * @param pRect         Section of bitmap to scroll
	 * @return              Error return code
	 */
	ErrorCode scrollRight(int nPixels, CBofRect *pRect = nullptr);


	ErrorCode scrollLeft(int nPixels, CBofRect *pRect = nullptr) {
		return scrollRight(-nPixels, pRect);
	}

	/**
	 * Scrolls current bitmap vertically
	 * @param nPixels       Number of pixels to scroll by
	 * @return              Error return code
	 */
	ErrorCode scrollUp(int nPixels);

	static void setUseBackdrop(bool b) {
		_bUseBackdrop = b;
	}
	static bool getUseBackdrop() {
		return _bUseBackdrop;
	}
};

//////////////////////////////////////////////////////////////////////////////
//
// Misc graphics routines
//
//////////////////////////////////////////////////////////////////////////////

/**
 * Loads specified bitmap (and possibly re-maps to palette)
 * @param pszFileName       Bitmap to open
 * @param pPalette          Palette for re-mapping
 * @param bSharedPal        Shared palette flag
 * @return                  Pointer to bitmap
 */
extern CBofBitmap *loadBitmap(const char *pszFileName, CBofPalette *pPalette = nullptr, bool bSharedPal = false);

/**
 * Paints specified bitmap to specified window
 * @param pWindow           Window to paint to
 * @param pszFileName       Bitmap filename
 * @param pDstRect          Destination area to paint to
 * @param pSrcRect          Source area to paint from
 * @param pPalette          Optional palette to re-map with
 * @param nMaskColor        Optional transparent color
 * @return                  Error return code
 */
extern ErrorCode paintBitmap(CBofWindow *pWindow, const char *pszFileName, CBofRect *pDstRect = nullptr,
                             CBofRect *pSrcRect = nullptr, CBofPalette *pPalette = nullptr, int nMaskColor = NOT_TRANSPARENT);

} // namespace SpaceBar
} // namespace Bagel

#endif
