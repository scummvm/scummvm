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

#include "bagel/hodjnpodj/boflib/bitmaps.h"

namespace Bagel {
namespace HodjNPodj {

CBitmap *FetchScreenBitmap(CDC *pDC, CPalette *pPalette, const int x, const int y, const int dx, const int dy) {
	error("TODO: FetchScreenBitmap");
	return nullptr;
}

CBitmap *FetchBitmap(CDC *pDC, CPalette **pPalette, const char *pszPathName) {
	error("TODO: FetchBitmap");
	return nullptr;
}

CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const char *pszName) {
	error("TODO: FetchResourceBitmap");
	return nullptr;
}

CBitmap *FetchResourceBitmap(CDC *pDC, CPalette **pPalette, const int nResID) {
	error("TODO: FetchResourceBitmap");
	return nullptr;
}

CBitmap *ExtractBitmap(CDC *pDC, CBitmap *pBitmap, CPalette *pPalette, const int x, const int y, const int dx, const int dy) {
	error("TODO: ExtractBitmap");
	return nullptr;
}

BOOL PaintBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x, const int y, const int dx, const int dy) {
	error("TODO: PaintBitmap");
	return false;
}

BOOL PaintBitmap(CDC *pDC, CPalette *pPalette, const char *pszName, const int x, const int y, const int dx, const int dy) {
	error("TODO: PaintBitmap");
	return false;
}

BOOL PaintMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, const int x, const int y, const int dx, const int dy) {
	error("TODO: PaintMaskedBitmap");
	return false;
}

BOOL PaintMaskedDIB(CDC *pDC, CPalette *pPalette, const char *pszName, const int x, const int y, const int dx, const int dy) {
	error("TODO: PaintMaskedDIB");
	return false;
}

BOOL PaintMaskedDIB(CDC *pDC, CPalette *pPalette, CDibDoc *pDIB, const int x, const int y, const int dx, const int dy) {
	error("TODO: PaintMaskedDIB");
	return false;
}

BOOL PaintMaskedResource(CDC *pDC, CPalette *pPalette, const int resId, const int x, const int y, const int dx, const int dy) {
	error("TODO: PaintMaskedResource");
	return false;
}

BOOL PaintMaskedResource(CDC *pDC, CPalette *pPalette, const char *pszPathName, const int x, const int y, const int dx, const int dy) {
	error("TODO: PaintMaskedResource");
	return false;
}

BOOL PaintBlockEffect(CDC *pDC, CBitmap *pBitmap, CPalette *pPalette, int nBlockSize) {
	error("TODO: PaintBlockEffect");
	return false;
}

BOOL PaintBlockEffect(CDC *pDC, CDibDoc *pDIB, CPalette *pPalette, int nBlockSize) {
	error("TODO: PaintBlockEffect");
	return false;
}

BOOL PaintBlockEffect(CDC *pDC, COLORREF rgbColor, CPalette *pPalette, int nBlockSize, int nX, int nY, int nWidth, int nHeight) {
	error("TODO: PaintBlockEffect");
	return false;
}

BOOL BltBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, CRect *pDstRect, DWORD nMode) {
	error("TODO: BltBitmap");
	return false;
}

BOOL BltMaskedBitmap(CDC *pDC, CPalette *pPalette, CBitmap *pBitmap, CRect *pSrcRect, const int x, const int y) {
	error("TODO: BltMaskedBitmap");
	return false;
}

CSize GetBitmapSize(CBitmap *pBitmap) {
	error("TODO: GetBitmapSize");
	return CSize();
}

} // namespace HodjNPodj
} // namespace Bagel
