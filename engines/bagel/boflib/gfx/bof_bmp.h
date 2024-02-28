
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

#ifndef BAGEL_BOFLIB_GFX_BOF_BMP_H
#define BAGEL_BOFLIB_GFX_BOF_BMP_H

#include "bagel/boflib/boffo.h"

#include "bagel/boflib/cache.h"
#include "bagel/boflib/bof_error.h"
#include "bagel/boflib/bof_object.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/rect.h"
#include "bagel/boflib/size.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/gfx/bof_palette.h"
#include "bagel/boflib/gfx/bof_window.h"

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
public:
	CBofBitmap(VOID);
	CBofBitmap(INT dx, INT dy, CBofPalette *pPalette, BOOL bOwnPalette = FALSE, UBYTE HUGE *pPrivateBuff = nullptr);
	CBofBitmap(const CHAR *pszFileName, CBofPalette *pPalette = NULL, BOOL bOwnPalette = FALSE);

	virtual ~CBofBitmap();

	// for construction
	//
	ERROR_CODE BuildBitmap(CBofPalette *pPalette);
	ERROR_CODE LoadBitmap(const CHAR *pszFileName, CBofPalette *pPalette);
	VOID ReleaseBitmap(VOID);

	//
	// Palette routines
	//

	VOID SetPalette(CBofPalette *pPalette, BOOL bOwnPalette = FALSE);

	CBofPalette *GetPalette(VOID) { return (m_pPalette); }

	BOOL IsOwnPalette(VOID) { return (m_bOwnPalette); }

	VOID ReMapPalette(CBofPalette *pPalette);

	//
	// misc routines
	//

	UBYTE HUGE *GetPixelAddress(INT x, INT y);
	UBYTE HUGE *GetPixelAddress(CBofPoint *pPoint) { return (GetPixelAddress(pPoint->x, pPoint->y)); }

	CBofSize GetSize(VOID) { return (CBofSize(m_nDX, m_nDY)); }

	CBofRect GetRect(VOID) { return (CBofRect(0, 0, m_nDX - 1, m_nDY - 1)); }

	VOID SetReadOnly(BOOL bReadOnly) { m_bReadOnly = bReadOnly; }

	BOOL GetReadOnly(VOID) { return (m_bReadOnly); }

	BOOL IsTopDown(VOID) { return (m_bTopDown); }

	INT Width() { return (m_nDX); }

	INT WidthBytes() { return (m_nScanDX); }

	INT Height() { return (m_nDY); }

	const CHAR *GetFileName(VOID);

	//
	// Drawing routines
	//
	ERROR_CODE Paint(CBofWindow *pWnd, INT x, INT y, CBofRect *pSrcRect = NULL, INT nMaskColor = NOT_TRANSPARENT);
	ERROR_CODE Paint(CBofWindow *pWnd, CBofRect *pDstRect = NULL, CBofRect *pSrcRect = NULL, INT nMaskColor = NOT_TRANSPARENT);

	ERROR_CODE PaintMaskBackdrop(CBofWindow *pWnd, CBofRect *pDstRect = NULL, CBofRect *pSrcRect = NULL, INT nMaskColor = NOT_TRANSPARENT);

	ERROR_CODE Paint(CBofBitmap *pBmp, INT x, INT y, CBofRect *pSrcRect = NULL, INT nMaskColor = NOT_TRANSPARENT);
	ERROR_CODE Paint(CBofBitmap *pBmp, CBofRect *pDstRect = NULL, CBofRect *pSrcRect = NULL, INT nMaskColor = NOT_TRANSPARENT);

	//
	// Special Paint routines Optimized for specific tasks
	//

	// Stretches 4 pixel wide strips
	ERROR_CODE PaintStretch4(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect);
	ERROR_CODE PaintStretchOpt(CBofBitmap *pBmp, CBofRect *pDstRect, CBofRect *pSrcRect, INT nOptSize);

	// Paints one bitmap onto another (both are the same size, and depth)
	ERROR_CODE Paint1To1(CBofBitmap *pBmp);

#if BOF_WINDOWS && !BOF_WINMAC

	HDC GetDC(VOID);
	VOID ReleaseDC(HDC hDC);

#elif BOF_MAC || BOF_WINMAC
	PixMapHandle GetMacPixMap(VOID);
