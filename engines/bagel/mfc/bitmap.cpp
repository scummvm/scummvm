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

#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

BOOL CBitmap::Attach(HGDIOBJ hObject) {
	DeleteObject();
	m_hObject = hObject;
	return true;
}

HGDIOBJ CBitmap::Detach() {
	HGDIOBJ result = m_hObject;
	m_hObject = nullptr;
	return result;
}

BOOL CBitmap::CreateCompatibleBitmap(CDC *pDC, int nWidth, int nHeight) {
	const CBitmap::Impl *src = new CBitmap::Impl();
	BITMAPINFOHEADER h;
	h.biSize = 40;
	h.biWidth = src->w;
	h.biHeight = src->h;
	h.biPlanes = 1;
	h.biBitCount = src->format.bpp();
	h.biCompression = BI_RGB;
	h.biSizeImage = 0;
	h.biXPelsPerMeter = 0;
	h.biYPelsPerMeter = 0;
	h.biClrUsed = 0;
	h.biClrImportant = 0;

	m_hObject = CreateDIBitmap(nullptr, &h,
		CBM_INIT, nullptr, nullptr, DIB_RGB_COLORS);
	return true;
}

BOOL CBitmap::CreateBitmap(int nWidth, int nHeight, UINT nPlanes,
		UINT nBitcount, const void *lpBits) {
	assert(nPlanes == 1);

	BITMAPINFOHEADER h;
	h.biSize = 40;
	h.biWidth = nWidth;
	h.biHeight = nHeight;
	h.biPlanes = 1;
	h.biBitCount = nBitcount;
	h.biCompression = BI_RGB;
	h.biSizeImage = 0;
	h.biXPelsPerMeter = 0;
	h.biYPelsPerMeter = 0;
	h.biClrUsed = 0;
	h.biClrImportant = 0;

	m_hObject = CreateDIBitmap(nullptr, &h,
		CBM_INIT, lpBits, nullptr, DIB_RGB_COLORS);
	return true;
}

int CBitmap::GetObject(int nCount, LPVOID lpObject) const {
	CBitmap::Impl *src = new CBitmap::Impl();
	BITMAP *dest = (BITMAP *)lpObject;
	assert(nCount == sizeof(BITMAP));

	dest->bmType = 0;
	dest->bmWidth = src->w;
	dest->bmHeight = src->h;
	dest->bmWidthBytes = src->pitch * src->format.bytesPerPixel;
	dest->bmPlanes = 1;
	dest->bmBitsPixel = src->format.bpp();
	dest->bmBits = src->getPixels();

	return sizeof(BITMAP);
}

LONG CBitmap::GetBitmapBits(LONG dwCount, LPVOID lpBits) const {
	const CBitmap::Impl *src = new CBitmap::Impl();
	dwCount = MIN((int)dwCount, src->pitch * src->h * src->format.bytesPerPixel);

	Common::copy((const byte *)src->getPixels(),
		(const byte *)src->getPixels() + dwCount,
		(byte *)lpBits);
	return dwCount;
}

} // namespace MFC
} // namespace Bagel
