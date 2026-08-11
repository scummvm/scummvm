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

#ifndef HODJNPODJ_HNPLIBS_BITMAPS_H
#define HODJNPODJ_HNPLIBS_BITMAPS_H

#include "bagel/afxwin.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"

namespace Bagel {
namespace HodjNPodj {

/**
 * Return a rectangular area of the screen in a CBitmap object.
 * @param pDC       Pointer to device context to be used for display
 * @param pPalette  Pointer to color palette to be used for the bitmap
 * @param x,y       Upper left hand corner of bitmap to fetch
 * @param dx,dy     Size of rectangular area to retrive
 * @return      Pointer to bitmap object or nullptr
 */
extern CBitmap *FetchScreenBitmap(CDC *pDC, CPalette *pPalette, const int x, const int y, const int dx, const int dy);

/**
 * Loads a bitmap file
 * @param pDC           Pointer to device context to be used for display
 * @param pPalette      Address of pointer where to store palette from the DIB
 * @param pszPathName   Pointer to path string for disk based DIB file
 * @return      Pointer to bitmap object or nullptr
 */
extern CBitmap *FetchBitmap(CDC *pDC, CPalette **pPalette, const char *pszPathName);

/**
 * Loads a bitmap from a string named resource
 * @param pDC           Pointer to device context to be used for display
 * @param pPalette      Address of pointer where to store palette from the DIB
 * @param pszName       Resource string name
 * @return      Pointer to bitmap object or nullptr
 */
extern CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const char *pszName);

/**
 * Loads a bitmap from a numeric Id resource
 * @param pDC           Pointer to device context to be used for display
 * @param pPalette      Address of pointer where to store palette from the DIB
 * @param nResID        Resource Id
 * @return      Pointer to bitmap object or nullptr
 */
extern CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const int nResID);

/**
 * Creates a bitmap from a section of another bitmap
 * @param pDC           Pointer to device context to be used for display
 * @param pBase         Pointer to source bitmap for extraction
 * @param pPalette      Pointer to color palette to be used for the bitmap
 * @param x,y           Upper left hand corner of bitmap to fetch
 * @param dx,dy         Size of rectangular area to retrive
 * @return  Pointer to bitmap object or nullptr
 */
extern CBitmap *ExtractBitmap(CDC *pDC, CBitmap *pBitmap, CPalette *pPalette, const int x, const int y, const int dx, const int dy);

bool PaintBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);
bool PaintBitmap(CDC *pDC, CPalette *pPalette, const char *pszName, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);

bool PaintMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);

bool PaintMaskedDIB(CDC *pDC, CPalette *pPalette, const char *pszName, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);
bool PaintMaskedDIB(CDC *pDC, CPalette *pPalette, CDibDoc *pDIB, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);
bool PaintMaskedResource(CDC *pDC, CPalette *pPalette, const int resId, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);
bool PaintMaskedResource(CDC *pDC, CPalette *pPalette, const char *pszPathName, const int x = 0, const int y = 0, const int dx = 0, const int dy = 0);

bool PaintBlockEffect(CDC *pDC, CBitmap *pBitmap, CPalette *pPalette, int nBlockSize);
bool PaintBlockEffect(CDC *pDC, CDibDoc *pDIB, CPalette *pPalette, int nBlockSize);
bool PaintBlockEffect(CDC *pDC, COLORREF rgbColor, CPalette *pPalette, int nBlockSize, int nX, int nY, int nWidth, int nHeight);

bool BltBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, CRect *pDstRect, uint32 nMode);
bool BltMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, const int x, const int y);

CSize GetBitmapSize(CBitmap *pBitmap);
CPalette *GetPalette(CDC *pDC, const char *pszBmpName);

} // namespace HodjNPodj
} // namespace Bagel

#endif
