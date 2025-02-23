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

#ifndef BAGEL_HODJNPODJ_BOFLIB_BITMAPS_H
#define BAGEL_HODJNPODJ_BOFLIB_BITMAPS_H

#include "bagel/mfc/afx.h"

namespace Bagel {
namespace HodjNPodj {

CBitmap *FetchScreenBitmap(CDC *pDC, CPalette *pPalette, const int x, const int y, const int dx, const int dy);

CBitmap *FetchBitmap(CDC *pDC, CPalette **pPalette, const char *pszPathName);
CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const char *pszName);
CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const int nResID);

CBitmap *ExtractBitmap(CDC *pDC, CBitmap *pBitmap, CPalette *pPalette, const int x, const int y, const int dx, const int dy);

BOOL PaintBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);
BOOL PaintBitmap(CDC *pDC, CPalette *pPalette, const char *pszName, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);

BOOL PaintMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);

BOOL PaintMaskedDIB(CDC *pDC, CPalette *pPalette, const char *pszName, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);
BOOL PaintMaskedDIB(CDC *pDC, CPalette *pPalette, CDibDoc *pDIB, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);
BOOL PaintMaskedResource(CDC *pDC, CPalette *pPalette, const int resId, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);
BOOL PaintMaskedResource(CDC *pDC, CPalette *pPalette, const char *pszPathName, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);

BOOL PaintBlockEffect(CDC *pDC, CBitmap *pBitmap, CPalette *pPalette, int nBlockSize);
BOOL PaintBlockEffect(CDC *pDC, CDibDoc *pDIB, CPalette *pPalette, int nBlockSize);
BOOL PaintBlockEffect(CDC *pDC, COLORREF rgbColor, CPalette *pPalette, int nBlockSize, int nX, int nY, int nWidth, int nHeight);

BOOL BltBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, CRect *pDstRect, DWORD nMode);
BOOL BltMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, const int x, const int y);

CSize GetBitmapSize(CBitmap *pBitmap);

} // namespace HodjNPodj
} // namespace Bagel

#endif
