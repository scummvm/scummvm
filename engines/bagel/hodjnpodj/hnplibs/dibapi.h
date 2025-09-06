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

#ifndef HODJNPODJ_HNPLIBS_DIBAPI_H
#define HODJNPODJ_HNPLIBS_DIBAPI_H

#include "graphics/managed_surface.h"
#include "bagel/afxwin.h"

namespace Bagel {
namespace HodjNPodj {

/* Handle to a DIB */
typedef Graphics::ManagedSurface *HDIB;

/* DIB constants */
#define PALVERSION   0x300

/* DIB Macros*/
#define IS_WIN30_DIB(lpbi)      ( true )

#define RECTWIDTH(lpRect)       ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)      ((lpRect)->bottom - (lpRect)->top)
#define DeleteBitmap(hbm)       DeleteObject((HGDIOBJ)(HBITMAP)(hbm))
#define SelectBitmap(hdc, hbm)  ((HBITMAP)SelectObject((hdc),(HGDIOBJ)(HBITMAP)(hbm)))

// WIDTHBYTES performs uint32-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of uint32-aligned bytes needed
// to hold those bits.

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

/* Function prototypes */
bool      PaintDIB(HDC, LPRECT, HDIB, LPRECT, CPalette *pPal);


/**
 * This function creates a palette from a DIB by allocating memory for the
 * logical palette, reading and storing the colors from the DIB's color table
 * into the logical palette, creating a palette from this logical palette,
 * and then returning the palette's handle. This allows the DIB to be
 * displayed using the best possible colors (important for DIBs with 256 or
 * more colors).
 * @param hDIB		Specifies the DIB
 * @return		Specifies the palette
 */
extern bool CreateDIBPalette(HDIB hDIB, CPalette *cPal);

/**
 * This function calculates the address of the DIB's bits and returns a
 * pointer to the DIB bits.
 * @param hDIB		Pointer to packed-DIB memory block
 * @return		Pointer to the DIB bits
 */
extern char *FindDIBBits(HDIB lpbi);

/**
 * This function gets the width of the bitmap.
 * @param lpbi		Bitmap pointer
 * @return			Width
 */
extern uint32 DIBWidth(HDIB lpDIB);

/**
 * This function gets the height of the bitmap.
 * @param lpbi		Bitmap pointer
 * @return			Height
 */
extern uint32 DIBHeight(HDIB lpDIB);

/**
 * Gets the size required to store the DIB's palette
 * @param lpbi		Pointer to packed-DIB memory block
 * @return		Size of the color palette of the DIB
 */
extern uint16 PaletteSize(HDIB lpDIB);

/**
 * Gets the number of colors in the palette
 * @param lpbi		Pointer to packed-DIB memory block
 * @return		Number of the palette colors.
 */
extern uint16 DIBNumColors(HDIB lpDIB);

HANDLE    CopyHandle(HANDLE h);

/**
 * Convert a device-independent bitmap (DIB) to a device-dependent
 * bitmap (DDB).  The DIB must be packed; i.e. same as a .BMP file.
 * @return		A handle to the DDB (HBITMAP).  If an error occurs, the return
 * value will be nullptr.
 */
extern HBITMAP DIBtoBitmap(HDC hDC, HPALETTE hPal, HDIB hDib);

/**
 * Convert a device-independent bitmap (DIB) to a device-dependent
 * bitmap (DDB), with the desired color palette mapped to the context.
 * The DIB must be packed; i.e. same as a .BMP file.
 * @return		A handle to the Bitmap (CBitmap *).  If an error occurs, the return
 * value will be nullptr.
 */
extern CBitmap *ConvertDIB(CDC *pDC, HDIB hDIB, CPalette *pPal);

/**
 * Reads in the specified DIB file into a global chunk of memory.
 * @param file		Bitmap file to read
 * @return			A handle to a dib (hDIB) if successful.
 * nullptr if an error occurs.
 */
extern HDIB ReadDIBFile(CFile &file);

/**
 * Reads in the specified DIB from a resource entry.
 * @param pszPathName	Resource name
 * @return		A handle to a dib (hDIB) if successful.
 * nullptr if an error occurs.
 */
extern HDIB ReadDIBResource(const char *pszPathName);

void      InitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr,
                                       uint32 dwWidth,
                                       uint32 dwHeight,
                                       int nBPP);

void      ShowMemoryInfo(const char *chMessage, const char *chTitle);

CPalette *DuplicatePalette(CPalette *pPal);

} // namespace HodjNPodj
} // namespace Bagel

#endif
