
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
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/cache.h"
#include "bagel/boflib/error.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/size.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/gfx/palette.h"

namespace Bagel {

// color constants
//
#define NOT_TRANSPARENT -1

#if BOF_MAC || BOF_WINMAC
#define COLOR_WHITE 0
#define COLOR_BLACK 255
#else
#define COLOR_WHITE 255
#define COLOR_BLACK 0
#endif

#define CBMP_FADE_SPEED 10
#define CBMP_FADE_SIZE 4

#define CBMP_CURT_SPEED 8 // Must be a power of 2

#define CBMP_LINE_SPEED 32 // Should be a power of 4

// forward declare CBofWindow
class CBofWindow;

#include "common/pack-start.h" // START STRUCT PACKING

struct bofBITMAPINFOHEADER {
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} PACKED_STRUCT;
typedef bofBITMAPINFOHEADER BOFBITMAPINFOHEADER;

struct bofBITMAPINFO {
	BOFBITMAPINFOHEADER bmiHeader;
	BOFRGBQUAD bmiColors[1];
} PACKED_STRUCT;
typedef bofBITMAPINFO BOFBITMAPINFO;

struct bofBITMAPFILEHEADER {
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
} PACKED_STRUCT;
typedef bofBITMAPFILEHEADER BOFBITMAPFILEHEADER;

struct bofBITMAP_EX {
	BOFBITMAPFILEHEADER m_cFileHeader;
	BOFBITMAPINFOHEADER m_cInfoHeader;
	BOFRGBQUAD m_cRgbValues[256];
} PACKED_STRUCT;
typedef bofBITMAP_EX BITMAP_EX;

#include "common/pack-end.h" // END STRUCT PACKING

class CBofBitmap : public CBofError, public CBofObject, public CCache {
protected:
	/**
	 * Does the actual allocation for this bitmap
	 * @return  TRUE is this bitmap was successfully loaded into the cache
	 */
	VIRTUAL BOOL Alloc();

	/**
	 * Frees the data used by this bitmap (removes from cache)
	 */
	VIRTUAL VOID Free();

	//
	// data members
	//
	static BOOL m_bUseBackdrop;

	CHAR m_szFileName[MAX_FNAME];

	BITMAP_EX m_cBitmapInfo;

#if BOF_WINDOWS && !BOF_WINMAC

	HBITMAP m_hBitmap;
	HBITMAP m_hOldBmp;
	HDC m_hDC;
	HPALETTE m_hPalOld;

#elif BOF_MAC || BOF_WINMAC

	STATIC PixMapHandle m_stPixMap; // bit map to pixel map
#if SYNCPALETTES
	GrafPtr m_oldGrafPort;          // original grafport
	GrafPtr m_newGrafPort;          // original grafport
#endif
#endif
	Graphics::ManagedSurface _bitmap;

	UBYTE *m_pBits = nullptr;

	CBofPalette *m_pPalette = nullptr;

	INT m_nScanDX = 0;
	INT m_nDX = 0;
	INT m_nDY = 0;
	BOOL m_bTopDown = FALSE;

	BOOL m_bOwnPalette = FALSE;
	BOOL m_bReadOnly = FALSE;
	BOOL m_bInitialized = FALSE;

	BOOL m_bPrivateBmp = FALSE;

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
	 * @param bOwnPalette   TRUE if destructor should delete palette
	 */
	CBofBitmap(INT dx, INT dy, CBofPalette *pPalette, BOOL bOwnPalette = FALSE, UBYTE *pPrivateBuff = nullptr);

	/**
	 * Constructs a CBofBitmap
	 * @param pszFileName   Path and Filename for Bitmap on disk
	 * @param pPalette      Palette to use for this bitmap
	 * @param bOwnPalette   TRUE if destructor should delete palette
	 */
	CBofBitmap(const CHAR *pszFileName, CBofPalette *pPalette = nullptr, BOOL bOwnPalette = FALSE);

	/**
	 * Destructor
	 */
	virtual ~CBofBitmap();

	/**
	 * Allocates the structures needed for a CBofBitmap
	 * @param       Palette to be assigned into this bitmap
	 */
	ERROR_CODE BuildBitmap(CBofPalette *pPalette);

	/**
	 * Loads the specified bitmap from disk
	 * @param pszFileName   Filename
	 * @param pPalette      Palette
	 * @return              Error return code
	 */
	ERROR_CODE LoadBitmap(const CHAR *pszFileName, CBofPalette *pPalette);

	/**
	 * Frees the data used by this bitmap
	 */
	VOID ReleaseBitmap();

	//
	// Palette routines
	//

	/**
	 * Assigns specified palette to this bitmap
	 * @param pBofPalette   Pointer to CBofPalette to be assigned
	 * @param bOwnPalette   TRUE if bitmap is to own this palette
	 */
	VOID SetPalette(CBofPalette *pPalette, BOOL bOwnPalette = FALSE);

