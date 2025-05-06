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

#include "bagel/afxwin.h"

namespace Bagel {
namespace HodjNPodj {

/* Handle to a DIB */
DECLARE_HANDLE(HDIB);

/* DIB constants */
#define PALVERSION   0x300

/* DIB Macros*/

#ifdef _DEBUG
#define IS_WIN30_DIB(lpbi)		((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#else
#define IS_WIN30_DIB(lpbi)		( TRUE )
#endif

#define RECTWIDTH(lpRect)		((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)		((lpRect)->bottom - (lpRect)->top)
#define	DeleteBitmap(hbm)		DeleteObject((HGDIOBJ)(HBITMAP)(hbm))
#define SelectBitmap(hdc, hbm)  ((HBITMAP)SelectObject((hdc),(HGDIOBJ)(HBITMAP)(hbm)))

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

/* Function prototypes */
BOOL      WINAPI  PaintDIB(HDC, LPRECT, HDIB, LPRECT, CPalette *pPal);
BOOL      WINAPI  CreateDIBPalette(HDIB hDIB, CPalette *cPal);
LPSTR     WINAPI  FindDIBBits(LPSTR lpbi);
DWORD     WINAPI  DIBWidth(LPSTR lpDIB);
DWORD     WINAPI  DIBHeight(LPSTR lpDIB);
WORD      WINAPI  PaletteSize(LPSTR lpbi);
WORD      WINAPI  DIBNumColors(LPSTR lpbi);
HANDLE    WINAPI  CopyHandle(HANDLE h);
HBITMAP   WINAPI  DIBtoBitmap(HDC hDC, HPALETTE hPal, LPBITMAPINFO lpbih);

CBitmap *WINAPI  ConvertDIB(CDC *pDC, HDIB hDIB, CPalette *pPal);

BOOL      WINAPI  SaveDIB(HDIB hDib, CFile &file);
HDIB      WINAPI  ReadDIBFile(CFile &file);
HDIB 	  WINAPI  ReadDIBResource(const char *pszPathName);

HANDLE    WINAPI  BitmapToDIB(HBITMAP hBitmap, HPALETTE hPal);
void      WINAPI  InitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr,
	DWORD dwWidth,
	DWORD dwHeight,
	int nBPP);

void	  WINAPI  ShowMemoryInfo(const char *chMessage, const char *chTitle);

CPalette *DuplicatePalette(CPalette *pPal);

} // namespace HodjNPodj
} // namespace Bagel

#endif
