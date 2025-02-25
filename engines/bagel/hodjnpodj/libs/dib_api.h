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

#ifndef HODJNPODJ_LIBS_DIB_API_H
#define HODJNPODJ_LIBS_DIB_API_H

#include "bagel/bagel.h"
#include "bagel/mfc/afx.h"

namespace Bagel {
namespace HodjNPodj {

/* DIB constants */
#define PALVERSION   0x300

/* DIB Macros*/

#define IS_WIN30_DIB(lpbi)		( TRUE )

#define RECTWIDTH(lpRect)		((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)		((lpRect)->bottom - (lpRect)->top)
#define	DeleteBitmap(hbm)		DeleteObject((HGDIOBJ)(HBITMAP)(hbm))
#define SelectBitmap(hdc, hbm)  ((HBITMAP)SelectObject((hdc),(HGDIOBJ)(HBITMAP)(hbm)))

// WIDTHBYTES performs uint32-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of uint32-aligned bytes needed
// to hold those bits.

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

/* Function prototypes */
bool PaintDIB(HDC, LPRECT, HDIB, LPRECT, CPalette *pPal);
bool CreateDIBPalette(HDIB hDIB, CPalette *cPal);
const char *FindDIBBits(const char *lpbi);
uint32 DIBWidth(const char *lpDIB);
uint32 DIBHeight(const char *lpDIB);
uint16 PaletteSize(const char *lpbi);
uint16 DIBNumColors(const char *lpbi);
HANDLE CopyHandle(HANDLE h);
HBITMAP DIBtoBitmap(HDC hDC, HPALETTE hPal, LPBITMAPINFO lpbih);

CBitmap *ConvertDIB(CDC *pDC, HDIB hDIB, CPalette *pPal);

bool SaveDIB(HDIB hDib, CFile &file);
HDIB ReadDIBFile(CFile &file);
HDIB ReadDIBResource(const char *pszPathName);

HANDLE itmapToDIB(HBITMAP hBitmap, HPALETTE hPal);
void InitBitmapInfoHeader(LPBITMAPINFOHEADER lpBmInfoHdr,
	uint32 dwWidth, uint32 dwHeight, int nBPP);

void ShowMemoryInfo(char *chMessage, char *chTitle);

CPalette *DuplicatePalette(CPalette *pPal);

} // namespace HodjNPodj
} // namespace Bagel

#endif