#endif

	ERROR_CODE CaptureScreen(CBofWindow *pWnd, CBofRect *pSrcRect, CBofRect *pDstRect = NULL);

	CBofBitmap *ExtractBitmap(CBofRect *pRect);

	ERROR_CODE FadeIn(CBofWindow *pWnd, INT x = 0, INT y = 0, INT nMaskColor = NOT_TRANSPARENT, INT nBlockSize = CBMP_FADE_SIZE, INT nSpeed = CBMP_FADE_SPEED);
	ERROR_CODE Curtain(CBofWindow *pWnd, INT nSpeed = CBMP_CURT_SPEED, INT nMaskColor = NOT_TRANSPARENT);
	ERROR_CODE FadeLines(CBofWindow *pWnd, CBofRect *pDstRect = NULL, CBofRect *pSrcRect = NULL, INT nSpeed = CBMP_LINE_SPEED, INT nMaskColor = NOT_TRANSPARENT);

	UBYTE ReadPixel(CBofPoint *pPoint);
	UBYTE ReadPixel(INT x, INT y);
	VOID WritePixel(CBofPoint *pPoint, UBYTE iColor);
	VOID WritePixel(INT x, INT y, UBYTE iColor);

	VOID Circle(INT x, INT y, USHORT nRadius, UBYTE iColor);
	VOID Circle(CBofPoint *pCenter, USHORT nRadius, UBYTE iColor);

	VOID Line(INT nSrcX, INT nSrcY, INT nDstX, INT nDstY, UBYTE iColor);
	VOID Line(CBofPoint *pSrc, CBofPoint *pDest, UBYTE iColor);

	VOID FillCircle(INT x, INT y, USHORT nRadius, UBYTE iColor);
	VOID FillCircle(CBofPoint *pCenter, USHORT nRadius, UBYTE iColor);

	VOID DrawRect(CBofRect *cRect, UBYTE iColor);
	VOID FillRect(CBofRect *cRect, UBYTE iColor);

	VOID FloodFill(INT x, INT y, UBYTE cFillColor);

	ERROR_CODE FlipHorizontal(CBofRect *pRect = NULL);
	ERROR_CODE FlipVertical(CBofRect *pRect = NULL);
	VOID FlipVerticalFast(VOID);
	VOID FlipBits(VOID);

	ERROR_CODE ScrollRight(INT nPixels, CBofRect *pRect = NULL);
	ERROR_CODE ScrollLeft(INT nPixels, CBofRect *pRect = NULL) { return (ScrollRight(-nPixels, pRect)); }

	ERROR_CODE ScrollUp(INT nPixels, CBofRect *pRect = NULL);
	ERROR_CODE ScrollDown(INT nPixels, CBofRect *pRect = NULL) { return (ScrollUp(-nPixels, pRect)); }

	// Debug, and perfomance testing routines
	//
	ERROR_CODE PaintPalette(CBofWindow *pWin, INT x, INT y);

#if BOF_DEBUG
	static DOUBLE FPSTest(CBofWindow *pWnd, CBofPalette *pPalette);
	static DOUBLE OffScreenFPSTest(CBofPalette *pPalette);
#endif

	static VOID SetUseBackdrop(BOOL b) { m_bUseBackdrop = b; }
	static BOOL GetUseBackdrop(VOID) { return (m_bUseBackdrop); }

protected:
	VIRTUAL BOOL Alloc(VOID);
	VIRTUAL VOID Free(VOID);

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

	STATIC PixMapHandle m_stPixMap; // jwl 07.10.96 bit map to pixel map
#if SYNCPALETTES
	GrafPtr m_oldGrafPort;          // jwl 07.01.96 original grafport
	GrafPtr m_newGrafPort;          // jwl 07.01.96 original grafport
#endif
#endif

	UBYTE HUGE *m_pBits;

	CBofPalette *m_pPalette;

	INT m_nScanDX;
	INT m_nDX;
	INT m_nDY;
	BOOL m_bTopDown;

	BOOL m_bOwnPalette;
	BOOL m_bReadOnly;
	BOOL m_bInitialized;

	BOOL m_bPrivateBmp;
};

//////////////////////////////////////////////////////////////////////////////
//
// Misc graphics routines
//
//////////////////////////////////////////////////////////////////////////////
CBofBitmap *LoadBitmap(const CHAR *pszFileName, CBofPalette *pPalette = NULL, BOOL bSharedPal = FALSE);
ERROR_CODE PaintBitmap(CBofWindow *pWindow, const CHAR *pszFileName, CBofRect *pDstRect = NULL, CBofRect *pSrcRect = NULL, CBofPalette *pPalette = NULL, INT nMaskColor = NOT_TRANSPARENT);
ERROR_CODE PaintBitmap(CBofBitmap *pWindow, const CHAR *pszFileName, CBofRect *pDstRect = NULL, CBofRect *pSrcRect = NULL, CBofPalette *pPalette = NULL, INT nMaskColor = NOT_TRANSPARENT);
CBofSize GetBitmapSize(const CHAR *pszFileName);

CBofPalette *LoadPalette(const CHAR *pszFileName);

} // namespace Bagel

#endif