	CBofPalette *GetPalette() {
		return m_pPalette;
	}

	BOOL IsOwnPalette() {
		return m_bOwnPalette;
	}

	/**
	 * Remaps the bitmap-bits to use the colors in the specified palette.
	 * @param pPalette      Pointer to CBofPalette to be mapped in
	 */
	VOID ReMapPalette(CBofPalette *pPalette);

	//
	// misc routines
	//

	/**
	 * Returns the bit address of the (x, y) location in this bmp
	 * @param x     Column in m_pBits
	 * @param y     Row in m_pBits
	 * @return      Address of (x,y) in bitmap surface
	 */
	UBYTE *GetPixelAddress(INT x, INT y);
	UBYTE *GetPixelAddress(CBofPoint *pPoint) {
		return (GetPixelAddress(pPoint->x, pPoint->y));
	}

	CBofSize GetSize() {
		return (CBofSize(m_nDX, m_nDY));
	}

	CBofRect GetRect() {
		return (CBofRect(0, 0, m_nDX - 1, m_nDY - 1));
	}

	VOID SetReadOnly(BOOL bReadOnly) {
		m_bReadOnly = bReadOnly;
	}

	BOOL GetReadOnly() {
		return (m_bReadOnly);
	}

	BOOL IsTopDown() {
		return (m_bTopDown);
	}

	INT Width() {
		return (m_nDX);
	}

	INT WidthBytes() {
		return (m_nScanDX);
	}

	INT Height() {
		return (m_nDY);
	}

	/**
	 * Returns current bitmap's filename (if any)
	 * @return      Pointer to bitmap's filename
	 */
	const CHAR *GetFileName();

	//
	// Drawing routines
	//
	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pWnd          Destination device for painting
	 * @param x             Destination column
	 * @param y             Destimation row
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    Transparency color
	 * @return              error return code
	 */
	ERROR_CODE Paint(CBofWindow *pWnd, INT x, INT y, CBofRect *pSrcRect = nullptr, INT nMaskColor = NOT_TRANSPARENT);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pWnd          Destination Device to paint to
	 * @param pDstRect      Destination rectangle (for stretching)
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    transparency color
	 * @return              Error return code
	 */
	ERROR_CODE Paint(CBofWindow *pWnd, CBofRect *pDstRect = nullptr, CBofRect *pSrcRect = nullptr, INT nMaskColor = NOT_TRANSPARENT);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pWnd          Destination Device to paint to
	 * @param pDstRect      Destination rectangle (for stretching)
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    Transparency color
	 * @return              Error return code
	 */
	ERROR_CODE PaintMaskBackdrop(CBofWindow *pWnd, CBofRect *pDstRect = nullptr, CBofRect *pSrcRect = nullptr, INT nMaskColor = NOT_TRANSPARENT);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pBmp          Destination bitmap to paint to
	 * @param x             Destination column
	 * @param y             Destimation row
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    Transparency color
	 * @return              Error return code
	 */
	ERROR_CODE Paint(CBofBitmap *pBmp, INT x, INT y, CBofRect *pSrcRect = nullptr, INT nMaskColor = NOT_TRANSPARENT);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pBmp          Destination bitmap to paint to
	 * @param pDstRect      Destination rectangle (for stretching)
	 * @param pSrcRect      Source rectangle from bitmap
	 * @param nMaskColor    Transparency color
	 * @return              Error return code.
	 */
	ERROR_CODE Paint(CBofBitmap *pBmp, CBofRect *pDstRect = nullptr, CBofRect *pSrcRect = nullptr, INT nMaskColor = NOT_TRANSPARENT);

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
	ERROR_CODE PaintStretch4(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect);

	/**
	 * Stretches a multiple of 4 pixel wide strips from source to destination
	 * @param pBmp          Destination bitmap to paint to
	 * @param pDstRect      Destination rectangle (for stretching)
	 * @param pSrcRect      Source rectangle from bitmap
	 * @return              Error return code
	 */
	ERROR_CODE PaintStretchOpt(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect, INT nOptSize);

	/**
	 * Paints some or all of the bitmap directly to the screen
	 * @param pBmp      Destination bitmap to paint to
	 * @return          Error return code
	 */
	ERROR_CODE Paint1To1(CBofBitmap *pBmp);

#if BOF_WINDOWS && !BOF_WINMAC

	HDC GetDC();
	VOID ReleaseDC(HDC hDC);

#elif BOF_MAC || BOF_WINMAC
	PixMapHandle GetMacPixMap();
#endif

	/** Copy specified section of screen (or window) to bitmamp.
	 * @param pWnd      Window to capture
	 * @param pSrcRect  Source rectangle in window
	 * @param pDstRect  Destination area to copy image to
	 * @return          Error return code
	 */
	ERROR_CODE CaptureScreen(CBofWindow *pWnd, CBofRect *pSrcRect, CBofRect *pDstRect = nullptr);

	/**
	 * Creates a new bitmap based on a section of current bitmap
	 * @param pRect     Section of bitmap to extract
	 * @return          New bitmap
	 */
	CBofBitmap *ExtractBitmap(CBofRect *pRect);

	/**
	 * Peforms a "Fade" onto the specified window
	 * @param pWnd          Pointer to window to fade into
	 * @param x             Fade upper left X
	 * @param y             Fade upper left Y
	 * @param nMaskColor    Transparency color (if any)
	 * @param nBlockSize    Size of Fade Blocks
	 * @param nSpeed        Speed for fade (not implimented yet)
	 * @return              Error return code
	 */
	ERROR_CODE FadeIn(CBofWindow *pWnd, INT x = 0, INT y = 0, INT nMaskColor = NOT_TRANSPARENT, INT nBlockSize = CBMP_FADE_SIZE, INT nSpeed = CBMP_FADE_SPEED);

	ERROR_CODE Curtain(CBofWindow *pWnd, INT nSpeed = CBMP_CURT_SPEED, INT nMaskColor = NOT_TRANSPARENT);
	ERROR_CODE FadeLines(CBofWindow *pWnd, CBofRect *pDstRect = nullptr, CBofRect *pSrcRect = nullptr, INT nSpeed = CBMP_LINE_SPEED, INT nMaskColor = NOT_TRANSPARENT);

	/**
	 * Returns the color at the (x, y) location in this bmp
	 * @param pPoint    Point in m_pBits
	 * @return          Color Index of specified point location in m_pBits
	 */
	UBYTE ReadPixel(CBofPoint *pPoint);

	/**
	 * Returns the color at the (x, y) location in this bmp
	 * @param x         X position
	 * @param y         Y position
	 * @return          Color Index of specified (x,y) location in m_pBits
	 */
	UBYTE ReadPixel(INT x, INT y);

	/**
	 * Assigns the specified color to the (x, y) location
	 * @param pPoint    Point in m_pBits to write to
	 */
	VOID WritePixel(CBofPoint *pPoint, UBYTE iColor);

	/**
	 * Assigns the specified color to the (x, y) location
	 * @param x         X position
	 * @param y         Y position
	 * @param iColor    Pixel value
	 */
	VOID WritePixel(INT x, INT y, UBYTE iColor);

	/**
	 * Writes a circle into this bitmap
	 * @param x         X center position
	 * @param y         Y center position
	 * @param nRadius   Radius of circle
	 * @param iColor    Pixel value
	 */
	VOID Circle(INT x, INT y, USHORT nRadius, UBYTE iColor);

	/**
	 * Writes a circle into this bitmap
	 * @param pCenter   Center position
	 * @param nRadius   Radius of circle
	 * @param iColor    Pixel value
	 */
	VOID Circle(CBofPoint *pCenter, USHORT nRadius, UBYTE iColor);

	/**
	 * Writes a line into this bitmap
	 * @param nSrcX     Endpoint 1 x
	 * @param nSrcY     Endpoint 1 y
	 * @param nDstX     Endpoint 2 x
	 * @param nDstY     Endpoint 2 y
	 * @param iColor    Pixel value
	 */
	VOID Line(INT nSrcX, INT nSrcY, INT nDstX, INT nDstY, UBYTE iColor);

	/**
	 * Writes a line into this bitmap
	 * @param pSrc      Endpoint 1
	 * @param pDest     Endpoint 2
	 * @param iColor    Pixel value
	 */
	VOID Line(CBofPoint *pSrc, CBofPoint *pDest, UBYTE iColor);

	/**
	 * Writes a filled circle into this bitmap
	 * @param x         X center position
	 * @param y         Y center position
	 * @param nRadius   Radius of circle
	 * @param iColor    Pixel value
	 */
	VOID FillCircle(INT x, INT y, USHORT nRadius, UBYTE iColor);

	/**
	 * Writes a filled circle into this bitmap
	 * @param pCenter   Center position
	 * @param nRadius   Radius of circle
	 * @param iColor    Pixel value
	 */
	VOID FillCircle(CBofPoint *pCenter, USHORT nRadius, UBYTE iColor);

	/**
	 * Writes a Rectangle into this bitmap
	 * @param cRect     Pointer to rectabgle Coordinates
	 * @param iColor    Color of rectangle
	 */
	VOID DrawRect(CBofRect *cRect, UBYTE iColor);

	/**
	 * Writes a filled in Rectangle to this bitmap
	 * @param cRect     Pointer to rectangle Coordinates
	 * @param iColor    Color of rectangle
	 */
	VOID FillRect(CBofRect *cRect, UBYTE iColor);

	/**
	 * Performs a flood-fill on this bitmap using specified color
	 * @param x             Column for start mof flood
	 * @param y             Row for start mof flood
	 * @param iFillColor    Color to fill with
	 */
	VOID FloodFill(INT x, INT y, UBYTE cFillColor);

	/**
	 * Flips specified rectangle in bitmap horizontally
	 * @param pRect         Rectangle to flip
	 * @return              Error return code
	 */
	ERROR_CODE FlipHorizontal(CBofRect *pRect = nullptr);

	/**
	 * Flips specified rectangle in bitmap vertically
	 * @param pRect         Rectangle to flip
	 * @return              Error return code
	 */
	ERROR_CODE FlipVertical(CBofRect *pRect = nullptr);
	VOID FlipVerticalFast();
	VOID FlipBits();

	/**
	 * Scrolls current bitmap horizontally
	 * @param nPixels       Number of pixels to scroll by
	 * @param pRect         Section of bitmap to scroll
	 * @return              Error return code
	 */
	ERROR_CODE ScrollRight(INT nPixels, CBofRect *pRect = nullptr);


	ERROR_CODE ScrollLeft(INT nPixels, CBofRect *pRect = nullptr) {
		return (ScrollRight(-nPixels, pRect));
	}

	/**
	 * Scrolls current bitmap vertially
	 * @param nPixels       Number of pixels to scroll by
	 * @param pRect         Section of bitmap to scroll
	 * @return              Error return code
	 */
	ERROR_CODE ScrollUp(INT nPixels, CBofRect *pRect = nullptr);


	ERROR_CODE ScrollDown(INT nPixels, CBofRect *pRect = nullptr) {
		return (ScrollUp(-nPixels, pRect));
	}

	// Debug, and perfomance testing routines
	//
	ERROR_CODE PaintPalette(CBofWindow *pWin, INT x, INT y);

#if BOF_DEBUG
	/**
	 * Tests the Frames Per Second for a 640x480x256 bitmap
	 * @param pWnd          Window to paint to
	 * @param pPalette      Palette for btimap
	 * @return              Frames per second
	 */
	static DOUBLE FPSTest(CBofWindow *pWnd, CBofPalette *pPalette);

	/**
	 * Tests the Frames Per Second for offscreen bit-blt
	 * @param pPalette      Palette for the offscreen bitmaps
	 * @return              Frames per Second.
	 */
	static DOUBLE OffScreenFPSTest(CBofPalette *pPalette);
#endif

	static VOID SetUseBackdrop(BOOL b) {
		m_bUseBackdrop = b;
	}
	static BOOL GetUseBackdrop() {
		return (m_bUseBackdrop);
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
 * @param pSharedPal        Shared palette flag
 * @return                  Pointer to bitmap
 */
extern CBofBitmap *LoadBitmap(const CHAR *pszFileName, CBofPalette *pPalette = nullptr, BOOL bSharedPal = FALSE);

/**
 * Paints specified bitmap to specfied window
 * @param pWindow           Window to paint to
 * @param pszFileName       Bitmap filename
 * @param pDstRect          Destination area to paint to
 * @param pSrcRect          Source area to paint from
 * @param pPalette          Optional palette to re-map with
 * @param nMaskColor        Optional transparent color
 * @return                  Error return code
 */
extern ERROR_CODE PaintBitmap(CBofWindow *pWindow, const CHAR *pszFileName, CBofRect *pDstRect = nullptr,
                              CBofRect *pSrcRect = nullptr, CBofPalette *pPalette = nullptr, INT nMaskColor = NOT_TRANSPARENT);

/**
 * Paints specified bitmap to specfied bitmap
 * @param pBmp              Bitmap to paint to
 * @param pszFileName       Bitmap filename
 * @param pDstRect          Destination area to paint to
 * @param pSrcRect          Source area to paint from
 * @param pPalette          Optional palette to re-map with
 * @param nMaskColor        Optional transparent color
 * @return                  Error return code
 */
extern ERROR_CODE PaintBitmap(CBofBitmap *pBmp, const CHAR *pszFileName, CBofRect *pDstRect = nullptr,
                              CBofRect *pSrcRect = nullptr, CBofPalette *pPalette = nullptr, INT nMaskColor = NOT_TRANSPARENT);

/**
 * Retrieves the size of the specified bitmap
 * @param pszFileName       Filename
 * @return                  Size of bitmap
 */
extern CBofSize GetBitmapSize(const CHAR *pszFileName);

/**
 * Loads specified palette
 * @param pszFileName       Bitmap to open to get palette from
 * @return                  Pointer to palette
 */
extern CBofPalette *LoadPalette(const CHAR *pszFileName);

} // namespace Bagel

#endif
